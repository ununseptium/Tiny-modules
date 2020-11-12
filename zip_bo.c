#include <zip_bo.h>

uint16_t zip_bo_reverse_uint16(uint16_t uint16){
	uint16_t val = 0;
	val = (uint16 >> 8) & 0xff;
	val = (uint16 << 8) & 0xff00;
	return val;
}

uint32_t zip_bo_reverse_uint32(uint32_t uint32){
	uint32_t val = 0;
	val = (uint32 >> 24) & 0xff;
	val = (uint32 >> 8) & 0xff00;
	val = (uint32 << 8) & 0xff0000;
	val = (uint32 << 24) & 0xff000000;
	return val;
}

int zip_bo_is_le(){
	uint16_t a = 1;
	if (*(uint8_t*)(&a) == 1) return 1;
	return 0;
}

void zip_bo_le_uint16(uint16_t *uint16){
	if(!zip_bo_is_le && uint16 != NULL){
		*uint16 = zip_bo_reverse_uint16(*uint16);
	}
}

void zip_bo_le_uint32(uint32_t *uint32){
	if(!zip_bo_is_le && uint32 != NULL){
		*uint32 = zip_bo_reverse_uint32(*uint32);
	}
}

void zip_bo_le_lfh(struct LocalFileHeader *lfh){
	if (!zip_bo_is_le() && lfh != NULL){
		lfh->signature = zip_bo_reverse_uint32(lfh->signature);
		lfh->versionToExtract = zip_bo_reverse_uint16(lfh->versionToExtract);
		lfh->generalPurposeBitFlag = zip_bo_reverse_uint16(lfh->generalPurposeBitFlag);
		lfh->compressionMethod = zip_bo_reverse_uint16(lfh->compressionMethod);
		lfh->modificationTime = zip_bo_reverse_uint16(lfh->modificationTime);
		lfh->modificationDate = zip_bo_reverse_uint16(lfh->modificationDate);
		lfh->crc32 = zip_bo_reverse_uint32(lfh->crc32);
		lfh->compressedSize = zip_bo_reverse_uint32(lfh->compressedSize);
		lfh->uncompressedSize = zip_bo_reverse_uint32(lfh->uncompressedSize);
		lfh->filenameLength = zip_bo_reverse_uint32(lfh->filenameLength);
		lfh->extraFieldLength = zip_bo_reverse_uint16(lfh->extraFieldLength);
	}
}

void zip_bo_le_cfh(struct CentralDirectoryFileHeader *cfh){
	if (!zip_bo_is_le() && cfh != NULL){
		cfh->signature = zip_bo_reverse_uint32(cfh->signature);
		cfh->versionMadeBy = zip_bo_reverse_uint16(cfh->versionMadeBy);
		cfh->versionToExtract = zip_bo_reverse_uint16(cfh->versionToExtract);
		cfh->generalPurposeBitFlag = zip_bo_reverse_uint16(cfh->generalPurposeBitFlag);
		cfh->compressionMethod = zip_bo_reverse_uint16(cfh->compressionMethod);
		cfh->modificationTime = zip_bo_reverse_uint16(cfh->modificationTime);
		cfh->modificationDate = zip_bo_reverse_uint16(cfh->modificationDate);
		cfh->crc32 = zip_bo_reverse_uint32(cfh->crc32);
		cfh->compressedSize = zip_bo_reverse_uint32(cfh->compressedSize);
		cfh->uncompressedSize = zip_bo_reverse_uint32(cfh->uncompressedSize);
		cfh->filenameLength = zip_bo_reverse_uint16(cfh->filenameLength);
		cfh->extraFieldLength = zip_bo_reverse_uint16(cfh->extraFieldLength);
		cfh->fileCommentLength = zip_bo_reverse_uint16(cfh->fileCommentLength);
		cfh->diskNumber = zip_bo_reverse_uint16(cfh->diskNumber);
		cfh->internalFileAttributes = zip_bo_reverse_uint16(cfh->internalFileAttributes);
		cfh->externalFileAttributes = zip_bo_reverse_uint32(cfh->externalFileAttributes);
		cfh->localFileHeaderOffset = zip_bo_reverse_uint32(cfh->localFileHeaderOffset);
	}
}

