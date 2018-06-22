
// activation_run.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// ActivationRunApp:
// See activation_run.cpp for the implementation of this class
//

class ActivationRunApp : public CWinApp
{
public:
	ActivationRunApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern ActivationRunApp theApp;