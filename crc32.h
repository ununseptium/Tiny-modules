
#ifndef CRC32_H
#define CRC32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint-gcc.h>
#include <limits.h>
#include <errno.h>

#ifndef BYTE_VALUES_COUNT
#define BYTE_VALUES_COUNT 256
#endif

#ifndef BIT_IN_BYTE
#define BIT_IN_BYTE 8
#endif

typedef uint32_t CRC32_CACHE[BYTE_VALUES_COUNT];


typedef struct {
    uint32_t poly;
    uint32_t init;
    uint32_t xorout;
    uint8_t refin;
    uint8_t refout;
} crc32_config;

typedef struct {
	uint32_t temp_crc32;
	size_t whole_data_size;
} interim_crc_t;

static void swap(void *a, void *b, size_t size);

static void bit_reverse_order(void *arr, size_t size);

interim_crc_t* crc32_lazy(interim_crc_t *interim_crc, const uint8_t *data, size_t size, crc32_config crc32_conf);

uint32_t crc32_lazy_execute(interim_crc_t *interim_crc, crc32_config crc32_conf);

uint32_t crc32(const uint8_t *data, size_t size, crc32_config crc32_conf);

static void crc32_cache(crc32_config config, char *cache_file_name);

void crc32_load_cache(crc32_config conf, char *file_name, CRC32_CACHE cached_xors);

uint32_t crc32_optimized(const uint8_t *data, size_t size, crc32_config conf, CRC32_CACHE cache);



#ifdef __cplusplus
}
#endif

#endif /* CRC32_H */