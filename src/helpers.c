#include "helpers.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void *malloc_or_die(size_t nbytes)
{
	void *const buf = malloc(nbytes);
	if (buf == NULL) {
		INFO("failed to allocate memory: %s", strerror(errno));
		exit(1);
	}
	return buf;
}

bool host_is_little_endian(void)
{
	volatile uint_fast16_t x = 1;
	return *(char*)&x;
}

u16 u16_to_other_endian(u16 num)
{
	return ((num & 0x00ff) << 8) | ((num & 0xff00) >> 8);
}
