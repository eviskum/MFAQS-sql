// SQLINP.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "SQLINP.h"
#include <stdio.h>
#include <fstream.h>


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
// CSQLINPApp

BEGIN_MESSAGE_MAP(CSQLINPApp, CWinApp)
	//{{AFX_MSG_MAP(CSQLINPApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSQLINPApp construction

CSQLINPApp::CSQLINPApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSQLINPApp object

CSQLINPApp theApp;




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
CString tmpstr;

int readconfig(char sqlfile[], int mode)
{
	TCHAR tmp_str[1000];
	int returnval = TRUE;
	TRY
	{
		CStdioFile SQLFile(sqlfile, CFile::modeRead | CFile::shareDenyWrite);
		if (mode == 1) {
			SQLFile.ReadString(comment);
			SQLFile.ReadString(db2_handle);
			SQLFile.ReadString(db2_connect);
			SQLFile.ReadString(sql_handle);
			SQLFile.ReadString(sql_connect);
			SQLFile.ReadString(db2_select);
			SQLFile.ReadString(sql_once);
			SQLFile.ReadString(sql_insert);
		} else {
			SQLFile.ReadString(comment);
			SQLFile.ReadString(sql_handle);
			SQLFile.ReadString(sql_connect);
			SQLFile.ReadString(sql_once);
			SQLFile.ReadString(sql_insert);
		}
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
	TCHAR tmp_str[1000];

	LJNORM("Starter MS SQL import utility")

	if (argc != 2 && argc != 3 && argc != 4) {
		LJERR("Forkert brug !!!! Korrekt brug: SQLINP conf.sql data.dat [SQL|VSAM] [update]")
		return(DLLERROR);
	}

	CString update;

	if (argc == 2) {
		if (readconfig(argv[0], 1) == FALSE) return (DLLERROR);
	} else {
		if (strcmp(argv[2], "VSAM") == 0 || strcmp(argv[2], "vsam") == 0) {
			if (readconfig(argv[0], 2) == FALSE) return (DLLERROR);
			if (argc == 4) update = argv[3];
		} else if (strcmp(argv[2], "SQL") == 0 || strcmp(argv[2], "sql") == 0) {
			if (readconfig(argv[0], 1) == FALSE) return (DLLERROR);
			if (argc == 4) update = argv[3];
		} else {
			if (argc == 4) {
				LJERR("For mange parametre")
				return (DLLERROR);
			}
			update = argv[2];
			if (readconfig(argv[0], 1) == FALSE) return (DLLERROR);
		}
	}

	CDatabase MSSQL;

	TRY {
		LJNORM(" connecter MS-SQL... ")
		MSSQL.Open(sql_handle, FALSE, FALSE, sql_connect, FALSE);
	}
	CATCH(CDBException, e) {
		sprintf(tmp_str,"FEJL i ODBC-connect: %s", e->m_strError); 
		LJERR(tmp_str)
		return(DLLERROR);
	}
	END_CATCH
	
	int records = 0;

	int retry = 0;
	TRY {
		MSSQL.SetQueryTimeout(300);
		MSSQL.ExecuteSQL(sql_once);
	}
	CATCH(CDBException, e) {
		CString errortxt = e->m_strError;
		if (errortxt.Find("Timeout") >= 0) {
			retry = 1;
			LJNORM(" timeout - proever igen...")
		} else {
			sprintf(tmp_str, "FEJL i sql-eksekvering: %s", e->m_strError);
			LJERR(tmp_str)
			return(DLLERROR);
		}
	}
	END_CATCH
	if (retry == 1) {
		retry = 0;
		TRY {
			MSSQL.ExecuteSQL(sql_once);
		}
		CATCH(CDBException, e) {
			CString errortxt = e->m_strError;
			if (errortxt.Find("Timeout") >= 0) {
				retry = 1;
				LJNORM(" timeout - proever sidste gang...")
			} else {
				sprintf(tmp_str, "FEJL i sql-eksekvering: %s", e->m_strError);
				LJERR(tmp_str)
				return(DLLERROR);
			}
		}
		END_CATCH
	}
	if (retry == 1) {
		TRY {
			MSSQL.ExecuteSQL(sql_once);
		}
		CATCH(CDBException, e) {
			sprintf(tmp_str, "FEJL i sql-eksekvering: %s", e->m_strError);
			LJERR(tmp_str)
			return(DLLERROR);
		}
		END_CATCH
	}

	CStdioFile TEMPFILE;
	CFileException e;
	if (!TEMPFILE.Open (argv[1], CFile::modeRead | CFile::shareDenyWrite, &e)) {
		sprintf(tmp_str, "Kunne ikke hente datafil: '%s' - proever igen", argv[1]);
		LJNORM(tmp_str)
		Sleep(500000);
		if (!TEMPFILE.Open (argv[1], CFile::modeRead | CFile::shareDenyWrite, &e)) {
			sprintf(tmp_str, "Kunne ikke hente datafil: '%s'", argv[1]);
			LJERR(tmp_str)
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
			return(DLLERROR);
		}
	}
	
	while (TEMPFILE.ReadString(tmpstr)) {
		TRY {
			MSSQL.ExecuteSQL(tmpstr);
		}
		CATCH(CDBException, e) {
			sprintf(tmp_str, "FEJL i MSSQL insert: %s", e->m_strError);
			LJERR(tmp_str)
			MSSQL.Close();
			return(DLLERROR);
		}
		END_CATCH
		if ((records++ % 10000) == 0 && records > 1) {
			sprintf(tmp_str, "..har flyttet %d records", records-1);
			LNORM(tmp_str)
		}
	}
	TEMPFILE.Close();

	if (update.GetLength() > 0) {
		CString update_str = "update warehouse.dwhm.tabel_statistik set sidsteopdatering = GetDate() where tabel = '";
		update_str += update;
		update_str += "'";
		TRY {
			MSSQL.ExecuteSQL(update_str);
		}
		CATCH(CDBException, e) {
			sprintf(tmp_str, "FEJL i MSSQL insert: %s", e->m_strError);
			LJERR(tmp_str)
			MSSQL.Close();
			return(DLLERROR);
		}
		END_CATCH
	}
	sprintf(tmp_str, "Slut -- har flyttet %d records", records);
	LNORM(tmp_str)
	MSSQL.Close();
	return(DLLSUCCES);
}
