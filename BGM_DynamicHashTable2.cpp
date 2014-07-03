#include "BGM_DynamicHashTable2.h"

namespace BGM
{
	DynamicHashTable::DynamicHashTable(const char* fileName, StudentFileManager& studentFile, BPlusTree& bPTree
											, char *externalBuffer1, char *externalBuffer2, vector<SaveInfo>& infoVector)
		: FileManager(fileName), studentFile(studentFile), bPTree(bPTree)
		, externalBuffer1((unsigned*)externalBuffer1), externalBuffer2((unsigned*)externalBuffer2), infoVector(infoVector)
		, currentTable((blockId_t*)buffer)
		, thatNumberOfData(*((int*)(externalBuffer1+NUMBEROFDATA_OFFSET))), thatNextSlot(*((int*)(externalBuffer1+NEXTSLOT_OFFSET)))
		, thatLastSlot(*((int*)(externalBuffer1+LASTSLOT_OFFSET))), thatMask(*((unsigned*)(externalBuffer1+STUDENTFILEMANAGER_MASK_OFFSET)))
		, thatDataList((Student*)(externalBuffer1+DATALIST_OFFSET))
	{
		if(numberOfBlock==0 && currentBlock==0)
		{
			tableMask = 0x00000000;
			currentBlock = newBlock();
			currentTable[0] = studentFile.getCurrentBlock();
			rootTableLocation = currentBlock;
			rootTable[0] = currentTable[0];
			recentKey = 0x00000000;
		}
		else
		{
			blockId_t temp = currentBlock;
			loadBlock(0);
			tableMask = *((unsigned*)(buffer+HASHTABLEMASK_OFFSET));
			recentKey = *((unsigned*)(buffer+RECENTKEY_OFFSET));
			rootTableLocation = *((blockId_t*)(buffer+ROOTTABLELOCATION_OFFSET));
			loadBlock(rootTableLocation);
			for(int i=0; i<BLOCKSIZE/sizeof(blockId_t); i++)
				rootTable[i] = currentTable[i];
			loadBlock(temp);
		}
	}

	DynamicHashTable::~DynamicHashTable()
	{
		fseek(file, HASHTABLEMASK_OFFSET, SEEK_SET);
		fwrite(&tableMask, 4, 1, file);
		fseek(file, RECENTKEY_OFFSET, SEEK_SET);
		fwrite(&recentKey, 4, 1, file);
		fseek(file, ROOTTABLELOCATION_OFFSET, SEEK_SET);
		fwrite(&rootTableLocation, 4, 1, file);
		if(currentBlock == rootTableLocation)
			for(unsigned int i=0; i<BLOCKSIZE/sizeof(blockId_t); i++)
				currentTable[i] = rootTable[i];
		else
			storeBlock(rootTableLocation, rootTable);
	}

	blockId_t DynamicHashTable::hashFunction(unsigned key)
	{
		unsigned keyValue = key;
		char *keyPart = (char*)(&keyValue);
		blockId_t result=0;
		result += keyPart[0];
		result *= 257;
		result += keyPart[1];
		result *= 257;
		result += keyPart[2];
		result *= 257;
		result += keyPart[3];

		return result;
	}

