#include<Windows.h>
#include<Commctrl.h>
#include"resource.h"

#define LOGIN_PROMPT L"Enter user's name" //при нажатии на поле пароля, незаполнении поля логина и нажатии на кнопку ОК выдаётся сообщение 
                                           //об ошибке "Введите логин"
                                           //при незаполнении поля логина и нажатии кнопки ОК выдаётся сообщение об ошибке "Введите логин"
                                           //при не изменении поля логина и нажатии кнопки ОК выдаётся сообщение об ошибке "Введите логин"
                                           //выйти можно только с помощью клавиши Cancel
                                           //текст в поле логина стирается руками
#define LOGIN_ERROR_MSG L"Заполните поле \"Login\"!"

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc, 0);
    return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hEditLogin = NULL;
    HWND hEditPassword = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        hEditLogin = GetDlgItem(hwnd, IDC_EDIT_LOGIN);
        hEditPassword = GetDlgItem(hwnd, IDC_EDIT_PASSWORD);
        SendMessageW(hEditLogin, EM_SETLIMITTEXT, 31, 0);
        SetWindowTextW(hEditLogin, LOGIN_PROMPT); 
    }
    break;
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            WCHAR loginBuffer[256];
            GetWindowTextW(hEditLogin, loginBuffer, 256);

            if (wcscmp(loginBuffer, LOGIN_PROMPT) == 0 || loginBuffer[0] == L'\0')
            {
                MessageBoxW(hwnd, LOGIN_ERROR_MSG, L"Validation Error", MB_OK | MB_ICONERROR);
                SetFocus(hEditLogin);
                return TRUE;
            }

            MessageBoxW(hwnd, L"OK button was pressed", L"Info", MB_OK | MB_ICONINFORMATION);
            EndDialog(hwnd, 0);
            break;
        }
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;
        }
        break;
    }
    case WM_NOTIFY:
    {
        if (((LPNMHDR)lParam)->code == EN_SETFOCUS && ((LPNMHDR)lParam)->hwndFrom == GetDlgItem(hwnd, IDC_EDIT_LOGIN))
        {
            HWND hEditLogin = GetDlgItem(hwnd, IDC_EDIT_LOGIN);
            WCHAR buffer[256];
            GetWindowTextW(hEditLogin, buffer, 256);

            if (wcscmp(buffer, LOGIN_PROMPT) == 0)
            {
                SetWindowTextW(hEditLogin, L"");
            }
        }
        else if (((LPNMHDR)lParam)->code == EN_KILLFOCUS && ((LPNMHDR)lParam)->hwndFrom == GetDlgItem(hwnd, IDC_EDIT_LOGIN))
        {
            HWND hEditLogin = GetDlgItem(hwnd, IDC_EDIT_LOGIN);
            const int SIZE = 256;
            WCHAR sz_buffer[SIZE] = {};
            GetWindowTextW(hEditLogin, sz_buffer, SIZE);

            if (sz_buffer[0] == L'\0')
            {
                SetWindowTextW(hEditLogin, LOGIN_PROMPT);
            }
        }
        break;
    }
    case WM_CLOSE:
        EndDialog(hwnd, 0);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}