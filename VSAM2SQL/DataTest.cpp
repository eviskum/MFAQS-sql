#include "StdAfx.h"

#include <stdlib.h>
#include "ConfigFile.h"
#include "SQLData.h"

int StringTest(TransData& datatrans, unsigned char buf[], TEST *testdata) {
	SQLData data(datatrans, buf, SQL_STRING, testdata->data.vsam_string.length);
	if (testdata->compare == '>') {
		if (data > testdata->data.vsam_string.teststring) return TRUE;
	} else if (testdata->compare == '=') {
		if (data == testdata->data.vsam_string.teststring) return TRUE;
	} else if (testdata->compare == '<') {
		if (data < testdata->data.vsam_string.teststring) return TRUE;
	}
	return FALSE;
}

int IntTest(TransData& datatrans, unsigned char buf[], TEST *testdata) {
	SQLData data(datatrans, buf, SQL_INT);
	if (testdata->compare == '>') {
		if (atoi(data) > testdata->data.vsam_int.testint) return TRUE;
	} else if (testdata->compare == '=') {
		if (atoi(data) == testdata->data.vsam_int.testint) return TRUE;
	} else if (testdata->compare == '<') {
		if (atoi(data) < testdata->data.vsam_int.testint) return TRUE;
	}
	return FALSE;
}

int SmallTest(TransData& datatrans, unsigned char buf[], TEST *testdata) {
	SQLData data(datatrans, buf, SQL_SMALL);
	if (testdata->compare == '>') {
		if (atoi(data) > testdata->data.vsam_small.testsmall) return TRUE;
	} else if (testdata->compare == '=') {
		if (atoi(data) == testdata->data.vsam_small.testsmall) return TRUE;
	} else if (testdata->compare == '<') {
		if (atoi(data) < testdata->data.vsam_small.testsmall) return TRUE;
	}
	return FALSE;
}

int DecTest(TransData& datatrans, unsigned char buf[], TEST *testdata) {
	SQLData data(datatrans, buf, SQL_DEC, testdata->data.vsam_dec.length, testdata->data.vsam_dec.prec);
	if (testdata->compare == '>') {
		if (atof(data) > atof((char *) testdata->data.vsam_dec.testdec)) return TRUE;
	} else if (testdata->compare == '=') {
		if (atof(data) == atof((char *) testdata->data.vsam_dec.testdec)) return TRUE;
	} else if (testdata->compare == '<') {
		if (atof(data) < atof((char *) testdata->data.vsam_dec.testdec)) return TRUE;
	}
	return FALSE;
}

int NumTest(TransData& datatrans, unsigned char buf[], TEST *testdata) {
	SQLData data(datatrans, buf, SQL_NUM, testdata->data.vsam_num.length, testdata->data.vsam_num.prec);
	if (testdata->compare == '>') {
		if (atof(data) > atof((char *) testdata->data.vsam_num.testnum)) return TRUE;
	} else if (testdata->compare == '=') {
		if (atof(data) == atof((char *) testdata->data.vsam_num.testnum)) return TRUE;
	} else if (testdata->compare == '<') {
		if (atof(data) < atof((char *) testdata->data.vsam_num.testnum)) return TRUE;
	}
	return FALSE;
}

int TestField(TransData& datatrans, unsigned char buf[], TEST *testdata) {
	UCHAR buf2[5000];
	int record_ok = TRUE;
	switch(testdata->test_type) {
	case SQL_STRING:
		memcpy(buf2, &buf[testdata->data.vsam_string.start],
			testdata->data.vsam_string.length);
		if (StringTest(datatrans, buf2, testdata)) record_ok = FALSE;
		break;
	case SQL_INT:
		memcpy(buf2, &buf[testdata->data.vsam_int.start], 4);
		if (IntTest(datatrans, buf2, testdata)) record_ok = FALSE;
		break;
	case SQL_SMALL:
		memcpy(buf2, &buf[testdata->data.vsam_small.start], 4);
		if (SmallTest(datatrans, buf2, testdata)) record_ok = FALSE;
		break;
	case SQL_DEC:
		memcpy(buf2, &buf[testdata->data.vsam_dec.start],
			testdata->data.vsam_dec.length);
		if (DecTest(datatrans, buf2, testdata)) record_ok = FALSE;
		break;
	case SQL_NUM:
		memcpy(buf2, &buf[testdata->data.vsam_num.start],
			testdata->data.vsam_num.length);
		if (NumTest(datatrans, buf2, testdata)) record_ok = FALSE;
		break;
	}
	return record_ok;
}