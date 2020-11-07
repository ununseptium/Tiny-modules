#ifndef _ZIP_SYS_H
#define _ZIP_SYS_H

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <crc32.h>
#include <zip_amd64.h>
#include <zip_bo.h>

#ifdef __WIN32__
#include <windows.h>
#define zip_fpos_t uint64_amd64_t
#endif

#define WINDOWS_OS_VER 10
#define NTFS_SIZE 36
#define WINDOWS_OS_DATA_SIZE (sizeof(FILETIME)*3)
#define PATHTREE_TMP_FILE tmp/pathtree.tmp
#define FILEOS void

#define ZIP_MMF_READ_ONLY 1
#define ZIP_MMF_READ_WRITE 2

#define ZIP_MMF_VIEW_READ 1
#define ZIP_MMF_VIEW_WRITE 2

#ifndef _FILEDATA_DEFINED
#define _FILEDATA_DEFINED
typedef struct _filedata{
	uint16_t os_ver;
	uint16_t modification_time;
	uint16_t modification_date;
	uint16_t internal_attrs;
	uint32_t external_attrs;
	char *absolute_filename;
	char *relative_filename;
	// os dependent data
	void* os_data;
} filedata_t;
#endif

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
}__attribute__((packed));
#endif

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

#ifndef _ZIP64_END_OF_CENTRAL_DIRECTORY_DEFINED
#define _ZIP64_END_OF_CENTRAL_DIRECTORY_DEFINED
struct Zip64EndOfCentralDirectory {
    uint32_t signature;
    uint64_amd64_t sizeOfZip64EndOfCentralDirectory;
    uint16_t versionMadeBy;
    uint16_t versionToExtract;
    uint32_t diskNumber;
    uint32_t startDiskNumber;
    uint64_amd64_t numberCentralDirectoryRecord;
    uint64_amd64_t totalCentralDirectoryRecord;
    uint64_amd64_t sizeOfCentralDirectory;
    uint64_amd64_t centralDirectoryOffset;
}__attribute__((packed));
#endif

#ifndef _ZIP64_END_OF_CENTRAL_DIRECTORY_LOCATOR_DEFINED
#define _ZIP64_END_OF_CENTRAL_DIRECTORY_LOCATOR_DEFINED
struct Zip64EndOfCentralDirectoryLocator {
    uint32_t signature;
    uint32_t diskNumber;
    uint64_amd64_t zip64EndOfCentralDirectoryOffset;
    uint32_t totalDisks;
}__attribute__((packed));
#endif

typedef void* fileinfo_t;

uint16_t zip_sys_get_os_version(fileinfo_t fi);

uint16_t zip_sys_get_modification_time(fileinfo_t fi);

uint16_t zip_sys_get_modification_date(fileinfo_t fi);

const char* zip_sys_get_relative_filename(fileinfo_t fi);

const char* zip_sys_get_absolute_filename(fileinfo_t fi);

void* zip_sys_get_extra_data_lfh(uint16_t* extra_data_size, fileinfo_t fi, uint64_amd64_t uncompressed_size, uint64_amd64_t compressed_size);

void* zip_sys_get_extra_data_cfh(
		uint16_t* extra_data_size, fileinfo_t fi, uint64_amd64_t lfd_offset,
		struct LocalFileHeader lfh, void *extra_data_lfh
);

static uint16_t zip_sys_get_zip64_extra_block(
		uint8_t *zip64_extra_block, struct Zip64ExtraField zip64_field
);

static uint16_t zip_sys_get_ntfs_extra_block(uint8_t *ntfs_extra_block, fileinfo_t fi);

void* zip_sys_get_pre_eocd_data(uint16_t *pre_data_size, uint64_amd64_t cdfh_offset, uint64_amd64_t cdfh_total, uint64_amd64_t cdfh_size);

uint32_t zip_sys_process_zip64(
		const uint8_t *extra_data, uint16_t extra_data_size, struct Zip64ExtraField zip64_field
);

uint16_t zip_sys_get_internal_attrs(fileinfo_t fi);

uint32_t zip_sys_get_external_attrs(fileinfo_t fi);

uint32_t zip_sys_is_folder(const char* filename);

uint32_t zip_sys_is_file_exist(const char* filename);

void* zip_sys_collect_pathtree_info(const char* path);

fileinfo_t zip_sys_process_first_file(void* pathtree_info);

uint32_t zip_sys_process_next_file(void* pathtree_info, fileinfo_t fi);

uint32_t zip_sys_reset_pathtree_info_pos(void* pathtree_info, fileinfo_t fi);

uint32_t zip_sys_close_process(void* pathtree_info, fileinfo_t fi);

uint32_t zip_sys_create_dir(const char *dirname);

FILEOS* zip_sys_fopen(const char *filename, const char *mode);

static uint32_t zip_sys_lookup_win(FILEOS* pathtree_file, const char* cur_filename, const char* path_to_pack);

static uint32_t zip_sys_write_filedata(FILEOS *file, filedata_t *fd, uint32_t os_data_size);

static uint32_t zip_sys_read_filedata(FILEOS *file, filedata_t *fd, uint32_t os_data_size);

static void zip_sys_free_filedata_fields(filedata_t *fd);

static void zip_sys_free_filedata(filedata_t *fd);

static char* zip_sys_cut_to_relative_filename(const char *abs_filename, const char *root, char *rel_filename);

FILEOS* zip_sys_fopen(const char *filename, const char *mode);

int32_t zip_sys_fclose(FILEOS *stream);

size_t zip_sys_fwrite(const void *ptr, size_t size, size_t nmemb, FILEOS *stream);

size_t zip_sys_fread(void *ptr, size_t size, size_t nmemb, FILEOS *stream);

uint32_t zip_sys_fgetpos(FILEOS *stream, zip_fpos_t *pos);

uint32_t zip_sys_fsetpos(FILEOS *stream, zip_fpos_t *pos);

uint32_t zip_sys_fseek(FILEOS *stream, intmax_t offset, uint32_t whence);

uint32_t zip_sys_get_file_size(FILEOS *stream, uintmax_t *size);

void* zip_sys_create_mmf(FILEOS *stream, uintmax_t size, uint32_t access_mode);

void* zip_sys_map_view(void *mmf, uint32_t access_mode, uintmax_t offset, uintmax_t view_size, void* file_offset);

uint32_t zip_sys_unmap_view(void *map_view, uintmax_t view_size);

uint32_t zip_sys_close_mmf(void *mmf);

uint32_t zip_sys_f2f_data_transfer(
		FILEOS *stream_out, uintmax_t offset_out,
		FILEOS *stream_in, uintmax_t offset_in,
		uintmax_t data_size, uint32_t *crc32
);

#endif
