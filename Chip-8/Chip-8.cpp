// Chip-8.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Chip-8.h"
#include <shobjidl.h>
#include <sys/types.h>  
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include "Chip8.h"
#include "Delay.h"

#define MAX_LOADSTRING 100
#define DELAY_TIMER_ID 0x42

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HFONT sourceFont;

void OpenFileDialog();
void PopulateSourceDisplay(HWND hDlg);
void RunEmulation(HWND hDlg);
void RedrawDisplay(HWND hDlg);
void SetCurrentInstructionSelect(HWND sourceListBox);

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DisplayData(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Emulate(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Delay(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	Chip8::GetInstance(); // Create the singleton for the Chip8 processing engine
	Delay::GetInstance(); // Set up the default delay

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CHIP8, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
	sourceFont = CreateFont(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, L"Courier New");

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHIP8));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	DeleteObject(sourceFont);

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHIP8));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CHIP8);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int x;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case ID_FILE_OPEN:
				OpenFileDialog();
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
			case ID_CHIP_8_DISPLAY_SOURCE:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DISPLAY_DATA), hWnd, DisplayData);
				break;
			case ID_CHIP_8_EMULATE:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_EMULATION_DIALOG), hWnd, Emulate);
				break;
			case ID_CHIP_8_SETDELAY:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DELAY), hWnd, Delay);
				break;
			case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_CHAR:
		x = 3;
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK DisplayData(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		HWND listBox = GetDlgItem(hDlg, IDC_LIST1);

		PopulateSourceDisplay(listBox);
	}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Emulate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	UNREFERENCED_PARAMETER(lParam);
	int x;
	Chip8 *instance = Chip8::GetInstance();
	HWND sourceListBox = GetDlgItem(hDlg, IDC_DEBUG_LIST);
	HWND singleStepButton = GetDlgItem(hDlg, IDC_SINGLE_STEP);

	switch (message)
	{
	case WM_INITDIALOG:
	{
		HWND listBox = GetDlgItem(hDlg, IDC_DISPLAY_LISTBOX);
		HFONT font;

		font = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, L"Courier New");
		SendMessage(listBox, WM_SETFONT, (WPARAM)font, TRUE);

		// Hide the debug windo
		HWND sourceListBox = GetDlgItem(hDlg, IDC_DEBUG_LIST);
		PopulateSourceDisplay(sourceListBox);
		ShowWindow(sourceListBox, SW_HIDE);

		// Hide the single step button
		HWND singleStepButton = GetDlgItem(hDlg, IDC_SINGLE_STEP);
		ShowWindow(singleStepButton, SW_HIDE);
	}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case ID_START:
			{
				HWND startButton = GetDlgItem(hDlg, ID_START);
				if (instance->IsPaused())
				{
					SendMessage(startButton, WM_SETTEXT, 0, (LPARAM)L"Pause");
					if (instance->IsInit())
						instance->Reset();
					ShowWindow(singleStepButton, SW_HIDE);
					instance->Executing();
					RunEmulation(hDlg);
				}
				else
				{
					SendMessage(startButton, WM_SETTEXT, 0, (LPARAM)L"Resume");
					instance->Pause();
					if (IsWindowVisible(sourceListBox))
					{
						ShowWindow(singleStepButton, SW_SHOW);
					}
				}
			}
				break;
			case IDC_DEBUG_BUTTON:
			{
				HWND debugButton = GetDlgItem(hDlg, IDC_DEBUG_BUTTON);
				if (IsWindowVisible(sourceListBox))
				{
					ShowWindow(sourceListBox, SW_HIDE);
					ShowWindow(singleStepButton, SW_HIDE);
					SendMessage(debugButton, WM_SETTEXT, 0, (LPARAM)L"Show Debug");
				}
				else
				{
					// If we open up the debug box before executing, we need to set everything up
					// so that we can single step from the first instruction
					if (instance->IsInit())
						instance->Reset();
					SetCurrentInstructionSelect(sourceListBox);
					ShowWindow(sourceListBox, SW_SHOW);
					ShowWindow(singleStepButton, SW_SHOW);
					SendMessage(debugButton, WM_SETTEXT, 0, (LPARAM)L"Hide Debug");
				}
				break;
			}
			case IDC_SINGLE_STEP:
				if (instance->IsInit())
				{
					HWND startButton = GetDlgItem(hDlg, ID_START);
					SendMessage(startButton, WM_SETTEXT, 0, (LPARAM)L"Resume");
					instance->Pause();
				}
				RunEmulation(hDlg);
				SetCurrentInstructionSelect(sourceListBox);
				break;
			case IDCANCEL:
				instance->Reset();
				EndDialog(hDlg, LOWORD(wParam));
		}
		return (INT_PTR)TRUE;

		break;
	case WM_TIMER:
		KillTimer(hDlg, DELAY_TIMER_ID);
		RunEmulation(hDlg);
		break;
	case WM_KEYDOWN:
		x = 3;
		return (INT_PTR)TRUE;
		break;
	case WM_CHAR:
		x = 3;
		return (INT_PTR)TRUE;
		break;
	case WM_VKEYTOITEM:
		Chip8::GetInstance()->KeyPress((char)LOWORD(wParam));
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Delay(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		HWND edit = GetDlgItem(hDlg, IDC_EDIT1);
		SendMessage(edit, EM_SETLIMITTEXT, 2, 0);
	}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			LPTSTR lpWord = new TCHAR[3 + 1]; // Allow for 3 digits and the NULL
			if (lpWord)
			{
				memset(lpWord, '\0', 3 + 1 * sizeof(TCHAR));
				GetDlgItemText(hDlg, IDC_EDIT1, lpWord, 3);
			}
			Delay::GetInstance()->SetDelay(_wtoi(lpWord));
			delete[]lpWord;
			EndDialog(hDlg, LOWORD(wParam));
		}
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			break;
		}
		return (INT_PTR)TRUE;

		break;
	}
	return (INT_PTR)FALSE;
}

