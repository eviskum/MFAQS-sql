// VSAM2SQL.cpp : Defines the initialization routines for the DLL.
//

#include "Stdafx.h"
#include <afxtempl.h>
#include "VSAM2SQL.h"
#include <fstream.h>
#include "ConfigFile.h"
#include "TransData.h"
#include "SQLData.h"
#include "DataTest.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CVSAM2SQLApp

BEGIN_MESSAGE_MAP(CVSAM2SQLApp, CWinApp)
	//{{AFX_MSG_MAP(CVSAM2SQLApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSAM2SQLApp construction

CVSAM2SQLApp::CVSAM2SQLApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CVSAM2SQLApp object

CVSAM2SQLApp theApp;



CArray<CString, CString> dataout;


#define DLLSUCCES	1
#define DLLERROR	2
#define LOGNORMAL	1
#define LOGERROR	2
#define LNORM( txt )	Log(txt, LOGNORMAL, FALSE);
#define LERR( txt )		Log(txt, LOGERROR, FALSE);
#define LJNORM( txt )	Log(txt, LOGNORMAL, TRUE);
#define LJERR( txt )	Log(txt, LOGERROR, TRUE);

HANDLE hLogEvent, hLogReady;
TCHAR log_txt[500];
int log_mode, log_job;
int configsucces, transsucces;

/* DEBUG */
void PD (int i);


void Log(LPCTSTR txt, int mode, int job) {
	WaitForSingleObject(hLogReady, INFINITE);
	strcpy(log_txt, txt);
	log_mode = mode;
	log_job = job;
	SetEvent(hLogEvent);
}

extern "C" int PASCAL EXPORT DllGetLog(LPTSTR txt, int *mode, int *job)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	strcpy(txt, log_txt);
	*mode = log_mode;
	*job = log_job;
	return(0);
}

extern "C" int PASCAL EXPORT DllLogReg( HANDLE set_event, HANDLE set_ready )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	hLogEvent = set_event;
	hLogReady = set_ready;
	return(0);
}

extern "C" int PASCAL EXPORT DllWorker ( int argc, char *argv[] )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	TCHAR tmp_str[200];

/*DEBUG*/
PD(1);

	LJNORM("VSAM indlasenings --> temp fil utility")
	configsucces = TRUE; transsucces = TRUE;
/*DEBUG*/
PD(2);
	ConfigFile conffile(argv[0]);
	if (configsucces == FALSE) return (DLLERROR);
/*DEBUG*/
PD(30);
	TransData datatrans(argv[1]);
/*DEBUG*/
PD(40);
	if (transsucces == FALSE) return (DLLERROR);
	CFile datafile;
	int bytes;
	UCHAR buf[5000];
	UCHAR buf2[5000];

