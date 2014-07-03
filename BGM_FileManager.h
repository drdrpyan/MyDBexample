#ifndef __BGM_FILEMANAGER__
#define __BGM_FILEMANAGER__

#include "BGM_DBsystem.h"
#include <iostream>

#define NUMBEROFBLOCK_OFFSET 0
#define CURRENTBLOCK_OFFSET 4
#define NEXTBLOCK_OFFSET 8
#define LASTBLOCK_OFFSET 12

namespace BGM
{
	class FileManager
	{
	protected:
		FILE *file;
		char buffer[BLOCKSIZE];
		unsigned numberOfBlock;
		blockId_t currentBlock;
		blockId_t nextBlock;
		blockId_t lastBlock;
	public:
		FileManager(const char* fileName);
		~FileManager(void);

		unsigned getNumberOfBlock(void) const;
		blockId_t getCurrentBlock(void) const;
		blockId_t getNextBlock(void) const;
		blockId_t getLastBlock(void) const;

		blockId_t newBlock(void);
		void loadBlock(blockId_t id);
		void loadBlock(blockId_t id, void *externalBuffer);
		void storeBlock(void);
		void storeBlock(blockId_t id);
		void storeBlock(blockId_t id, void *externalBuffer);
		void deleteBlock(blockId_t id);
	};

	inline unsigned FileManager::getNumberOfBlock(void) const
	{
		return numberOfBlock;
	}

	inline blockId_t FileManager::getCurrentBlock(void) const
	{
		return currentBlock;
	}

	inline blockId_t FileManager::getNextBlock(void) const
	{
		return nextBlock;
	}

	inline blockId_t FileManager::getLastBlock(void) const
	{
		return lastBlock;
	}

}

#endif