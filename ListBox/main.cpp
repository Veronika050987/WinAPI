#include<Windows.h>
#include<Commctrl.h>
#include<cstdio>
#include"resource.h"
#include<string>
#include<vector>

std::vector<std::string> g_list_values = { "This", "is", "my", "First", "List", "Box", "Хорошо", "живёт", "на свете", "Винни", "Пух", "Как-то так" };

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AddEditDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct AddEditData 
{
    int index = -1;
    std::string value;
    HWND hListBox = NULL;
};

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)DlgProc, 0);
    return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hListBox;

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
        SendMessage(hwnd, WM_SETICON, 0, (LPARAM)hIcon);

        hListBox = GetDlgItem(hwnd, IDC_LIST1);
        for (const auto& value : g_list_values)
        {
            SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)value.c_str()); // LB_ADDSTRING takes CHAR*
        }
    }
    break;
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            CONST INT SIZE = 256;
            CHAR sz_buffer[SIZE] = {};
            CHAR sz_message[SIZE] = {};

            int i = SendMessage(hListBox, LB_GETCURSEL, 0, 0); // If sent to a multiple-selection list box, LB_GETCURSEL returns the index of the item that has the focus rectangle.
            if (i != LB_ERR)
            {
                SendMessageA(hListBox, LB_GETTEXT, i, (LPARAM)sz_buffer);
                sprintf(sz_message, "You selected item %i with value '%s'", i, sz_buffer);
            }
            else
            {
                strcpy_s(sz_message, SIZE, "Please select an item.");
            }

            MessageBoxA(hwnd, sz_message, "Info", MB_OK | MB_ICONINFORMATION);
        }
        break;

        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        case IDC_BUTTON_ADD:
        {
            AddEditData data;
            data.hListBox = hListBox;

            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_ADDEDIT), hwnd, (DLGPROC)AddEditDlgProc, (LPARAM)&data);

            SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
            for (const auto& value : g_list_values)
            {
                SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)value.c_str());
            }
        }
        break;

        case IDC_BUTTON_DELETE:
        {
            int i = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
            if (i != LB_ERR)
            {
                g_list_values.erase(g_list_values.begin() + i);

                SendMessage(hListBox, LB_DELETESTRING, i, 0);

                if (SendMessage(hListBox, LB_GETCOUNT, 0, 0) > 0)
                {
                    SendMessage(hListBox, LB_SETCURSEL, min(i, (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0) - 1), 0);
                }
            }
            else
            {
                MessageBox(hwnd, "Please select an item to delete.", "Error", MB_OK | MB_ICONERROR);
            }
        }
        break;
        }
    }
    break;

    case WM_NOTIFY:
    {
        if (((LPNMHDR)lParam)->hwndFrom == hListBox && ((LPNMHDR)lParam)->code == NM_DBLCLK)
        {
            int i = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
            if (i != LB_ERR)
            {
                AddEditData data;
                data.index = i;
                data.value = g_list_values[i];
                data.hListBox = hListBox;

                DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_ADDEDIT), hwnd, (DLGPROC)AddEditDlgProc, (LPARAM)&data);

                SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
                for (const auto& value : g_list_values)
                {
                    SendMessageA(hListBox, LB_ADDSTRING, 0, (LPARAM)value.c_str());
                }
            }
            else
            {
                MessageBox(hwnd, "Please select an item to edit.", "Error", MB_OK | MB_ICONERROR);
            }
        }
    }
    break;

    case WM_CLOSE:
        EndDialog(hwnd, 0);
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK AddEditDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static AddEditData* pData;

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        pData = (AddEditData*)lParam;
        if (pData->index != -1)
        {
            SetWindowTextA(GetDlgItem(hwnd, IDC_EDIT_ADDEDIT), pData->value.c_str()); // Set initial value for editing
        }
    }
    break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            CONST INT SIZE = 256;
            CHAR sz_buffer[SIZE] = {};
            GetWindowTextA(GetDlgItem(hwnd, IDC_EDIT_ADDEDIT), sz_buffer, SIZE);
            std::string new_value = sz_buffer;

            if (new_value.empty())
            {
                MessageBox(hwnd, "Value cannot be empty.", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            if (pData->index == -1)
            {
                g_list_values.push_back(new_value);
            }
            else
            {
                g_list_values[pData->index] = new_value;
            }

            EndDialog(hwnd, 1);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hwnd, 0);
            return TRUE;
        }
    }
    break;

    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return TRUE;
    }

    return FALSE;
}