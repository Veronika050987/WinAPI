#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>
#include <string>
#include <fstream>
#include <commdlg.h>//print
#include <iostream>
#include <vector>
#include <richedit.h> //Rich Edit Control
#include "resource.h" //for ICO

HINSTANCE hInstance;
HWND hMainWindow;
HWND hEdit;
std::wstring filePath;
bool isModified = false;

// Modifications history (undo/redo)
std::vector<std::wstring> undoHistory;
std::vector<std::wstring> redoHistory;
int historyIndex = -1;
const int MAX_HISTORY = 50;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool OpenFile(HWND hwnd);
bool SaveFile(HWND hwnd);
bool SaveFileAs(HWND hwnd);
bool QuerySave(HWND hwnd); // unsaved modifications control
void SetWindowTitle(HWND hwnd, const std::wstring& filename);
void UpdateHistory(HWND hwnd); // add state into history
void Undo(HWND hwnd);
void Redo(HWND hwnd);
void AppendText(HWND hwnd, const std::wstring& text); // add text
void SetBold(HWND hwnd);
void SetItalic(HWND hwnd);
void SetRegular(HWND hwnd);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, 
    _In_ int nCmdShow) 
{
    ::hInstance = hInstance;

    //window class registration
    const wchar_t CLASS_NAME[] = L"NotepadWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc)) 
    {
        MessageBox(nullptr, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // window creation
    hMainWindow = CreateWindowEx
    (
        0,
        CLASS_NAME,
        L"Notepad",  // Initial title
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (hMainWindow == nullptr) 
    {
        MessageBox(nullptr, L"Failed to create main window.", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    // RichEdit control init
    LoadLibrary(TEXT("Riched20.dll")); // Riched20.dll для простого форматирования, Riched32.dll для RTF.  Riched20.dll достаточно для bold/italic.

    //Rich Edit Control creation
    hEdit = CreateWindowEx
    (
        WS_EX_CLIENTEDGE,
        RICHEDIT_CLASS,
        L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_HSCROLL,
        0, 0, 0, 0,  // position and size are set in WM_SIZE
        hMainWindow,
        (HMENU)1001, // element control ID
        hInstance,
        nullptr
    );

    if (hEdit == nullptr) 
    {
        MessageBox(nullptr, L"Failed to create edit control.", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    //(Arial, 12pt)
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

    // (Word Wrap)
#ifndef EM_WW_PARAGRAPH
#define EM_WW_PARAGRAPH 1
#endif

    SendMessage(hEdit, EM_SETWORDWRAPMODE, EM_WW_PARAGRAPH, 0);

    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    AppendMenu(hFileMenu, MF_STRING, 1, L"&New");
    AppendMenu(hFileMenu, MF_STRING, 2, L"&Open...");
    AppendMenu(hFileMenu, MF_STRING, 3, L"&Save");
    AppendMenu(hFileMenu, MF_STRING, 4, L"Save &As...");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hFileMenu, MF_STRING, 6, L"E&xit");

    HMENU hEditMenu = CreateMenu();
    AppendMenu(hEditMenu, MF_STRING, 7, L"&Undo\tCtrl+Z");
    AppendMenu(hEditMenu, MF_STRING, 8, L"&Redo\tCtrl+Y");
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hEditMenu, MF_STRING, 9, L"&Bold\tCtrl+B");
    AppendMenu(hEditMenu, MF_STRING, 10, L"&Italic\tCtrl+I");
    AppendMenu(hEditMenu, MF_STRING, 11, L"&Regular\tCtrl+R");

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, L"&Edit");
    SetMenu(hMainWindow, hMenu);

    // messages
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch (msg) 
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // choice
        switch (wmId) 
        {
        case 1:  // New
            if (QuerySave(hwnd)) { //save control
                filePath = L"";
                SetWindowText(hEdit, L"");
                SetWindowTitle(hwnd, L""); // "Notepad"
                isModified = false;
                //Clear History
                undoHistory.clear();
                redoHistory.clear();
                historyIndex = -1;
            }
            break;

        case 2:  // Open...
            if (QuerySave(hwnd)) 
            {
                if (OpenFile(hwnd)) 
                {
                    isModified = false;
                    //Clear History
                    undoHistory.clear();
                    redoHistory.clear();
                    historyIndex = -1;
                }
            }
            break;
        case 3:  // Save
            if (!filePath.empty()) 
            {
                if (SaveFile(hwnd)) 
                {
                    isModified = false;
                }
            }
            else 
                SaveFileAs(hwnd);
            break;
        case 4:  // Save As...
            if (SaveFileAs(hwnd)) 
                isModified = false;
            break;

        case 6:  // Exit
        {
            if (QuerySave(hwnd)) 
                DestroyWindow(hwnd);
            break;
        }
        case 7: // Undo
            Undo(hwnd);
            break;
        case 8: // Redo
            Redo(hwnd);
            break;
        case 9: // Bold
            SetBold(hwnd);
            break;
        case 10: // Italic
            SetItalic(hwnd);
            break;
        case 11: // Regular
            SetRegular(hwnd);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
    break;

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        MoveWindow(hEdit, 0, 0, width, height, TRUE);
        break;
    }

    case WM_SETFOCUS:
        SetFocus(hEdit); // Edit Control
        break;

    case WM_CLOSE:
    {
        if (QuerySave(hwnd))
            DestroyWindow(hwnd);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case EN_CHANGE:
    {
        if (LOWORD(wParam) == 1001) 
        {
            isModified = true;
            std::wstring filename;
            if (!filePath.empty()) 
                filename = filePath.substr(filePath.find_last_of(L"\\/") + 1) + L"*";
            
            SetWindowTitle(hwnd, filename);
        }
        break;
    }

    case WM_KEYDOWN:
        if ((GetKeyState(VK_CONTROL) & 0x8000)) 
        {
            switch (wParam) 
            {
            case 'Z':  // Ctrl+Z
                Undo(hwnd);
                break;
            case 'Y':  // Ctrl+Y
                Redo(hwnd);
                break;
            case 'B': // Ctrl + B
                SetBold(hwnd);
                break;
            case 'I': // Ctrl + I
                SetItalic(hwnd);
                break;
            case 'R': // Ctrl + R
                SetRegular(hwnd);
                break;
            }
        }
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

bool OpenFile(HWND hwnd) 
{
    OPENFILENAME ofn = {};
    wchar_t filename[256] = L"";
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename) / sizeof(filename[0]);
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY;

    if (GetOpenFileName(&ofn)) 
    {
        filePath = filename;
        std::wifstream file(filePath);

        if (!file.is_open()) 
        {
            DWORD error = GetLastError();
            LPVOID lpMsgBuf;
            FormatMessage
            (
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf,
                0, NULL
            );

            MessageBox(hwnd, (LPCTSTR)lpMsgBuf, L"Error opening file", MB_OK | MB_ICONERROR);
            LocalFree(lpMsgBuf);
            return false;
        }

        // Read the entire file content into a wstring
        std::wstring filenameOnly = filePath.substr(filePath.find_last_of(L"\\/") + 1);
        SetWindowTitle(hwnd, filenameOnly);

        std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
            (std::istreambuf_iterator<wchar_t>()));
        file.close();

        // Set the text in the RichEdit control
        SetWindowText(hEdit, content.c_str());

        return true;
    }
    return false;
}

bool SaveFile(HWND hwnd) 
{
    if (filePath.empty())
        return SaveFileAs(hwnd);

    std::wofstream file(filePath);

    if (!file.is_open()) 
    {
        DWORD error = GetLastError();
        LPVOID lpMsgBuf;
        FormatMessage
        (
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0, NULL
        );

        MessageBox(hwnd, (LPCTSTR)lpMsgBuf, L"Error saving file", MB_OK | MB_ICONERROR);
        LocalFree(lpMsgBuf);
        return false;
    }

    int len = GetWindowTextLength(hEdit);
    std::wstring content(len + 1, L'\0'); // buffer creation
    GetWindowText(hEdit, &content[0], len + 1);
    file << content;
    file.close();

    std::wstring filenameOnly = filePath.substr(filePath.find_last_of(L"\\/") + 1);
    SetWindowTitle(hwnd, filenameOnly);

    return true;
}

bool SaveFileAs(HWND hwnd) 
{
    OPENFILENAME ofn = {};
    wchar_t filename[256] = L"";
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename) / sizeof(filename[0]);
    ofn.lpstrDefExt = L"txt"; // Default extension
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;


    if (GetSaveFileName(&ofn)) 
    {
        filePath = filename;
        return SaveFile(hwnd);
    }
    return false;
}

//Window title creation
void SetWindowTitle(HWND hwnd, const std::wstring& filename) 
{
    std::wstring title = L"Notepad";
    if (!filename.empty()) 
        title = filename + L" - Notepad"; // If there is file name we add it
    
    SetWindowText(hwnd, title.c_str());
}

// unsaved modifications control
bool QuerySave(HWND hwnd) 
{
    if (isModified) 
    {
        int result = MessageBox(hwnd, L"Do you want to save changes?", L"Notepad", MB_YESNOCANCEL | MB_ICONWARNING);
        if (result == IDYES) 
        {
            if (SaveFile(hwnd)) 
                return true;
            else 
                return false;
        }
        else if (result == IDCANCEL) 
            return false;
    }
    return true;
}

void UpdateHistory(HWND hwnd) 
{
    int len = GetWindowTextLength(hEdit);
    std::wstring content(len + 1, L'\0');
    GetWindowText(hEdit, &content[0], len + 1);

    if (!redoHistory.empty()) 
        redoHistory.clear();

    if (undoHistory.size() >= MAX_HISTORY) 
        undoHistory.erase(undoHistory.begin());

    undoHistory.push_back(content);
    historyIndex = undoHistory.size() - 1;
}

void Undo(HWND hwnd) 
{
    if (historyIndex >= 0) 
    {
        int len = GetWindowTextLength(hEdit);
        std::wstring content(len + 1, L'\0');
        GetWindowText(hEdit, &content[0], len + 1);

        redoHistory.push_back(content);
        SetWindowText(hEdit, undoHistory[historyIndex].c_str());
        historyIndex--;

        isModified = true;
    }
}

void Redo(HWND hwnd) 
{
    if (redoHistory.size() > 0) 
    {
        historyIndex++;

        int len = GetWindowTextLength(hEdit);
        std::wstring content(len + 1, L'\0');
        GetWindowText(hEdit, &content[0], len + 1);
        undoHistory.push_back(content);

        std::wstring textForRedo = redoHistory.back();
        SetWindowText(hEdit, textForRedo.c_str());
        redoHistory.pop_back();
        isModified = true;
    }
}

void SetBold(HWND hwnd) 
{
    CHARFORMAT cf;
    memset(&cf, 0, sizeof(CHARFORMAT));
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_BOLD;
    cf.dwEffects = CFE_BOLD;

    SendMessage(hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void SetItalic(HWND hwnd) 
{
    CHARFORMAT cf;
    memset(&cf, 0, sizeof(CHARFORMAT));
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_ITALIC;
    cf.dwEffects = CFE_ITALIC;

    SendMessage(hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void SetRegular(HWND hwnd) 
{
    CHARFORMAT cf;
    memset(&cf, 0, sizeof(CHARFORMAT));
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_BOLD | CFM_ITALIC;  // delete formatting
    cf.dwEffects = 0;                   // close effects

    SendMessage(hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}