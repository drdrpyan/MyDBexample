#include "BGM_BPlusTree.h"

namespace BGM
{
	BPTData::BPTData(unsigned ID, float score, blockId_t leftChild_locate)
		: ID(ID), score(score), leftChild(leftChild_locate)
	{
	}

	BPTData::BPTData(const BPTData& thatData)
		: ID(thatData.ID), score(thatData.score), leftChild(thatData.leftChild)
	{
	}

	BPlusTree::BPlusTree(const char *fileName, char *externalBuffer)
		: FileManager(fileName)
		, nodeType(*(bool*)(buffer+NODETYPE_OFFSET)), numberOfData(*(int*)(buffer+BPT_NUMBEROFDATA_OFFSET))
		, next(*(blockId_t*)(buffer+NEXT_OFFSET)), rightChild(*(blockId_t*)(buffer+RIGHTCHILD_OFFSET))
		, dataList((BPTData*)(buffer+BPT_DATALIST_OFFSET))
		, externalBuffer(externalBuffer)
		, thatNodeType(*(bool*)(externalBuffer+NODETYPE_OFFSET)), thatNumberOfData(*(int*)(externalBuffer+BPT_NUMBEROFDATA_OFFSET))
		, thatNext(*(blockId_t*)(externalBuffer+NEXT_OFFSET)), thatRightChild(*(blockId_t*)(externalBuffer+RIGHTCHILD_OFFSET))
		, thatDataList((BPTData*)(externalBuffer+BPT_DATALIST_OFFSET))
	{
		if(numberOfBlock==0 && currentBlock==0)
		{
			root = currentBlock = newBlock();
			nodeType = LEAF;
			numberOfData = 0;
			next = 0;
		}
		else
		{
			blockId_t temp = currentBlock;
			loadBlock(0);
			root = *((blockId_t*)(buffer+ROOT_OFFSET));
			loadBlock(temp);
		}
	}

	BPlusTree::~BPlusTree()
	{
		fseek(file, ROOT_OFFSET, SEEK_SET);
		fwrite(&root, 4, 1, file);
	}

	blockId_t BPlusTree::searchData(const BPTData& data)
	{
		if(root != currentBlock)
		{
			storeBlock();
			loadBlock(root);
		}

		int i;
		while(nodeType) // true==nonleaf
		{
			for(i=0; i<numberOfData && data>dataList[i]; i++);
			(i<numberOfData) ? loadBlock(dataList[i].leftChild) : loadBlock(rightChild);
		}

		return currentBlock;
	}

	blockId_t BPlusTree::searchDataWithTrace(const BPTData& data)
	{
		if(root != currentBlock)
		{
			storeBlock();
			loadBlock(root);
		}
		if(trace.size()>0)
			trace.clear();

		int i;
		while(nodeType) // true==nonleaf
		{
			trace.push_back(currentBlock);
			for(i=0; i<numberOfData && data>dataList[i]; i++);
			(i<numberOfData) ? loadBlock(dataList[i].leftChild) : loadBlock(rightChild);
		}

		return currentBlock;
	}

	void BPlusTree::insertData(const BPTData& data)
	{
		int targetNode = searchDataWithTrace(data);
		if(currentBlock != targetNode)
		{
			storeBlock();
			loadBlock(targetNode);
		}
		insertDataToList(data);

		if(isOverflowed())
			splitLeafNode();
	}

	void BPlusTree::insertDataToList(const BPTData& data)
	{
		int i;
		for(i=0; i<numberOfData && data>dataList[i]; i++);
		for(int j=numberOfData; j>i; j--)
			dataList[j] = dataList[j-1];
		dataList[i] = data;
		numberOfData++;
	}

	void BPlusTree::insertDataToList(const BPTData& data, blockId_t rightChild)
	{
		int i;
		for(i=0; i<numberOfData && data>dataList[i]; i++);
		for(int j=numberOfData; j>i; j--)
			dataList[j] = dataList[j-1];
		dataList[i] = data;
		(i<numberOfData) ?	dataList[i+1].leftChild=rightChild : (this->rightChild)=rightChild;
		numberOfData++;
	}

	void BPlusTree::splitRoot(const BPTData& data, blockId_t rightChild)
	{
			root = newBlock();
			thatNodeType = NONLEAF;
			thatNumberOfData = 1;
			thatDataList[0] = data;
			thatRightChild = rightChild;
			storeBlock(root, externalBuffer);
			trace.clear();
	}

	void BPlusTree::splitLeafNode()
	{
		thatNumberOfData = 0;
		for(int i=DATA_PER_BPTNODE/2+1; i<DATA_PER_BPTNODE+1; i++)
			thatDataList[thatNumberOfData++] = dataList[i];
		thatNodeType = nodeType;
		thatNext = next;
		next = newBlock();
		numberOfData = DATA_PER_BPTNODE/2+1;
		
		storeBlock();
		storeBlock(next, externalBuffer);

		BPTData dataToParent(dataList[DATA_PER_BPTNODE/2]);
		dataToParent.leftChild = currentBlock;

		if(currentBlock == root)
			splitRoot(dataToParent, next);
		else
			insertNonLeafNode(dataToParent, next);
	}

