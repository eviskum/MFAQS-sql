// SQLUNLOAD.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "SQLUNLOAD.h"

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
// CSQLUNLOADApp

BEGIN_MESSAGE_MAP(CSQLUNLOADApp, CWinApp)
	//{{AFX_MSG_MAP(CSQLUNLOADApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSQLUNLOADApp construction

CSQLUNLOADApp::CSQLUNLOADApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSQLUNLOADApp object

CSQLUNLOADApp theApp;

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


TCHAR tmp_str[1000];

#define LEFT 0
#define RIGHT 1

class element {
private:
	int type;
	int length;
	CString sepstr;
	CString resstr;
public:
	element(int set_type, int set_length, CString& set_sepstr);
	CString& getstr(CString& elm_data);
};

class elements {
private:
	element *elm_array[250];
	int no_element;
	CString startsep;
	CString resstr;
public:
	elements();
	~elements();
	void set_startsep(CString& set_startsep);
	void setup_element(int set_type, int set_length, CString& set_sepstr);
	void insert_data(int index, CString& elm_data);
	CString& getfilestr();
};

element::element(int set_type, int set_length, CString& set_sepstr) {
	type = set_type;
	length = set_length;
	sepstr = set_sepstr;
}

CString& element::getstr(CString& elm_data) {
	if (elm_data.GetLength() == length) {
		resstr = elm_data + sepstr;
	} else if (elm_data.GetLength() > length) {
		resstr = elm_data.Left(length) + sepstr;
	} else {
		if (type == LEFT) {
			resstr = elm_data + CString(' ', length - elm_data.GetLength()) + sepstr;
		} else {
			resstr = CString(' ', length - elm_data.GetLength()) + elm_data + sepstr;
		}
	}
	return(resstr);
}

elements::elements() {
	no_element = 0;
}

elements::~elements() {
	for (int i = 0; i < no_element; i++) delete elm_array[i];
}

void elements::setup_element(int set_type, int set_length, CString& set_sepstr) {
	elm_array[no_element++] = new element(set_type, set_length, set_sepstr);
}

void elements::insert_data(int index, CString& elm_data) {
	if (index == 0) {
		resstr = startsep;
	}
	resstr += elm_array[index]->getstr(elm_data);
}

CString& elements::getfilestr() {
	resstr += '\n';
	return(resstr);
}

void elements::set_startsep(CString& set_startsep) {
	startsep = set_startsep;
}


class unloadconf {
private:
	CString comment;
	CString txtmask;
public:
	CString sql_handle, sql_connect, statement;
	unloadconf(LPCSTR filename);
	int error;
	void setup_txtmask(elements& elm_inst);
};

unloadconf::unloadconf(LPCSTR filename) {
	error = FALSE;
	CStdioFile CONFFILE;
	CFileException e;
	if (!CONFFILE.Open (filename, CFile::modeRead | CFile::shareExclusive, &e)) {
		sprintf(tmp_str, "Kunne ikke hente config: '%s'", filename);
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
		error = TRUE;
	}

	if (error == FALSE) {
		CString tmpstr;
		CONFFILE.ReadString(comment);
		CONFFILE.ReadString(sql_handle);
		CONFFILE.ReadString(sql_connect);
		statement.Empty();
		tmpstr.Empty();
		int index;
		do {
			statement += tmpstr;
			CONFFILE.ReadString(tmpstr);
		} while ((index = tmpstr.Find(';')) == -1);
		statement += tmpstr.Left(index);
		CONFFILE.ReadString(tmpstr);
		txtmask = tmpstr.Left(tmpstr.Find(';'));
		CONFFILE.Close();
	}
}

void unloadconf::setup_txtmask(elements& elm_inst) {
	int index = 0, type, length, startnumber;
	CString sepstr;
	while (txtmask[index] != 'L' && txtmask[index] != 'R' && index < txtmask.GetLength()) index++;
	if (index > 0) elm_inst.set_startsep(txtmask.Left(index));
	else elm_inst.set_startsep(CString(""));
	while (index < txtmask.GetLength()) {
		if (txtmask[index++] == 'L') type = LEFT;
		else type = RIGHT;
		startnumber = index;
		while (index < txtmask.GetLength() && txtmask[index] >= '0' && txtmask[index] <= '9') index++;
		length = atoi((LPCSTR) txtmask.Mid(startnumber, index - startnumber));
		startnumber = index;
		while (index < txtmask.GetLength() && txtmask[index] != 'L' &&
			txtmask[index] != 'R') index ++;
		sepstr = txtmask.Mid(startnumber, index-startnumber);
		elm_inst.setup_element(type, length, sepstr);
	}
}


