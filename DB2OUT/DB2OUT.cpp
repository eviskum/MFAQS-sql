// DB2OUT.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "DB2OUT.h"
#include <fstream>
using namespace std;

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
// CDB2OUTApp

BEGIN_MESSAGE_MAP(CDB2OUTApp, CWinApp)
	//{{AFX_MSG_MAP(CDB2OUTApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDB2OUTApp construction

CDB2OUTApp::CDB2OUTApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDB2OUTApp object

CDB2OUTApp theApp;





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


CString comment;
CString db2_handle, db2_connect, sql_handle, sql_connect;
CString db2_select, sql_once, sql_insert;
char tmpstr[20000];

int readconfig(char sqlfile[])
{
	TCHAR tmp_str[1000];
	int returnval = TRUE;
	TRY
	{
		CStdioFile SQLFile(sqlfile, CFile::modeRead | CFile::shareDenyWrite);
		SQLFile.ReadString(comment);
		SQLFile.ReadString(db2_handle);
		SQLFile.ReadString(db2_connect);
		SQLFile.ReadString(sql_handle);
		SQLFile.ReadString(sql_connect);
		SQLFile.ReadString(db2_select);
		SQLFile.ReadString(sql_once);
		SQLFile.ReadString(sql_insert);
		SQLFile.Close();
	}
	CATCH(CFileException, e) {
		sprintf(tmp_str, "Kunne ikke hente configurationsfil: '%s'", sqlfile);
		LJERR(tmp_str)
		switch (e->m_cause) {
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
		returnval = FALSE;
	}
	END_CATCH
	return (returnval);
}


extern "C" int PASCAL EXPORT DllWorker ( int argc, char *argv[] )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	TCHAR tmp_str[200];

	LJNORM("Starter DB2 --> temp fil utility")

	if (argc != 3) {
		LJERR("Forkert brug !!!! Korrekt brug: DB2OUT conf.sql temp_fil.dat trigger")
		return(DLLERROR);
	}

	if (readconfig(argv[0]) == FALSE) return (DLLERROR);

	CStdioFile tempfile;
	if (!tempfile.Open(argv[1], CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite)) {
		LJNORM("Kunne ikke aabne fil - proever igen")
		Sleep(500000);
		if (!tempfile.Open(argv[1], CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite)) {
			LJERR("FEJL kunne ikke aabne temp-fil")
			return(DLLERROR);
		}
	}

	TRY
	{
		CFile::Remove(argv[2]);
	}
	CATCH( CFileException, e )
	{
		LJERR("FEJL: kunne ikke slette trigger-fil")
		tempfile.Close();
		return(DLLERROR);
	}
	END_CATCH

	CDatabase DB2;

	TRY {
		LJNORM(" connecter DB2... ")
		DB2.Open(db2_handle, FALSE, FALSE, db2_connect, FALSE);
	}
	CATCH(CDBException, e) {
		sprintf(tmp_str, "\nFEJL i ODBC-connect: %s\n", e->m_strError);
		LJERR(tmp_str)
		return(DLLERROR);
	}
	END_CATCH
	
	CRecordset DB2_Table(&DB2);
	LJNORM("fetcher DB2 recordset...");
	TRY {
		DB2_Table.Open(CRecordset::forwardOnly, db2_select);
	}
	CATCH(CDBException, e) {
		sprintf(tmp_str, "\nFEJL i DB2-fetch: %s\n", e->m_strError);
		LJERR(tmp_str)
		return(DLLERROR);
	}
	END_CATCH
	short fields = DB2_Table.GetODBCFieldCount( );

	int records = 0;

	if( !DB2_Table.IsBOF( ) ) {
		while ( !DB2_Table.IsEOF( ) ) {
			int k = 0, j = 0;
			for (short i = 0; i < fields; i++) {
				CString bufferstring;
				DB2_Table.GetFieldValue(i, bufferstring);
				for(;(sql_insert[j] != '%' && sql_insert[j] != '$' &&
					j < (int) strlen(sql_insert)); j++, k++)
					tmpstr[k] = sql_insert[j];
				tmpstr[k] = '\0'; j++;
				if (bufferstring.IsEmpty() == TRUE) {
					strcat(tmpstr, "NULL");
					k = strlen(tmpstr);
				} else {
					if (sql_insert[j-1] == '$') {
						tmpstr[k++] = '\'';
						tmpstr[k] = '\0';
					}
					strcat(tmpstr, bufferstring.GetBuffer(1024));
					bufferstring.ReleaseBuffer();
					for (unsigned int l = k; l < strlen(tmpstr); l++) if (tmpstr[l] == '\'') tmpstr[l] = '´';
					k = strlen(tmpstr);
					if (sql_insert[j-1] == '$') {
						tmpstr[k++] = '\'';
						tmpstr[k] = '\0';
					}
				}
			}
			while (sql_insert[j] != '\0') tmpstr[k++] = sql_insert[j++];
			tmpstr[k] = '\n';
			tmpstr[k+1] = '\0';

			tempfile.WriteString(tmpstr);
			
			if ((records++ % 10000) == 0 && records > 1) {
				sprintf(tmp_str, "...har hentet %d records", records-1);
				LJNORM(tmp_str)
			}
			DB2_Table.MoveNext( );
		}
	}

	tempfile.Close();

	sprintf(tmp_str, "Slut -- har flyttet %d records", records);
	LJNORM(tmp_str)
	DB2_Table.Close();
	DB2.Close();
	return(DLLSUCCES);
}




