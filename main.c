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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#include "hough.h"
#include "filters.h"

int main(int argc, char **argv)
{
	FILE *fp = NULL;
	FILE *fp_out = NULL;
	long fsize;
	unsigned char *img = NULL;
	unsigned char *img_out = NULL;
	struct hough_param *hp = NULL;
	struct hough_param_circle *hp_circle = NULL;
	int i;
	int j;
	int width;
	int height;

	if (argc < 6) {
		printf("Usage: %s in_image_path out_image_path width height form(0: line, 1: circle)\n", argv[0]);
		return -EINVAL;
	}

	width = atoi(argv[3]);
	height = atoi(argv[4]);

	fp = fopen(argv[1], "rb");
	if (!fp) {
		printf("invalid image path [%s]\n", argv[1]);
		return -EINVAL;
	}

	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	img = (unsigned char *)malloc(fsize * sizeof(unsigned char));
	if (!img) {
		printf("failed to allocate: %ld bytes\n", fsize);
		return -ENOMEM;
	}

	fread(img, fsize, 1, fp);

	fclose(fp);

	fp_out = fopen(argv[2], "wb");
	if (!fp_out) {
		printf("invalid image path [%s]\n", argv[2]);
		return -EINVAL;
	}

	img_out = (unsigned char *)malloc(fsize * sizeof(unsigned char));
	if (!img_out) {
		printf("failed to allocate: %ld bytes\n", fsize);
		return -ENOMEM;
	}

	memset(img_out, 0, width * height);

	if (!atoi(argv[5])) {
//		edge_filter(img, img_out, width, height);
		memcpy(img_out, img, width * height);

		hp = find_line(img_out, width, height);

		memset(img_out, 0, width * height);

		printf("line, theta: %d, rho: %d\n", hp->theta, hp->rho);

		draw_overlay(hp, img_out, width, height);

		free(hp->points);
		free(hp);

		for (i = 0; i < hp->nrho; i++)
			free(hough[i]);
		free(hough);
	} else {
//		histo_eq(img, width, height);
//		histo_eq_max_filter(img, width, height);
//		histo_eq_average_filter(img, width, height);
//		linear_threshold(img, width, height);
//		basic_threshold(img, width, height);
//		histo_eq_max_filter(img, width, height);
//		median_filter(img, width, height);
//		smooth_filter(img, width, height);
//		gaussian_filter(img, width, height, 1.0);
//		edge_filter(img, img_out, width, height);
//		threshold(img_out, width, height, 255, 255);
		canny_edge(img, img_out, width, height, 1.4, 70, 150);
//		memcpy(img_out, img, width * height);

		hp_circle = find_circle(img_out, width, height);

		memset(img_out, 0, width * height);

		printf("circle, a: %d, b: %d, radix: %d\n", hp_circle->a, hp_circle->b, hp_circle->radius);

		draw_overlay_circle(hp_circle, img, width, height);

		memcpy(img_out, img, width * height);

		free(hp_circle->points);
		free(hp_circle);

		for (i = 0; i < width; i++) {
			for (j = 0; j < height; j++)
				free(hough_circle[i][j]);

			free(hough_circle[i]);
		}
		free(hough_circle);
	}

	fwrite(img_out, width * height, 1, fp_out);

	fclose(fp_out);
	free(img_out);
	free(img);

	return 0;
}
