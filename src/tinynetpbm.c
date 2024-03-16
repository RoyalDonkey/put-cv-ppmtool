#include "tinynetpbm.h"
#include "helpers.h"
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <math.h>

struct ppm_image *ppm_read(const char *fpath)
{
	assert(fpath != NULL);

	FILE *f = fopen(fpath, "rb");
	if (f == NULL) {
		INFO("failed to open '%s': %s", fpath, strerror(errno));
		return NULL;
	}

	struct ppm_image *const img = malloc_or_die(sizeof(struct ppm_image));
	if (3 != fscanf(f, "P6 %"PRIuPTR" %"PRIuPTR" %"PRIu16, &img->w, &img->h, &img->maxval)) {
		INFO("'%s' is not a valid PPM file", fpath);
		return NULL;
	}

	if (img->maxval == 0) {
		INFO("'%s' is not a valid PPM file: maxval cannot be 0", fpath);
		return NULL;
	}

	if (img->h == 0 || img->w == 0) {
		/* Image is empty, but valid */
		img->pixels = NULL;
		return img;
	}

	/* Safely compute product of width, height and sizeof(struct ppm_pixel) */
	const size_t n_pixels = img->h * img->w;
	if (n_pixels / img->h != img->w) {
		ERROR_L("'%s' is too large", fpath);
		return NULL;
	}
	const size_t n_pixel_bytes = n_pixels * sizeof(struct ppm_pixel);
	if (n_pixel_bytes / sizeof(struct ppm_pixel) != n_pixels) {
		ERROR_L("'%s' is too large", fpath);
		return NULL;
	}

	img->pixels = malloc_or_die(n_pixel_bytes);

	if (!isspace(fgetc(f))) {
		INFO("'%s' is not a valid PPM file", fpath);
		return NULL;
	}

	if (img->maxval < 256) {
		for (size_t y = 0; y < img->h; y++) {
			for (size_t x = 0; x < img->w; x++) {
				struct ppm_pixel *const dest = &img->pixels[y * img->w + x];
				u8 r, g, b;
				fread(&r, 1, 1, f);
				fread(&g, 1, 1, f);
				fread(&b, 1, 1, f);
				dest->r = r; /* Let C handle u8->u16 conversion */
				dest->g = g;
				dest->b = b;
			}
		}
	} else {
		for (size_t y = 0; y < img->h; y++) {
			for (size_t x = 0; x < img->w; x++) {
				struct ppm_pixel *const dest = &img->pixels[y * img->w + x];
				fread(&dest->r, 2, 1, f);
				fread(&dest->g, 2, 1, f);
				fread(&dest->b, 2, 1, f);
				if (host_is_little_endian()) {
					dest->r = u16_to_other_endian(dest->r);
					dest->g = u16_to_other_endian(dest->g);
					dest->b = u16_to_other_endian(dest->b);
				}
			}
		}
	}

	fclose(f);

	printf("read a PPM file: %"PRIuPTR"x%"PRIuPTR", maxval %"PRIu16"\n", img->w, img->h, img->maxval);
	return img;
}

int ppm_write(const struct ppm_image *img, const char *fpath)
{
	assert(img != NULL);
	assert(fpath != NULL);

	FILE *f = fopen(fpath, "wb");
	if (f == NULL) {
		INFO("failed to open '%s' for writing: %s", fpath, strerror(errno));
		return 2;
	}
	if (0 > fprintf(f, PPM_MAGIC"\n%"PRIuPTR" %"PRIuPTR"\n%"PRIu16"\n", img->w, img->h, img->maxval)) {
		INFO("failed to write to '%s': %s", fpath, strerror(errno));
		return 1;
	}

	if (img->maxval < 256) {
		for (size_t y = 0; y < img->h; y++) {
			for (size_t x = 0; x < img->w; x++) {
				struct ppm_pixel *const pix = &img->pixels[y * img->w + x];
				u8 src = pix->r; /* Let C handle u16->u8 conversion */
				fwrite(&src, 1, 1, f);
				src = pix->g;
				fwrite(&src, 1, 1, f);
				src = pix->b;
				fwrite(&src, 1, 1, f);
			}
		}
	} else {
		for (size_t y = 0; y < img->h; y++) {
			for (size_t x = 0; x < img->w; x++) {
				struct ppm_pixel *const pix = &img->pixels[y * img->w + x];
				u16 src = host_is_little_endian() ? u16_to_other_endian(pix->r) : pix->r;
				fwrite(&src, 2, 1, f);
				src = host_is_little_endian() ? u16_to_other_endian(pix->g) : pix->g;
				fwrite(&src, 2, 1, f);
				src = host_is_little_endian() ? u16_to_other_endian(pix->b) : pix->b;
				fwrite(&src, 2, 1, f);
			}
		}
	}

	fclose(f);
	return 0;
}

