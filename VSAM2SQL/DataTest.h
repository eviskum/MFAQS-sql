
#ifndef _DATATEST_H_
#define _DATATEST_H_

#include "SQLData.h"
#include "TransData.h"

int StringTest(TransData& datatrans, unsigned char buf[], TEST *testdata);
int IntTest(TransData& datatrans, unsigned char buf[], TEST *testdata);
int SmallTest(TransData& datatrans, unsigned char buf[], TEST *testdata);
int DecTest(TransData& datatrans, unsigned char buf[], TEST *testdata);
int NumTest(TransData& datatrans, unsigned char buf[], TEST *testdata);
int TestField(TransData& datatrans, unsigned char buf[], TEST *testdata);

#endif