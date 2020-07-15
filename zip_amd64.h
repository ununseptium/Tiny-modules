#ifndef _ZIP_AMD64_H
#define _ZIP_AMD64_H

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <zip_sys.h>

#define is_x32 (UINTMAX_MAX==0xFFFFFFFF)
#define FILEOS void

#ifndef _UINT64_AMD64_DEFINED
#define _UINT64_AMD64_DEFINED
typedef struct _uint64_amd64{
	uint32_t low_half;
	uint32_t high_half;
} uint64_amd64_t;
#endif

#ifndef _ZIP64_EXTRA_FIELD_DEFINED
#define _ZIP64_EXTRA_FIELD_DEFINED
struct Zip64ExtraField {
	uint64_amd64_t *uncompressedSize;
	uint64_amd64_t *compressedSize;
	uint64_amd64_t *correspondingHeaderOffset;
};
#endif

void uint64_amd64_plus_uint64_amd64(uint64_amd64_t *a, uint64_amd64_t b);

void uint64_amd64_minus_uint64_amd64(uint64_amd64_t *a, uint64_amd64_t b);

void uint64_amd64_plus_uint32(uint64_amd64_t *uint64, uint32_t uint32);

void uint64_amd64_minus_uint32(uint64_amd64_t *uint64, uint32_t uint32);

uint32_t zip_amd64_fseek(FILEOS *stream, uint64_amd64_t offset, uint32_t whence);

uint32_t zip_amd64_get_file_size(FILEOS *stream, uint64_amd64_t *size);

uint32_t zip_amd64_f2f_data_transfer(
		FILEOS *stream_out, uint64_amd64_t offset_out,
		FILEOS *stream_in, uint64_amd64_t offset_in,
		uint64_amd64_t data_size, uint32_t *crc32
);

uint32_t uint64_amd64_eq_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b);

uint32_t uint64_amd64_gt_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b);

uint32_t uint64_amd64_ge_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b);

uint32_t uint64_amd64_lt_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b);

uint32_t uint64_amd64_le_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b);

uint32_t uint64_amd64_eq_uint32(uint64_amd64_t a, uint32_t b);

uint32_t uint64_amd64_gt_uint32(uint64_amd64_t a, uint32_t b);

uint32_t uint64_amd64_ge_uint32(uint64_amd64_t a, uint32_t b);

uint32_t uint64_amd64_lt_uint32(uint64_amd64_t a, uint32_t b);

uint32_t uint64_amd64_le_uint32(uint64_amd64_t a, uint32_t b);

void uint64_amd64_assign_uint64_amd64(uint64_amd64_t *a, uint64_amd64_t b);

void uint64_amd64_assign_uint32(uint64_amd64_t *a, uint32_t b);

#endif