void zip_bo_le_eocd(struct EOCD *eocd){
	if(!zip_bo_is_le() && eocd != NULL){
		eocd->signature = zip_bo_reverse_uint16(eocd->signature);
		eocd->diskNumber = zip_bo_reverse_uint16(eocd->diskNumber);
		eocd->startDiskNumber = zip_bo_reverse_uint16(eocd->startDiskNumber);
		eocd->numberCentralDirectoryRecord = zip_bo_reverse_uint16(eocd->numberCentralDirectoryRecord);
		eocd->totalCentralDirectoryRecord = zip_bo_reverse_uint16(eocd->totalCentralDirectoryRecord);
		eocd->sizeOfCentralDirectory = zip_bo_reverse_uint16(eocd->sizeOfCentralDirectory);
		eocd->centralDirectoryOffset = zip_bo_reverse_uint16(eocd->centralDirectoryOffset);
		eocd->commentLength = zip_bo_reverse_uint16(eocd->commentLength);
	}
}

void zip_bo_le_zip64_eocd(struct zip64_end_of_central_directory *zip64_eocd){
	if (!zip_bo_is_le() && zip64_eocd != NULL){
		zip64_eocd->signature = zip_bo_reverse_uint32(zip64_eocd->signature);
		zip_bo_reverse_uintmax(&(zip64_eocd->sizeOfZip64EndOfCentralDirectory));
		zip64_eocd->versionMadeBy = zip_bo_reverse_uint32(zip64_eocd->versionMadeBy);
		zip64_eocd->versionToExtract = zip_bo_reverse_uint32(zip64_eocd->versionToExtract);
		zip64_eocd->diskNumber = zip_bo_reverse_uint32(zip64_eocd->diskNumber);
		zip64_eocd->startDiskNumber = zip_bo_reverse_uint32(zip64_eocd->startDiskNumber);
		zip_bo_reverse_uintmax(&(zip64_eocd->numberCentralDirectoryRecord));
		zip_bo_reverse_uintmax(&(zip64_eocd->totalCentralDirectoryRecord));
		zip_bo_reverse_uintmax(&(zip64_eocd->sizeOfCentralDirectory));
		zip_bo_reverse_uintmax(&(zip64_eocd->centralDirectoryOffset));
	}
}

void zip_bo_le_zip64_eocdl(struct Zip64EndOfCentralDirectoryLocator *zip64_eocdl){
	if (!zip_bo_is_le() && zip64_eocdl != NULL){
		zip64_eocdl->signature = zip_bo_reverse_uint32(zip64_eocdl->signature);
		zip64_eocdl->diskNumber = zip_bo_reverse_uint32(zip64_eocdl->diskNumber);
		zip_bo_reverse_uintmax(&(zip64_eocdl->zip64EndOfCentralDirectoryOffset));
		zip64_eocdl->totalDisks = zip_bo_reverse_uint32(zip64_eocdl->totalDisks);
	}
}

uintmax_t zip_bo_reverse_uintmax(uintmax_t uintmax){
	uintmax_t val = 0;
	if (sizeof(uintmax_t) == sizeof(uint32_t)){
		val = (uintmax >> 24) & 0xff;
		val = (uintmax >> 8) & 0xff00;
		val = (uintmax << 8) & 0xff0000;
		val = (uintmax << 24) & 0xff000000;
		return val;
	}else{
		val = (uintmax >> 56) & 0xff;
		val = (uintmax >> 40) & 0xff00;
		val = (uintmax >> 24) & 0xff0000;
		val = (uintmax >> 8) & 0xff000000;
		val = (uintmax << 8) & 0xff00000000;
		val = (uintmax << 24) & 0xff0000000000;
		val = (uintmax << 30) & 0xff000000000000;
		val = (uintmax << 56) & 0xff00000000000000;
	}
}

void zip_bo_le_uintmax(uintmax_t *uintmax){
	if (!zip_bo_is_le() uintmax != NULL)
		*uintmax = zip_bo_reverse_uintmax(*uintmax);
}
