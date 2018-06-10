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
#include "mandelbrot.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>

#define nfree(p) do { free(p); p = NULL; } while (0)
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

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
	"\tCopyright (C) 2011-2016 Remy Oukaour <http://www.remyoukaour.com>.\n"\
	"\tMIT License.\n"\
	"\tThis is free software: you are free to change and redistribute it.\n"\
	"\tThere is NO WARRANTY, to the extent permitted by law.\n")

FILE* fp;

void create_file_header(int pw, int ph) {
	fprintf(fp, "P6\n%d %d\n255\n", pw, ph);
}

void append_colors_to_file(int r, int g, int b) {
    fprintf(fp, "%c%c%c", r, g, b);
}

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


MAND_COLOR **get_colors(char *s, MAND_COLOR **cs, int *ncs) {
	int i, j, n;
	char *buffer, *token;
	*ncs = 0;
	for (i = 0; 1; i++, s = NULL) {
		token = strtok_r(s, "/", &buffer);
		if (token == NULL)
			break;
		j = (*ncs)++;
		cs = realloc(cs, *ncs * (sizeof (MAND_COLOR *)));
		cs[j] = malloc(sizeof (MAND_COLOR *));
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

int main(int argc, char *argv[]) {
	int opt, long_optind, i, encs = 0;
	MAND_COLOR **ecs = NULL;
	char ec_d[] = "000", cs_d[] = "000/fff";
	char *opts = "w:c:d:i:r:s:j:ne:p:h";
	struct option long_opts[] = {
		{"center", required_argument, NULL, 'c'},
		{"width", required_argument, NULL, 'w'},
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
		640, // pw
		0.0, 0.0, // cx, cy
		4.0, 4.0, // w, h
		128, // imax
		2.0, // r
		SET_MANDELBROT, // set
		MAND_FALSE, 0.0, 0.0, // j, jr, ji
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
			man.j = MAND_TRUE;
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
			man.nic = MAND_TRUE;
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
		fp = fopen(argv[optind], "w");
		if (fp == NULL) {
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
    int total;
	MAND_COLOR **palette = make_palette(man.cs, man.ncs, &total, man.ec);
	mandelbrot(&man, palette, total, create_file_header, append_colors_to_file);
	for (i = 0; i <= total; i++) {
		nfree(palette[i]);
	}
	nfree(palette);
	for (i = 0; i < man.ncs; i++) {
		nfree(man.cs[i]);
	}
	nfree(man.cs);
	nfree(man.ec);
	fclose(fp);
	return EXIT_SUCCESS;
}
