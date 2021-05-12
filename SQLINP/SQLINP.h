// SQLINP.h : main header file for the SQLINP DLL
//

#if !defined(AFX_SQLINP_H__4216B847_2554_11D3_901E_00104BA3042C__INCLUDED_)
#define AFX_SQLINP_H__4216B847_2554_11D3_901E_00104BA3042C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSQLINPApp
// See SQLINP.cpp for the implementation of this class
//

class CSQLINPApp : public CWinApp
{
public:
	CSQLINPApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSQLINPApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSQLINPApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SQLINP_H__4216B847_2554_11D3_901E_00104BA3042C__INCLUDED_)
