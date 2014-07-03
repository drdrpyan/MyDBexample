#include "BGM_DynamicHashTable.h"

namespace BGM
{
	DynamicHashTable::DynamicHashTable(const char* fileName, StudentFileManager& studentFileManager, void *externalBuffer, vector<SaveInfo>& saveInfoVector)
		: FileManager(fileName), studentFile(studentFileManager), currentTable(currentBlock)
		, hashTable((blockId_t*)buffer), externalBuffer((char*)externalBuffer)
		, saveInfoVector(saveInfoVector)
	{
		if(currentBlock==0 && lastBlock == 0)
		{
			mask = 0x00000000;
			parentMask = 0x00000000;
			currentBlock = newBlock();
			rootTable = (blockId_t*)buffer;
			rootTable[0] = studentFile.getCurrentBlock();
		}
		else
		{
			blockId_t temp = currentTable;
			loadBlock(0);

			mask = *((unsigned*)(buffer+DYNAMICHASH_MASK_OFFSET));
			parentMask = *((unsigned*)(buffer+PARENTMASK_OFFSET));
			if(mask & DEPTH1MASK)
			{
				rootTable = new unsigned[ROOTTABLE_SIZE];
				for(int i=0; i<ROOTTABLE_SIZE; i++)
					rootTable[i] = ((unsigned*)(buffer+ROOTTABLE_OFFSET))[i];
			}
			else if(mask > DEPTH3MASK)
			{
				loadBlock(*((unsigned*)(buffer+ROOTTABLE_OFFSET)));
				rootTable = new unsigned[BLOCKSIZE/sizeof(unsigned)];
				for(int i=0; i<(BLOCKSIZE/sizeof(unsigned)); i++)
					rootTable[i] = ((unsigned*)buffer)[i];
			}
			else
				rootTable = (unsigned*)buffer;

			loadBlock(temp);
		}
	}

	DynamicHashTable::~DynamicHashTable()
	{
		fseek(file, DYNAMICHASH_MASK_OFFSET, SEEK_SET);
		fwrite(&mask, 4, 1, file);
		fseek(file, PARENTMASK_OFFSET, SEEK_SET);
		fwrite(&parentMask, 4, 1, file);

		if(mask & DEPTH1MASK)
		{
			fseek(file, ROOTTABLE_OFFSET, SEEK_SET);
			fwrite(rootTable, ROOTTABLE_SIZE, 1, file);
			delete rootTable;
		}
		else if(mask > DEPTH3MASK)
		{
			int rootTableBlock = newBlock();
			fseek(file, ROOTTABLE_OFFSET, SEEK_SET);
			fwrite(&rootTableBlock, 4, 1, file);
			storeBlock(rootTableBlock, rootTable);
			delete rootTable;
		}
		
		storeBlock(currentTable);
	}

	unsigned DynamicHashTable::hashFunction(unsigned key) const
	{
		unsigned k = key;
		
		unsigned hashValue = 0;
		char *keyPart = (char*)&k;	

		hashValue += keyPart[0];
		hashValue *= 257;
		hashValue += keyPart[1];
		hashValue *= 257;
		hashValue += keyPart[2];
		hashValue *= 257;
		hashValue += keyPart[3];

		return hashValue;
	}

	unsigned DynamicHashTable::splitDataBlock(char *additionalBuffer)
	{
		const Student * const thisList = studentFile.getDataList();
		studentFile.setMask(studentFile.getMask() ? studentFile.getMask()<<1 : 0x00000001);
		int& thatNumberOfData = *((int*)(additionalBuffer+NUMBEROFDATA_OFFSET)) = 0;
		int& thatNextSlot = *((int*)(additionalBuffer+NEXTSLOT_OFFSET));
		int& thatLastSlot = *((int*)(additionalBuffer+LASTSLOT_OFFSET));
		unsigned& thatMask = *((unsigned*)(additionalBuffer+STUDENTFILEMANAGER_MASK_OFFSET)) = studentFile.getMask();
		Student* thatList = (Student*)(additionalBuffer+DATALIST_OFFSET);
		unsigned thatBlock = studentFile.newBlock();

		for(int i=0; i<STUDENTS_PER_BLOCK; i++)
		{
			if(hashFunction(thisList[i].getID()) & thatMask)
			{
				thatList[thatNumberOfData++] = thisList[i];
				studentFile.deleteData(i);

				//for espa
				//unsigned j;
				//for(j=0; j<saveInfoVector.size() && saveInfoVector[j].studentId!=thatList[thatNumberOfData-1].getID(); j++);
				//saveInfoVector[j].blockNumber = thatBlock;
			}
		}
		thatNextSlot = thatNumberOfData;
		thatLastSlot = thatNumberOfData;

		studentFile.storeBlock(studentFile.getCurrentBlock());
		studentFile.storeBlock(thatBlock, additionalBuffer);
		return thatBlock;
	}

	void DynamicHashTable::expandTable()
	{
		unsigned newMask = (mask<<1)|(0x00000001);
		unsigned splitMask = mask^newMask;
		for(unsigned i=0; i<=mask; i++)
			hashTable[splitMask&i] = hashTable[i];
		mask = newMask;
	}

	blockId_t DynamicHashTable::bucketIs(unsigned key)
	{
		unsigned tableIndex;
		unsigned hashValue = hashFunction(key);
		if(mask & DEPTH1MASK)
		{
			blockId_t secondTable = rootTable[((hashValue&DEPTH1MASK)>>20)&(0x0000003f)];
			loadBlock(secondTable);
			blockId_t thirdTable = hashTable[((hashValue&DEPTH2MASK)>>10)&(0x000003ff)];
			loadBlock(thirdTable);
			tableIndex = hashValue&DEPTH3MASK;
		}
		else if(mask > DEPTH3MASK)
		{
			blockId_t secondTable = rootTable[((hashValue&DEPTH2MASK)>>10)&(0x000003ff)];
			loadBlock(secondTable);
			tableIndex = hashValue&DEPTH3MASK;
		}
		else
			tableIndex = hashValue&mask;

		if(studentFile.getCurrentBlock() == hashTable[tableIndex])
		{
			studentFile.storeBlock(currentBlock);
			studentFile.loadBlock(hashTable[tableIndex]);
		}
		if(!studentFile.isFull())
			return hashTable[tableIndex];
		else
		{
			if(mask==0 || !((studentFile.getMask()<<1)&mask) )
					expandTable();

			unsigned newDataBlock = splitDataBlock(externalBuffer);
			for(unsigned i=tableIndex+studentFile.getMask(); i<=mask; i+=studentFile.getMask())
				hashTable[i] = newDataBlock;

			return bucketIs(key);
		}
	}

	blockId_t DynamicHashTable::searchKey(unsigned key)
	{
		unsigned hashValue = hashFunction(key);
		if(mask & DEPTH1MASK)
		{
			blockId_t secondTable = rootTable[((hashValue&DEPTH1MASK)>>20)&(0x0000003f)];
			loadBlock(secondTable);
			blockId_t thirdTable = hashTable[((hashValue&DEPTH2MASK)>>10)&(0x000003ff)];
			loadBlock(thirdTable);
			return hashTable[hashValue&DEPTH3MASK];
		}
		else if(mask > DEPTH3MASK)
		{
			blockId_t secondTable = rootTable[((hashValue&DEPTH2MASK)>>10)&(0x000003ff)];
			loadBlock(secondTable);
			return hashTable[hashValue&DEPTH3MASK];
		}
		else
			return hashTable[hashValue&mask];
	}
}