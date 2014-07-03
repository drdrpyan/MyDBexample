#ifndef __BGM_DATAFILEMANAGER__
#define __BGM_DATAFILEMANAGER__

// numberOfEntry	nextSlot	empty		
// 0 (4byte)		4 (4byte)	8 (4088byte)

#include "BGM_FileManager.h"

#define NUMBEROFDATA_OFFSET 0
#define NEXTSLOT_OFFSET 4
#define LASTSLOT_OFFSET 8

namespace BGM
{
	template<typename T>
	class DataFileManager :	public FileManager
	{
	protected:
		const int dataSize;
		const int dataPerBlock;
		int & numberOfData;
		int & nextSlot;
		int & lastSlot;
		T* const dataList;

		//사용불가
		virtual void deleteData(const T& data)=0;
		virtual T searchData(const T& data)=0;
	public:
		DataFileManager(const char* fileName);

		int getDataSize(void) const;
		int getDataPerBlock(void) const;
		int getNumberOfData(void) const;
		int getNextSlot(void) const;
		int getLastSlot(void) const;
		bool isFull(void) const;
		const T* getDataList(void) const;
		blockId_t insertData(const T& data);		
	};

	template<typename T>
	DataFileManager<T>::DataFileManager(const char* fileName) 
		: FileManager(fileName)
		, dataSize(sizeof(T)), dataPerBlock((BLOCKSIZE-8)/sizeof(T))
		, numberOfData(*(int*)((char*)buffer+NUMBEROFDATA_OFFSET)), nextSlot(*(int*)((char*)buffer+NEXTSLOT_OFFSET))
		, lastSlot(*(int*)((char*)buffer+LASTSLOT_OFFSET))
		, dataList((T*)((char*)buffer + BLOCKSIZE - ((BLOCKSIZE-8)/sizeof(T))*sizeof(T)))
	{
		if(numberOfBlock == 0)
		{
			currentBlock = newBlock();
			numberOfData = 0;
			nextSlot = 0;
			lastSlot = 0;
		}
	}

	template<typename T>
	inline int DataFileManager<T>::getDataSize(void) const
	{
		return dataSize;
	}

	template<typename T>
	inline int DataFileManager<T>::getDataPerBlock(void) const
	{
		return dataPerBlock;
	}

	template<typename T>
	inline int DataFileManager<T>::getNumberOfData(void) const
	{
		return numberOfData;
	}

	template<typename T>
	inline int DataFileManager<T>::getNextSlot(void) const
	{
		return nextSlot;
	}

	template<typename T>
	inline int DataFileManager<T>::getLastSlot(void) const
	{
		return lastSlot;
	}

	template<typename T>
	inline const T* DataFileManager<T>::getDataList(void) const
	{
		return dataList;
	}

	template<typename T>
	inline bool DataFileManager<T>::isFull(void) const
	{
		return numberOfData==dataPerBlock;
	}
	
	template<typename T>
	blockId_t DataFileManager<T>::insertData(const T& data)
	{
		if(isFull())
		{
			storeBlock(currentBlock);
			currentBlock = newBlock();
			numberOfData = 0;
			nextSlot = 0;
			lastSlot = 0;
		}

		int idx = nextSlot;
		numberOfData++;
		nextSlot = (numberOfData>lastSlot) ? lastSlot+=1 : *((int*)(dataList+nextSlot));
		dataList[idx] = data;
		return currentBlock;
	}
}

#endif