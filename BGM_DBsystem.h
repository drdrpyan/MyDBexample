#ifndef __BGM_DB_SYSTEM__
#define __BGM_DB_SYSTEM__

#define BLOCKSIZE 4096
#define MAX_BLOCK_NUMBER 40139882
#define STUDENTS_PER_BLOCK 107
typedef unsigned blockId_t;

#include <vector>
using std::vector;
typedef struct {
    unsigned blockNumber;
    unsigned studentId;
} SaveInfo;

#include <iostream>
#include <fstream>

#endif