#include "Solver.h"
#include "Ligand.h"
#include "LigandCSVLoader.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>

#define IDM_FILE_EXIT 40001
#define IDM_CALC_RUN 40002
#define IDC_LIGAND_COMBO 50001
#define IDC_METAL_COMBO 50002
#define IDC_TOTAL_LIGAND 50003
#define IDC_TOTAL_METAL 50004
#define IDC_OUTPUT_TEXT 50005

static CationSystem g_solver;

static void InitializeAppData() {
    InitializeLigandData();

    g_solver.Clear();
    for (auto& ligand : GetAllLigands()) {
        g_solver.AddLigand(ligand);
    }
    for (auto& metal : GetAllMetals()) {
        g_solver.AddMetal(metal);
    }

    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0;
    params.pH = 7.0;
    params.volume = 1.0;
    g_solver.SetParameters(params);
}

static void PopulateCombo(HWND parent, int ctrlId, const std::vector<std::string>& items) {
    HWND combo = GetDlgItem(parent, ctrlId);
    if (!combo) return;
    SendMessage(combo, CB_RESETCONTENT, 0, 0);
    for (const auto& item : items) {
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)item.c_str());
    }
    if (!items.empty()) {
        SendMessage(combo, CB_SETCURSEL, 0, 0);
    }
}

static void SetOutputText(HWND parent, const std::string& text) {
    HWND out = GetDlgItem(parent, IDC_OUTPUT_TEXT);
    if (!out) return;
    SetWindowText(out, text.c_str());
}

static std::string GetTextFromCtrl(HWND parent, int ctrlId) {
    HWND ctrl = GetDlgItem(parent, ctrlId);
    if (!ctrl) return "";
    char buf[256] = {0};
    GetWindowText(ctrl, buf, sizeof(buf));
    return std::string(buf);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HMENU hMenu = CreateMenu();
        HMENU hFile = CreatePopupMenu();
        AppendMenu(hFile, MF_STRING, IDM_CALC_RUN, "Run Calculation");
        AppendMenu(hFile, MF_STRING, IDM_FILE_EXIT, "Exit");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, "File");
        SetMenu(hwnd, hMenu);

        CreateWindow("STATIC", "Ligand:", WS_VISIBLE | WS_CHILD, 20, 20, 80, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 110, 20, 180, 200, hwnd, (HMENU)IDC_LIGAND_COMBO, NULL, NULL);

        CreateWindow("STATIC", "Metal:", WS_VISIBLE | WS_CHILD, 20, 60, 80, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 110, 60, 180, 200, hwnd, (HMENU)IDC_METAL_COMBO, NULL, NULL);

        CreateWindow("STATIC", "Total Ligand (M):", WS_VISIBLE | WS_CHILD, 20, 100, 120, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("EDIT", "0.001", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 150, 100, 140, 20, hwnd, (HMENU)IDC_TOTAL_LIGAND, NULL, NULL);

        CreateWindow("STATIC", "Total Metal (M):", WS_VISIBLE | WS_CHILD, 20, 140, 120, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("EDIT", "0.0001", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 150, 140, 140, 20, hwnd, (HMENU)IDC_TOTAL_METAL, NULL, NULL);

        CreateWindow("STATIC", "Output:", WS_VISIBLE | WS_CHILD, 20, 180, 80, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 20, 205, 420, 140, hwnd, (HMENU)IDC_OUTPUT_TEXT, NULL, NULL);

        InitializeAppData();
        PopulateCombo(hwnd, IDC_LIGAND_COMBO, g_solver.GetAvailableLigands());
        PopulateCombo(hwnd, IDC_METAL_COMBO, g_solver.GetAvailableMetals());
        SetOutputText(hwnd, "Ready. Select ligand/metal and choose File > Run Calculation.");
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_EXIT:
            PostQuitMessage(0);
            return 0;
        case IDM_CALC_RUN: {
            int ligandIndex = SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO, CB_GETCURSEL, 0, 0);
            int metalIndex = SendDlgItemMessage(hwnd, IDC_METAL_COMBO, CB_GETCURSEL, 0, 0);
            if (ligandIndex < 0 || metalIndex < 0) {
                SetOutputText(hwnd, "Please select a ligand and a metal.");
                return 0;
            }

            char ligandBuf[128] = {0};
            char metalBuf[128] = {0};
            SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO, CB_GETLBTEXT, ligandIndex, (LPARAM)ligandBuf);
            SendDlgItemMessage(hwnd, IDC_METAL_COMBO, CB_GETLBTEXT, metalIndex, (LPARAM)metalBuf);

            double totalLigand = atof(GetTextFromCtrl(hwnd, IDC_TOTAL_LIGAND).c_str());
            double totalMetal = atof(GetTextFromCtrl(hwnd, IDC_TOTAL_METAL).c_str());
            if (totalLigand <= 0 || totalMetal <= 0) {
                SetOutputText(hwnd, "Total ligand and metal must be > 0.");
                return 0;
            }

            auto res = g_solver.CalculateTotalToFree(totalLigand, totalMetal, ligandBuf, metalBuf);
            char out[512];
            sprintf(out, "Ligand: %s\r\nMetal: %s\r\nTotal ligand = %.8g M\r\nTotal metal = %.8g M\r\nFree ligand = %.8g M\r\nFree metal = %.8g M\r\nComplex = %.8g M\r\n", ligandBuf, metalBuf, res.totalLigand, res.totalMetal, res.freeLigand, res.freeMetal, res.complex);
            SetOutputText(hwnd, out);
            return 0;
        }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "CationEngineMainWindow";

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Cation-Ligand Equilibrium Engine", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 480, 420, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        MessageBox(NULL, "Failed to create main window", "Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

#else

int main() {
    std::cout << "Cation-Ligand Equilibrium Solver Demo" << std::endl;
    std::cout << "=====================================" << std::endl;

    InitializeLigandData();

    CationSystem solver;

    auto allLigands = GetAllLigands();
    for (const auto& ligand : allLigands) {
        solver.AddLigand(ligand);
    }

    auto allMetals = GetAllMetals();
    for (const auto& metal : allMetals) {
        solver.AddMetal(metal);
    }

    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0;
    params.pH = 7.0;
    params.volume = 1.0;
    solver.SetParameters(params);

    std::cout << "Available ligands:" << std::endl;
    auto ligands = solver.GetAvailableLigands();
    for (const auto& ligand : ligands) {
        std::cout << "  - " << ligand << std::endl;
    }

    std::cout << "\nAvailable metals:" << std::endl;
    auto metals = solver.GetAvailableMetals();
    for (const auto& metal : metals) {
        std::cout << "  - " << metal << std::endl;
    }

    auto result1 = solver.CalculateFreeToTotal(1.0e-3, 1.0e-4, "EDTA", "Ca2");
    std::cout << "Free ligand: " << result1.freeLigand << " M\n";
    std::cout << "Free metal: " << result1.freeMetal << " M\n";
    std::cout << "Complex: " << result1.complex << " M\n";

    return 0;
}

#endif
