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

#ifndef HOUGH_H
#define HOUGH_H

extern int **hough;
extern int ***hough_circle;

struct point {
	int x;
	int y;
};

struct hough_param {
	int theta;
	int rho;
	int nrho;
	int mag;
	int resolution;
	int thresh;
	struct point *points;
	int points_size;
};

struct hough_param_circle {
	int a;
	int b;
	int radius;
	int resolution;
	int thresh;
	struct point *points;
	int points_size;
};

struct hough_param *find_line(unsigned char *img, int width, int height);
struct hough_param_circle *find_circle(unsigned char *img, int width, int height);
void draw_overlay(struct hough_param *hp, unsigned char *img, int width, int height);
void draw_overlay_circle(struct hough_param_circle *hp, unsigned char *img, int width, int height);

#endif /* HOUGH_H */
