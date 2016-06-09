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
	unsigned char *px = NULL;
	struct hough_param *hp = NULL;
	int i;
	int j;
	int theta;
	int rho;
	int width;
	int height;

	if (argc < 5) {
		printf("Usage: %s in_image_path out_image_path width height\n", argv[0]);
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

	egde_filter(img, img_out, width, height);

	hp = find_line(img_out, width, height);

	printf("line, theta: %d, rho: %d\n", hp->theta, hp->rho);

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			px = img_out + i * height + j;

			if(*px) {
				 *px = 0;

//				printf("compute point at (%d, %d)\n", i, j);

				for (theta = 0; theta < hp->resolution; theta++) {
					rho = i * cos_lut[theta] + j * sin_lut[theta];

					if(rho > 0 && rho < hp->nrho)
//						printf("%d, %d [%d]\n", rho, theta, hough[rho][theta]);

					if (rho > 0 && rho < hp->nrho && hough[rho][theta] >= hp->thresh) {
//						printf("print gray pixel !\n");
						*px = 127;
					}
				}
			}
		}
	}

	fwrite(img_out, width * height, 1, fp_out);

	fclose(fp_out);

	for (i = 0; i < hp->nrho; i++)
		free(hough[i]);

	free(hough);
	free(img_out);
	free(img);
	free(hp);

	return 0;
}
