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

#ifndef FILTERS_H
#define FILTERS_H

void egde_filter(unsigned char *in, unsigned char *out, int width, int height);
void histo_eq(unsigned char *src, int width, int height);
void histo_eq_max_filter(unsigned char *src, int width, int height);
void histo_eq_average_filter(unsigned char *src, int width, int height);
void linear_threshold(unsigned char *src, int width, int height);
void basic_threshold(unsigned char *src, int width, int height);
void threshold(unsigned char *src, int width, int height, int min, int max);
void median_filter(unsigned char *src, int width, int height);
void smooth_filter(unsigned char *src, int width, int height);
void gaussian_filter(unsigned char *src, int width, int height, float sigma);

#endif /* FILTERS_H */
