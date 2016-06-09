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


//		x_diff = points[0].x - points[j].x;
//		y_diff = points[0].y - points[j].y;

		/* Increment Hough accumulator */
		for(i = 0; i < HOUGH_RES; i++) {

			angle = (i * M_PI / 50) - M_PI;
//			rho = (sin_lut[i] * y_diff) + (cos_lut[i] * x_diff);
			rho = (sin(angle) * y_diff) + (cos(angle) * x_diff);
			if(rho > 0 && rho < nrho) {

				hough[rho][i]++;

				/* New angle takes over lead */
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
	hp->resolution = HOUGH_RES;
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

void draw_overlay(struct hough_param *hp, unsigned char *img, int width, int height)
{
	int i;
	int j;
	int theta;
	int rho;
	int x;
	int y;
	int start_x;
	int start_y;
	int end_x;
	int end_y;
	int angle;
	int new_angle;
	unsigned char *px = NULL;
	struct point a;
	struct point b;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			px = img + i * height + j;

			if(*px) {
				 *px = 0;

				for (theta = 0; theta < hp->resolution; theta++) {
//					rho = i * cos_lut[theta] + j * sin_lut[theta];
					angle = (theta * M_PI / 50) - M_PI;

					rho = (sin(angle) * j) + (cos(angle) * i);

					if (rho > 0 && rho < hp->nrho && hough[rho][theta] >= hp->thresh) {
//						x = rho * cos(angle);
//						y = rho * sin(angle);
//
//						if (x < 0)
//							x = 0;
//
//						if (y < 0)
//							y = 0;
//
////						if (angle < 90)
////							new_angle = 90 + angle;
////						else
////							new_angle = 90 - (180 - angle);
//
//						start_x = x + (hp->nrho * cos(new_angle));
//						start_y = y + (hp->nrho * sin(new_angle));
//
//						end_x = x - (hp->nrho * cos(new_angle));
//						end_y = y - (hp->nrho * sin(new_angle));
//
//						if (start_x < 0)
//							start_x = 0;
//						else if (start_x > width)
//							start_x = width;
//
//						if (start_y < 0)
//							start_y = 0;
//						else if (start_y > height)
//							start_y = height;
//
//						if (end_x < 0)
//							end_x = 0;
//						else if (end_x > width)
//							end_x = width;
//
//						if (end_y < 0)
//							end_y = 0;
//						else if (end_y > height)
//							end_y = height;
//
//						a.x = start_x;
//						a.y = start_y;
//
//						b.x = end_x;
//						b.y = end_y;

//						draw_line(img, width, height, a, b);

//						printf("point (%d, %d)\n", x, y);
//						printf("start (%d, %d) , end(%d, %d)\n", start_x, end_x, start_y, end_y);

						*px = 127;
					}
				}
			}
		}
	}

	free(hp);
}
