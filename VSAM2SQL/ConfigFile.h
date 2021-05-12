#include <afx.h>

#ifndef _CONFIGFILE_H_
#define _CONFIGFILE_H_

typedef struct {
	int test_type;
	union {
		struct { int start; int length; UCHAR teststring[100]; } vsam_string;
		struct { int start; int testint; } vsam_int;
		struct { int start; int testsmall; } vsam_small;
		struct { int start; int length; int prec; UCHAR testdec[100]; } vsam_dec;
		struct { int start; int length; int prec; UCHAR testnum[100]; } vsam_num;
	} data;
	TCHAR compare;
} TEST;


typedef struct {
	int vsam_type;
	union {
		struct { int start; int length; } vsam_string;
		struct { int start; } vsam_int;
		struct { int start; } vsam_small;
		struct { int start; int length; int prec; } vsam_dec;
		struct { int start; int length; int prec; } vsam_num;
		struct { int start; int length; int prec; } vsam_zoned;
		struct { int start; } vsam_date;
	} data;
	int nullable;
	TEST test;
} VSAM_DATA ;

class ConfigFile
{
private:
	int record_len;
	int no_fields;
	int test_record;
	TEST recordtest;
	VSAM_DATA fieldconfig[500];
	CStdioFile conffile;
	CString dnsodbc, connectodbc, sqlinsert;
	void SetNullInfo(TEST *testinfo, int startidx);
	void ReadConfigLine(int line_no);
public:
	ConfigFile(LPCTSTR filename);
	int GetRecordLen();
	int GetFields();
	VSAM_DATA *GetFieldConfig(int field_no);
	TEST *GetRecordTest();
	CString& GetInsert();
};

#endif