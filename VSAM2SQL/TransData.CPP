#include "StdAfx.h"

#include <iostream.h>
#include "TransData.h"

#define DLLSUCCES	1
#define DLLERROR	2
#define LOGNORMAL	1
#define LOGERROR	2
#define LNORM( txt )	Log(txt, LOGNORMAL, FALSE);
#define LERR( txt )		Log(txt, LOGERROR, FALSE);
#define LJNORM( txt )	Log(txt, LOGNORMAL, TRUE);
#define LJERR( txt )	Log(txt, LOGERROR, TRUE);

extern void Log(LPCTSTR txt, int mode, int job);
extern int transsucces;

/*DEBUG*/
void PD (int i);


TransData::TransData(LPCTSTR filename)
{
	CString data, idxstr, cnvstr;
	unsigned int idxval, cnvval;
/*DEBUG*/
PD(31);

	for (int i = 0; i < 256; i++) translate_data[i] = i;
/*DEBUG*/
PD(32);

	if (filename != NULL) {
		TRY
		{
/*DEBUG*/
PD(33);
			CStdioFile convfile(filename, CFile::modeRead);
			while (convfile.ReadString(data) != FALSE) {
				if (data.GetLength() >= 5) {
					idxstr = data.Left(2); sscanf(idxstr, "%x", &idxval);
					cnvstr = data.Mid(3,2); sscanf(cnvstr, "%x", &cnvval);
					if (0 <= idxval && idxval <= 255 && 0 <= cnvval && cnvval <= 255)
						translate_data[idxval] = cnvval;
				}
			}
			convfile.Close();
/*DEBUG*/
PD(34);
		}
		CATCH( CFileException, e )
		{
			LJERR(CString("Kunne ikke hente konverteringsdata fra: ") + filename)
			transsucces = FALSE;
		}
		END_CATCH
	}
}

void TransCString::Translate(TransData& settrans_data)
{
	UCHAR *getdatatrans;

	getdatatrans = settrans_data.GetTransData();
	for (int i = 0; i < 256; i++) {
		translate_data[i] = getdatatrans[i];
	}
	for (i = 0; i < GetLength(); i++) {
		unsigned char tmp = GetAt(i);
		int index = tmp * 1;
		SetAt(i, (TCHAR) translate_data[index]);
	}
}
