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

#include "img_utils.h"

#define sign(x) ((x<0)?-1:((x>0)?1:0))

enum { TOP = 0x1, BOTTOM = 0x2, RIGHT = 0x4, LEFT = 0x8 };

static unsigned int compute_outcode(int x, int y, int xmin, int ymin, int xmax, int ymax)
{
	unsigned int oc = 0;

	if (y > ymax)
		oc |= TOP;
	else if (y < ymin)
		oc |= BOTTOM;


	if (x > xmax)
		oc |= RIGHT;
	else if (x < xmin)
		oc |= LEFT;

	return oc;
}

void clip_line(struct point *p1, struct point *p2, int xmin, int ymin, int xmax, int ymax)
{
	int accept;
	int done;
	unsigned int outcode1, outcode2;
	int x, y, x1, y1, x2, y2;
	int outcode_ex;

	x1 = p1->x;
	y1 = p1->y;

	x2 = p2->x;
	y2 = p2->y;

	accept = 0;
	done = 0;

	outcode1 = compute_outcode(x1, y1, xmin, ymin, xmax, ymax);
	outcode2 = compute_outcode(x2, y2, xmin, ymin, xmax, ymax);
	do
	{
		if (outcode1 == 0 && outcode2 == 0) {
			accept = 1;
			done = 1;
		} else if (outcode1 & outcode2) {
			done = 1;
		} else {
			outcode_ex = outcode1 ? outcode1 : outcode2;
			if (outcode_ex & TOP) {
				x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1);
				y = ymax;
			} else if (outcode_ex & BOTTOM) {
				x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1);
				y = ymin;
			} else if (outcode_ex & RIGHT) {
				y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1);
				x = xmax;
			} else {
				y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1);
				x = xmin;
			}

			if (outcode_ex == outcode1) {
				x1 = x;
				y1 = y;
				outcode1 = compute_outcode (x1, y1, xmin, ymin, xmax, ymax);
			} else {
				x2 = x;
				y2 = y;
				outcode2 = compute_outcode (x2, y2, xmin, ymin, xmax, ymax);
			}
		}
	} while (!done);

	if (accept) {
		p1->x = x1;
		p1->y = y1;

		p2->x = x2;
		p2->y = y2; 
	}
}

void draw_line(unsigned char *img, int width, int height, struct point start, struct point end)
{

	int dx,dy,sdx,sdy,px,py,dxabs,dyabs,i;
	float slope;
	unsigned char *p = NULL;
	int x1 = start.x;
	int y1 = start.y;
	int x2 = end.x;
	int y2 = end.y;
	dx = x2 - x1;      /* the horizontal distance of the line */
	dy = y2 - y1;      /* the vertical distance of the line */
	dxabs = abs(dx);
	dyabs = abs(dy);
	sdx = sign(dx);
	sdy = sign(dy);
	if (dxabs >= dyabs) {
		slope = (float)dy / (float)dx;
		for (i = 0; i != dx; i += sdx) {
			px = i + x1;
			py = slope * i + y1;
			p = img + py * width + px;
			*p = 255;
		}
	}
	else {
		slope = (float)dx / (float)dy;
		for (i = 0; i != dy; i += sdy) {
			px = slope * i + x1;
			py = i + y1;
			p = img + py * width + px;
			*p = 255;
		}
	}

}