/*DEBUG*/
PD(50);
	LJNORM("Begynder datakonvertering")
	CString inputic1file(argv[2]);
	int ic1lockindex = inputic1file.Find("ic1lock",0);
	CString inputfile = inputic1file.Left(ic1lockindex) + inputic1file.Mid(ic1lockindex+7);
	if (!datafile.Open(inputfile, CFile::modeRead)) {
		LJERR("FEJL i indlaesning af inddata-fil. ")
		return(DLLERROR);
	}
	TRY
	{
		bytes = datafile.Read(buf, conffile.GetRecordLen());
	}
	CATCH( CFileException, e )
	{
		LJERR("FEJL: kunne ikke laese fra datafil")
		datafile.Close();
		return(DLLERROR);
	}
	END_CATCH

	VSAM_DATA *vsam;
	TEST *testdata;
	int record_ok;

	Sleep(5000);

	TRY
	{
		CFile::Remove(argv[2]);
	}
	CATCH( CFileException, e )
	{
		LJERR("FEJL: kunne ikke slette trigger-fil")
		datafile.Close();
		return(DLLERROR);
	}
	END_CATCH
	CStdioFile tempfile;
	if (!tempfile.Open(argv[3], CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive)) {
		LJERR("FEJL i aabning af temp-fil. ")
		datafile.Close();
		return(DLLERROR);
	}
	int records = 0;

	while (bytes == conffile.GetRecordLen()) {
		testdata = conffile.GetRecordTest();
		if (testdata == NULL) record_ok = TRUE;
		else {
			record_ok = FALSE;
			switch(testdata->test_type) {
			case SQL_STRING:
				memcpy(buf2, &buf[testdata->data.vsam_string.start],
					testdata->data.vsam_string.length);
				if (StringTest(datatrans, buf2, testdata)) record_ok = TRUE;
				break;
			case SQL_INT:
				memcpy(buf2, &buf[testdata->data.vsam_int.start], 4);
				if (IntTest(datatrans, buf2, testdata)) record_ok = TRUE;
				break;
			case SQL_SMALL:
				memcpy(buf2, &buf[testdata->data.vsam_small.start], 4);
				if (SmallTest(datatrans, buf2, testdata)) record_ok = TRUE;
				break;
			case SQL_DEC:
				memcpy(buf2, &buf[testdata->data.vsam_dec.start],
					testdata->data.vsam_dec.length);
				if (DecTest(datatrans, buf2, testdata)) record_ok = TRUE;
				break;
			case SQL_NUM:
				memcpy(buf2, &buf[testdata->data.vsam_num.start],
					testdata->data.vsam_num.length);
				if (NumTest(datatrans, buf2, testdata)) record_ok = TRUE;
				break;
			}
		}
		if (record_ok == TRUE) {
			dataout.RemoveAll();
			for (int i = 0; i < conffile.GetFields(); i++) {
				vsam = conffile.GetFieldConfig(i);
				switch(vsam->vsam_type) {
				case SQL_STRING:
					if (vsam->nullable && TestField(datatrans, buf, &(vsam->test)) == FALSE) {
						dataout.Add(CString("NULL"));
					} else {
						memcpy(buf2, &buf[vsam->data.vsam_string.start], vsam->data.vsam_string.length);
						{
							SQLData data(datatrans, buf2, SQL_STRING, vsam->data.vsam_string.length);
							for (int k = 0; k < data.GetLength(); k++)
								if (data.GetAt(k) == '\'') data.SetAt(k, '´');
							dataout.Add(CString("'") + data + "'");
						}
					}
					break;
				case SQL_INT:
					if (vsam->nullable && TestField(datatrans, buf, &(vsam->test)) == FALSE) {
						dataout.Add(CString("NULL"));
					} else {
						memcpy(buf2, &buf[vsam->data.vsam_int.start], 4);
						{
							SQLData data(datatrans, buf2, SQL_INT);
							dataout.Add(data);
						}
					}
					break;
				case SQL_SMALL:
					if (vsam->nullable && TestField(datatrans, buf, &(vsam->test)) == FALSE) {
						dataout.Add(CString("NULL"));
					} else {
						memcpy(buf2, &buf[vsam->data.vsam_small.start], 2);
						{
							SQLData data(datatrans, buf2, SQL_SMALL);
							dataout.Add(data);
						}
					}
					break;
				case SQL_DEC:
					if (vsam->nullable && TestField(datatrans, buf, &(vsam->test)) == FALSE) {
						dataout.Add(CString("NULL"));
					} else {
						memcpy(buf2, &buf[vsam->data.vsam_dec.start], vsam->data.vsam_dec.length);
						{
							SQLData data(datatrans, buf2, SQL_DEC, vsam->data.vsam_dec.length, vsam->data.vsam_dec.prec);
							dataout.Add(data);
						}
					}
					break;
				case SQL_NUM:
					if (vsam->nullable && TestField(datatrans, buf, &(vsam->test)) == FALSE) {
						dataout.Add(CString("NULL"));
					} else {
						memcpy(buf2, &buf[vsam->data.vsam_num.start], vsam->data.vsam_num.length);
						{
							SQLData data(datatrans, buf2, SQL_NUM, vsam->data.vsam_num.length, vsam->data.vsam_num.prec);
							dataout.Add(data);
						}
					}
					break;
				case SQL_ZONED:
					if (vsam->nullable && TestField(datatrans, buf, &(vsam->test)) == FALSE) {
						dataout.Add(CString("NULL"));
					} else {
						memcpy(buf2, &buf[vsam->data.vsam_zoned.start], vsam->data.vsam_zoned.length+1);
						{
							SQLData data(datatrans, buf2, SQL_ZONED, vsam->data.vsam_zoned.length, vsam->data.vsam_zoned.prec);
							dataout.Add(data);
						}
					}
					break;
				case SQL_DATE_1:
				case SQL_DATE_2:
				case SQL_DATE_7:
				case SQL_DATE_8:
					if (vsam->nullable && TestField(datatrans, buf, &(vsam->test)) == FALSE) {
						dataout.Add(CString("NULL"));
					} else {
						memcpy(buf2, &buf[vsam->data.vsam_dec.start], 8);
						{
							SQLData data(datatrans, buf2, vsam->vsam_type, vsam->data.vsam_dec.length,
								vsam->data.vsam_dec.prec);
							dataout.Add(CString("'") + data + "'");
						}
					}
					break;
				case SQL_DATE_3:
				case SQL_DATE_4:
					if (vsam->nullable && TestField(datatrans, buf, &(vsam->test)) == FALSE) {
						dataout.Add(CString("NULL"));
					} else {
						memcpy(buf2, &buf[vsam->data.vsam_dec.start], 6);
						{
							SQLData data(datatrans, buf2, vsam->vsam_type, vsam->data.vsam_dec.length,
								vsam->data.vsam_dec.prec);
							dataout.Add(CString("'") + data + "'");
						}
					}
					break;
				case SQL_DATE_5:
				case SQL_DATE_6:
					if (vsam->nullable && TestField(datatrans, buf, &(vsam->test)) == FALSE) {
						dataout.Add(CString("NULL"));
					} else {
						memcpy(buf2, &buf[vsam->data.vsam_dec.start], 10);
						{
							SQLData data(datatrans, buf2, vsam->vsam_type, vsam->data.vsam_dec.length,
								vsam->data.vsam_dec.prec);
							dataout.Add(CString("'") + data + "'");
						}
					}
					break;
				}
			}

			{
				TCHAR sql_insert[5000], tmpstr[5000];
				strcpy(sql_insert, conffile.GetInsert());
				int k = 0, j = 0;
				for (i = 0; i < conffile.GetFields(); i++) {
					for(;(sql_insert[j] != '%' && j < (int) strlen(sql_insert)); j++, k++)
						tmpstr[k] = sql_insert[j];
					tmpstr[k] = '\0'; j++;
					strcat(tmpstr, dataout[i]);
					k = strlen(tmpstr);
				}
				while (sql_insert[j] != '\0') tmpstr[k++] = sql_insert[j++];
				tmpstr[k] = '\n';
				tmpstr[k+1] = '\0';
				TRY
				{
					tempfile.WriteString(tmpstr);
				}
				CATCH( CFileException, e )
				{
					LJERR("FEJL: kunne ikke skrive til temp-fil")
					tempfile.Close();
					datafile.Close();
					return(DLLERROR);
				}
				END_CATCH
				records++;
			}
		}
		TRY
		{
			bytes = datafile.Read(buf, conffile.GetRecordLen());
		}
		CATCH( CFileException, e )
		{
			LJERR("FEJL: kunne ikke skrive til temp-fil")
			tempfile.Close();
			datafile.Close();
			return(DLLERROR);
		}
		END_CATCH
	}
	tempfile.Close();
	datafile.Close();
	sprintf(tmp_str, "Slut -- har flyttet %d records", records);
	LJNORM(tmp_str)
	return(DLLSUCCES);
}