void PopulateSourceDisplay(HWND listBox)
{
	unsigned short programSize = Chip8::GetInstance()->GetProgramSize();
	int listBoxIndex = 0;
	SendMessage(listBox, WM_SETFONT, (WPARAM)sourceFont, TRUE);
//	DeleteObject(font);
	if (0 == programSize)
	{
		SendMessage(listBox, LB_INSERTSTRING, 0, (LPARAM)L"No source to display. Please load a ROM first.");
	}
	else
	{
		for (int x = START_CHIP_8_PROGRAM; x < START_CHIP_8_PROGRAM+programSize; x+=2)
		{
			std::wostringstream  display;
			unsigned short opcode = Chip8::GetInstance()->DecodeInstructionAt(x, display);
//			display << "Opcode #" << std::setw(4) << listBoxIndex << ": 0x" << std::uppercase << std::setfill(L'0') << std::setw(4) << std::hex << opcode;
			SendMessage(listBox, LB_INSERTSTRING, listBoxIndex++, (LPARAM)display.str().c_str());
		}
	}
}

void RunEmulation(HWND hDlg)
{
	Chip8 *instance = Chip8::GetInstance();
	bool continueExecuting = true;

	HWND sourceListBox = GetDlgItem(hDlg, IDC_DEBUG_LIST);
	if (IsWindowVisible(sourceListBox))
	{
		SetCurrentInstructionSelect(sourceListBox);
	}

	{
		unsigned long long before = GetTickCount64();
		int status = instance->ExecuteNextInstruction();

		if (status & 0x1) continueExecuting = false;
		if (status & 0x2) RedrawDisplay(hDlg);
		if (status & 0x4) Beep(750, 100);

		unsigned long long after = GetTickCount64();
		unsigned long long duration = after - before;
		if (continueExecuting && !instance->IsPaused())
		{
			int delay = Delay::GetInstance()->GetDelay();
			SetTimer(hDlg, DELAY_TIMER_ID, ((duration > delay) ? 0 : delay), (TIMERPROC) nullptr);
/*
			if (!PostMessage(hDlg, WM_COMMAND, ID_CONTINUE, 0))
			{
				LPVOID lpMsgBuf;
				LPVOID lpDisplayBuf;
				DWORD dw = GetLastError();

				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dw,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&lpMsgBuf,
					0, NULL);

			}
			*/
		}
		else
			int x = 32;
	}

}

void RedrawDisplay(HWND hDlg)
{
	HWND listBox = GetDlgItem(hDlg, IDC_DISPLAY_LISTBOX);

	SendMessage(listBox, LB_RESETCONTENT, 0,0);
	for (int x = 0; x < 32; x++) // Loop through the rows
	{
		unsigned long long mask = 0x0000000000000001;
		std::wostringstream  display;
		unsigned long long displayRow = Chip8::GetInstance()->GetDisplayRow(x);
		while (0 != mask)
		{
			display << ((displayRow & mask) ? "*" : " ");
			mask <<= 1;
		}
		SendMessage(listBox, LB_INSERTSTRING, x, (LPARAM)display.str().c_str());
	}
}
void OpenFileDialog()
{
	IFileOpenDialog *pFileOpen;

	HRESULT hr;

	COMDLG_FILTERSPEC rgSpec[] =
	{
		{ L"Chip 8 Roms", L"*.rom" },
	};

	// Create the FileOpenDialog object.
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

	if (SUCCEEDED(hr))
	{
		pFileOpen->SetFileTypes(1, rgSpec);
		// Show the Open dialog box.
		hr = pFileOpen->Show(NULL);
		IShellItem *pItem;
		hr = pFileOpen->GetResult(&pItem);
		if (SUCCEEDED(hr))
		{
			PWSTR pszFilePath;
			hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

			struct _stat64i32 results;
			std::wostringstream  display;

			if (_wstat(pszFilePath, &results) == 0)
			{
				display << pszFilePath << " is " << results.st_size << " bytes. Max size is " << MAX_PROGRAM_SIZE;
			}
			if (SUCCEEDED(hr))
			{
				wchar_t *memblock = new wchar_t[CHIP_8_MEMORY_SIZE];
				std::wifstream  file(pszFilePath, std::ios::in | std::ios::binary);
				if (file.is_open())
				{
					file.read(memblock, results.st_size);
					char t = LOBYTE(memblock[0]);
					display << " " << (int) t;
					int returnCode = Chip8::GetInstance()->LoadProgram(memblock, results.st_size);
					if (-1 == returnCode)
					{
						MessageBox(NULL, display.str().c_str(), L"Invalid ROM file.", MB_OK);
					}
				}
				MessageBox(NULL, display.str().c_str(), L"My Box", MB_OK);
				CoTaskMemFree(pszFilePath);
			}
			pItem->Release();
		}
	}
	pFileOpen->Release();
}

void SetCurrentInstructionSelect(HWND sourceListBox)
{
	unsigned short pc = Chip8::GetInstance()->GetPC();
	unsigned long offset = (pc - START_CHIP_8_PROGRAM) / 2;
	SendMessage(sourceListBox, LB_SETCURSEL, offset, 0);
}