void StripEOL(CString &bufferstring)
{
	int idx1, idx2, index;

	while ((idx1 = bufferstring.Find('\r')) >= 0 || (idx2 = bufferstring.Find('\n')) >= 0) {
		if ((idx1 >= 0) && (idx1 < idx2)) index = idx1;
		else if ((idx2 >= 0) && (idx2 < idx1)) index = idx1;
		else index = -1;
		if (index >= 0) {
			int offset = 1;
			while (index+offset < bufferstring.GetLength() &&
				(bufferstring[index+offset] == '\r' || bufferstring[index+offset] == '\n'))
				offset++;
			bufferstring.Delete(index, offset);
			bufferstring.Insert(index, ' ');
		}
	}
}


extern "C" int PASCAL EXPORT DllWorker ( int argc, char *argv[] )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	LJNORM("Starter SQL unload utility")

	if (argc != 3 && argc != 2) {
		LJERR("Forkert brug !!!! Korrekt brug: SQLUNLOAD conf.sql out.file trigfile")
		return(DLLERROR);
	}

	TRY
	{
		if (argc == 3) CFile::Remove(argv[2]);
	}
	CATCH( CFileException, e )
	{
		LJERR("FEJL: kunne ikke slette trigger-fil")
		return(DLLERROR);
	}
	END_CATCH

	unloadconf conf(argv[0]);
	if (conf.error == TRUE) {
		fprintf(stderr, "Kunne ikke hente configurationsfil: %s\n", argv[0]);
		exit(1);
	}
	elements elms;
	conf.setup_txtmask(elms);

	CStdioFile tempfile;
	if (!tempfile.Open(argv[1], CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive)) {
		LJERR("FEJL kunne ikke aabne temp-fil")
		return(DLLERROR);
	}

	CDatabase MSSQL;

	TRY {
		LJNORM(" connecter MSSQL... ")
		MSSQL.Open(conf.sql_handle, FALSE, FALSE, conf.sql_connect, FALSE);
	}
	CATCH(CDBException, e) {
		sprintf(tmp_str, "\nFEJL i ODBC-connect: %s\n", e->m_strError);
		LJERR(tmp_str)
		return(DLLERROR);
	}
	END_CATCH
	
	CRecordset MSSQL_Table(&MSSQL);
	LJNORM("fetcher MS SQL recordset...")
	TRY {
		MSSQL.SetQueryTimeout(3600);
		MSSQL_Table.Open(CRecordset::forwardOnly, conf.statement);
	}
	CATCH(CDBException, e) {
		sprintf(tmp_str, "\nFEJL i MSSQL-fetch: %s\n", e->m_strError);
		LJERR(tmp_str)
		return(DLLERROR);
	}
	END_CATCH
	short fields = MSSQL_Table.GetODBCFieldCount( );

	int records = 0;

	if( !MSSQL_Table.IsBOF( ) ) {
		while ( !MSSQL_Table.IsEOF( ) ) {
			int k = 0, j = 0;
			for (short i = 0; i < fields; i++) {
				CString bufferstring;
				MSSQL_Table.GetFieldValue(i, bufferstring);
				StripEOL(bufferstring);
				elms.insert_data(i, bufferstring);
			}

			tempfile.WriteString((LPCSTR) elms.getfilestr());
			
			if ((records++ % 10000) == 0 && records > 1) {
				sprintf(tmp_str, "...har hentet %d records", records-1);
				LJNORM(tmp_str)
			}
			MSSQL_Table.MoveNext( );
		}
	}

	sprintf(tmp_str, "Slut -- har flyttet %d records", records);
	LJNORM(tmp_str)
	MSSQL_Table.Close();
	MSSQL.Close();

	tempfile.Close();

	return(DLLSUCCES);
}
