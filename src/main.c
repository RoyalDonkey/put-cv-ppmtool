#include "tinynetpbm.h"
#include "helpers.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 3) {
		INFO("exactly 2 arguments expected (input, output)%s", "");
		return 1;
	}

	const char *const fpath_in = argv[1];
	const char *const fpath_out = argv[2];

	/* Read PPM file */
	struct ppm_image *const ppm = ppm_read(fpath_in);
	if (ppm == NULL) {
		return 2;
	}

	/* Convert to grayscale (comment/uncomment and rebuild to change) */
	ppm_map(ppm, ppm_pixel_to_gray_avg);
	/* ppm_map(ppm, ppm_pixel_to_gray_weighted); */

	/* Convert to PGM file */
	struct pgm_image *const pgm = pgm_from_ppm(ppm);

	/* Equalize histogram */
	pgm_histogram_equalize(pgm);

	/* Write the result */
	if (pgm_write(pgm, fpath_out)) {
		return 3;
	}

	/* Cleanup */
	ppm_free(ppm);
	pgm_free(pgm);

	return 0;
}
