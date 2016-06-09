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
#include <string.h>
#include <math.h>

#include "filters.h"

#define MAX_BRIGHTNESS	255
#define GRAY_LEVELS	256

#define min(x, y) ({                            \
		typeof(x) _min1 = (x);                  \
		typeof(y) _min2 = (y);                  \
		(void) (&_min1 == &_min2);              \
		_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                            \
		typeof(x) _max1 = (x);                  \
		typeof(y) _max2 = (y);                  \
		(void) (&_max1 == &_max2);              \
		_max1 > _max2 ? _max1 : _max2; })

static int edge_sobel_kernel_x[] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
static int edge_sobel_kernel_y[] = {1, 2, -1, 0, 0, 0, -1, -2, -1};

static int convolve_kernel(unsigned char *px, int *kernel, int size)
{
	int i;
	int sum = 0;

	for (i = 0; i < size; i++) {
		sum += px[i] * kernel[i];
	}

	return sum;
}

void egde_filter(unsigned char *in, unsigned char *out, int width, int height)
{
	int i;
	int j;
	int gx;
	int gy;
	int g;
	int min = 255;
	int max = 0;
	int start_x = 1;
	int start_y = 1;
	unsigned char *p = NULL;
	unsigned char px[9];

	for (i = start_x; i < (height - start_x); i++) {
		for (j = start_y; j < (width - start_y); j++) {
			p = out + i * width + j;

			px[0] = *(in + (i - 1) * width + (j - 1));
			px[1] = *(in + (i - 1) * width + j);
			px[2] = *(in + (i - 1) * width + (j + 1));
			px[3] = *(in + i * width + (j -1));
			px[4] = *(in + i * width + j);
			px[5] = *(in + i * width + (j + 1));
			px[6] = *(in + (i + 1) * width + (j - 1));
			px[7] = *(in + (i + 1) * width + j);
			px[8] = *(in + (i + 1) * width + (j + 1));

			gx = convolve_kernel(px, edge_sobel_kernel_x, 9);

			if (gx > MAX_BRIGHTNESS)
				gx = MAX_BRIGHTNESS;
			if (gx < 0)
				gx = 0;
			gy = convolve_kernel(px, edge_sobel_kernel_y, 9);

			if (gy > MAX_BRIGHTNESS)
				gy = MAX_BRIGHTNESS;
			if (gy < 0)
				gy = 0;

			g = sqrt(gx * gx + gy * gy);

			*p = g;
		}
	}
}


void histo_eq(unsigned char *src, int width, int height)
{
	int i;
	int val;
	int sum = 0;
	int size = width * height;
	float constant;
	unsigned int histo[256];
	unsigned int histo_sum[256];

	 for (i= 0; i < GRAY_LEVELS; i++) {
		sum += histo[i];
		histo_sum[i] = sum;
	}

	constant = (float)230 / (float)size;
//	constant = (float)255 / (float)size;

	for (i = 0; i < size; i++) {
		val = *(src + i);
		*(src + i) = histo_sum[val] * constant;
	}
 
}

void linear_threshold(unsigned char *src, int width, int height)
{
	int i;
	int val;
	int max = 0;
	int min = 255;
	int size = width * height;

	for (i = 0; i < size; i++) {
		val = *(src + i);

		min = min(min, val);
		max = max(max, val);
	}

	printf("min: %d, max: %d\n", min, max);

	for (i = 0; i < size; i++) {
		val = *(src + i);
		val = (val - min) * MAX_BRIGHTNESS / (max - min);

		*(src + i) = val;
	}
}
