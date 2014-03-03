NAME
        mandelbrot - generates an image of the Mandelbrot set or certain
                     related fractals

SYNOPSIS
        mandelbrot [OPTION]... [FILE]

DESCRIPTION
        Generates a PPM image of the Mandelbrot set or certain related fractals.

        -w, --width=WIDTH
                image width in pixels (default 640)

        -c, --center=X/Y
                center coordinates (default 0/0)

        -d, --dim=W/H
                region dimensions (default 4/4)

        -i, --iterations=IMAX
                maximum number of iterations (default 128)

        -r, --radius=RADIUS
                escape radius (default 2)

        -s, --set=SET
                select the fractal to generate (default 0)
                0   Mandelbrot
                1   Mandelbar
                2   Burning ship
                3+  Multibrot

        -j, --julia=JR/JI
                generate a Julia set for the chosen fractal

        -n, --normalize
                use normalized iteration count (smooth gradients, but slower)

        -e, --color=RRGGBB/RGB
                color for unescaped points in the set (default 000)

        -p, --palette=RRGGBB/RGB/...
                "keyframe" colors for a palette (default 000/FFF)

        -h, --help
                display this help and exit

        With no FILE, or when FILE is -, write to standard output.

AUTHOR
        Written by Remy Oukaour <remy.oukaour@gmail.com>.

COPYRIGHT
        Copyright (C) 2011-2014 Remy Oukaour <http://www.remyoukaour.com>.
        MIT License.
        This is free software: you are free to change and redistribute it.
        There is NO WARRANTY, to the extent permitted by law.
