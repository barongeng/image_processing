/*
 * Copyright (C) 2016  Raphaël Poggi <poggi.raph@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "hough.h"

#define HOUGH_RES_SHT	180
#define HOUGH_RES_CHT	360
#define RADIUS_BEGIN	10
#define RADIUS_END	54
#define RADIUS_SIZE	(RADIUS_END + 1)

#define sign(x) ((x<0)?-1:((x>0)?1:0))

int **hough;
int ***hough_circle;

//static void draw_line(unsigned char *img, int width, int height, struct point start, struct point end)
//{
//	int i;
//	int j;
//	int sum_i;
//	int sum_j;
//	int delta_x;
//	int delta_y;
//	unsigned char *px = NULL;
//	unsigned char *start_px = NULL;
//	unsigned char *end_px = NULL;
//
//	j = start.x;
//	i = start.y;
//
//	delta_x = end.x - start.x;
//	delta_y = end.y - start.y;
//
//	printf("%d -> %d %d -> %d\n", j, delta_x, i, delta_y);
//
//	start_px = img + start.y * width + start.x;
//	end_px = img + end.y * width + end.x;
//
//	px = start_px;
//
//	while(i < delta_y && j < delta_x) {
//		px = img + i * width + j;
//
//		*px = 255;
//
//		if (delta_y)
//			i++;
//
//		if (delta_x)
//			j++;
//	}
//
//
//
//}


static void draw_line(unsigned char *img, int width, int height, struct point start, struct point end)
{

	int dx,dy,sdx,sdy,px,py,dxabs,dyabs,i;
	float slope;
	unsigned char *p = NULL;
	int x1 = start.x;
	int y1 = start.y;
	int x2 = end.x;
	int y2 = end.y;
	dx = x2 - x1;      /* the horizontal distance of the line */
	dy = y2 - y1;      /* the vertical distance of the line */
	dxabs = abs(dx);
	dyabs = abs(dy);
	sdx = sign(dx);
	sdy = sign(dy);
	if (dxabs >= dyabs) {
		slope = (float)dy / (float)dx;
		for (i = 0; i != dx; i += sdx) {
			px = i + x1;
			py = slope * i + y1;
			p = img + py * width + px;
			*p = 255;
		}
	}
	else {
		slope = (float)dx / (float)dy;
		for (i = 0; i != dy; i += sdy) {
			px = slope * i + x1;
			py = i + y1;
			p = img + py * width + px;
			*p = 255;
		}
	}

}

static struct hough_param *find_best_line(struct point *points, int size, int width, int height)
{
	int i;
	int j;
	int angle;
	int theta_best = 0;
	int rho;
	int nrho;
	int rho_best = 0;
	int x;
	int y;
	struct hough_param *hp;

	nrho = sqrt(width * width + height * height);

	hough = (int **)malloc(nrho * sizeof(int *));
	memset(hough, 0, nrho * sizeof(int *));

	for (i = 0; i < nrho; i++) {
		hough[i] = (int *)malloc(HOUGH_RES_SHT * sizeof(int));
		memset(hough[i], 0, HOUGH_RES_SHT * sizeof(int));
	}

	/* Test each angle for steps along path */
	for(j = 0; j < size; j++) {

		x = points[j].x;
		y = points[j].y;

		if (!x && !y)
			continue;

		for(i = 0; i < HOUGH_RES_SHT; i++) {

			angle = i * M_PI / 180;
			rho = (sin(angle) * y) + (cos(angle) * x);
			if(rho > 0 && rho < nrho) {

				hough[rho][i]++;

				if(hough[rho][i] > hough[rho_best][theta_best]) {
					theta_best = i;
					rho_best = rho;
				}
			}
		}
	}

	hp = (struct hough_param *)malloc(sizeof(struct hough_param));
	hp->theta = theta_best;
	hp->rho = rho_best;
	hp->nrho = nrho;
	hp->resolution = HOUGH_RES_SHT;
	hp->mag = hough[hp->rho][hp->theta];
	hp->thresh = (hough[hp->rho][hp->theta] * 70) / 100;

	printf("accumulator max: %d\n", hough[rho_best][theta_best]);
	printf("hp->theta: %d\n", hp->theta);
	printf("hp->rho: %d\n", hp->rho);
	printf("hp->nrho: %d\n", hp->nrho);
	printf("hp->resolution: %d\n", hp->resolution);
	printf("hp->thresh: %d\n", hp->thresh);

	return hp;
}

struct hough_param *find_line(unsigned char *img, int width, int height)
{
	int i;
	int j;
	int idx = 0;
	int points_size;
	struct point *points = NULL;
	struct hough_param *hp = NULL;

