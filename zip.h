#ifndef _ZIP_H
#define _ZIP_H

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zip_amd64.h>
#include <zip_bo.h>
#include <zip_sys.h>

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

#ifndef CENTRAL_DIRECTORY_FILE_HEADER_DEFINED
#define CENTRAL_DIRECTORY_FILE_HEADER_DEFINED
struct CentralDirectoryFileHeader{
	uint32_t signature;
	uint16_t versionMadeBy;
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
	uint16_t fileCommentLength;
	uint16_t diskNumber;
	uint16_t internalFileAttributes;
	uint32_t externalFileAttributes;
	uint32_t localFileHeaderOffset;
}__attribute__((packed));
#endif

#ifndef _EOCD_DEFINED
#define _EOCD_DEFINED
struct EOCD {
	uint32_t signature;
	uint16_t diskNumber;
	uint16_t startDiskNumber;
	uint16_t numberCentralDirectoryRecord;
	uint16_t totalCentralDirectoryRecord;
	uint32_t sizeOfCentralDirectory;
	uint32_t centralDirectoryOffset;
	uint16_t commentLength;
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

#define FILEOS void

uint32_t fill_LFH(
		struct LocalFileHeader *lfh, 
		uint16_t generalPurposeBitFlag, uint16_t compressionMethod, uint32_t crc32_value,
		uintmax_t compressedSize, uintmax_t uncompressedSize,
		uint16_t filenameLength, uint16_t extraFieldLength, fileinfo_t fi
);

int write_LFH(
		FILEOS* zipf, uintmax_t cur_record_offset, fileinfo_t fi, 
		uint32_t (*compress_fnc)(FILEOS* file_in, FILEOS* file_out, uint32_t *crc32), uint16_t compress_mode, 
		uint32_t (*crypt_fnc)(FILEOS* file_in, FILEOS* file_out, uint32_t *crc32), uint16_t crypt_mode,
		uintmax_t *lfh_size
);

uint32_t zip_compress_and_encrypt_data_file(
		const char *filename_in, char *filename_out,
		uint32_t (*compress_fnc)(FILEOS* file_in, FILEOS* file_out, uint32_t *crc32),
		uint32_t (*crypt_fnc)(FILEOS* file_in, FILEOS* file_out, uint32_t *crc32),
		uint32_t *crc32
);

int zip_find_next_lfh(FILEOS *zipf, uintmax_t *offset);

void fill_CDFH(
		struct CentralDirectoryFileHeader* cdfh, struct LocalFileHeader lfh, fileinfo_t fi,
	   	uint16_t extrad_size, uintmax_t lfh_offset, char *comment
);

uint32_t write_CDFH(FILEOS *zipf, uintmax_t corresponding_lfh_offset, fileinfo_t fi, char *comment, uintmax_t *cfh_size);

uint32_t write_EOCD(FILEOS* zipf, uintmax_t cdfh_offset, uintmax_t cdfh_total, uintmax_t cdfh_size, char* comment);

uint32_t zip_pack(
		char* path_to_pack, char* archive_name,
		uint32_t (*compress_fnc)(FILEOS* file_in, FILEOS* file_out), uint16_t compress_method,
		uint32_t (*crypt_fnc)(FILEOS* file_in, FILEOS* file_out), uint16_t crypt_method,
		char* comment
);

#endif