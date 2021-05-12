// MiSQL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "MiSQL.h"

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
// CMiSQLApp

BEGIN_MESSAGE_MAP(CMiSQLApp, CWinApp)
	//{{AFX_MSG_MAP(CMiSQLApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiSQLApp construction

CMiSQLApp::CMiSQLApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMiSQLApp object

CMiSQLApp theApp;





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
CString sql_handle, sql_connect;
CString tmpstr;



void StatementlogOpen();
void Statementlog(LPCTSTR logentry);
void StatementlogClose();


extern "C" int PASCAL EXPORT DllWorker ( int argc, char *argv[] )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	TCHAR tmp_str[1000];

	LJNORM("Starter MiSQL import utility")

	if (argc < 1 || argc > 2) {
		LJERR("Forkert brug !!!! Korrekt brug: MiSQL conf.sql trigfile")
		return(DLLERROR);
	}

	if (argc > 1) {
		TRY
		{
			CFile::Remove(argv[1]);
		}
		CATCH( CFileException, e )
		{
			LJERR("FEJL: kunne ikke slette trigger-fil")
			return(DLLERROR);
		}
		END_CATCH
	}

//	StatementlogOpen();

	CStdioFile CONFFILE;
	CFileException e;
	if (!CONFFILE.Open (argv[0], CFile::modeRead | CFile::shareExclusive, &e)) {
		sprintf(tmp_str, "Kunne ikke hente datafil: '%s'", argv[0]);
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
//		StatementlogClose();
		return(DLLERROR);
	}

	CString comment, sql_handle, sql_connect, tmpstr, statement;
	CONFFILE.ReadString(comment);
	CONFFILE.ReadString(sql_handle);
	CONFFILE.ReadString(sql_connect);
	while (CONFFILE.ReadString(tmpstr)) statement += tmpstr;
	CONFFILE.Close();

	CDatabase MSSQL;

	TRY {
		LJNORM(" connecter MS-SQL... ")
		MSSQL.Open((LPCSTR) sql_handle, FALSE, FALSE, (LPCSTR) sql_connect, FALSE);
	}
	CATCH(CDBException, e) {
		sprintf(tmp_str,"FEJL i ODBC-connect: %s", e->m_strError); 
		LJERR(tmp_str)
//		StatementlogClose();
		return(DLLERROR);
	}
	END_CATCH

	int cont = TRUE, idx, execute, i, no_statements = 0;
	statement += ' ';
	CString temp_statement;
	while (cont) {
		idx = statement.Find(';');
		if (idx >= 0) {
			temp_statement = statement.Left(idx);
			statement = statement.Mid(idx+1);
		} else {
			temp_statement = statement;
			cont = FALSE;
		}
		execute = FALSE;
		for (i = 0; i < temp_statement.GetLength(); i++)
			if (temp_statement[i] > ' ') execute = TRUE;
		if (execute) {
			if (temp_statement.Find("#PROG") == 0) {
				CString command;
				command = temp_statement.Mid(6);
				sprintf(tmp_str, "MiSQL command (%s)", (LPCTSTR) command);
				LJNORM(tmp_str)
				system((LPCTSTR) command);
			} else {
				TRY {
//					Statementlog((LPCTSTR) temp_statement);
					sprintf(tmp_str, "MiSQL statement (%d)", no_statements+1);
					LJNORM(tmp_str)
					MSSQL.SetQueryTimeout(7200);
					MSSQL.ExecuteSQL(temp_statement);
					no_statements++;
				}
				CATCH(CDBException, e) {
					sprintf(tmp_str, "FEJL i sql-eksekvering: %s", e->m_strError);
					LJERR(tmp_str)
					LJERR(temp_statement)
					MSSQL.Close();
//					StatementlogClose();
					return(DLLERROR);
				}
				END_CATCH
			}
		}
	}

	MSSQL.Close();

	sprintf(tmp_str, "Antal statements udfoert: %d", no_statements);
	LJNORM(tmp_str)
//	StatementlogClose();

	return(DLLSUCCES);
}


CStdioFile logfile;
int logsucces;

void StatementlogOpen() {
	logsucces = logfile.Open("D:\\MFAQS\\DLL\\MISQL.log",
		CFile::modeCreate | CFile::modeNoTruncate |
		CFile::modeReadWrite  | CFile::shareDenyWrite);
	if (logsucces) logfile.SeekToEnd();
}

void Statementlog(LPCTSTR logentry) {
	if (logsucces) {
		CString logdata(logentry);
		logdata += "\n";
		logfile.WriteString(logdata);
		logfile.Flush();
	}
}

void StatementlogClose() {
	if (logsucces) logfile.Close();
}
