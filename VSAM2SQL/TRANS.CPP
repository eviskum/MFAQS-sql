#include <afx.h>
#include <iostream.h>
#include "TransData.h"



class Configuration
{
private:
	class SectionSettings
	{
	public:
		int no_keys;
		CString section;
		CString key_name[100];
		CString key_value[100];
	} ConfigData[250];
	int no_sections;
public:
	Configuration(LPCTSTR filename);
	int GetKeyValue(LPCTSTR section, LPCTSTR key, CString& res_val);
	int GetKeysCount(LPCTSTR section);
	int GetKey(LPCTSTR, int index, CString& res_key);
};

Configuration::Configuration(LPCTSTR filename)
{
	CString data;
	int activesection = 0;
	no_sections = 0;

	TRY
	{
		CStdioFile conffile(filename, CFile::modeRead);
		while (conffile.ReadString(data) == TRUE) {
			if (data.GetLength() >= 2) {
				if (data[0] != ';') {
					if (data[0] == '[' || activesection != 0) {
						if (data[0] == '[') {
							int end_index = data.Find((TCHAR) ']');
							if (end_index != -1) {
								activesection = 1;
								ConfigData[no_sections].section = data.Mid(1, end_index);
								ConfigData[no_sections].section.MakeUpper();
								ConfigData[no_sections++].no_keys = 0;
							}
						} else {
							int assign_idx = data.Find((TCHAR) '=');
							if (assign_idx != -1 && assign_idx > 0 &&
								data.GetLength() > assign_idx + 1) {
								CString str_key_name(data.Left(assign_idx));
								CString str_key_value(data.Mid(assign_idx + 1));
								str_key_name.TrimLeft(); str_key_name.TrimRight();
								str_key_value.TrimLeft(); str_key_value.TrimRight();
								if (!str_key_name.IsEmpty()) {
									str_key_name.MakeUpper();
									ConfigData[no_sections-1].key_name[ConfigData[no_sections-1].no_keys] =
										str_key_name;
									ConfigData[no_sections-1].key_value[ConfigData[no_sections-1].no_keys++] =
										str_key_value;
								}
							}
						}
					}
				}
			}
		}
		conffile.Close();
	}
	CATCH( CFileException, e )
	{
		cerr << "\nKunne ikke hente konfigurationsdata fra: " << filename << endl;
		exit(-1);
	}
	END_CATCH
}

int Configuration::GetKeyValue(LPCTSTR section, LPCTSTR key, CString& res_val)
{
	CString section_str(section);
	section_str.MakeUpper();
	CString key_str(section);
	key_str.MakeUpper();
	for(int i; i < no_sections; i++)
		if (section_str == ConfigData[i].section)
			for (int j; j < ConfigData[i].no_keys; j++)
				if (key_str == ConfigData[i].key_name[j]) {
					res_val = ConfigData[i].key_value[j];
					return (TRUE);
				}
	return(FALSE);
}

int Configuration::GetKeysCount(LPCTSTR section)
{
	CString section_str(section);
	section_str.MakeUpper();
	for(int i; i < no_sections; i++)
		if (section_str == ConfigData[i].section)
			return (ConfigData[i].no_keys);
	return (-1);
}

int Configuration::GetKey(LPCTSTR section, int index, CString& res_key)
{
	CString section_str(section);
	section_str.MakeUpper();
	for(int i; i < no_sections; i++)
		if (section_str == ConfigData[i].section) {
			res_key = ConfigData[i].key_value[index];
			return (TRUE);
		}
	return (FALSE);
}

typedef struct {
	int vsam_type;
	union {
		struct { int start; int length } vsam_string;
		struct { int start; } vsam_int;
		struct { int start; } vsam_small;
		struct { int start; int length; int prec; } vsam_dec;
		struct { int start; int type; } vsam_date;
	} data;
} VSAM_DATA ;


class VsamData
{
private:
	int fields;
	VSAM_DATA vsam_data;
	int record_length;
	CString table;
	int GetSQLFieldString(const char string[], int index, CString& res_str);
public:
	VsamData(LPCTSTR filename);
	int GetRecordLength();
	CString& GetTableName();
	int GetSQLString(const char string[], CString& res_str);
}

VsamData::VsamData(LPCTSTR filename)
{
	CString tempdata;
	Configuration vsamconf(filename);
	vsamconf.GetKeyValue("Tabeldefinition", "SQLtabelnavn", table);
	vsamconf.GetKeyValue("Tabeldefinition", "recordlaengde", tempdata);
	record_length = atoi(tempdata);
	vsamconf.GetKeyValue("Tabeldefinition", "felter", tempdata);
	fields = atoi(tempdata);
	for (int i = 0; i < fields; i++) {






int main( void ) {
	return (0);
}