	points_size = width * height * sizeof(struct point);
	points = (struct point *)malloc(points_size);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			if (*(img + i * width + j) == 255) {
				points[idx].x = j;
				points[idx].y = i;
				idx++;
			}
		}
	}

	hp = find_best_line(points, width * height, width, height);

	hp->points = points;
	hp->points_size = idx;

	return hp;
}

static struct hough_param_circle *find_best_circle(struct point *points, int size, int width, int height)
{
	int i;
	int j;
	int a;
	int b;
	int r;
	int a_best = 0;
	int b_best = 0;
	int r_best = 0;
	int angle;
	int x;
	int y;
	struct hough_param_circle *hp;

	hough_circle = (int ***)malloc(width * sizeof(int **));
	memset(hough_circle, 0, width * sizeof(int **));

	for (i = 0; i < width; i++) {
		hough_circle[i] = (int *)malloc(height * sizeof(int *));
		memset(hough_circle[i], 0, height * sizeof(int *));
		for (j = 0; j < height; j++) {
			hough_circle[i][j] = (int *)malloc(RADIUS_SIZE * sizeof(int));
			memset(hough_circle[i][j], 0, RADIUS_SIZE * sizeof(int));
		}
	}

	for(j = 0; j < size; j++) {

		x = points[j].x;
		y = points[j].y;

		if (!x && !y)
			continue;

		for(i = 0; i < HOUGH_RES_CHT; i++) {

			angle = i * M_PI / 180;

			for (r = RADIUS_BEGIN; r <= RADIUS_END; r++) {
				a = x - (r * cos(angle));
				b = y - (r * sin(angle));

				if(a > 0 && a < width && b > 0 && b < height) {

					hough_circle[a][b][r]++;

					if (hough_circle[a][b][r] > hough_circle[a_best][b_best][r_best]) {
						a_best = a;
						b_best = b;
						r_best = r;
					}
				}
			}
		}
	}

	hp = (struct hough_param_circle *)malloc(sizeof(struct hough_param_circle));
	hp->a = a_best;
	hp->b = b_best;
	hp->radius = r_best;
	hp->resolution = HOUGH_RES_CHT;
	hp->thresh = (hough_circle[hp->a][hp->b][hp->radius] * 70) / 100;

	printf("accumulator max: %d\n", hough_circle[a_best][b_best][r_best]);
	printf("hp->a: %d\n", hp->a);
	printf("hp->b: %d\n", hp->b);
	printf("hp->radius: %d\n", hp->radius);
	printf("hp->resolution: %d\n", hp->resolution);
	printf("hp->thresh: %d\n", hp->thresh);

	return hp;
}

struct hough_param_circle *find_circle(unsigned char *img, int width, int height)
{
	int i;
	int j;
	int idx = 0;
	int points_size;
	struct point *points = NULL;
	struct hough_param_circle *hp = NULL;

	points_size = width * height * sizeof(struct point);
	points = (struct point *)malloc(points_size);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			if (*(img + i * width + j) == 255) {
				points[idx].x = j;
				points[idx].y = i;
				idx++;
			}
		}
	}

	hp = find_best_circle(points, width * height, width, height);

	hp->points = points;
	hp->points_size = idx;

	return hp;
}

