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

static int edge_sobel_kernel_x[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
static int edge_sobel_kernel_y[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
static int median_array[3 * 3];

static int convolve_kernel(unsigned char *px, int *kernel, int size)
{
	int i;
	int sum = 0;

	for (i = 0; i < size; i++) {
		sum += px[i] * kernel[i];
	}

	return sum;
}

void edge_filter(unsigned char *in, unsigned char *out, int width, int height)
{
	int i;
	int j;
	int gx;
	int gy;
	int g;
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
			gy = convolve_kernel(px, edge_sobel_kernel_y, 9);

			g = sqrt(gx * gx + gy * gy);

//			if (g > MAX_BRIGHTNESS)
//				g = MAX_BRIGHTNESS;

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

	for (i = 0; i < size; i++) {
		val = *(src + i);
		histo[val]++;
	}

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

void histo_eq_max_filter(unsigned char *src, int width, int height)
{
	int i;
	int j;
	int val;
	int sum = 0;
	int max = 0;
	int size = width * height;
	float constant;
	unsigned int histo[256];
	unsigned int histo_sum[256];

	for (i = 0; i < size; i++) {
		val = *(src + i);
		histo[val]++;
	}

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

	/* max between 10 points */
	for (i = 0; i < GRAY_LEVELS; i ++) {
		for (j = i; j < (i + 10); j++) {
			max = max(max, histo[j]);
		}

		histo[i] = max;
		max = 0;
	}
 
}

void histo_eq_average_filter(unsigned char *src, int width, int height)
{
	int i;
	int j;
	int val;
	int sum = 0;
	int size = width * height;
	float constant;
	unsigned int histo[256];
	unsigned int histo_sum[256];

	for (i = 0; i < size; i++) {
		val = *(src + i);
		histo[val]++;
	}

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

	/* average between 10 points */
	for (i = 0; i < GRAY_LEVELS; i ++) {
		val = 0;
		for (j = i; j < (i + 10); j++) {
			val += histo[j];
		}

		histo[i] = val / 10;
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

void basic_threshold(unsigned char *src, int width, int height)
{
	int i;
	int val;
	int max = 126;
	int min = 128;
	int size = width * height;

	for (i = 0; i < size; i++) {
		val = *(src + i);

		if (val > max)
			val = 255;
		else if (val < min)
			val = 0;

		*(src + i) = val;
	}
}

void threshold(unsigned char *src, int width, int height, int min, int max)
{
	int i;
	int val;
	int size = width * height;

	for (i = 0; i < size; i++) {
		val = *(src + i);

		if (val > max)
			val = 255;
		else if (val < min)
			val = 0;

		*(src + i) = val;
	}
}

static void insert_array(unsigned char val)
{
	static int i = 0;

	if (i == 9)
		i = 0;

	median_array[i] = val;
	i++;
}

static void sort(void)
{
	int i, j, t;

	for (i = 1; i < 9; i++) {
		t = median_array[i];
		j = i - 1;

		while ((j >= 0) && (t < median_array[j])) {
			median_array[j + 1] = median_array[j];
			j--;
		}

		median_array[j + 1] = t;
	}
}

static unsigned char median(void)
{
	return median_array[9 / 2];
}

void median_filter(unsigned char *src, int width, int height)
{
	int k,l;
	int i = 1;
	int j = 1;
	int start_x = j * width + i;
	unsigned int size = width * height - 1;

	for (i = start_x; i < size; i++) {
		for (k = i - width; k <= i + width; k += width) {
			for (l = k - 1; l <= k + 1; l++)
				insert_array(src[l]);

		}

		sort();
		src[i] = median();

	}
}

void smooth_filter(unsigned char *src, int width, int height)
{
	int k,l;
	int i = 1;
	int j = 1;
	int val = 0;
	int start_x = j * width + i;
	unsigned int size = width * height - 1;

	for (i = start_x; i < size; i++) {
		val = 0;
		for (k = i - width; k <= i + width; k += width) {
			for (l = k - 1; l <= k + 1; l++)
				val += src[l];
		}

		src[i] = (val / 9);
	}
}

static float get_gaussian(unsigned char *buffer, float *kernel, int len)
{
	int i;
	float val = 0.0;

	for (i = 0; i < len; i++)
		val += (float)buffer[i] * kernel[i];

	return val;
}

void gaussian_filter(unsigned char *src, int width, int height, float sigma)
{
	int i;
	float sum = 0.0;
	float *kernel = NULL;
	int len = width * height;
	int g_size = 2 * (int)(2 * sigma) + 3;
	unsigned char *out = NULL;

	out = (unsigned char *)malloc(len);
	kernel = (float *)malloc(g_size * g_size * sizeof(float));

	memcpy(out, src, len);

	for (i = -g_size; i <= g_size; i++) {
		kernel[i + g_size] = exp(-0.5 * i * i / (sigma * sigma));
		sum += kernel[i +  g_size];
	}

	for (i = -g_size; i <= g_size; i++)
		kernel[i + g_size] /= sum;

	for (i = g_size; i < (len - g_size); i++)
		src[i] = get_gaussian(&out[i - g_size], kernel, g_size * g_size);


	free(out);
	free(kernel);
}

void canny_edge(unsigned char *src, unsigned char *out, int width, int height, float sigma, int tmin, int tmax)
{
	int i;
	int j;
	int k;
	int gx;
	int gy;
	int g;
	int nn;
	int ss;
	int ww;
	int ee;
	int nw;
	int ne;
	int sw;
	int se;
	int p;
	float dir;
	int start_x = 1;
	int start_y = 1;
	unsigned char px[9];
	unsigned char hyst_px[9];
	unsigned char *edge = (unsigned char *)malloc(width * height * sizeof(unsigned char));

	gaussian_filter(src, width, height, sigma);

	edge_filter(src, out, width, height);

	memcpy(edge, out, width * height);

	for (i = start_x; i < (height - start_x); i++) {
		for (j = start_y; j < (width - start_y); j++) {
			p = i * width + j;

			px[0] = *(src + (i - 1) * width + (j - 1));
			px[1] = *(src + (i - 1) * width + j);
			px[2] = *(src + (i - 1) * width + (j + 1));
			px[3] = *(src + i * width + (j -1));
			px[4] = *(src + i * width + j);
			px[5] = *(src + i * width + (j + 1));
			px[6] = *(src + (i + 1) * width + (j - 1));
			px[7] = *(src + (i + 1) * width + j);
			px[8] = *(src + (i + 1) * width + (j + 1));

			gx = convolve_kernel(px, edge_sobel_kernel_x, 9);
			gy = convolve_kernel(px, edge_sobel_kernel_y, 9);

			/* compute direction */
			nn = p - width;
			ss = p + width;
			ww = p + 1;
			ee = p - 1;
			nw = nn + 1;
			ne = nn - 1;
			sw = ss + 1;
			se = ss - 1;

			dir = (float)(fmod(atan2(gy, gx) + M_PI, M_PI) / M_PI) * 8;

			if (((dir <= 1 || dir > 7) && edge[p] > edge[ee] && edge[p] > edge[ww]) || // 0 deg
				((dir > 1 && dir <= 3) && edge[p] > edge[nw] && edge[p] > edge[se]) || // 45 deg
				((dir > 3 && dir <= 5) && edge[p] > edge[nn] && edge[p] > edge[ss]) || // 90 deg
				((dir > 5 && dir <= 7) && edge[p] > edge[ne] && edge[p] > edge[sw]))   // 135 deg
				out[p] = edge[p];
			else
				out[p] = 0;

		}
	}


	memcpy(src, out, width * height);


//	for (i = start_x; i < (height - start_x); i++) {
//		for (j = start_y; j < (width - start_y); j++) {
//			if (*(src + i * width + j) >= tmax)
//				*(src + i * width + j) = MAX_BRIGHTNESS;
//			else if (*(src + i * width + j) <= tmin)
//				*(src + i * width + j) = 0;
//			else
//				*(src + i * width + j) = 127;
//		}
//	}
//
	memset(out, 0, width * height);

	for (i = start_x; i < (height - start_x); i++) {
		for (j = start_y; j < (width - start_y); j++) {
			p = i * width + j;

			if (src[p] >= tmax && out[p] == 0) {
				out[p] = MAX_BRIGHTNESS;
				px[0] = p;
				g = 1;

				do
				{
					g--;

					hyst_px[0] = px[g] - width;
					hyst_px[1] = px[g] + width;
					hyst_px[2] = px[g] + 1;
					hyst_px[3] = px[g] - 1;
					hyst_px[4] = px[g] - width + 1;
					hyst_px[5] = px[g] - width - 1;
					hyst_px[6] = px[g] + width + 1;
					hyst_px[7] = px[g] + width - 1;

					for (k = 0; k < 8; k++) {
						if (src[hyst_px[k]] >= tmin && out[hyst_px[k]] == 0) {
							out[hyst_px[k]] = MAX_BRIGHTNESS;
							px[g] = hyst_px[k];
							g++;
						}
					}
				} while (g);
			}
		}
	}

	free(edge);
}
