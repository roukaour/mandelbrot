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

#define SET_MANDELBROT 0
#define SET_MANDELBAR 1
#define SET_BURNING_SHIP 2

#define MAND_FALSE 0
#define MAND_TRUE 1

typedef struct {
	int r, g, b;
} MAND_COLOR;

typedef struct {
	int pw;
	long double cx, cy, w, h;
	int imax;
	long double r;
	int set;
	int j;
	long double jr, ji;
	MAND_COLOR *ec, **cs;
	int ncs;
	int nic;
} MANDELBROT;
#ifdef __cplusplus
extern "C" {
#endif    
    MAND_COLOR **make_palette(MAND_COLOR **cs, int ncs, int *total, MAND_COLOR *ec);
    void mandelbrot(MANDELBROT *man, MAND_COLOR** palette, int total, void (*dimensions_found)(int pw, int ph, void* e), void* dim_arg, void (*color_found)(int r, int g, int b, void* e), void* color_arg);

#ifdef __cplusplus
}  // End of extern "C" block
#endif
