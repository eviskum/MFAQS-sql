#include "StdAfx.h"

#include <fstream.h>
#include "SQLData.h"


SQLData::SQLData(TransData& settrans_data, unsigned char data_string[], int type,
				 int length, int prec) : TransCString()
{
	char temp_str[10000];

	switch (type) {
	case SQL_STRING:
		if (length == DEFAULT_LENGTH) CString::operator=(data_string);
		else {
			data_string[length] = '\0';
			CString::operator=((LPCTSTR) data_string);
			Translate(settrans_data);
		}
		break;
	case SQL_INT:
		{
			unsigned long temp_long_1 =
				(unsigned char) data_string[0] * 256 * 256 * 256 +
				(unsigned char) data_string[1] * 256 * 256 +
				(unsigned char) data_string[2] * 256 + (unsigned char) data_string[3];
			sprintf(temp_str, "%lu", temp_long_1);
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_SMALL:
		{
			unsigned long temp_long_2 =
				(unsigned char) data_string[0] * 256 + (unsigned char) data_string[1];
			sprintf(temp_str, "%lu", temp_long_2);
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_DEC:
		{
			if (length == DEFAULT_LENGTH) length = 13;
			if (prec == DEFAULT_PREC) prec = 2;
			int data_index = 0, high_low = HIGH;
			int limit = (length % 2 == 1) ? length : length + 1;
			for (int i = 0; i <= limit+1; i++) {
				if (high_low == HIGH) {
					temp_str[i] = (unsigned char) data_string[data_index] / 16 + '0';
					high_low = LOW;
				} else {
					temp_str[i] = (unsigned char) data_string[data_index++] % 16 + '0';
					high_low = HIGH;
				}
			}
			if (length % 2 == 0)
				for (i = 0; i <= length; i++) temp_str[i] = temp_str[i+1];
			int sign = (temp_str[length] == '=') ? -1 : 1;
			for (i = length; i > length - prec; i--)
				temp_str[i] = temp_str[i-1];
			temp_str[length - prec] = '.';
			temp_str[(prec > 0) ? length+1 : length] = '\0';
			if (sign == -1) {
				for (i = strlen(temp_str); i >= 0; i--)
					temp_str[i+1] = temp_str[i];
				temp_str[0] = '-';
			}
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_NUM:
		if (length == DEFAULT_LENGTH) CString::operator=(data_string);
		else {
			data_string[length] = '\0';
			CString::operator=((LPCTSTR) data_string);
			Translate(settrans_data);
			int foundsign = FALSE, signindex;
			for (int i = 0; i < GetLength(); i++) {
				if (GetAt(i) == '-') {
					foundsign = TRUE;
					signindex = i;
				}
			}
			if (foundsign) {
				for (int i = signindex; i > 0; i--) {
					SetAt(i, GetAt(i-1));
				}
				SetAt(0, '-');
			}
			if (prec > 0) {
				Insert(length-prec, '.');
			}
		}
		break;
	case SQL_ZONED:
		if (length == DEFAULT_LENGTH) CString::operator=(data_string);
		else {
			UCHAR signchar = data_string[length-1];
			unsigned char tmp_char = (unsigned char) data_string[0] / 16;
			(unsigned char) tmp_char = (unsigned char) tmp_char * 16;
			(unsigned char) data_string[length-1] = (unsigned char) tmp_char + (15 & data_string[length-1]);
			data_string[length] = '\0';
			CString::operator=((LPCTSTR) data_string);
			Translate(settrans_data);
			if (prec > 0) {
				Insert(length-prec, '.');
			}
			char temp_char = (unsigned char) signchar / 16 + '0';
			int sign = (temp_char == '=') ? -1 : 1;
			if (sign == -1)  Insert(0, '-');
		}
		break;
	case SQL_DATE_1: // YYYYMMDD
		{
			data_string[8] = '\0';
			TransCString tmp_data_str(data_string);
			tmp_data_str.Translate(settrans_data);
			temp_str[0] = tmp_data_str[0]; temp_str[1] = tmp_data_str[1];
			temp_str[2] = tmp_data_str[2]; temp_str[3] = tmp_data_str[3];
			temp_str[4] = '-';
			temp_str[5] = tmp_data_str[4]; temp_str[6] = tmp_data_str[5];
			temp_str[7] = '-';
			temp_str[8] = tmp_data_str[6]; temp_str[9] = tmp_data_str[7];
			temp_str[10] = '\0';
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_DATE_2: // DDMMYYYY
		{
			data_string[8] = '\0';
			TransCString tmp_data_str(data_string);
			tmp_data_str.Translate(settrans_data);
			temp_str[0] = tmp_data_str[4]; temp_str[1] = tmp_data_str[5];
			temp_str[2] = tmp_data_str[6]; temp_str[3] = tmp_data_str[7];
			temp_str[4] = '-';
			temp_str[5] = tmp_data_str[2]; temp_str[6] = tmp_data_str[3];
			temp_str[7] = '-';
			temp_str[8] = tmp_data_str[0]; temp_str[9] = tmp_data_str[1];
			temp_str[10] = '\0';
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_DATE_3: // YYMMDD
		{
			data_string[6] = '\0';
			TransCString tmp_data_str(data_string);
			tmp_data_str.Translate(settrans_data);
			if (tmp_data_str[0] > '7') {
				temp_str[0] = '1'; temp_str[1] = '9';
			} else {
				temp_str[0] = '2'; temp_str[1] = '0';
			}
			temp_str[2] = tmp_data_str[0]; temp_str[3] = tmp_data_str[1];
			temp_str[4] = '-';
			temp_str[5] = tmp_data_str[2]; temp_str[6] = tmp_data_str[3];
			temp_str[7] = '-';
			temp_str[8] = tmp_data_str[4]; temp_str[9] = tmp_data_str[5];
			temp_str[10] = '\0';
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_DATE_4: // DDMMYY
		{
			data_string[6] = '\0';
			TransCString tmp_data_str(data_string);
			tmp_data_str.Translate(settrans_data);
			if (tmp_data_str[4] > '7') {
				temp_str[0] = '1'; temp_str[1] = '9';
			} else {
				temp_str[0] = '2'; temp_str[1] = '0';
			}
			temp_str[2] = tmp_data_str[4]; temp_str[3] = tmp_data_str[5];
			temp_str[4] = '-';
			temp_str[5] = tmp_data_str[2]; temp_str[6] = tmp_data_str[3];
			temp_str[7] = '-';
			temp_str[8] = tmp_data_str[0]; temp_str[9] = tmp_data_str[1];
			temp_str[10] = '\0';
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_DATE_5: // YYYY-MM-DD
		{
			data_string[10] = '\0';
			TransCString tmp_data_str(data_string);
			tmp_data_str.Translate(settrans_data);
			temp_str[0] = tmp_data_str[0]; temp_str[1] = tmp_data_str[1];
			temp_str[2] = tmp_data_str[2]; temp_str[3] = tmp_data_str[3];
			temp_str[4] = '-';
			temp_str[5] = tmp_data_str[5]; temp_str[6] = tmp_data_str[6];
			temp_str[7] = '-';
			temp_str[8] = tmp_data_str[8]; temp_str[9] = tmp_data_str[9];
			temp_str[10] = '\0';
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_DATE_6: // DD-MM-YYYY
		{
			data_string[10] = '\0';
			TransCString tmp_data_str(data_string);
			tmp_data_str.Translate(settrans_data);
			temp_str[0] = tmp_data_str[6]; temp_str[1] = tmp_data_str[7];
			temp_str[2] = tmp_data_str[8]; temp_str[3] = tmp_data_str[9];
			temp_str[4] = '-';
			temp_str[5] = tmp_data_str[3]; temp_str[6] = tmp_data_str[4];
			temp_str[7] = '-';
			temp_str[8] = tmp_data_str[0]; temp_str[9] = tmp_data_str[1];
			temp_str[10] = '\0';
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_DATE_7: // YY-MM-DD
		{
			data_string[8] = '\0';
			TransCString tmp_data_str(data_string);
			tmp_data_str.Translate(settrans_data);
			if (tmp_data_str[0] > '7') {
				temp_str[0] = '1'; temp_str[1] = '9';
			} else {
				temp_str[0] = '2'; temp_str[1] = '0';
			}
			temp_str[2] = tmp_data_str[0]; temp_str[3] = tmp_data_str[1];
			temp_str[4] = '-';
			temp_str[5] = tmp_data_str[3]; temp_str[6] = tmp_data_str[4];
			temp_str[7] = '-';
			temp_str[8] = tmp_data_str[6]; temp_str[9] = tmp_data_str[7];
			temp_str[10] = '\0';
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	case SQL_DATE_8: // DD-MM-YY
		{
			data_string[8] = '\0';
			TransCString tmp_data_str(data_string);
			tmp_data_str.Translate(settrans_data);
			if (tmp_data_str[6] > '7') {
				temp_str[0] = '1'; temp_str[1] = '9';
			} else {
				temp_str[0] = '2'; temp_str[1] = '0';
			}
			temp_str[2] = tmp_data_str[6]; temp_str[3] = tmp_data_str[7];
			temp_str[4] = '-';
			temp_str[5] = tmp_data_str[3]; temp_str[6] = tmp_data_str[4];
			temp_str[7] = '-';
			temp_str[8] = tmp_data_str[0]; temp_str[9] = tmp_data_str[1];
			temp_str[10] = '\0';
		}
		CString::operator=((LPCTSTR) temp_str);
		break;
	default:
		break;
	}
}
