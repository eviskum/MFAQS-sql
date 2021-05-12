#include <afx.h>

#ifndef _TRANSDATA_H_
#define _TRANSDATA_H_

class TransData
{
private:
	UCHAR translate_data[256];
public:
	TransData(LPCTSTR filename);
	UCHAR *GetTransData() { return(translate_data); }
};

class TransCString : public CString
{
private:
	char translate_data[256];
public:
	TransCString() : CString() {;}
	TransCString(const char init_str[]) : CString(init_str) {;}
	TransCString(const TransCString &init_TCstr) : CString(init_TCstr) {;}
	TransCString(const CString &init_Cstr) : CString(init_Cstr) {;}
	void Translate(TransData& settrans_data);
};

#endif