/*
 * mandelbrot - generates an image of the Mandelbrot set or certain related
 * fractals.
 *
 * Copyright (C) 2011-2016 Remy Oukaour <http://www.remyoukaour.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "mandelbrot.h"

#ifdef __GNUC__
#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_ ## x
#endif

#define LOG2 0.69314718055994530941723212145817


typedef int (*MAND_COLOR_INDEX)(int, int, int, long double, long double, long double);


MAND_COLOR **make_palette(MAND_COLOR **cs, int ncs, int *total, MAND_COLOR *ec) {
	int i, dr, dg, db, maxd, d, at;
	MAND_COLOR **palette = malloc(sizeof (MAND_COLOR **));
	palette[0] = malloc(sizeof (MAND_COLOR *));
	palette[0]->r = cs[0]->r;
	palette[0]->g = cs[0]->g;
	palette[0]->b = cs[0]->b;
	for (i = 1, at = 0, *total = 1; i < ncs; i++, at += maxd) {
		dr = cs[i]->r - cs[i-1]->r;
		dg = cs[i]->g - cs[i-1]->g;
		db = cs[i]->b - cs[i-1]->b;
		maxd = abs(abs(dr) > abs(dg) ?
			(abs(dr) > abs(db) ? dr : db) :
			(abs(dg) > abs(db) ? dg : db));
		*total += maxd;
		palette = realloc(palette, *total * (sizeof (MAND_COLOR *)));
		for (d = 1; d <= maxd; d++) {
			palette[at+d] = malloc(sizeof (MAND_COLOR *));
			palette[at+d]->r = (int)(cs[i-1]->r + (long double)dr*d/maxd);
			palette[at+d]->g = (int)(cs[i-1]->g + (long double)dg*d/maxd);
			palette[at+d]->b = (int)(cs[i-1]->b + (long double)db*d/maxd);
		}
	}
	palette = realloc(palette, (*total + 1) * (sizeof (MAND_COLOR *)));
	palette[*total] = malloc(sizeof (MAND_COLOR *));
	palette[*total]->r = ec != NULL ? ec->r : 0;
	palette[*total]->g = ec != NULL ? ec->g : 0;
	palette[*total]->b = ec != NULL ? ec->b : 0;
	return palette;
}

int banded_color(int total, int n, int imax, long double UNUSED(zr), long double UNUSED(zi), long double UNUSED(logr)) {
	return n * total / imax;
}

int smooth_color(int total, int n, int imax, long double zr, long double zi, long double logr) {
	int c = total;
	long double mu;
	if (n < imax) {
		mu = n - log(log(sqrt(zr*zr + zi*zi)) / logr) / LOG2;
		c = mu * total / imax;
		if (c < 0)
			c = 0;
		else if (c >= total)
			c = total - 1;
	}
	return c;
}

void mandelbrot(MANDELBROT *man, MAND_COLOR** palette, int total, void (*dimensions_found)(int pw, int ph, void* e), void* extra_dim, void (*color_found)(int r, int g, int b, void* e), void* extra_color) {
	int pw = man->pw, ph = man->h * man->pw / man->w;
    dimensions_found(pw, ph, extra_dim);
	int set = man->set, j = man->j;
	int x, y, n, c, p;
	long double r2 = man->r * man->r, logr = log(man->r);
	long double xmin = man->cx - man->w/2, ymin = man->cy - man->h/2;
	long double dx = man->w / pw, dy = man->h / ph;
	long double jr = man->jr, ji = man->ji;
	long double cr, ci, zr, zi, zro, zio, q, qt1, qt2, t;
	MAND_COLOR_INDEX color = man->nic ? smooth_color : banded_color;
	for (y = ph - 1; y >= 0; y--) {
		ci = ymin + dy * y;
		for (x = 0; x < pw; x++) {
			cr = xmin + dx * x;
			zr = cr;
			zi = ci;
			n = 0;
			if (set == SET_MANDELBROT && !j) {
				// cardioid test
				qt1 = zr - 0.25;
				qt2 = zi * zi;
				q = qt1 * qt1 + qt2;
				q *= q + qt1;
				if (q < qt2 * 0.25) {
					n = man->imax;
				}
				else {
					// period-2 bulb test
					q = zr + 1.0;
					q = q * q + qt2;
					if (q < 0.0625) {
						n = man->imax;
					}
				}
			}
			for (; n < man->imax; n++) {
				if (set == SET_MANDELBAR) {
					zi = -zi;
				}
				else if (set == SET_BURNING_SHIP) {
					zr = zr < 0 ? -zr : zr;
					zi = zi < 0 ? zi : -zi;
				}
				zro = zr;
				zio = zi;
				p = set;
				do {
					t = zr*zro - zi*zio;
					zi = zr*zio + zi*zro;
					zr = t;
				} while (p-- > 2);
				zr += j ? jr : cr;
				zi += j ? ji : ci;
				if (zr*zr + zi*zi > r2)
					break;
			}
			c = color(total, n, man->imax, zr, zi, logr);
            color_found(palette[c]->r, palette[c]->g, palette[c]->b, extra_color);
		}
	}
}
