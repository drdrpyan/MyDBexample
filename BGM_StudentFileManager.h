#ifndef __BGM_STUDENTSFILEMANAGER__
#define __BGM_STUDENTSFILEMANAGER__

// numberOfEntry	nextSlot	mask		empty			students
// 0 (4byte)		4 (4byte)	8(4byte)	12 (18byte)		30 (38*107 = 4066byte)

#include "BGM_DataFileManager.h"
#include "Student.h"

#define STUDENTFILEMANAGER_MASK_OFFSET 12
#define DATALIST_OFFSET 30
#define STUDENTS_PER_BLOCK 107


namespace BGM
{
	class StudentFileManager : public DataFileManager<Student>
	{
	private:
		unsigned & mask;

		virtual Student searchData(const Student& student); //예외처리 불가. 사용 안함
		virtual void deleteData(const Student& student);
	public:
		StudentFileManager(const char* fileName);
		unsigned getMask(void) const;
		void setMask(unsigned m);
		bool isFull() const;
		void deleteData(signed int index);
		void deleteData(unsigned id);

		float getScore(blockId_t block, unsigned ID);

		//for debug
		void printBlock();
	};

	inline bool StudentFileManager::isFull() const
	{
		return numberOfData==STUDENTS_PER_BLOCK;
	}
}

#endif