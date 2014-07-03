#include "BGM_StudentFileManager.h"

namespace BGM
{
	StudentFileManager::StudentFileManager(const char* fileName)
		: DataFileManager<Student>::DataFileManager(fileName), mask(*((unsigned*)(buffer+STUDENTFILEMANAGER_MASK_OFFSET)))
	{
		if(numberOfBlock==1 && currentBlock==1 && numberOfData==0)
			mask = 0x00000000;
	}

	unsigned StudentFileManager::getMask() const
	{
		return mask;
	}

	void StudentFileManager::setMask(unsigned m)
	{
		mask = m;
	}

	void StudentFileManager::deleteData(signed int index)
	{
		int* slotHead = (int*)(DataFileManager<Student>::dataList + index);
		(*slotHead) = nextSlot;
		dataList[index].setID(0);

		////delete test
		//for(int i=1; i<38; i++)
		//	slotHead[i] = 0x00000000;

		nextSlot = index;
		numberOfData--;
	}

	void StudentFileManager::deleteData(unsigned id)
	{
		int i;
		for(i=0; DataFileManager<Student>::dataList[i].getID()!=id && i<STUDENTS_PER_BLOCK; i++);
		
		if(i == STUDENTS_PER_BLOCK)
			return;
		else
		{
			int* slotHead = (int*)(DataFileManager<Student>::dataList + i);

			(*slotHead) = nextSlot;
			nextSlot = i;
			numberOfData--;

			//if(numberOfBlock == 0)
			//	deleteBlock(currentBlock);
		}
	}

	//사용불가
	Student StudentFileManager::searchData(const Student& student)
	{
		int i;
		unsigned id = student.getID();
		for(i=0; DataFileManager<Student>::dataList[i].getID()!=id && i<STUDENTS_PER_BLOCK; i++);
		
		return DataFileManager<Student>::dataList[i];
	}

	//사용불가
	void StudentFileManager::deleteData(const Student& student)
	{
	}

	float StudentFileManager::getScore(blockId_t block, unsigned ID)
	{
		if(currentBlock != block)
		{
			storeBlock();
			loadBlock(block);
		}

		int i;
		for(i=0; i<STUDENTS_PER_BLOCK && dataList[i].getID()!= ID ; i++);
		
		if(i == STUDENTS_PER_BLOCK || dataList[i].getID()==0)
			return -1;
		else
		{
			float result = dataList[i].getScore();
			deleteData(i);
			return result;
		}
	}

	//for debug
	void StudentFileManager::printBlock()
	{
		char filename[50];
		strcpy(filename, "DataBlock");
		unsigned current = currentBlock;
		for(int i=0; i<current; current/=10)
		{
			filename[9+i] = '0'+current%10;
			filename[9+i+1] = 0;
		}
		strcat(filename, ".csv");

		std::ofstream outFile(filename);
		outFile << "numberOfData," << numberOfData << ",nextSlot," << nextSlot <<",lastSlot," << lastSlot << std::endl;
		for(int i=0; i<STUDENTS_PER_BLOCK; i++)
		{
			if((dataList[i].getName())[0]!='n' && (dataList[i].getName())[1]!='a')
				outFile << '-' << (dataList[i].getName())[0] << ',';
			else
				outFile << dataList[i].getID() << ',';
		}
		outFile << std::endl;
		outFile.close();
	}

}