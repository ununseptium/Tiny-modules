#ifndef _ZIP_BO_H
#define _ZIP_BO_H

#include <stdint.h>

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

#ifndef _ZIP64_END_OF_CENTRAL_DIRECTORY_DEFINED
#define _ZIP64_END_OF_CENTRAL_DIRECTORY_DEFINED
struct zip64_end_of_central_directory {
    uint32_t signature;
    uintmax_t sizeOfZip64EndOfCentralDirectory;
    uint16_t versionMadeBy;
    uint16_t versionToExtract;
    uint32_t diskNumber;
    uint32_t startDiskNumber;
    uintmax_t numberCentralDirectoryRecord;
    uintmax_t totalCentralDirectoryRecord;
    uintmax_t sizeOfCentralDirectory;
    uintmax_t centralDirectoryOffset;
}__attribute__((packed));
#endif

#ifndef _ZIP64_END_OF_CENTRAL_DIRECTORY_LOCATOR_DEFINED
#define _ZIP64_END_OF_CENTRAL_DIRECTORY_LOCATOR_DEFINED
struct zip64_end_of_central_directory_locator {
    uint32_t signature;
    uint32_t diskNumber;
    uintmax_t zip64EndOfCentralDirectoryOffset;
    uint32_t totalDisks;
}__attribute__((packed));
#endif

uint16_t zip_bo_reverse_uint16(uint16_t uint16);

uint32_t zip_bo_reverse_uint32(uint32_t uint32);

int zip_bo_is_le();

void zip_bo_le_uint16(uint16_t *uint16);

void zip_bo_le_uint32(uint32_t *uint32);

void zip_bo_le_lfh(struct LocalFileHeader *lfh);

void zip_bo_le_cfh(struct CentralDirectoryFileHeader *cfh);

void zip_bo_le_eocd(struct EOCD *eocd);

void zip_bo_le_zip64_eocd(struct zip64_end_of_central_directory *zip64_eocd);

void zip_bo_le_zip64_eocdl(struct zip64_end_of_central_directory_locator *zip64_eocdl);

uintmax_t zip_bo_reverse_uintmax(uintmax_t uintmax);

void zip_bo_le_uintmax(uintmax_t *uintmax);

#endif