void draw_overlay(struct hough_param *hp, unsigned char *img, int width, int height)
{
	int i;
	int j;
	int theta;
	int rho;
	int x;
	int y;
	int center_x = width / 2;
	int center_y = height / 2;
	int center_nrho = hp->nrho / 2;
	double tsin;
	double tcos;
	struct point p0;
	struct point p1;
	struct point p2;
	int angle;
	int max_length = sqrt(width * width + height * height);
	unsigned char *px = NULL;

	for (i = 0; i < hp->nrho; i++) {
		for (j = 0; j < HOUGH_RES_SHT; j++) {
			if (hough[i][j] >= hp->thresh) {
				angle = j * M_PI / 180;

				tcos = cos(angle);
				tsin = sin(angle);

				x = tcos * i;
				y = tsin * i;

				p0.x = x + max_length * tcos;
				p0.y = y + max_length * tsin;
				p1.x = x - max_length * tcos;
				p1.y = y - max_length * tsin;

				if (p0.x < 0)
					p0.x = 0;
				else if (p0.x > width)
					p0.x = width;

				if (p1.x < 0)
					p1.x = 0;
				else if (p1.x > width)
					p1.x = width;

				if (p0.y < 0)
					p0.y = 0;
				else if (p0.y > height)
					p0.y = height;

				if (p1.y < 0)
					p1.y = 0;
				else if (p1.y > height)
					p1.y = height;

//				p0.x = 1;
//				p0.y = (-tcos/tsin) * p0.x  + (i / tsin);
//				p1.x = width - 1;
//				p1.y = (-tcos/tsin) * p1.x  + (i / tsin);

//				p0.x = (x + 1000 * -tsin);
//				p0.y = (y + 1000 * tcos);
//				p1.x = (x - 1000 * -tsin);
//				p1.y = (y - 1000 * tcos);
//
//					printf("got point at (%d, %d)\n", x, y);

//				if ((angle < (M_PI * 0.25)) || (angle > (M_PI * 0.75))) {

//				p0.x = 1;
//				p0.y = (-tcos/tsin) * p0.x  + (i / tsin);
//				p1.x = width - 1;
//				p1.y = (-tcos/tsin) * p1.x  + (i / tsin);

//				p0.x = (x + 1000 * -tsin);
//				p0.y = (y + 1000 * tcos);
//				p1.x = (x - 1000 * -tsin);
//				p1.y = (y - 1000 * tcos);
//
//					printf("got point at (%d, %d)\n", x, y);

//				if ((angle < (M_PI * 0.25)) || (angle > (M_PI * 0.75))) {
//					for (y = 0; y < height; y++) {
//						x = ((double)((i - center_nrho) - ((y - center_y) * tsin)) / tcos) + center_x;	
//						if (x >= 0 && x < width)
//							*(img + y * width + x) = 255;
//					}
//				} else {
//					for (x = 0; x < width; x++) {
//						y = ((double)((i - center_nrho) - ((x - center_x) * tcos)) / tsin) + center_y;
//						if (y >= 0 && y < height)
//							*(img + y * width + x) = 255;
//					}
//				}
//
//				p0.x = a * i;
//				p0.y = b * i;
//				p1.x = 0;
//				p1.y = 0;
//				p1.x = p0.x + 1000 * -b;
//				p1.y = p0.y + 1000 * a;
//				p2.x = p0.x - 1000 * -b;
//				p2.y = p0.y - 1000 * a;
//				if (j >= 45 && j <= 135) {
//					a.x = 0;
//					a.y = ((double)(i - (hp->nrho / 2)) - ((a.x - (width / 2)) * cos(angle))) / (sin(angle) + (height / 2));
//
//					b.x = width;
//					a.y = ((double)(i - (hp->nrho / 2)) - ((a.x - (width / 2)) * cos(angle))) / (sin(angle) + (height / 2));
//				} else {
//					a.y = 0;
//					a.y = ((double)(i - (hp->nrho / 2)) - ((b.y - (height / 2)) * sin(angle))) / (cos(angle) + (width / 2));
//
//					b.y = height;
//					a.y = ((double)(i - (hp->nrho / 2)) - ((b.y - (height / 2)) * sin(angle))) / (cos(angle) + (width / 2));
//				}
//				printf("(%d, %d)\n", p0.x, p0.y);
				printf("draw line (%d, %d) (%d, %d)\n", p0.x, p0.y, p1.x, p1.y);

				draw_line(img, width, height, p0, p1);
			}
		}
	}
//
//	for (i = 0; i < hp->points_size; i++) {
//		x = hp->points[i].x;
//		y = hp->points[i].y;
//
//		px = img + y * width + x;
//
//		for (theta = 0; theta < hp->resolution; theta++) {
//			angle = theta * M_PI / 180;
//
//			rho = (sin(angle) * y) + (cos(angle) * x);
//			if (rho > 0 && rho < hp->nrho && hough[rho][theta] >= hp->thresh)
//				*px = 255;
//		}
//
//	}

	free(hp->points);
	free(hp);
}

void draw_overlay_circle(struct hough_param_circle *hp, unsigned char *img, int width, int height)
{
	int i;
	int j;
	int a;
	int b;
	int r;
	int theta;
	int x;
	int y;
	int angle;
	unsigned char *px = NULL;

	for (i = 0; i < hp->points_size; i++) {
		x = hp->points[i].x;
		y = hp->points[i].y;

		px = img + x * height + y;

		*px = 0;

		for (theta = 0; theta < hp->resolution; theta++) {
			angle = theta * M_PI / 180;

			for (r = RADIUS_BEGIN; r <= RADIUS_END; r++) {
				a = x - (r * cos(angle));
				b = y - (r * sin(angle));

				if (a > 0 && a < width && b > 0 && b < height && hough_circle[a][b][r] >= hp->thresh)
					*px = 127;
			}
		}
	}

	free(hp->points);
	free(hp);
}
