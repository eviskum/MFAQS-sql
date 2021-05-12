#include "StdAfx.h"
#include <afxtempl.h>
#include <fstream.h>
#include "SQLData.h"
#include "ConfigFile.h"


#define DLLSUCCES	1
#define DLLERROR	2
#define LOGNORMAL	1
#define LOGERROR	2
#define LNORM( txt )	Log(txt, LOGNORMAL, FALSE);
#define LERR( txt )		Log(txt, LOGERROR, FALSE);
#define LJNORM( txt )	Log(txt, LOGNORMAL, TRUE);
#define LJERR( txt )	Log(txt, LOGERROR, TRUE);

extern void Log(LPCTSTR txt, int mode, int job);
extern int configsucces;

/*DEBUG*/
void PD (int i);


CArray<CString,CString> cmd_lex;

void CommandLexer(LPCTSTR command)
{
	int pingmode;
	CString cmd(command);
	cmd_lex.RemoveAll();
	int start_idx = 0, end_idx = 0, length = cmd.GetLength();
	while (end_idx < length-1) {
		pingmode = FALSE;
		while (start_idx < length && cmd[start_idx] == ' ') start_idx++;
		if (cmd[start_idx] == '\'') {
			pingmode = TRUE;
			end_idx = start_idx + 1;
		} else end_idx = start_idx;
		while (end_idx < length && ((pingmode == FALSE && cmd[end_idx] != ' ') ||
									(pingmode && cmd[end_idx] != '\''))) end_idx++;
		if (start_idx < length) {
			if (pingmode) cmd_lex.Add(cmd.Mid(start_idx+1, end_idx-start_idx-1));
			else cmd_lex.Add(cmd.Mid(start_idx, end_idx-start_idx));
		}
		start_idx = end_idx;
	}
}

ConfigFile::ConfigFile(LPCTSTR filename)
{
	TCHAR buf[5000];
/*DEBUG*/
PD(3);

	CFileException e;
	if (!conffile.Open(filename, CFile::modeRead, &e)) {
		LJERR("FEJL i aabning af vsam-config-fil. ")
		switch (e.m_cause) {
		case CFileException::none:
			LJERR("Aarsag: No error occurred.") break;
		case CFileException::generic:
			LJERR("Aarsag: An unspecified error occurred.") break;
		case CFileException::fileNotFound:
			LJERR("Aarsag: The file could not be located.") break;
		case CFileException::badPath:
			LJERR("Aarsag: All or part of the path is invalid.") break;
		case CFileException::tooManyOpenFiles:
			LJERR("Aarsag: The permitted number of open files was exceeded.") break;
		case CFileException::accessDenied:
			LJERR("Aarsag: The file could not be accessed.") break;
		case CFileException::invalidFile:
			LJERR("Aarsag: There was an attempt to use an invalid file handle.") break;
		case CFileException::removeCurrentDir:
			LJERR("Aarsag: The current working directory cannot be removed.") break;
		case CFileException::directoryFull:
			LJERR("Aarsag: There are no more directory entries.") break;
		case CFileException::badSeek:
			LJERR("Aarsag: There was an error trying to set the file pointer.") break;
		case CFileException::hardIO:
			LJERR("Aarsag: There was a hardware error.") break;
		case CFileException::sharingViolation:
			LJERR("Aarsag: SHARE.EXE was not loaded, or a shared region was locked.") break;
		case CFileException::lockViolation:
			LJERR("Aarsag: There was an attempt to lock a region that was already locked.") break;
		case CFileException::diskFull:
			LJERR("Aarsag: The disk is full.") break;
		case CFileException::endOfFile:
			LJERR("Aarsag: The end of file was reached.") break;
		}
		configsucces = FALSE;
	}
	if (configsucces == TRUE) {
		TRY {
/*DEBUG*/
PD(4);
			conffile.ReadString(buf, 5000);
			conffile.ReadString(buf, 5000);
			buf[strlen(buf)-1] = '\0';
			dnsodbc = buf;
			conffile.ReadString(buf, 5000);
			buf[strlen(buf)-1] = '\0';
			connectodbc = buf;
			conffile.ReadString(buf, 5000);
			conffile.ReadString(buf, 5000);
			buf[strlen(buf)-1] = '\0';
			sqlinsert = buf;
			conffile.ReadString(buf, 5000);
			record_len = atoi(buf);
			conffile.ReadString(buf, 5000);
			no_fields = atoi(buf);
			conffile.ReadString(buf, 5000);
			buf[strlen(buf)-1] = '\0';
/*DEBUG*/
PD(5);
		}
		CATCH(CFileException, e) {
			LJERR("FEJL i indlaesning af vsam-config-fil. ")
			conffile.Close();
			configsucces = FALSE;
		}
		END_CATCH
	}
	if (configsucces == TRUE) {
/*DEBUG*/
PD(6);
		CommandLexer(buf);
		if (cmd_lex[0] == "RECORDTEST") {
			if (cmd_lex[1] == "STRING") {
				recordtest.test_type =  SQL_STRING;
				recordtest.data.vsam_string.start = atoi(cmd_lex[2]) - 1;
				recordtest.data.vsam_string.length = atoi(cmd_lex[3]);
				recordtest.compare = cmd_lex[4][0];
				strcpy((char *) recordtest.data.vsam_string.teststring, cmd_lex[5]);
			} else if (cmd_lex[1] == "INT") {
				recordtest.test_type =  SQL_INT;
				recordtest.data.vsam_int.start = atoi(cmd_lex[2]) - 1;
				recordtest.compare = cmd_lex[3][0];
				recordtest.data.vsam_int.testint = atoi(cmd_lex[4]);
			} else if (cmd_lex[1] == "SMALL" || cmd_lex[1] == "SMALLINT") {
				recordtest.test_type =  SQL_SMALL;
				recordtest.data.vsam_int.start = atoi(cmd_lex[2]) - 1;
				recordtest.compare = cmd_lex[3][0];
				recordtest.data.vsam_small.testsmall = atoi(cmd_lex[4]);
			} else if (cmd_lex[1] == "DEC") {
				recordtest.test_type =  SQL_DEC;
					recordtest.data.vsam_dec.start = atoi(cmd_lex[2]) - 1;
				recordtest.data.vsam_dec.length = atoi(cmd_lex[3]);
				recordtest.data.vsam_dec.prec = atoi(cmd_lex[4]);
				recordtest.compare = cmd_lex[5][0];
				strcpy((char *) recordtest.data.vsam_dec.testdec, cmd_lex[6]);
			} else if (cmd_lex[1] == "NUM") {
				recordtest.test_type =  SQL_NUM;
				recordtest.data.vsam_num.start = atoi(cmd_lex[2]) - 1;
				recordtest.data.vsam_num.length = atoi(cmd_lex[3]);
				recordtest.data.vsam_num.prec = atoi(cmd_lex[4]);
				recordtest.compare = cmd_lex[5][0];
				strcpy((char *) recordtest.data.vsam_num.testnum, cmd_lex[6]);
			}
			test_record = TRUE;
		} else {
			test_record = FALSE;
		}
/*DEBUG*/
PD(7);
	}
	if (configsucces == TRUE) {
/*DEBUG*/
PD(8);
		for (int i = 0; i < no_fields; i++) {
			if (configsucces == TRUE) ReadConfigLine(i);
		}
/*DEBUG*/
PD(15);
		conffile.Close();
	}
/*DEBUG*/
PD(16);
}

