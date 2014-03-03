/*
 * mandelbrot - generates an image of the Mandelbrot set or certain related
 * fractals.
 *
 * Copyright (C) 2011-2014 Remy Oukaour <http://www.remyoukaour.com>.
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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#ifdef __GNUC__
#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_ ## x
#endif

#define eprintf(args...) fprintf(stderr, args)
#define nfree(p) do { free(p); p = NULL; } while (0)

#define FALSE 0
#define TRUE 1

#define LOG2 0.69314718055994530941723212145817

#define SET_MANDELBROT 0
#define SET_MANDELBAR 1
#define SET_BURNING_SHIP 2

#define PROGRAM_NAME "mandelbrot"

#define USAGE_TEXT (\
	"usage: " PROGRAM_NAME " [OPTION]... [FILE]\n"\
	"Enter '" PROGRAM_NAME " -h' for more information.\n")

#define HELP_TEXT (\
	"NAME\n"\
	"\t" PROGRAM_NAME " - generates an image of the Mandelbrot set or certain\n"\
	"\t             related fractals\n"\
	"\n"\
	"SYNOPSIS\n"\
	"\t" PROGRAM_NAME " [OPTION]... [FILE]\n"\
	"\n"\
	"DESCRIPTION\n"\
	"\tGenerates a PPM image of the Mandelbrot set or certain related fractals.\n"\
	"\n"\
	"\t-w, --width=WIDTH\n"\
	"\t\timage width in pixels (default 640)\n"\
	"\n"\
	"\t-c, --center=X/Y\n"\
	"\t\tcenter coordinates (default 0/0)\n"\
	"\n"\
	"\t-d, --dim=W/H\n"\
	"\t\tregion dimensions (default 4/4)\n"\
	"\n"\
	"\t-i, --iterations=IMAX\n"\
	"\t\tmaximum number of iterations (default 128)\n"\
	"\n"\
	"\t-r, --radius=RADIUS\n"\
	"\t\tescape radius (default 2)\n"\
	"\n"\
	"\t-s, --set=SET\n"\
	"\t\tselect the fractal to generate (default 0)\n"\
	"\t\t0   Mandelbrot\n"\
	"\t\t1   Mandelbar\n"\
	"\t\t2   Burning ship\n"\
	"\t\t3+  Multibrot\n"\
	"\n"\
	"\t-j, --julia=JR/JI\n"\
	"\t\tgenerate a Julia set for the chosen fractal\n"\
	"\n"\
	"\t-n, --normalize\n"\
	"\t\tuse normalized iteration count (smooth gradients, but slower)\n"\
	"\n"\
	"\t-e, --color=RRGGBB/RGB\n"\
	"\t\tcolor for unescaped points in the set (default 000)\n"\
	"\n"\
	"\t-p, --palette=RRGGBB/RGB/...\n"\
	"\t\t\"keyframe\" colors for a palette (default 000/FFF)\n"\
	"\n"\
	"\t-h, --help\n"\
	"\t\tdisplay this help and exit\n"\
	"\n"\
	"\tWith no FILE, or when FILE is -, write to standard output.\n"\
	"\n"\
	"AUTHOR\n"\
	"\tWritten by Remy Oukaour <remy.oukaour@gmail.com>.\n"\
	"\n"\
	"COPYRIGHT\n"\
	"\tCopyright (C) 2011-2014 Remy Oukaour <http://www.remyoukaour.com>.\n"\
	"\tMIT License.\n"\
	"\tThis is free software: you are free to change and redistribute it.\n"\
	"\tThere is NO WARRANTY, to the extent permitted by law.\n")

typedef int (*COLOR_INDEX)(int, int, int, long double, long double, long double);

typedef struct {
	int r, g, b;
} COLOR;

typedef struct {
	FILE *fp;
	int pw;
	long double cx, cy, w, h;
	int imax;
	long double r;
	int set;
	int j;
	long double jr, ji;
	COLOR *ec, **cs;
	int ncs;
	int nic;
} MANDELBROT;

int hex2int(char t, char o) {
	int hex = t;
	if (t >= '0' && t <= '9')
		hex -= '0';
	else if (t >= 'a' && t <= 'f')
		hex -= 'a' - 10;
	else if (t >= 'A' && t <= 'F')
		hex -= 'A' - 10;
	else
		return 0;
	hex = hex * 16 + o;
	if (o >= '0' && o <= '9')
		hex -= '0';
	else if (o >= 'a' && o <= 'f')
		hex -= 'a' - 10;
	else if (o >= 'A' && o <= 'F')
		hex -= 'A' - 10;
	else
		return 0;
	return hex;
}

COLOR **get_colors(char *s, COLOR **cs, int *ncs) {
	int i, j, n;
	char *buffer, *token;
	*ncs = 0;
	for (i = 0; 1; i++, s = NULL) {
		token = strtok_r(s, "/", &buffer);
		if (token == NULL)
			break;
		j = (*ncs)++;
		cs = realloc(cs, *ncs * (sizeof (COLOR *)));
		cs[j] = malloc(sizeof (COLOR *));
		n = strlen(token);
		if (n == 6) {
			cs[j]->r = hex2int(token[0], token[1]);
			cs[j]->g = hex2int(token[2], token[3]);
			cs[j]->b = hex2int(token[4], token[5]);
		}
		else if (n == 3) {
			cs[j]->r = hex2int(token[0], token[0]);
			cs[j]->g = hex2int(token[1], token[1]);
			cs[j]->b = hex2int(token[2], token[2]);
		}
		else {
			cs[j]->r = 0;
			cs[j]->g = 0;
			cs[j]->b = 0;
		}
	}
	return cs;
}

COLOR **make_palette(COLOR **cs, int ncs, int *total, COLOR *ec) {
	int i, dr, dg, db, maxd, d, at;
	COLOR **palette = malloc(sizeof (COLOR **));
	palette[0] = malloc(sizeof (COLOR *));
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
		palette = realloc(palette, *total * (sizeof (COLOR *)));
		for (d = 1; d <= maxd; d++) {
			palette[at+d] = malloc(sizeof (COLOR *));
			palette[at+d]->r = (int)(cs[i-1]->r + (long double)dr*d/maxd);
			palette[at+d]->g = (int)(cs[i-1]->g + (long double)dg*d/maxd);
			palette[at+d]->b = (int)(cs[i-1]->b + (long double)db*d/maxd);
		}
	}
	palette = realloc(palette, (*total + 1) * (sizeof (COLOR *)));
	palette[*total] = malloc(sizeof (COLOR *));
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

void mandelbrot(MANDELBROT *man) {
	int pw = man->pw, ph = man->h * man->pw / man->w;
	int set = man->set, j = man->j;
	int x, y, n, c, total, p, i;
	long double r2 = man->r * man->r, logr = log(man->r);
	long double xmin = man->cx - man->w/2, ymin = man->cy - man->h/2;
	long double dx = man->w / pw, dy = man->h / ph;
	long double jr = man->jr, ji = man->ji;
	long double cr, ci, zr, zi, zro, zio, q, qt1, qt2, t;
	COLOR_INDEX color = man->nic ? smooth_color : banded_color;
	COLOR **palette = make_palette(man->cs, man->ncs, &total, man->ec);
	fprintf(man->fp, "P6\n%d %d\n255\n", pw, ph);
	for (y = ph - 1; y >= 0; y--) {
		ci = ymin + dy * y;
		for (x = 0; x < pw; x++) {
			cr = xmin + dx * x;
			zr = cr;
			zi = ci;
			n = 0;
			if (set == SET_MANDELBROT) {
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
			fprintf(man->fp, "%c%c%c", palette[c]->r, palette[c]->g, palette[c]->b);
		}
	}
	for (i = 0; i <= total; i++) {
		nfree(palette[i]);
	}
	nfree(palette);
}

int main(int argc, char *argv[]) {
	extern char *optarg;
	extern int optind, optopt, opterr;
	int opt, long_optind, i, encs = 0;
	COLOR **ecs = NULL;
	char ec_d[] = "000", cs_d[] = "000/fff";
	char *opts = "w:c:d:i:r:s:j:ne:p:h";
	struct option long_opts[] = {
		{"width", required_argument, NULL, 'w'},
		{"center", required_argument, NULL, 'c'},
		{"dim", required_argument, NULL, 'd'},
		{"iterations", required_argument, NULL, 'i'},
		{"radius", required_argument, NULL, 'r'},
		{"set", required_argument, NULL, 's'},
		{"julia", required_argument, NULL, 'j'},
		{"normalize", no_argument, NULL, 'n'},
		{"color", required_argument, NULL, 'e'},
		{"palette", required_argument, NULL, 'p'},
		{"help", no_argument, NULL, 'h'},
		{NULL, no_argument, NULL, 0}
	};
	MANDELBROT man = {
		stdout, // fp
		640, // pw
		0.0, 0.0, // cx, cy
		4.0, 4.0, // w, h
		128, // imax
		2.0, // r
		SET_MANDELBROT, // set
		FALSE, 0.0, 0.0, // j, jr, ji
		NULL, NULL, 0, // ec, cs, ncs
		0 // nic
	};
	opterr = 0;
	while ((opt = getopt_long(argc, argv, opts, long_opts, &long_optind)) != -1) {
		switch (opt) {
		case 'w':
			sscanf(optarg, "%d", &man.pw);
			break;
		case 'c':
			sscanf(optarg, "%Lf/%Lf", &man.cx, &man.cy);
			break;
		case 'd':
			sscanf(optarg, "%Lf/%Lf", &man.w, &man.h);
			break;
		case 'i':
			sscanf(optarg, "%d", &man.imax);
			break;
		case 'r':
			sscanf(optarg, "%Lf", &man.r);
			break;
		case 's':
			sscanf(optarg, "%d", &man.set);
			break;
		case 'j':
			man.j = TRUE;
			sscanf(optarg, "%Lf/%Lf", &man.jr, &man.ji);
			break;
		case 'e':
			ecs = get_colors(optarg, ecs, &encs);
			man.ec = encs > 0 ? ecs[0] : NULL;
			for (i = 1; i < encs; i++)
				nfree(ecs[i]);
			nfree(ecs);
			break;
		case 'p':
			man.cs = get_colors(optarg, man.cs, &man.ncs);
			break;
		case 'n':
			man.nic = TRUE;
			break;
		case 'h':
			printf(HELP_TEXT);
			exit(EXIT_SUCCESS);
		case '?':
		default:
			eprintf(PROGRAM_NAME ": ");
			if (strchr("wcdirsjp", optopt) != NULL)
				eprintf("option '-%c' requires an argument\n", optopt);
			else
				eprintf("invalid option -- %c\n", optopt);
			eprintf(USAGE_TEXT);
			exit(EXIT_FAILURE);
		}
	}
	if (optind == argc - 1 && strcmp(argv[optind], "-")) {
		man.fp = fopen(argv[optind], "w");
		if (man.fp == NULL) {
			eprintf(PROGRAM_NAME ": %s: %s\n", argv[optind], strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	else if (optind > argc) {
		eprintf(PROGRAM_NAME ": too many operands or wrong operand order\n");
		eprintf(USAGE_TEXT);
		exit(EXIT_FAILURE);
	}
	if (man.ncs == 1) {
		nfree(man.cs[0]);
		nfree(man.cs);
		man.ncs = 0;
	}
	if (man.ncs == 0) {
		man.cs = get_colors(cs_d, man.cs, &man.ncs);
	}
	if (man.ec == NULL) {
		ecs = get_colors(ec_d, ecs, &encs);
		man.ec = ecs[0];
		nfree(ecs);
	}
	mandelbrot(&man);
	for (i = 0; i < man.ncs; i++) {
		nfree(man.cs[i]);
	}
	nfree(man.cs);
	nfree(man.ec);
	fclose(man.fp);
	return EXIT_SUCCESS;
}
