#include "Student.h"

Student::Student(const char* name, unsigned ID, float score, const char* dept) : ID(ID), score(score)
{
	int i;
	for(i=0; name[i] && i<NAME_LENGTH; i++)
		(this->name)[i] = name[i];
	if(i<NAME_LENGTH)
		this->name[i] = '\0';

	for(i=0; dept[i] && i<DEPT_LENGTH; i++)
		(this->dept)[i] = dept[i];
	if(i<DEPT_LENGTH)
		this->dept[i] = '\0';
}

Student::Student(const Student& student) : ID(student.ID), score(student.score)
{
	int i;
	for(i=0; name[i] && i<NAME_LENGTH; i++)
		(this->name)[i] = name[i];
	if(i<NAME_LENGTH)
		this->name[i] = '\0';

	for(i=0; dept[i] && i<DEPT_LENGTH; i++)
		(this->dept)[i] = dept[i];
	if(i<DEPT_LENGTH)
		this->dept[i] = '\0';
}