#ifndef TINYNETPBM_H
#define TINYNETPBM_H

#include <stdlib.h>
#include "helpers.h"

/* Magic numbers at the start of NetPBM files. */
#define PPM_MAGIC "P6"
#define PGM_MAGIC "P5"

/* A single pixel of a PPM image. */
struct ppm_pixel {
	u16 r;
	u16 g;
	u16 b;
};

/* A PPM image. */
struct ppm_image {
	size_t w;
	size_t h;
	u16 maxval;
	struct ppm_pixel *pixels;
};

/* A PGM image. */
struct pgm_image {
	size_t w;
	size_t h;
	u16 maxval;
	u16 *pixels;
};


/****************************/
/*      PPM FUNCTIONS       */
/****************************/

/* Loads a PPM file into memory. */
struct ppm_image *ppm_read(const char *fpath);

/* Writes a PPM image into a file. Returns 0 on success. */
int ppm_write(const struct ppm_image *img, const char *fpath);

/* Apply a map function to each pixel in a PPM image. */
void ppm_map(struct ppm_image *img, void (*map)(struct ppm_pixel*));

/* Free a PPM image from memory. */
void ppm_free(struct ppm_image *img);


/****************************/
/*      PGM FUNCTIONS       */
/****************************/

/* Loads a PGM file into memory. */
struct pgm_image *pgm_read(const char *fpath);

/* Creates a PGM image from a PPM image.
 *
 * `ppm` must be convertible to PGM, i.e. all its pixels must have equal
 * rgb values. If that is not the case, this function will fail with `NULL`. */
struct pgm_image *pgm_from_ppm(const struct ppm_image *ppm);

/* Writes a PGM image into a file. Returns 0 on success. */
int pgm_write(const struct pgm_image *img, const char *fpath);

/* Computes histogram of a PGM image. */
u64 *pgm_compute_histogram(const struct pgm_image *img);

/* Performs histogram equalization of a PGM image. */
void pgm_histogram_equalize(struct pgm_image *img);

/* Applies gamma correction to a PGM image. */
void pgm_gamma_correction(struct pgm_image *img, double gamma);

/* Applies a convolution of a kernel over a PGM image. Returns 0 on success.
 *
 * `kernel` is a pointer to a 1d array containing `kernel_h` rows of `kernel_w`
 * numbers. `kernel_h` and `kernel_w` must be non-0, odd numbers.
 *
 * Examples:
 *     1 2 3
 *     4 5 6  =>  `pgm_apply_kernel(img, {1, 2, 3, 4, 5, 6, 7, 8, 9}, 3, 3)`
 *     7 8 9
 *
 *     1 2
 *     3 4    =>   `pgm_apply_kernel(img, {1, 2, 3, 4, 5, 6}, 2, 3)`
 *     5 6
 */
int pgm_convolve(struct pgm_image *img, const double *kernel, size_t kernel_w, size_t kernel_h);

/* Applies Otsu's threshold detection on a PGM image. */
void pgm_otsu_threshold(struct pgm_image *img);

/* Free a PGM image from memory. */
void pgm_free(struct pgm_image *img);

/****************************/
/*     PIXEL FUNCTIONS      */
/****************************/

/* Converts a PPM pixel to grayscale using a simple average. */
void ppm_pixel_to_gray_avg(struct ppm_pixel *pix);

/* Converts a PPM pixel to grayscale using a weighted average. */
void ppm_pixel_to_gray_weighted(struct ppm_pixel *pix);

#endif /* TINYNETPBM_H */
