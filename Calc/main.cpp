#include <Windows.h>
#include "resource.h"
#include <string>
#include <sstream>

CONST CHAR g_sz_CLASS_NAME[] = "MyCalc";

INT WINAPI WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void CreateButton(HWND hwnd, int id, const char* text, int x, int y, int width, int height);
void UpdateDisplay(HWND hwnd, const std::string& text);
std::string GetDisplayText(HWND hwnd);

HWND hEditDisplay = NULL;

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
    //1) Регистрация класса окна:
    WNDCLASSEX wClass;
    ZeroMemory(&wClass, sizeof(wClass));

    wClass.style = 0;
    wClass.cbSize = sizeof(wClass);
    wClass.cbWndExtra = 0;
    wClass.cbClsExtra = 0;

    wClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    wClass.hInstance = hInstance;
    wClass.lpszMenuName = NULL;
    wClass.lpszClassName = g_sz_CLASS_NAME;
    wClass.lpfnWndProc = (WNDPROC)WndProc;

    if (!RegisterClassEx(&wClass))
    {
        MessageBox(NULL, "Class registration failed", "", MB_OK | MB_ICONERROR);
        return 0;
    }

    //2) Создание окна:
    HWND hwnd = CreateWindowEx
    (
        NULL,
        g_sz_CLASS_NAME,
        g_sz_CLASS_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    //3) Запуск цикла сообщений:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

INT WINAPI WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        hEditDisplay = CreateWindowEx
        (
            WS_EX_CLIENTEDGE,
            "EDIT", "0",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
            10, 10,
            300, 22,
            hwnd,
            (HMENU)IDC_EDIT_DISPLAY,
            GetModuleHandle(NULL),
            NULL
        );

        int buttonWidth = 50;
        int buttonHeight = 40;
        int buttonSpacingX = 10;
        int buttonSpacingY = 10;
        int startX = 10;
        int startY = 40;

        // Create number buttons
        int buttonId = IDC_BUTTON_7;
        for (int row = 0; row < 3; ++row) 
        {
            for (int col = 0; col < 3; ++col) 
            {
                CreateButton(hwnd, buttonId, std::to_string(buttonId - IDC_BUTTON_7 + 1).c_str(),
                    startX + col * (buttonWidth + buttonSpacingX),
                    startY + row * (buttonHeight + buttonSpacingY),
                    buttonWidth, buttonHeight);
                buttonId++;
            }
        }
        //Button 0
        CreateButton(hwnd, IDC_BUTTON_0, "0",
            startX + 1 * (buttonWidth + buttonSpacingX),
            startY + 3 * (buttonHeight + buttonSpacingY),
            buttonWidth, buttonHeight);

        CreateButton(hwnd, IDC_BUTTON_POINT, ".", startX + 2 * (buttonWidth + buttonSpacingX), 
            startY + 3 * (buttonHeight + buttonSpacingY), buttonWidth, buttonHeight);

        CreateButton(hwnd, IDC_BUTTON_PLUS, "+", startX + 3 * (buttonWidth + buttonSpacingX), 
            startY, buttonWidth, buttonHeight);
        CreateButton(hwnd, IDC_BUTTON_MINUS, "-", startX + 3 * (buttonWidth + buttonSpacingX), 
            startY + 1 * (buttonHeight + buttonSpacingY), buttonWidth, buttonHeight);
        CreateButton(hwnd, IDC_BUTTON_ASTER, "*", startX + 3 * (buttonWidth + buttonSpacingX), 
            startY + 2 * (buttonHeight + buttonSpacingY), buttonWidth, buttonHeight);
        CreateButton(hwnd, IDC_BUTTON_SLASH, "/", startX + 3 * (buttonWidth + buttonSpacingX), 
            startY + 3 * (buttonHeight + buttonSpacingY), buttonWidth, buttonHeight);

        CreateButton(hwnd, IDC_BUTTON_BSP, "<=", startX, startY + 3 * (buttonHeight + buttonSpacingY), 
            buttonWidth, buttonHeight);
        CreateButton(hwnd, IDC_BUTTON_CLR, "C", startX + 1 * (buttonWidth + buttonSpacingX), 
            startY + 3 * (buttonHeight + buttonSpacingY), buttonWidth, buttonHeight);
        CreateButton(hwnd, IDC_BUTTON_EQUAL, "=", startX + 2 * (buttonWidth + buttonSpacingX), 
            startY + 3 * (buttonHeight + buttonSpacingY), buttonWidth, buttonHeight);
    }
    break;

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        switch (id)
        {
        case IDC_BUTTON_0:
        case IDC_BUTTON_1:
        case IDC_BUTTON_2:
        case IDC_BUTTON_3:
        case IDC_BUTTON_4:
        case IDC_BUTTON_5:
        case IDC_BUTTON_6:
        case IDC_BUTTON_7:
        case IDC_BUTTON_8:
        case IDC_BUTTON_9:
        case IDC_BUTTON_POINT:
        {
            std::string currentText = GetDisplayText(hwnd);
            std::string buttonText;
            if (id == IDC_BUTTON_POINT)
            {
                buttonText = ".";
            }
            else
            {
                std::stringstream ss;
                ss << (id - IDC_BUTTON_0);
                buttonText = ss.str();
            }

            if (currentText == "0")
            {
                currentText = buttonText;
            }
            else
            {
                currentText += buttonText;
            }
            UpdateDisplay(hwnd, currentText);
            break;
        }
        case IDC_BUTTON_PLUS:
        case IDC_BUTTON_MINUS:
        case IDC_BUTTON_ASTER:
        case IDC_BUTTON_SLASH:
        {
            std::string currentText = GetDisplayText(hwnd);
            if (!currentText.empty())
            {
                std::string buttonText;

                switch (id)
                {
                case IDC_BUTTON_PLUS:
                    buttonText = "+";
                    break;
                case IDC_BUTTON_MINUS:
                    buttonText = "-";
                    break;
                case IDC_BUTTON_ASTER:
                    buttonText = "*";
                    break;
                case IDC_BUTTON_SLASH:
                    buttonText = "/";
                    break;
                }
                currentText += buttonText;

                UpdateDisplay(hwnd, currentText);
            }
            break;
        }
        case IDC_BUTTON_BSP: // Backspace
        {
            std::string currentText = GetDisplayText(hwnd);
            if (!currentText.empty())
            {
                currentText.pop_back();
                if (currentText.empty())
                {
                    currentText = "0";
                }
                UpdateDisplay(hwnd, currentText);
            }
            break;
        }
        case IDC_BUTTON_CLR: // Clear
        {
            UpdateDisplay(hwnd, "0");
            break;
        }
        case IDC_BUTTON_EQUAL:
        {
            std::string expression = GetDisplayText(hwnd);

            double result = 0;
            char operation = '\0';
            std::stringstream ss(expression);
            double num;

            if (ss >> num)
            {
                result = num;
                while (ss >> operation >> num)
                {
                    switch (operation)
                    {
                    case '+': result += num; break;
                    case '-': result -= num; break;
                    case '*': result *= num; break;
                    case '/':
                        if (num != 0) {
                            result /= num;
                        }
                        else
                        {
                            UpdateDisplay(hwnd, "Error: Division by zero");
                            break;
                        }
                        break;
                    default:
                        UpdateDisplay(hwnd, "Error: Invalid operation");
                        break;
                    }
                }
            }
            else
            {
                UpdateDisplay(hwnd, "Error: Invalid expression");
                break;
            }

            std::stringstream resultStream;
            resultStream << result; 

            UpdateDisplay(hwnd, resultStream.str());
            break;
        }

        }
        break;
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return FALSE;
}

void CreateButton(HWND hwnd, int id, const char* text, int x, int y, int width, int height)
{
    CreateWindowExA
    (
        0, "BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, width, height,
        hwnd, (HMENU)id, GetModuleHandle(NULL), NULL
    );
}

void UpdateDisplay(HWND hwnd, const std::string& text)
{
    if (hEditDisplay != NULL)
    {
        SetWindowTextA(hEditDisplay, text.c_str());
    }
}

std::string GetDisplayText(HWND hwnd)
{
    if (hEditDisplay == NULL) return "";
    const int bufferSize = 256;
    char buffer[bufferSize];
    GetWindowTextA(hEditDisplay, buffer, bufferSize);
    return std::string(buffer);
}