#ifndef _ZIP_SAFE_H
#define _ZIP_SAFE_H

#include <zip_sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _LOCAL_FILE_HEADER_DIFENED
#define _LOCAL_FILE_HEADER_DIFENED
struct LocalFileHeader{
	uint32_t signature;
	uint16_t versionToExtract;
	uint16_t generalPurposeBitFlag;
	uint16_t compressionMethod;
	uint16_t modificationTime;
	uint16_t modificationDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t filenameLength;
	uint16_t extraFieldLength;
} __attribute__((packed));
#endif

#ifndef _ZIP64_EXTRA_FIELD_DEFINED
#define _ZIP64_EXTRA_FIELD_DEFINED
struct zip64_extra_field {
	uintmax_t *uncompressedSize;
	uintmax_t *compressedSize;
	uintmax_t *correspondingHeaderOffset;
};
#endif

#define FILEOS void

#define MAX_STREAMS 32

static uint32_t zip_safe_replace_stream(FILEOS* str1, FILEOS* str2);

static void zip_safe_collect_garbage();

void zip_safe_add_file_to_delete(char* file_path);

uint16_t zip_safe_get_modification_time(fileinfo_t fi);

uint16_t zip_safe_get_modification_date(fileinfo_t fi);

const char* zip_safe_get_relative_filename(fileinfo_t fi);

const char* zip_safe_get_absolute_filename(fileinfo_t fi);

void* zip_safe_get_extra_data_lfh(uint16_t* extra_data_size, fileinfo_t fi, uintmax_t uncompressed_size, uintmax_t compressed_size);

void* zip_safe_get_extra_data_cfh(
		uint16_t* extra_data_size, fileinfo_t fi, uintmax_t lfd_offset,
		struct LocalFileHeader lfh, void *extra_data_lfh
);

void* zip_safe_get_pre_eocd_data(uint16_t *pre_data_size, uintmax_t cdfh_offset, uintmax_t cdfh_total, uintmax_t cdfh_size);

uint32_t zip_safe_process_zip64(
		const uint8_t *extra_data, uint16_t extra_data_size, struct zip64_extra_field zip64_field
);

uint16_t zip_safe_get_internal_attrs(fileinfo_t fi);

uint32_t zip_safe_get_external_attrs(fileinfo_t fi);

uint32_t zip_safe_is_folder(const char* filename);

uint32_t zip_safe_is_file_exist(const char* filename);

void* zip_safe_collect_pathtree_info(const char* path);

fileinfo_t zip_safe_process_first_file(void* pathtree_info);

uint32_t zip_safe_process_next_file(void* pathtree_info, fileinfo_t fi);

void zip_safe_reset_pathtree_info_pos(void* pathtree_info, fileinfo_t fi);

void zip_safe_close_process(void* pathtree_info, fileinfo_t fi);

uint16_t zip_safe_get_os_version(fileinfo_t fi);

uint32_t zip_safe_create_dir(const char *dirname);

FILEOS* zip_safe_fopen(const char *filename, const char *mode);

void zip_safe_fclose(FILEOS *stream);

void zip_safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILEOS *stream);

void zip_safe_fread(void *ptr, size_t size, size_t nmemb, FILEOS *stream);

void zip_safe_fgetpos(FILEOS *stream, zip_fpos_t *pos);

void zip_safe_fsetpos(FILEOS *stream, zip_fpos_t *pos);

void zip_safe_fseek(FILEOS *stream, intmax_t offset, uint32_t whence);

void zip_safe_get_file_size(FILEOS *stream, uintmax_t *size);

void* zip_safe_create_mmf(FILEOS *stream, uintmax_t size, uint32_t access_mode);

void* zip_safe_map_view(void *mmf, uint32_t access_mode, uintmax_t offset, uintmax_t view_size, void* file_offset);

void zip_safe_unmap_view(void *map_view, uintmax_t view_size);

void zip_safe_close_mmf(void *mmf);

void zip_safe_f2f_data_transfer(
		FILEOS *stream_out, uintmax_t offset_out,
		FILEOS *stream_in, uintmax_t offset_in,
		uintmax_t data_size, uint32_t *crc32
);

#endif
