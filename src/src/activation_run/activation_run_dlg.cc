
// activation_run_dlg.cc : implementation file
//

#include "stdafx.h"
#include "activation_run.h"
#include "activation_run_dlg.h"
#include "afxdialogex.h"
#include "activation_win/dll_launcher.h"
#include "activation_win/activation_win.h"
#pragma comment(lib,"Winhttp.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ActivationRunDlg dialog



ActivationRunDlg::ActivationRunDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ActivationRunDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ActivationRunDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ActivationRunDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &ActivationRunDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// ActivationRunDlg message handlers

BOOL ActivationRunDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	WSADATA wsaData;
	WSAStartup(MAKEWORD(1,1), &wsaData);
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void ActivationRunDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR ActivationRunDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ActivationRunDlg::OnBnClickedOk()
{
	HANDLE ok_thread = reinterpret_cast<HANDLE>(::CreateThread(nullptr, 0,
		[](void* args) ->DWORD{
		for (int i = 0; i < 10; i++){
			OutputDebugStringA("registed start!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			DWORD itunes_pid = activation_win::SnapshotProcess(L"iTunes.exe");
			if (!itunes_pid){
				activation_win::Create(L"C:\\Program Files\\iTunes\\iTunes.exe", false);
				Sleep(1000);
				itunes_pid = activation_win::SnapshotProcess(L"iTunes.exe");
			}
			THREAD_DATA parameter = { 0 };
			parameter.fnRtlInitUnicodeString = (pRtlInitUnicodeString)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlInitUnicodeString");
			parameter.fnLdrLoadDll = (pLdrLoadDll)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "LdrLoadDll");
			const std::wstring module = activation_win::GetAbsolutePath(L"\\activation_win.dll");
			activation_win::SetShellcodeLdrModulePath(&parameter, module);
			activation_win::ProcessInternalExecute(&parameter, itunes_pid, 1);
			activation_win::Create(L"C:\\Program Files\\iTunes\\iTunes.exe", true);
			HANDLE thread = reinterpret_cast<HANDLE>(::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)
				[](void* args) ->DWORD{
				const std::wstring module = activation_win::GetAbsolutePath(L"\\activation_win.dll");
				HMODULE hmod = LoadLibraryW(module.c_str());
				while (hmod != nullptr&&activation_win::SnapshotProcess(L"iTunes.exe") != 0){
					Sleep(10);
				}
				FreeLibrary(hmod);
				return 0;
			}, nullptr, 0, nullptr));
			CloseHandle(thread);
			while (activation_win::SnapshotProcess(L"iTunes.exe") != 0){
				Sleep(10);
			}
			Sleep(1000);
			OutputDebugStringA("registed end!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		}
		return 0;
	}, nullptr, 0, nullptr));
	CloseHandle(ok_thread);
}