void ConfigFile::ReadConfigLine(int line_no)
{
	TCHAR buf[5000];
/*DEBUG*/
PD(9);
	TRY {
		conffile.ReadString(buf, 5000);
	}
	CATCH(CFileException, e) {
		LJERR("FEJL i indlaesning af vsam-config-fil. ")
		conffile.Close();
		configsucces = FALSE;
	}
	END_CATCH
/*DEBUG*/
PD(10);
	if (configsucces == TRUE) {
/*DEBUG*/
PD(11);
		buf[strlen(buf)-1] = '\0';
		CommandLexer(buf);
/*DEBUG*/
PD(12);
		if (cmd_lex[0] == "STRING") {
			fieldconfig[line_no].vsam_type =  SQL_STRING;
			fieldconfig[line_no].data.vsam_string.start = atoi(cmd_lex[1]) - 1;
			fieldconfig[line_no].data.vsam_string.length = atoi(cmd_lex[2]);
			if (cmd_lex.GetSize() > 3 && cmd_lex[3] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 4);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "INT") {
			fieldconfig[line_no].vsam_type =  SQL_INT;
			fieldconfig[line_no].data.vsam_int.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
					fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "SMALL" || cmd_lex[0] == "SMALLINT") {
			fieldconfig[line_no].vsam_type =  SQL_SMALL;
			fieldconfig[line_no].data.vsam_small.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
					fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "DEC") {
			fieldconfig[line_no].vsam_type =  SQL_DEC;
			fieldconfig[line_no].data.vsam_dec.start = atoi(cmd_lex[1]) - 1;
			fieldconfig[line_no].data.vsam_dec.length = atoi(cmd_lex[2]);
			fieldconfig[line_no].data.vsam_dec.prec = atoi(cmd_lex[3]);
			if (cmd_lex.GetSize() > 4 && cmd_lex[4] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 5);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "NUM") {
			fieldconfig[line_no].vsam_type =  SQL_NUM;
			fieldconfig[line_no].data.vsam_num.start = atoi(cmd_lex[1]) - 1;
			fieldconfig[line_no].data.vsam_num.length = atoi(cmd_lex[2]);
			fieldconfig[line_no].data.vsam_num.prec = atoi(cmd_lex[3]);
			if (cmd_lex.GetSize() > 4 && cmd_lex[4] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 5);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "ZONED") {
			fieldconfig[line_no].vsam_type =  SQL_ZONED;
			fieldconfig[line_no].data.vsam_zoned.start = atoi(cmd_lex[1]) - 1;
			fieldconfig[line_no].data.vsam_zoned.length = atoi(cmd_lex[2]);
			fieldconfig[line_no].data.vsam_zoned.prec = atoi(cmd_lex[3]);
				if (cmd_lex.GetSize() > 4 && cmd_lex[4] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 5);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "DATE1") {
			fieldconfig[line_no].vsam_type =  SQL_DATE_1;
			fieldconfig[line_no].data.vsam_date.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "DATE2") {
			fieldconfig[line_no].vsam_type =  SQL_DATE_2;
			fieldconfig[line_no].data.vsam_date.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "DATE3") {
			fieldconfig[line_no].vsam_type =  SQL_DATE_3;
			fieldconfig[line_no].data.vsam_date.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "DATE4") {
			fieldconfig[line_no].vsam_type =  SQL_DATE_4;
			fieldconfig[line_no].data.vsam_date.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "DATE5") {
			fieldconfig[line_no].vsam_type =  SQL_DATE_5;
			fieldconfig[line_no].data.vsam_date.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "DATE6") {
			fieldconfig[line_no].vsam_type =  SQL_DATE_6;
			fieldconfig[line_no].data.vsam_date.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "DATE7") {
			fieldconfig[line_no].vsam_type =  SQL_DATE_7;
			fieldconfig[line_no].data.vsam_date.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		} else if (cmd_lex[0] == "DATE8") {
			fieldconfig[line_no].vsam_type =  SQL_DATE_8;
			fieldconfig[line_no].data.vsam_date.start = atoi(cmd_lex[1]) - 1;
			if (cmd_lex.GetSize() > 2 && cmd_lex[2] == "NULLTEST") {
				fieldconfig[line_no].nullable = TRUE;
				SetNullInfo(&fieldconfig[line_no].test, 3);
			} else {
				fieldconfig[line_no].nullable = FALSE;
			}
		}
/*DEBUG*/
PD(13);
	}
/*DEBUG*/
PD(14);
}