	blockId_t DynamicHashTable::insertKey(unsigned key)
	{
		unsigned hashValue = hashFunction(key);
		unsigned target;
		//search
		if(tableMask & FIRSTMASK)
		{
			storeBlock();
			loadBlock(rootTable[getFirstTableMask(hashValue) & getFirstTableMask(tableMask)]);
			loadBlock(currentTable[getSecondTableMask(hashValue)]);
			target = currentTable[getThirdTableMask(hashValue)];
		}
		else if(tableMask & SECONDMASK)
		{
			if(rootTable[getSecondTableMask(hashValue) & getSecondTableMask(tableMask)] != currentBlock)
			{
				storeBlock();
				loadBlock(rootTable[getSecondTableMask(hashValue) & getSecondTableMask(tableMask)]);
			}
			target = currentTable[getSecondTableMask(hashValue)];
		}
		else
			target = rootTable[getThirdTableMask(hashValue & tableMask)];

		if(studentFile.getCurrentBlock() != target)
		{
			studentFile.storeBlock();
			studentFile.loadBlock(target);
		}

		if(!studentFile.isFull())
			return target;
		else
		{
			splitDataBlock((hashValue & tableMask));

			//return
			if(tableMask & FIRSTMASK)
			{
				storeBlock();
				loadBlock(rootTable[getFirstTableMask(hashValue) & getFirstTableMask(tableMask)]);
				loadBlock(currentTable[getSecondTableMask(hashValue)]);
				target = currentTable[getThirdTableMask(hashValue)];
			}
			else if(tableMask & SECONDMASK)
			{
				if(rootTable[getSecondTableMask(hashValue) & getSecondTableMask(tableMask)] != currentBlock)
				{
					storeBlock();
					loadBlock(rootTable[getSecondTableMask(hashValue) & getSecondTableMask(tableMask)]);
				}
				target = currentTable[getThirdTableMask(hashValue)];
			}
			else
				target = rootTable[hashValue & tableMask];

			return target;
		}
	}

	void DynamicHashTable::expandTable()
	{
		storeBlock();

		if(tableMask & FIRSTMASK) // hash table tree의 depth가 3일때
		{
			unsigned firstTableMask = getFirstTableMask(tableMask);
			for(unsigned i=0; i<=firstTableMask; i++)
			{
				rootTable[firstTableMask+1+i] = newBlock();
				loadBlock(rootTable[i]);
				for(unsigned j=0; j<BLOCKSIZE/sizeof(blockId_t); j++)
				{
					loadBlock(currentTable[j], externalBuffer1);
					externalBuffer2[j] = newBlock();
					storeBlock(externalBuffer2[j], externalBuffer1);
				}
				storeBlock(rootTable[firstTableMask+1+i], externalBuffer2);
			}
		}
		else if(tableMask == (SECONDMASK|THIRDMASK)) //depth 2에서 depth 3으로 확장
		{
			storeBlock(rootTableLocation, rootTable);
			rootTable[0] = rootTableLocation;
			rootTable[1] = newBlock();
			rootTableLocation = newBlock();

			for(unsigned i=0; i<BLOCKSIZE/sizeof(blockId_t); i++)
			{
				loadBlock(currentTable[i], externalBuffer1);
				externalBuffer2[i] = newBlock();
				storeBlock(externalBuffer2[i], externalBuffer1);
			}
		}
		else if(tableMask & SECONDMASK) //hash table tree의 depth가 2일 때
		{
			unsigned secondMask = getSecondTableMask(tableMask);
			for(unsigned i=0; i<=secondMask; i++)
			{
				rootTable[secondMask+1+i] = newBlock();
				loadBlock(rootTable[i]);
				storeBlock(rootTable[secondMask+1+i], currentTable);
			}
		}
		else if(tableMask == THIRDMASK) //depth 1에서 depth 2로 확장
		{
			unsigned sibling = newBlock();
			storeBlock(rootTableLocation, rootTable);
			storeBlock(sibling, rootTable);
			rootTable[0] = rootTableLocation;
			rootTable[1] = sibling;
			rootTableLocation = newBlock();
			
			loadBlock(rootTable[0]);
		}
		else //hash table tree의 depth가 1일 때
		{
			for(unsigned i=0; i<=tableMask; i++)
				rootTable[tableMask+1+i] = rootTable[i];
		}

		tableMask = (tableMask<<1) | 0x00000001;
	}

