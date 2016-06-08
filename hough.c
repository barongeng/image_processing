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

#include "hough.h"

#define HOUGH_RES	180

static struct hough_param *find_best_line(struct point *points, int size)
{
	int i;
	int j;
	int hough[10000][HOUGH_RES] = {{0}};
//	char houghTest[DMTX_HOUGH_RES];
//	int hough_min;
//	int hough_max;
	int theta_best = 0;
	int rho;
	int rho_offset = 0;
	int rho_offset_best = 0;
	int x_diff;
	int y_diff;
	struct hough_param *hp;

//
//	/* Predetermine which angles to test */
//	for(i = 0; i < DMTX_HOUGH_RES; i++) {
//		if(houghAvoid == DmtxUndefined) {
//			houghTest[i] = 1;
//		}
//		else {
//			houghMin = (houghAvoid + DMTX_HOUGH_RES/6) % DMTX_HOUGH_RES;
//			houghMax = (houghAvoid - DMTX_HOUGH_RES/6 + DMTX_HOUGH_RES) % DMTX_HOUGH_RES;
//			if(houghMin > houghMax)
//				houghTest[i] = (i > houghMin || i < houghMax) ? 1 : 0;
//			else
//				houghTest[i] = (i > houghMin && i < houghMax) ? 1 : 0;
//		}
//	}

	/* Test each angle for steps along path */
	for(j = 0; j < size; j++) {

		x_diff = points[0].x - points[j].x;
		y_diff = points[0].y - points[j].y;

		/* Increment Hough accumulator */
		for(i = 0; i < HOUGH_RES; i++) {

//			if((int)houghTest[i] == 0)
//				continue;

			rho = (sin_lut[i] * y_diff) - (cos_lut[i] * x_diff);
			if(rho >= -384 && rho <= 384) {

//				if(rho > 128)
//					rho_offset = 2;
//				else if(rho >= -128)
//					rho_offset = 1;
//				else
//					rho_offset = 0;
				rho_offset = abs(rho);

				hough[rho_offset][i]++;

				/* New angle takes over lead */
				if(hough[rho_offset][i] > hough[rho_offset_best][theta_best]) {
					theta_best = i;
					rho_offset_best = rho_offset;
				}
			}
		}
	}

	hp = (struct hough_param *)malloc(sizeof(struct hough_param));
	hp->theta = theta_best;
	hp->rho = rho_offset_best;

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

	hp = find_best_line(points, width * height);

	free(points);

	return hp;
}
