// DB2 (VM/VSE) --> MS SQL (NT)
// Esben Viskum, AO, april 1999
#include <afx.h>
#include <afxdb.h>
#include <stdio.h>
#include <fstream>
using namespace std;

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

int DllGetLog(LPTSTR txt, int *mode, int *job)
{
	strcpy(txt, log_txt);
	*mode = log_mode;
	*job = log_job;
	return(0);
}

int DllLogReg( HANDLE set_event, HANDLE set_ready )
{
	hLogEvent = set_event;
	hLogReady = set_ready;
	return(0);
}


char comment[101];
char db2_handle[101], db2_connect[101], sql_handle[101], sql_connect[101];
char db2_select[1001], sql_once[1001], sql_insert[1001];
char tmpstr[20000];

short readconfig(char sqlfile[])
{
	FILE *SQLFILE;
	if ((SQLFILE = fopen(sqlfile, "r")) == NULL) {
		fprintf(stderr, "Kunne ikke hente configurationsfil: %s\n", sqlfile);
		exit(1);
	}
	fgets(comment, 100, SQLFILE); comment[strlen(comment)-1] = '\0';
	fgets(db2_handle, 100, SQLFILE); db2_handle[strlen(db2_handle)-1] = '\0';
	fgets(db2_connect, 100, SQLFILE); db2_connect[strlen(db2_connect)-1] = '\0';
	fgets(sql_handle, 100, SQLFILE); sql_handle[strlen(sql_handle)-1] = '\0';
	fgets(sql_connect, 100, SQLFILE); sql_connect[strlen(sql_connect)-1] = '\0';
	fgets(db2_select, 1000, SQLFILE); db2_select[strlen(db2_select)-1] = '\0';
	fgets(sql_once, 1000, SQLFILE); sql_once[strlen(sql_once)-1] = '\0';
	fgets(sql_insert, 1000, SQLFILE); sql_insert[strlen(sql_insert)-1] = '\0';
	fclose(SQLFILE);
	return(0);
}


int DllWorker ( int argc, char *argv[] )
{
	TCHAR tmp_str[200];

	LJNORM("\nStarter DB2 --> MS SQL ODBC utility\n")

	if (argc != 1) {
		LJERR("Forkert brug !!!!\nKorrekt brug: DB2SQL conf.sql\n");
		return(DLLERROR);
	}

	readconfig(argv[0]);

	CDatabase DB2;
	CDatabase MSSQL;

	TRY {
		LJNORM(" connecter DB2...")
		DB2.Open(db2_handle, FALSE, FALSE, db2_connect, FALSE);
		LJNORM(" connecter MS-SQL...")
		MSSQL.Open(sql_handle, FALSE, FALSE, sql_connect, FALSE);
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

	TRY {
		MSSQL.ExecuteSQL(sql_once);
	}
	CATCH(CDBException, e) {
		sprintf(tmp_str, "\nFEJL i sql-eksekvering: %s\n", e->m_strError);
		LJERR(tmp_str)
		return(DLLERROR);
	}
	END_CATCH

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
			tmpstr[k] = '\0';
			TRY {
				MSSQL.ExecuteSQL(tmpstr);
			}
			CATCH(CDBException, e) {
				sprintf(tmp_str, "\nFEJL i MSSQL insert: %s\n", e->m_strError);
				LJERR(tmp_str)
				MSSQL.Close();
				DB2_Table.Close();
				DB2.Close();
				return(DLLERROR);
			}
			END_CATCH
			if ((records++ % 1000) == 0 && records > 1) {
				sprintf(tmp_str, "..har flyttet %d records", records);
				LJNORM(tmp_str)
			}
			DB2_Table.MoveNext( );
		}
	}

	sprintf(tmp_str, "\nSlut -- har flyttet %d records\n", records);
	LJNORM(tmp_str)
	MSSQL.Close();
	DB2_Table.Close();
	DB2.Close();
	return(DLLSUCCES);
}