void ConfigFile::SetNullInfo(TEST *testinfo, int startidx)
{
/*DEBUG*/
PD(20);
	if (cmd_lex[startidx] == "STRING") {
		testinfo->test_type = SQL_STRING;
		testinfo->data.vsam_string.start = atoi(cmd_lex[startidx+1]) - 1;
		testinfo->data.vsam_string.length = atoi(cmd_lex[startidx+2]);
		testinfo->compare = cmd_lex[startidx+3][0];
		strcpy((char *) testinfo->data.vsam_string.teststring, cmd_lex[startidx+4]);
	} else if (cmd_lex[startidx] == "INT") {
		testinfo->test_type = SQL_INT;
		testinfo->data.vsam_int.start = atoi(cmd_lex[startidx+1]) - 1;
		testinfo->compare = cmd_lex[startidx+2][0];
		testinfo->data.vsam_int.testint, atoi(cmd_lex[startidx+3]);
	} else if (cmd_lex[startidx] == "SMALL" || cmd_lex[startidx] == "SMALLINT") {
		testinfo->test_type = SQL_SMALL;
		testinfo->data.vsam_small.start = atoi(cmd_lex[startidx+1]) - 1;
		testinfo->compare = cmd_lex[startidx+2][0];
		testinfo->data.vsam_small.testsmall, atoi(cmd_lex[startidx+3]);
	} else if (cmd_lex[startidx] == "DEC") {
		testinfo->test_type = SQL_DEC;
		testinfo->data.vsam_dec.start = atoi(cmd_lex[startidx+1]) - 1;
		testinfo->data.vsam_dec.length = atoi(cmd_lex[startidx+2]);
		testinfo->data.vsam_dec.prec = atoi(cmd_lex[startidx+3]);
		testinfo->compare = cmd_lex[startidx+4][0];
		strcpy((char *) testinfo->data.vsam_dec.testdec, cmd_lex[startidx+5]);
	}
/*DEBUG*/
PD(21);
}

int ConfigFile::GetFields()
{
	return(no_fields);
}

int ConfigFile::GetRecordLen()
{
	return(record_len);
}

VSAM_DATA *ConfigFile::GetFieldConfig(int field_no)
{
	return(&fieldconfig[field_no]);
}

TEST *ConfigFile::GetRecordTest()
{
	if (test_record == FALSE) return NULL;
	else return &recordtest;
}

CString& ConfigFile::GetInsert()
{
	return (sqlinsert);
}