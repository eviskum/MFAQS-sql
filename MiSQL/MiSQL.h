// MiSQL.h : main header file for the MISQL DLL
//

#if !defined(AFX_MISQL_H__078113AB_42C5_11D3_9776_00104BA3042C__INCLUDED_)
#define AFX_MISQL_H__078113AB_42C5_11D3_9776_00104BA3042C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMiSQLApp
// See MiSQL.cpp for the implementation of this class
//

class CMiSQLApp : public CWinApp
{
public:
	CMiSQLApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMiSQLApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMiSQLApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISQL_H__078113AB_42C5_11D3_9776_00104BA3042C__INCLUDED_)
