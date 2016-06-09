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

#define HOUGH_RES	180

int **hough;

static struct hough_param *find_best_line(struct point *points, int size, int width, int height)
{
	int i;
	int j;
	int theta_best = 0;
	int rho;
	int nrho;
	int rho_best = 0;
	int x_diff;
	int y_diff;
	struct hough_param *hp;

	nrho = sqrt(width * width + height * height);

	hough = (int **)malloc(nrho * sizeof(int *));
	memset(hough, 0, nrho * sizeof(int *));

	for (i = 0; i < nrho; i++) {
		hough[i] = (int *)malloc(HOUGH_RES * sizeof(int));
		memset(hough[i], 0, HOUGH_RES * sizeof(int));
	}

	/* Test each angle for steps along path */
	for(j = 0; j < size; j++) {

		x_diff = points[j].x;
		y_diff = points[j].y;

		if (!x_diff && !y_diff)
			continue;

//		printf("compute point at (%d, %d): ", x_diff, y_diff);

//		x_diff = points[0].x - points[j].x;
//		y_diff = points[0].y - points[j].y;

		/* Increment Hough accumulator */
		for(i = 0; i < HOUGH_RES; i++) {

			rho = (sin_lut[i] * y_diff) + (cos_lut[i] * x_diff);
			if(rho > 0 && rho < nrho) {

				hough[rho][i]++;

//				printf("%d, %d [%d]\n", rho, i, hough[rho][i]);

				/* New angle takes over lead */
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
	hp->resolution = HOUGH_RES;
	hp->thresh = (hough[hp->rho][hp->theta] * 70) / 100;

	printf("accumulator max: %d\n", hough[rho_best][theta_best]);
	printf("hp->theta: %d\n", hp->theta);
	printf("hp->rho: %d\n", hp->rho);
	printf("hp->nrho: %d\n", hp->nrho);
	printf("hp->resolution: %d\n", hp->resolution);
	printf("hp->thresh: %d\n", hp->thresh);

//	for (i = 0; i < nrho; i++)
//		free(hough[i]);
//
//	free(hough);

	return hp;
}

struct hough_param *find_line(unsigned char *img, int width, int height)
{
	int i;
	int j;
	int idx = 0;
	struct point *points = NULL;
	struct hough_param *hp = NULL;

	points = (struct point *)malloc(width * height * sizeof(struct point));

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

	free(points);

	return hp;
}
