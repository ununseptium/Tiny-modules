#include "crc32.h"

size_t uint8_s = sizeof(uint8_t);
size_t uint32_s = sizeof(uint32_t);

static void swap(void *a, void *b, size_t size) {
    for (size_t byteindex = 0; byteindex < size; byteindex++) {
        uint8_t curbyte = ((uint8_t *) a)[byteindex];
        ((uint8_t *) a)[byteindex] = ((uint8_t *) b)[byteindex];
        ((uint8_t *) b)[byteindex] = curbyte;
    }
}

static void bit_reverse_order(void *arr, size_t size) {
    if (size == 1) {
        uint8_t byte = 0;
        for (int counter = 0; counter < BIT_IN_BYTE; counter++) {
            byte = byte << 1;
            if (*(uint8_t *) arr % 2 == 1) byte++;
            *(uint8_t *) arr = (*(uint8_t *) arr) >> 1;
        }
        *(uint8_t *) arr = byte;
    } else {
        for (size_t start_byte_index = 0, end_byte_index = size - 1;
             start_byte_index < (size_t) floor(size / 2.0);
             start_byte_index++, end_byte_index--) {
            swap(((uint8_t *) arr) + start_byte_index, ((uint8_t *) arr) + end_byte_index, uint8_s);
            bit_reverse_order((uint8_t *) arr + end_byte_index, uint8_s);
            bit_reverse_order((uint8_t *) arr + start_byte_index, uint8_s);
        }
    }
}

uint32_t crc32(const uint8_t *data, size_t size, crc32_config crc32_conf) {
    uint8_t *pinit = (uint8_t *) &crc32_conf.init;
    uint32_t crc_value = 0;
    for (size_t byte_index = 0; byte_index < size + uint32_s; byte_index++) {

        uint8_t dbyte = 0;
        if (byte_index < size) {
            dbyte = data[byte_index];
            if (crc32_conf.refin != 0) bit_reverse_order(&dbyte, uint8_s);
        }

        if (byte_index < uint32_s) dbyte = dbyte ^ pinit[byte_index];

        for (uint8_t counter = 0; counter < BIT_IN_BYTE; counter++) {
            if (crc_value & 0x80000000) crc_value = ((crc_value << 1) | ((dbyte >> 7) & 1)) ^ crc32_conf.poly;
            else crc_value = (crc_value << 1) | ((dbyte >> 7) & 1);
            dbyte = dbyte << 1;
        }
    }

    if (crc32_conf.refout) bit_reverse_order(&crc_value, uint32_s);
    crc_value = crc_value ^ crc32_conf.xorout;

    return crc_value;
}


static void crc32_cache(crc32_config config, char *cache_file_name) {
    errno = 0;
    FILE *file_cache = fopen(cache_file_name, "wb");
    if (errno != 0) {
        fclose(file_cache);
        return;
    }

    crc32_config cache_config = {config.poly, 0, 0, 0, 0};

    uint8_t byte = 0;
    if (config.refin != 0) {
        for (int counter = 0; counter <= UCHAR_MAX; counter++, byte++) {
            uint8_t rbyte = byte;
            bit_reverse_order(&rbyte, uint8_s);

            uint32_t crc_value = crc32(&rbyte, uint8_s, cache_config);
            uint8_t *pcrc_value = (uint8_t *) &crc_value;

            for (int offset = 0; offset < uint32_s; offset++) {
                bit_reverse_order(pcrc_value + offset, uint8_s);
            }

            if (fwrite(&crc_value, uint32_s, 1, file_cache) != 1) {
                fclose(file_cache);
                remove(cache_file_name);
                errno = EIO;
                return;
            }
        }
    } else {
        for (int counter = 0; counter <= UCHAR_MAX; counter++, byte++) {
            uint32_t crc_value = crc32(&byte, uint8_s, cache_config);

            if (fwrite(&crc_value, uint32_s, 1, file_cache) != 1) {
                fclose(file_cache);
                remove(cache_file_name);
                errno = EIO;
                return;
            }
        }
    }

    fclose(file_cache);
}

void crc32_load_cache(crc32_config conf, char *file_name, CRC32_CACHE cached_xors) {
    errno = 0;
    FILE *cache_file = fopen(file_name, "rb");
    if (errno == ENOENT) {
        errno = 0;
        crc32_cache(conf, file_name);
        if (errno != 0) return;
    } else if (errno != 0) return;

    cache_file = fopen(file_name, "rb");
    if (errno != 0) return;

    for (int counter = 0; counter < BYTE_VALUES_COUNT; counter++) {
        fread(cached_xors + counter, uint32_s, 1, cache_file);
        if (feof(cache_file)) {
            fclose(cache_file);
            errno = EIO;
            return;
        }
    }

    fclose(cache_file);
}

uint32_t crc32_optimized(const uint8_t *data, size_t size, crc32_config conf, CRC32_CACHE cache) {
    uint8_t *pinit = (uint8_t *) &conf.init;
    uint32_t crc_value = 0;
    for (size_t byte_index = 0; byte_index < size + uint32_s; byte_index++) {
        uint8_t curbyte = 0;
        if (byte_index < size) curbyte = data[byte_index];
        if (byte_index < uint32_s) curbyte = curbyte ^ pinit[byte_index];

        uint8_t highbyte = crc_value >> (uint8_s * BIT_IN_BYTE * 3) & UCHAR_MAX;
        crc_value = (crc_value << (uint8_s * BIT_IN_BYTE)) | curbyte;
        crc_value = crc_value ^ cache[highbyte];
    }

    uint8_t *pcrc_value = (uint8_t *) (&crc_value);

    if (conf.refin != 0)
        for (int offset = 0; offset < uint32_s; offset++) {
            bit_reverse_order(pcrc_value + offset, uint8_s);
        }

    crc_value = crc_value ^ conf.xorout;
    if (conf.refout != 0) bit_reverse_order(pcrc_value, uint32_s);

    return crc_value;
}