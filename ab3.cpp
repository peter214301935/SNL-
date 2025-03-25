#include <windows.h>
#include <commctrl.h>
#include <string>
#include <fstream>
#include <ctime>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

// 全局变量
HWND hInput, hMipsOutput, hErrorOutput, hCompileBtn, hSaveBtn, hClearBtn;
HWND hTabControl;

// 字符串转换工具函数
wstring StringToWString(const string& narrowStr) {
    int wideStrLength = MultiByteToWideChar(CP_ACP, 0, narrowStr.c_str(), -1, nullptr, 0);
    if (wideStrLength <= 0) return L"";

    vector<wchar_t> wideStr(wideStrLength);
    if (MultiByteToWideChar(CP_ACP, 0, narrowStr.c_str(), -1, wideStr.data(), wideStrLength) == 0) {
        return L"";
    }
    return wstring(wideStr.data());
}

string WStringToString(const wstring& wideStr) {
    int narrowStrLength = WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (narrowStrLength <= 0) return "";

    vector<char> narrowStr(narrowStrLength);
    if (WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), -1, narrowStr.data(), narrowStrLength, nullptr, nullptr) == 0) {
        return "";
    }
    return string(narrowStr.data());
}

// 模拟编译函数 - 替换为你的实际编译器
bool compileSNL(const string& snlCode, string& mipsCode, string& errorMsg) {
    if (snlCode.find("error") != string::npos) {
        errorMsg = "编译错误: 在代码中检测到 'error' 关键字\n";
        errorMsg += "第 5 行: 语法错误\n";
        errorMsg += "第 10 行: 未定义的变量";
        return false;
    }

    mipsCode = "# 自动生成的 MIPS 代码\n";

    // 使用安全的ctime_s版本
    time_t now = time(nullptr);
    if (now == -1) {
        mipsCode += "# 编译时间: [时间获取失败]\n";
    }
    else {
        char timeStr[26];
        if (ctime_s(timeStr, sizeof(timeStr), &now) == 0) {
            mipsCode += "# 编译时间: " + string(timeStr);
        }
        else {
            mipsCode += "# 编译时间: [时间格式化失败]\n";
        }
    }

    mipsCode += ".text\n";
    mipsCode += ".globl main\n\n";
    mipsCode += "main:\n";
    mipsCode += "    li $v0, 4       # 打印字符串\n";
    mipsCode += "    la $a0, hello\n";
    mipsCode += "    syscall\n\n";
    mipsCode += "    li $v0, 10      # 退出程序\n";
    mipsCode += "    syscall\n\n";
    mipsCode += ".data\n";
    mipsCode += "hello: .asciiz \"Hello from SNL compiler!\"\n";

    return true;
}

void saveToFile(const wstring& filename, const string& content) {
    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
        wstring message = L"内容已保存到:\n" + filename;
        MessageBoxW(NULL, message.c_str(), L"成功", MB_OK);
    }
    else {
        wstring errorMsg = L"无法保存文件:\n" + filename;
        MessageBoxW(NULL, errorMsg.c_str(), L"错误", MB_ICONERROR);
    }
}

wstring getCurrentTimeStamp() {
    time_t now = time(nullptr);
    if (now == -1) {
        return L"error_time";
    }

    tm localTm;
    if (localtime_s(&localTm, &now) != 0) {
        return L"error_time";
    }

    wchar_t buffer[80];
    if (wcsftime(buffer, sizeof(buffer) / sizeof(wchar_t), L"%Y%m%d_%H%M%S", &localTm) == 0) {
        return L"error_time";
    }

    return wstring(buffer);
}

