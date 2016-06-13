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

int **hough;
int ***hough_circle;

static struct hough_param *find_best_line(struct point *points, int size, int width, int height)
{
	int i;
	int j;
	double angle;
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
	double angle;
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
	double angle;
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

				p0.x = x + max_length * -tsin;
				p0.y = y + max_length * tcos;
				p1.x = x - max_length * -tsin;
				p1.y = y - max_length * tcos;

				clip_line(&p0, &p1, 0, 0, width, height);
				draw_line(img, width, height, p0, p1);
			}
		}
	}

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
	double angle;
	struct point p0;

	for (a = 0; a < width; a++) {
		for (b = 0; b < height; b++) {
			for (r = RADIUS_BEGIN; r <= RADIUS_END; r++) {
				if (hough_circle[a][b][r] >= hp->thresh) {
					angle = theta * M_PI / 180;

					x = a + r * cos(angle);
					y = b + r * sin(angle);

					p0.x = x;
					p0.y = y;

					draw_circle(img, width, height, p0, r);
				}
			}
		}
	}

	free(hp->points);
	free(hp);
}
