#ifndef __BGM_B_PLUS_TREE__
#define __BGM_B_PLUS_TREE__

#include "BGM_DBsystem.h"
#include "BGM_FileManager.h"
#include <limits.h>
#include <float.h>

#define DATA_PER_BPTNODE 339
#define LEAF false
#define NONLEAF true
#define ROOT_OFFSET 16
#define NODETYPE_OFFSET 0
#define BPT_NUMBEROFDATA_OFFSET 4
#define NEXT_OFFSET 8
#define RIGHTCHILD_OFFSET 8
#define BPT_DATALIST_OFFSET 16

namespace BGM
{
	class BPlusTree;
#pragma pack(push, 1)
	class BPTData
	{
	friend BPlusTree;
	private:
		unsigned ID;
		float score;
		union
		{
			blockId_t leftChild; //for nonleaf data
			blockId_t locate; //for leaf data
		};

		BPTData(unsigned ID, float score, blockId_t leftChild_locate);
		BPTData(const BPTData& thatData);
		bool operator==(const BPTData& data) const;
		bool operator<(const BPTData& data) const;
		bool operator<(float score) const;
		bool operator<=(const BPTData& data) const;
		bool operator<=(float score) const;
		bool operator>(const BPTData& data) const;
		bool operator>(float score) const;
		bool operator>=(const BPTData& data) const;
		bool operator>=(float score) const;
	};
#pragma pack(pop)

	inline bool BPTData::operator==(const BPTData& data) const
	{
		return ID==data.ID && score==data.score;
	}

	inline bool BPTData::operator<(const BPTData& data) const
	{
		return score < data.score;
	}

	inline bool BPTData::operator<(float score) const
	{
		return this->score < score;
	}

	inline bool BPTData::operator<=(const BPTData& data) const
	{
		return ((score==data.score)?(ID<=data.ID):(score<data.score));
	}

	inline bool BPTData::operator<=(float score) const
	{
		return this->score <= score;
	}

	inline bool BPTData::operator>(const BPTData& data) const
	{
		return score > data.score;
	}

	inline bool BPTData::operator>(float score) const
	{
		return this->score > score;
	}

	inline bool BPTData::operator>=(const BPTData& data) const
	{
		return ((score==data.score)?(ID>=data.ID):(score>data.score));
	}

	inline bool BPTData::operator>=(float score) const
	{
		return this->score >= score;
	}


	class BPlusTree : public FileManager
	{
	private:
		//block 0
		blockId_t root;
		//block n
		bool& nodeType;
		int& numberOfData;
		blockId_t& next;
		blockId_t& rightChild;
		BPTData *dataList;
		//externalBuffer
		char *externalBuffer;
		bool& thatNodeType;
		int& thatNumberOfData;
		blockId_t& thatNext;
		blockId_t& thatRightChild;
		BPTData *thatDataList;
		
		vector<blockId_t> trace;
	private:
		blockId_t searchData(const BPTData& data);
		blockId_t searchDataWithTrace(const BPTData& data);

		void insertData(const BPTData& data);
		void insertDataToList(const BPTData& data);
		void insertDataToList(const BPTData& data, blockId_t rightChild);
		
		void splitRoot(const BPTData& data, blockId_t rightChild);
		void splitLeafNode(void);
		void splitNonLeafNode(void);
		void insertNonLeafNode(const BPTData& data, blockId_t rightChild);
		
		bool isOverflowed();
	public:
		BPlusTree(const char *filename, char *externalBuffer);
		~BPlusTree();
		void insertData(unsigned ID, float score, blockId_t locate);
		void changeLocate(unsigned ID, float score, blockId_t locate);
		blockId_t searchData(float score);
		vector<blockId_t> searchRange(float lower, float upper);
		void deleteData(unsigned ID, float score);
		void printNode(blockId_t blockId);
	};

	inline bool BPlusTree::isOverflowed()
	{
		return numberOfData > DATA_PER_BPTNODE;
	}
}

#endif