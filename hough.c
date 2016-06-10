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

static draw_line(unsigned *img, int width, int height, struct point start, struct point end)
{
	int i;
	int j;
	int sum_i;
	int sum_j;
	int delta_x;
	int delta_y;
	unsigned char *px = NULL;
	unsigned char *start_px = NULL;
	unsigned char *end_px = NULL;

	i = start.x;
	j = start.y;

	delta_x = end.x - start.x;
	delta_y = end.y - start.y;

	start_px = img + start.x * width + start.y;
	end_px = img + end.x * width + end.y;

	px = start_px;

	while(i < delta_x && j < delta_y) {
		px = img + i * width + j;

		*px = 127;

		if (delta_x)
			i++;

		if (delta_y)
			j++;
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

			angle = (i * M_PI / 50) - M_PI;
			rho = (sin(angle) * y) + (cos(angle) * x);
			if(rho > 0 && rho < nrho) {

				hough[rho][i]++;

				if(hough[rho][i] > hough[rho_best][theta_best]) {
					theta_best = angle;
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
	hp->thresh = (hough[hp->rho][hp->theta] * 90) / 100;

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
			if (*(img + i * height + j) == 255) {
				points[idx].x = i;
				points[idx].y = j;
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

			angle = (i * M_PI / 50) - M_PI;

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
			if (*(img + i * height + j) == 255) {
				points[idx].x = i;
				points[idx].y = j;
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
	int angle;
	unsigned char *px = NULL;

	for (i = 0; i < hp->points_size; i++) {
		x = hp->points[i].x;
		y = hp->points[i].y;

		px = img + x * height + y;

		for (theta = 0; theta < hp->resolution; theta++) {
			angle = (theta * M_PI / 50) - M_PI;

			rho = (sin(angle) * x) + (cos(angle) * y);

			if (rho > 0 && rho < hp->nrho && hough[rho][theta] >= hp->thresh)
				*px = 127;
			else
				*px = 0;
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
	int angle;
	unsigned char *px = NULL;

	for (i = 0; i < hp->points_size; i++) {
		x = hp->points[i].x;
		y = hp->points[i].y;

		px = img + x * height + y;

		*px = 0;

		for (theta = 0; theta < hp->resolution; theta++) {
			angle = (theta * M_PI / 50) - M_PI;

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