	void BPlusTree::splitNonLeafNode()
	{
		thatNumberOfData = 0;
		for(int i=DATA_PER_BPTNODE/2+1; i<DATA_PER_BPTNODE+1; i++)
			thatDataList[thatNumberOfData++] = dataList[i];
		thatNodeType = nodeType;
		thatRightChild = rightChild;
		numberOfData = DATA_PER_BPTNODE/2;
		rightChild = dataList[DATA_PER_BPTNODE/2].leftChild;
		
		blockId_t thatBlock = newBlock();
		storeBlock();
		storeBlock(thatBlock, externalBuffer);

		BPTData dataToParent(dataList[DATA_PER_BPTNODE/2]);
		dataToParent.leftChild = currentBlock;

		if(currentBlock == root)
			splitRoot(dataToParent, thatBlock);
		else
			insertNonLeafNode(dataToParent, thatBlock);
	}

	void BPlusTree::insertNonLeafNode(const BPTData& data, blockId_t rightChild)
	{
		loadBlock(trace.back());
		trace.pop_back();

		insertDataToList(data, rightChild);
		if(isOverflowed())
			splitNonLeafNode();
		else
			trace.clear();
		storeBlock();
	}

	void BPlusTree::insertData(unsigned ID, float score, blockId_t locate)
	{
		insertData(BPTData(ID, score, locate));
	}

	void BPlusTree::changeLocate(unsigned ID, float score, blockId_t locate)
	{
		searchData(BPTData(ID, score, 0));
		int i;
		for(i=0; i<DATA_PER_BPTNODE && dataList[i].ID!=ID; i++);
		while(next!=0 && i==DATA_PER_BPTNODE)
		{
			loadBlock(next);
			for(i=0; i<DATA_PER_BPTNODE && dataList[i].ID!=ID; i++);
		}
		dataList[i].locate = locate;

	}

	blockId_t BPlusTree::searchData(float score)
	{
		if(root != currentBlock)
		{
			storeBlock();
			loadBlock(root);
		}

		int i;
		while(nodeType) // true==nonleaf
		{
			for(i=0; i<numberOfData && dataList[i]<score; i++);
			(i<numberOfData) ? loadBlock(dataList[i].leftChild) : loadBlock(rightChild);
		}

		return currentBlock;
	}

	vector<blockId_t> BPlusTree::searchRange(float lower, float upper)
	{
		vector<blockId_t> locateList;
		searchData(lower);
		int i;
		for(i=0; i<numberOfData && dataList[i].score<lower; i++);
		for(i; i<numberOfData && dataList[i].score<=upper; i++)
			locateList.push_back(dataList[i].locate);

		while(next!=0 && i==numberOfData)
		{
			loadBlock(next);
			for(i=0; i<numberOfData && dataList[i].score<=upper; i++)
				locateList.push_back(dataList[i].locate);
		}

		return locateList;
	}

	void BPlusTree::deleteData(unsigned ID, float score)
	{
		BPTData data(ID, score, 0);

		if(root != currentBlock)
		{
			storeBlock();
			loadBlock(root);
		}

		int i;
		while(nodeType)
		{
			for(i=0; i<numberOfData && data>dataList[i]; i++);

			if(i<numberOfData && data==dataList[i])
			{
				blockId_t returnLocate = dataList[i].leftChild;
				blockId_t deleteTarget = i;
				bool internalBuffer = true;
				bool nonLeafNode = true;

				while(nodeType)
				{
					loadBlock(dataList[deleteTarget].leftChild, externalBuffer);
					
					if(thatDataList[thatNumberOfData-1] ==  data)
					{
						dataList[deleteTarget].ID = thatDataList[thatNumberOfData-2].ID;
						dataList[deleteTarget].score = thatDataList[thatNumberOfData-2].score;
						deleteTarget=thatNumberOfData-2;
					}
					else
					{
						dataList[deleteTarget].ID = thatDataList[thatNumberOfData-1].ID;
						dataList[deleteTarget].score = thatDataList[thatNumberOfData-1].score;
						deleteTarget=thatNumberOfData-1;
					}

					storeBlock();
					for(int j=0; i<BLOCKSIZE; j++)
						buffer[j] = externalBuffer[j];
				}

				loadBlock(returnLocate);
			}

			(i<numberOfData) ? loadBlock(dataList[i].leftChild) : loadBlock(rightChild);
		}

		for(i=0; i<numberOfData && data>dataList[i]; i++);
		for(int j=i; j<numberOfData; j++)
			dataList[j] = dataList[j+1];
	}

	void BPlusTree::printNode(blockId_t blockId)
	{
		if(currentBlock != blockId)
		{
			storeBlock();
			loadBlock(blockId);
		}

		char filename[100];
		strcpy(filename, "indexNode");
		unsigned current = currentBlock;
		for(unsigned int i=0; i<current; current/=10)
		{
			filename[9+i] = '0'+current%10;
			filename[9+i+1] = 0;
		}
		strcat(filename, ".csv");

		std::ofstream outFile(filename);
		outFile << "currentBlock " << currentBlock << ",type " << nodeType << ",numberOfData " << numberOfData;
		if(nodeType == LEAF)
			outFile << ",next " << next << std::endl << std::endl;
		else
			outFile << ",rightChild " << rightChild << std::endl << std::endl;
		for(int i=0; i<numberOfData; i++)
			outFile << dataList[i].ID << ',' << dataList[i].score << ',' << dataList[i].locate << std::endl;

		outFile.close();

	}
}