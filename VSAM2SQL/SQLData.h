#include <afx.h>
#include "TransData.h"


#ifndef _SQLDATA_H_
#define _SQLDATA_H_

#define SQL_STRING		0
#define SQL_INT			1
#define SQL_SMALL		2
#define SQL_DEC			3
#define SQL_NUM			4
#define SQL_ZONED		5
#define SQL_DATE_1		6	// YYYYMMDD
#define SQL_DATE_2		7	// DDMMYYYY
#define SQL_DATE_3		8	// YYMMDD
#define SQL_DATE_4		9	// DDMMYY
#define SQL_DATE_5		10	// YYYY-MM-DD
#define SQL_DATE_6		11	// DD-MM-YYYY
#define SQL_DATE_7		12	// YY-MM-DD
#define SQL_DATE_8		13	// DD-MM-YY
#define DEFAULT_LENGTH	-1
#define DEFAULT_PREC	2

#define HIGH			0
#define LOW				1


class SQLData : public TransCString
{
private:
public:
	SQLData(TransData& settrans_data, unsigned char data_string[], int type = SQL_STRING,
		int length = DEFAULT_LENGTH, int prec = DEFAULT_PREC);
};

#endif