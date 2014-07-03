#include "BGM_FileManager.h"

namespace BGM
{
	FileManager::FileManager(const char* fileName)
	{
		if(!(file=fopen(fileName, "rb+")))
		{
			file = fopen(fileName, "wb+");
			numberOfBlock = 0;
			currentBlock = 0;
			nextBlock = 1;
			lastBlock = 0;
		}
		else
		{
			fread_s(buffer, BLOCKSIZE, BLOCKSIZE, 1, file);
			numberOfBlock = *((unsigned*)(buffer + NUMBEROFBLOCK_OFFSET));
			currentBlock = *((unsigned*)(buffer + CURRENTBLOCK_OFFSET));
			nextBlock = *((unsigned*)(buffer+NEXTBLOCK_OFFSET));
			lastBlock = *((unsigned*)(buffer+LASTBLOCK_OFFSET));
			loadBlock(currentBlock);
		}
	}
	
	FileManager::~FileManager(void)
	{
		storeBlock(currentBlock);
		fseek(file, 0, SEEK_SET);
		fwrite(&numberOfBlock, 4, 1, file);
		fwrite(&currentBlock, 4, 1, file);
		fwrite(&nextBlock, 4, 1, file);
		fwrite(&lastBlock, 4, 1, file);
		fclose(file);
	}

	blockId_t FileManager::newBlock(void)
	{
		int result = nextBlock;

		numberOfBlock++;
		if(numberOfBlock > lastBlock)
		{
			lastBlock++;
			nextBlock = lastBlock+1;
		}
		else
		{
			fseek(file, nextBlock*BLOCKSIZE, SEEK_SET);
			fread_s(buffer, BLOCKSIZE, BLOCKSIZE, 1, file);
			nextBlock = *((blockId_t*)buffer);
		}

		return result;
	}

	void FileManager::loadBlock(blockId_t id)
	{
		fseek(file, id*BLOCKSIZE, SEEK_SET);
		fread_s(buffer, BLOCKSIZE, BLOCKSIZE, 1, file);
		currentBlock = id;
	}

	void FileManager::loadBlock(blockId_t id, void *externalBuffer)
	{
		fseek(file, id*BLOCKSIZE, SEEK_SET);
		fread_s(externalBuffer, BLOCKSIZE, BLOCKSIZE, 1, file);
	}

	void FileManager::storeBlock(void)
	{
		fseek(file, currentBlock*BLOCKSIZE, SEEK_SET);
		fwrite(buffer, BLOCKSIZE, 1, file);
	}
	void FileManager::storeBlock(blockId_t id)
	{
		fseek(file, id*BLOCKSIZE, SEEK_SET);
		fwrite(buffer, BLOCKSIZE, 1, file);
	}

	void FileManager::storeBlock(blockId_t id, void *externalBuffer)
	{
		fseek(file, id*BLOCKSIZE, SEEK_SET);
		fwrite(externalBuffer, BLOCKSIZE, 1, file);
	}

	void FileManager::deleteBlock(blockId_t id)
	{
		fseek(file, id*BLOCKSIZE, SEEK_SET);
		fwrite(&nextBlock, 4, 1, file);
		nextBlock = id;
		numberOfBlock--;
	}
}