void Compile() {
    int len = GetWindowTextLengthW(hInput) + 1;
    if (len <= 1) {
        MessageBoxW(NULL, L"请输入 SNL 代码!", L"警告", MB_ICONWARNING);
        return;
    }

    vector<wchar_t> buffer(len);
    if (GetWindowTextW(hInput, buffer.data(), len) == 0) {
        MessageBoxW(NULL, L"获取文本失败!", L"错误", MB_ICONERROR);
        return;
    }

    wstring snlCodeW(buffer.data());
    string snlCode = WStringToString(snlCodeW);

    if (snlCode.empty()) {
        MessageBoxW(NULL, L"请输入 SNL 代码!", L"警告", MB_ICONWARNING);
        return;
    }

    string mipsCode, errorMsg;
    bool success = compileSNL(snlCode, mipsCode, errorMsg);

    wstring mipsCodeW = StringToWString(mipsCode);
    wstring errorMsgW = StringToWString(errorMsg);

    SetWindowTextW(hMipsOutput, mipsCodeW.c_str());
    SetWindowTextW(hErrorOutput, errorMsgW.c_str());

    if (success) {
        ShowWindow(GetDlgItem(hTabControl, 1), SW_HIDE);
        TabCtrl_SetCurSel(hTabControl, 0);
        wstring filename1 = L"snl_" + getCurrentTimeStamp() + L".txt";
        saveToFile(filename1, snlCode);
        wstring filename2 = L"mips_" + getCurrentTimeStamp() + L".txt";
        saveToFile(filename2, mipsCode);
    }
    else {
        ShowWindow(GetDlgItem(hTabControl, 1), SW_SHOW);
        TabCtrl_SetCurSel(hTabControl, 1);
    }
}

void SaveMIPS() {
    int len = GetWindowTextLengthW(hMipsOutput) + 1;
    if (len <= 1) {
        MessageBoxW(NULL, L"没有可保存的 MIPS 代码!", L"警告", MB_ICONWARNING);
        return;
    }

    vector<wchar_t> buffer(len);
    if (GetWindowTextW(hMipsOutput, buffer.data(), len) == 0) {
        MessageBoxW(NULL, L"获取文本失败!", L"错误", MB_ICONERROR);
        return;
    }

    wstring mipsCodeW(buffer.data());
    string mipsCode = WStringToString(mipsCodeW);

    if (mipsCode.empty()) {
        MessageBoxW(NULL, L"没有可保存的 MIPS 代码!", L"警告", MB_ICONWARNING);
        return;
    }

    wchar_t filename[MAX_PATH] = { 0 };
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn)) {
        saveToFile(filename, mipsCode);
    }
}

void ClearAll() {
    SetWindowTextW(hInput, L"");
    SetWindowTextW(hMipsOutput, L"");
    SetWindowTextW(hErrorOutput, L"");
    ShowWindow(GetDlgItem(hTabControl, 1), SW_HIDE);
    TabCtrl_SetCurSel(hTabControl, 0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
        if ((HWND)lParam == hCompileBtn) {
            Compile();
        }
        else if ((HWND)lParam == hSaveBtn) {
            SaveMIPS();
        }
        else if ((HWND)lParam == hClearBtn) {
            ClearAll();
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszClassName = L"SNLCompilerClass";

    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"窗口注册失败!", L"错误", MB_ICONERROR);
        return 0;
    }

    HWND hWnd = CreateWindowW(
        L"SNLCompilerClass", L"SNL 编译器",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        MessageBoxW(NULL, L"窗口创建失败!", L"错误", MB_ICONERROR);
        return 0;
    }

    // 创建控件
    CreateWindowW(L"STATIC", L"SNL 代码输入:", WS_CHILD | WS_VISIBLE,
        10, 10, 120, 20, hWnd, NULL, hInstance, NULL);

    hInput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        10, 30, 780, 200, hWnd, NULL, hInstance, NULL);

    hCompileBtn = CreateWindowW(L"BUTTON", L"编译", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 240, 100, 30, hWnd, NULL, hInstance, NULL);

    hClearBtn = CreateWindowW(L"BUTTON", L"清空", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        120, 240, 100, 30, hWnd, NULL, hInstance, NULL);

    hSaveBtn = CreateWindowW(L"BUTTON", L"保存 MIPS", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        230, 240, 100, 30, hWnd, NULL, hInstance, NULL);

    // 创建标签控件
    hTabControl = CreateWindowW(WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        10, 280, 780, 280, hWnd, NULL, hInstance, NULL);

    // 添加标签
    TCITEMW tie = { 0 };
    tie.mask = TCIF_TEXT;

    wchar_t tab1[] = L"MIPS 代码";
    tie.pszText = tab1;
    TabCtrl_InsertItem(hTabControl, 0, &tie);

    wchar_t tab2[] = L"错误信息";
    tie.pszText = tab2;
    TabCtrl_InsertItem(hTabControl, 1, &tie);

    // 创建标签页内容
    hMipsOutput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
        20, 310, 760, 240, hWnd, NULL, hInstance, NULL);

    hErrorOutput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
        20, 310, 760, 240, hWnd, NULL, hInstance, NULL);

    ShowWindow(hErrorOutput, SW_HIDE);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}