#ifndef __BGM_DYNAMIC_HASH_TABLE_2__
#define __BGM_DYNAMIC_HASH_TABLE_2__

#include "BGM_DBsystem.h"
#include "BGM_FileManager.h"
#include "BGM_StudentFileManager.h"
#include "BGM_BPlusTree.h"

#define HASHTABLEMASK_OFFSET 16
#define RECENTKEY_OFFSET 20
#define ROOTTABLELOCATION_OFFSET 24
#define FIRSTMASK 0x07f00000
#define SECONDMASK 0x000ffc00
#define THIRDMASK 0X000003ff

namespace BGM
{
	class DynamicHashTable : public FileManager
	{
	private:
		StudentFileManager& studentFile;
		BPlusTree& bPTree;
		blockId_t *externalBuffer1;
		blockId_t *externalBuffer2;
		blockId_t *currentTable;
		vector<SaveInfo>& infoVector;
		unsigned tableMask;
		unsigned recentKey;
		blockId_t rootTableLocation;
		blockId_t rootTable[BLOCKSIZE/sizeof(blockId_t)];
		//for split
		int& thatNumberOfData;
		int& thatNextSlot;
		int& thatLastSlot;
		unsigned& thatMask;
		Student *thatDataList;
	public:
		DynamicHashTable(const char *fileName, StudentFileManager& studentFile, BPlusTree& bPTree
							, char *externalBuffer1, char *externalBuffer2, vector<SaveInfo>& infoVector);
		~DynamicHashTable();
		unsigned hashFunction(unsigned key);
		blockId_t insertKey(unsigned key);
		void expandTable();
		void updateTable(unsigned usedTableIndex, unsigned newBlockMask, blockId_t newBlockId);
		void updateInfoVector(unsigned id, blockId_t newBlockId);
		blockId_t splitDataBlock(unsigned oldMask);
		blockId_t searchKey(unsigned key);
		unsigned getFirstTableMask(unsigned hashValue) const;
		unsigned getSecondTableMask(unsigned hashValue) const;
		unsigned getThirdTableMask(unsigned hashValue) const;
	};

	inline unsigned DynamicHashTable::getFirstTableMask(unsigned hashValue) const
	{
		return (hashValue>>20) & 0x0000007f;
	}

	inline unsigned DynamicHashTable::getSecondTableMask(unsigned hashValue) const
	{
		return (hashValue>>10) & 0x000003ff;
	}

	inline unsigned DynamicHashTable::getThirdTableMask(unsigned hashValue) const
	{
		return hashValue & 0x000003ff;
	}
}

#endif
