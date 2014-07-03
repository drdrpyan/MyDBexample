#ifndef __STUDENT__
#define __STUDENT__

#define NAME_LENGTH 20
#define DEPT_LENGTH 10

#pragma pack(push, 1)
class Student
{
private:
	char name[NAME_LENGTH];
	unsigned ID;
	float score;
	char dept[DEPT_LENGTH];
public:
	Student(const char *name, unsigned id, float score, const char *dept);
	Student(const Student& student);
	const char* getName(void) const;
	unsigned getID(void) const;
	void setID(unsigned newID);
	float getScore(void) const;
	void setScore(float newScore);
	const char* getDept(void) const;
};
#pragma pack(pop)

inline const char* Student::getName(void) const
{
	return name;
}

inline unsigned Student::getID(void) const
{
	return ID;
}

inline void Student::setID(unsigned newID)
{
	ID = newID;
}

inline float Student::getScore(void) const
{
	return score;
}

inline void Student::setScore(float newScore)
{
	score = newScore;
}

inline const char* Student::getDept(void) const
{
	return dept;
}

#endif