void ppm_map(struct ppm_image *img, void (*map)(struct ppm_pixel*))
{
	assert(img != NULL);
	assert(map != NULL);

	for (size_t y = 0; y < img->h; y++) {
		for (size_t x = 0; x < img->w; x++) {
			map(&img->pixels[y * img->w + x]);
		}
	}
}

void ppm_free(struct ppm_image *img)
{
	assert(img != NULL);
	if (img->pixels != NULL) {
		free(img->pixels);
	}
	free(img);
}

struct pgm_image *pgm_read(const char *fpath)
{
	assert(fpath != NULL);

	FILE *f = fopen(fpath, "rb");
	if (f == NULL) {
		INFO("failed to open '%s': %s", fpath, strerror(errno));
		return NULL;
	}

	struct pgm_image *const img = malloc_or_die(sizeof(struct pgm_image));
	if (3 != fscanf(f, "P6 %"PRIuPTR" %"PRIuPTR" %"PRIu16, &img->w, &img->h, &img->maxval)) {
		INFO("'%s' is not a valid PGM file", fpath);
		return NULL;
	}

	if (img->maxval == 0) {
		INFO("'%s' is not a valid PGM file: maxval cannot be 0", fpath);
		return NULL;
	}

	if (img->h == 0 || img->w == 0) {
		/* Image is empty, but valid */
		img->pixels = NULL;
		return img;
	}

	/* Safely compute product of width, height and sizeof(struct pgm_pixel) */
	const size_t n_pixels = img->h * img->w;
	if (n_pixels / img->h != img->w) {
		ERROR_L("'%s' is too large", fpath);
		return NULL;
	}
	const size_t n_pixel_bytes = n_pixels * sizeof(u16);
	if (n_pixel_bytes / sizeof(u16) != n_pixels) {
		ERROR_L("'%s' is too large", fpath);
		return NULL;
	}

	img->pixels = malloc_or_die(n_pixel_bytes);

	if (!isspace(fgetc(f))) {
		INFO("'%s' is not a valid PGM file", fpath);
		return NULL;
	}

	if (img->maxval < 256) {
		for (size_t y = 0; y < img->h; y++) {
			for (size_t x = 0; x < img->w; x++) {
				u16 *const dest = &img->pixels[y * img->w + x];
				u8 v;
				fread(&v, 1, 1, f);
				*dest = v; /* Let C handle u8->u16 conversion */
			}
		}
	} else {
		for (size_t y = 0; y < img->h; y++) {
			for (size_t x = 0; x < img->w; x++) {
				u16 *const dest = &img->pixels[y * img->w + x];
				fread(dest, 2, 1, f);
				if (host_is_little_endian()) {
					*dest = u16_to_other_endian(*dest);
				}
			}
		}
	}

	fclose(f);

	printf("read a PGM file: %"PRIuPTR"x%"PRIuPTR", maxval %"PRIu16"\n", img->w, img->h, img->maxval);
	return img;
}

struct pgm_image *pgm_from_ppm(const struct ppm_image *ppm)
{
	assert(ppm != NULL);
	struct pgm_image *const img = malloc_or_die(sizeof(struct pgm_image));
	img->pixels = malloc_or_die(ppm->w * ppm->h * sizeof(u16));
	for (size_t y = 0; y < ppm->h; y++) {
		for (size_t x = 0; x < ppm->w; x++) {
			const struct ppm_pixel *const pix = &ppm->pixels[y * ppm->w + x];
			if (pix->r != pix->g || pix->g != pix->b || pix->r != pix->b) {
				ERROR_L("cannot convert PPM to PGM: rgb values not equal%s", "");
				return NULL;
			}
			img->pixels[y * ppm->w + x] = pix->r;
		}
	}
	img->w = ppm->w;
	img->h = ppm->h;
	img->maxval = ppm->maxval;
	return img;
}

