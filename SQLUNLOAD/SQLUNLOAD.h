// SQLUNLOAD.h : main header file for the SQLUNLOAD DLL
//

#if !defined(AFX_SQLUNLOAD_H__CEB4189D_48EC_11D3_977A_00104BA3042C__INCLUDED_)
#define AFX_SQLUNLOAD_H__CEB4189D_48EC_11D3_977A_00104BA3042C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSQLUNLOADApp
// See SQLUNLOAD.cpp for the implementation of this class
//

class CSQLUNLOADApp : public CWinApp
{
public:
	CSQLUNLOADApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSQLUNLOADApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSQLUNLOADApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SQLUNLOAD_H__CEB4189D_48EC_11D3_977A_00104BA3042C__INCLUDED_)
