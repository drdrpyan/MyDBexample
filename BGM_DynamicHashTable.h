#ifndef __BGM_DYNAMIC_HASH_TABLE__
#define __BGM_DYNAMIC_HASH_TABLE__

#include "BGM_StudentFileManager.h"

#define DYNAMICHASH_MASK_OFFSET 16
#define PARENTMASK_OFFSET 20
#define ROOTTABLE_OFFSET 24

#define ROOTTABLE_SIZE 64
// 2^6=64 > 39
// 39(level1 table)*1023(level2 table)*1023(data block per table)*107(data per block)=4367165517 > 4294967296=2^32(최대 id 개수)
#define DEPTH1MASK 0x03f00000 // 0000 0011 1111 0000 0000 0000 0000 0000
#define DEPTH2MASK 0x000ffc00 // 0000 0000 0000 1111 1111 1100 0000 0000
#define DEPTH3MASK 0x000003ff // 0000 0000 0000 0000 0000 0011 1111 1111

namespace BGM
{
	class DynamicHashTable : public FileManager
	{
	private:
		StudentFileManager& studentFile;
		unsigned mask;
		unsigned parentMask;
		unsigned currentBlockMask;
		blockId_t& currentTable;
		blockId_t *rootTable;
		blockId_t *hashTable;
		char * const externalBuffer;

		vector<SaveInfo>& saveInfoVector;

	public:
		unsigned hashFunction(unsigned key) const;
		unsigned splitDataBlock(char *addtionalBuffer);
		void expandTable();
	public:
		DynamicHashTable(const char* fileName, StudentFileManager& studentFileManager, void *externalBuffer, vector<SaveInfo>& saveInfoVector);
		~DynamicHashTable();
		blockId_t bucketIs(unsigned key);
		blockId_t searchKey(unsigned key);
		//blockId_t deleteKey(unsigned key);
	};
}

#endif