int pgm_write(const struct pgm_image *img, const char *fpath)
{
	assert(img != NULL);
	assert(fpath != NULL);

	FILE *f = fopen(fpath, "wb");
	if (f == NULL) {
		INFO("failed to open '%s' for writing: %s", fpath, strerror(errno));
		return 2;
	}
	if (0 > fprintf(f, PGM_MAGIC"\n%"PRIuPTR" %"PRIuPTR"\n%"PRIu16"\n", img->w, img->h, img->maxval)) {
		INFO("failed to write to '%s': %s", fpath, strerror(errno));
		return 1;
	}

	if (img->maxval < 256) {
		for (size_t y = 0; y < img->h; y++) {
			for (size_t x = 0; x < img->w; x++) {
				u16 *const pix = &img->pixels[y * img->w + x];
				u8 src = (u8)(*pix); /* Let C handle u16->u8 conversion */
				fwrite(&src, 1, 1, f);
			}
		}
	} else {
		for (size_t y = 0; y < img->h; y++) {
			for (size_t x = 0; x < img->w; x++) {
				u16 *const pix = &img->pixels[y * img->w + x];
				u16 src = host_is_little_endian() ? u16_to_other_endian(*pix) : *pix;
				fwrite(&src, 2, 1, f);
			}
		}
	}

	fclose(f);
	return 0;
}

void pgm_histogram_equalize(struct pgm_image *img)
{
	assert(img != NULL);

	/* Allocate needed memory */
	u64 *hist = calloc_or_die((img->maxval + 1) * sizeof(u64));
	u16 *new_levels = malloc_or_die((img->maxval + 1) * sizeof(u16));

	/* Count no. pixels of each intensity (histogram) */
	for (size_t y = 0; y < img->h; y++) {
		for (size_t x = 0; x < img->w; x++) {
			const u16 pix = img->pixels[y * img->w + x];
			assert(hist[pix] != INT64_MAX);
			hist[pix]++;
		}
	}

	/* Find minimal intensity value present in img */
	u32 min_pix_value = UINT32_MAX; /* Sentinel value higher than img->maxval */
	for (size_t val = 0; val <= img->maxval; val++) {
		if (hist[val] > 0) {
			min_pix_value = val;
			break;
		}
	}
	if (min_pix_value > img->maxval) {
		ERROR_L("image empty or something went horribly wrong%s", "");
		return;
	}

	/* Convert to cumulative histogram */
	for (size_t val = 1; val <= img->maxval; val++) {
		hist[val] += hist[val - 1];
	}

	/* Number of occurrences of the minimal intensity value */
	const u16 h_min = hist[min_pix_value];

	/* Compute new levels (maps old intensity levels to new ones) */
	for (size_t val = 0; val <= img->maxval; val++) {
		new_levels[val] = 0.5 /* Rounding term */ + (
			(double)(hist[val] - h_min)
			/ (img->w * img->h - h_min)
			* img->maxval
		);
	}
	free(hist);

	/* Apply new levels */
	for (size_t y = 0; y < img->h; y++) {
		for (size_t x = 0; x < img->w; x++) {
			u16 *const pix = &img->pixels[y * img->w + x];
			*pix = new_levels[*pix];
		}
	}
	free(new_levels);
}

void pgm_gamma_correction(struct pgm_image *img, double gamma)
{
	assert(img != NULL);
	assert(fabs(gamma) >= MATH_EPS);

	for (size_t y = 0; y < img->h; y++) {
		for (size_t x = 0; x < img->w; x++) {
			u16 *const pix = &img->pixels[y * img->w + x];
			*pix = CLAMP(pow(*pix, 1. / gamma), 0, img->maxval);
		}
	}
}

void pgm_free(struct pgm_image *img)
{
	assert(img != NULL);
	if (img->pixels != NULL) {
		free(img->pixels);
	}
	free(img);
}

void ppm_pixel_to_gray_avg(struct ppm_pixel *pix)
{
	assert(pix != NULL);
	u16 avg = 0.5 + ((pix->r + pix->g + pix->b) / 3.0);
	pix->r = avg;
	pix->g = avg;
	pix->b = avg;
}

void ppm_pixel_to_gray_weighted(struct ppm_pixel *pix)
{
	assert(pix != NULL);
	u16 avg = 0.5 + (0.299 * pix->r + 0.587 * pix->g + 0.114 * pix->b);
	pix->r = avg;
	pix->g = avg;
	pix->b = avg;
}