	void DynamicHashTable::updateTable(unsigned usedTableIndex, unsigned newBlockMask, blockId_t newBlockId)
	{
		if(tableMask & FIRSTMASK)
		{
			unsigned currentMask;
			for(unsigned i=0; i<=getFirstTableMask(tableMask); i++)
			{
				loadBlock(rootTable[i]);
				for(unsigned j=0; j<BLOCKSIZE/sizeof(blockId_t); j++)
				{
					loadBlock(currentTable[j], externalBuffer1);
					for(unsigned k=0; k<BLOCKSIZE/sizeof(blockId_t); k++)
					{
						currentMask = (i<<20) | (j<<10) | k;
						if((currentMask&(usedTableIndex|newBlockMask)) == (usedTableIndex|newBlockMask))
							externalBuffer1[k] = newBlockId;
					}
				}
			}
		}
		else if(tableMask & SECONDMASK)
		{
			unsigned currentMask;
			for(unsigned i=0; i<=getSecondTableMask(tableMask); i++)
			{
				for(unsigned j=0; j<BLOCKSIZE/sizeof(blockId_t); j++)
				{
					currentMask = (i<<10) | j;
					if((currentMask&(usedTableIndex|newBlockMask)) == (usedTableIndex|newBlockMask))
							externalBuffer1[j] = newBlockId;
				}
			}
		}
		else
		{
			for(unsigned i=usedTableIndex|newBlockMask; i<=tableMask; i+=(newBlockMask<<1))
				rootTable[i] = newBlockId;
			for(unsigned i=usedTableIndex|newBlockMask; i>=(newBlockMask<<1); i-=(newBlockMask<<1))
				rootTable[i] = newBlockId;
		}
	}

	blockId_t DynamicHashTable::splitDataBlock(unsigned usedTableIndex)
	{
		//init
		blockId_t thatBlock = studentFile.newBlock();
		const Student *thisDataList = studentFile.getDataList();
		if(studentFile.getMask() == 0)
			studentFile.setMask(0x00000001);
		else
			studentFile.setMask(studentFile.getMask()<<1);
		thatMask = studentFile.getMask();
		thatNumberOfData = 0;

		//table expand
		if(!(thatMask & tableMask))
			expandTable();

		////data block split
		for(int i=0; i<STUDENTS_PER_BLOCK; i++)
		{
			if(hashFunction(thisDataList[i].getID()) & thatMask)
			{
				thatDataList[thatNumberOfData++] = thisDataList[i];
				//change index inforamtion
				bPTree.changeLocate(thisDataList[i].getID(), thisDataList[i].getScore(), thatBlock);
				//update infoVector
				updateInfoVector(thisDataList[i].getID(), thatBlock);

				studentFile.deleteData(i);				
			}
		}
		thatNextSlot = thatNumberOfData;
		thatLastSlot = thatNumberOfData;
		studentFile.storeBlock(studentFile.getCurrentBlock());
		studentFile.storeBlock(thatBlock, externalBuffer1);

		//table refresh
		updateTable(usedTableIndex, thatMask, thatBlock);

		return thatBlock;
	}

	void DynamicHashTable::updateInfoVector(unsigned id, blockId_t newBlockId)
	{
		unsigned i;
		for(i=0; i<infoVector.size() && infoVector[i].studentId!=id; i++);
		if(i != infoVector.size())
			infoVector[i].blockNumber = newBlockId;
	}

	blockId_t DynamicHashTable::searchKey(unsigned key)
	{
		unsigned hashValue = hashFunction(key);
		//search
		if(tableMask & FIRSTMASK)
		{
			storeBlock();
			loadBlock(rootTable[getFirstTableMask(hashValue) & getFirstTableMask(tableMask)]);
			loadBlock(currentTable[getSecondTableMask(hashValue)]);
			return currentTable[getThirdTableMask(hashValue)];
		}
		else if(tableMask & SECONDMASK)
		{
			if(rootTable[getSecondTableMask(hashValue)& getSecondTableMask(tableMask)] != currentBlock)
			{
				storeBlock();
				loadBlock(rootTable[getSecondTableMask(hashValue) & getSecondTableMask(tableMask)]);
			}
			return currentTable[getSecondTableMask(hashValue)];
		}
		else
			return rootTable[getThirdTableMask(hashValue) & getThirdTableMask(tableMask)];
	}
}