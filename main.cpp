#include "Solver.h"
#include "Ligand.h"
#include "LigandCSVLoader.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>

#define IDM_FILE_EXIT 40001
#define IDM_LIGAND_EDITOR 40002
#define IDM_HELP_ABOUT 40003

#define IDC_CALCULATE_BTN 50100
#define IDC_RESET_BTN 50110
#define IDC_TEMP_INPUT 50101
#define IDC_IONIC_INPUT 50102
#define IDC_PH_INPUT 50103

// Ligand section: 10 ligands
#define IDC_LIGAND_COMBO_BASE 51000
#define IDC_LIGAND_CONC_BASE 51100

// Cation section: 7 cations
#define IDC_CATION_FREE_BASE 52000
#define IDC_CATION_UNIT_BASE 52100
#define IDC_CATION_TOTAL_BASE 52200
#define IDC_CATION_RADIO_FREE_BASE 52300
#define IDC_CATION_RADIO_TOTAL_BASE 52400

#define IDC_OUTPUT_TEXT 53000

// Cation names (7 total)
static const char* g_cationNames[] = {"Ca2", "Mg2", "Ba2", "Cd2", "Sr2", "Mn2", "X1"};
static const int g_numCations = 7;
static const int g_numLigands = 10;

static CationSystem g_solver;
static int g_previousUnits[7] = {1, 1, 1, 1, 1, 1, 1}; // Track previous unit for each cation (default uM)

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

static void SetTextInCtrl(HWND parent, int ctrlId, const char* text) {
    HWND ctrl = GetDlgItem(parent, ctrlId);
    if (!ctrl) return;
    SetWindowText(ctrl, text);
}

static double GetDoubleFromCtrl(HWND parent, int ctrlId) {
    return atof(GetTextFromCtrl(parent, ctrlId).c_str());
}

// Helper: Convert concentration between units (0=nM, 1=uM, 2=mM)
// Returns raw value in target unit
static double ConvertConcentration(double valueMolar, int targetUnit) {
    switch (targetUnit) {
    case 0: return valueMolar * 1e9;   // M to nM
    case 1: return valueMolar * 1e6;   // M to uM
    case 2: return valueMolar * 1e3;   // M to mM
    }
    return valueMolar;
}

// Helper: Determine best unit for displaying a concentration (in Molar)
// Returns 0=nM, 1=uM, 2=mM based on what looks best
static int DetermineBestUnit(double valueMolar) {
    if (valueMolar <= 0) return 1; // Default to uM for zero/invalid
    
    double nM = valueMolar * 1e9;
    double uM = valueMolar * 1e6;
    double mM = valueMolar * 1e3;
    
    // Pick the unit that gives a value between 1 and 999
    if (nM >= 1 && nM < 1000) return 0; // nM
    if (uM >= 1 && uM < 1000) return 1; // uM
    return 2; // mM
}

static void PopulateCombo(HWND parent, int ctrlId, const std::vector<std::string>& items) {
    HWND combo = GetDlgItem(parent, ctrlId);
    if (!combo) return;
    SendMessage(combo, CB_RESETCONTENT, 0, 0);
    SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)"<None>");
    for (const auto& item : items) {
        SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)item.c_str());
    }
    SendMessage(combo, CB_SETCURSEL, 0, 0);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // Menu
        HMENU hMenu = CreateMenu();
        HMENU hFile = CreatePopupMenu();
        AppendMenu(hFile, MF_STRING, IDM_LIGAND_EDITOR, "Ligand Editor");
        AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
        AppendMenu(hFile, MF_STRING, IDM_FILE_EXIT, "Exit");
        
        HMENU hHelp = CreatePopupMenu();
        AppendMenu(hHelp, MF_STRING, IDM_HELP_ABOUT, "About");
        
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, "File");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, "Help");
        SetMenu(hwnd, hMenu);

        int y = 10;

        // Solution Parameters section
        CreateWindow("STATIC", "Solution Parameters:", WS_VISIBLE | WS_CHILD, 10, y, 150, 20, hwnd, NULL, NULL, NULL);
        y += 25;
        
        CreateWindow("STATIC", "Temperature (°C):", WS_VISIBLE | WS_CHILD, 20, y, 120, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("EDIT", "25.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, y, 80, 20, hwnd, (HMENU)IDC_TEMP_INPUT, NULL, NULL);
        y += 25;
        
        CreateWindow("STATIC", "Ionic Strength (mM):", WS_VISIBLE | WS_CHILD, 20, y, 120, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("EDIT", "100.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, y, 80, 20, hwnd, (HMENU)IDC_IONIC_INPUT, NULL, NULL);
        y += 25;
        
        CreateWindow("STATIC", "pH:", WS_VISIBLE | WS_CHILD, 20, y, 120, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("EDIT", "7.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, y, 80, 20, hwnd, (HMENU)IDC_PH_INPUT, NULL, NULL);
        y += 30;

        // Ligands section
        CreateWindow("STATIC", "Ligands (concentration in mM):", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, y, 300, 20, hwnd, NULL, NULL, NULL);
        y += 25;
        
        for (int i = 0; i < g_numLigands; i++) {
            CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 20, y, 120, 200, hwnd, (HMENU)(IDC_LIGAND_COMBO_BASE + i), NULL, NULL);
            CreateWindow("EDIT", "0", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, y, 60, 20, hwnd, (HMENU)(IDC_LIGAND_CONC_BASE + i), NULL, NULL);
            CreateWindow("STATIC", "mM", WS_VISIBLE | WS_CHILD, 220, y, 30, 20, hwnd, NULL, NULL, NULL);
            y += 22;
        }

        y += 10;

        // Cations section
        CreateWindow("STATIC", "Cations:", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, y, 350, 20, hwnd, NULL, NULL, NULL);
        y += 25;

        CreateWindow("STATIC", "Ion", WS_VISIBLE | WS_CHILD, 15, y, 40, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("STATIC", "Total (mM)", WS_VISIBLE | WS_CHILD, 80, y, 70, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("STATIC", "Free", WS_VISIBLE | WS_CHILD, 160, y, 80, 20, hwnd, NULL, NULL, NULL);
        CreateWindow("STATIC", "Unit", WS_VISIBLE | WS_CHILD, 250, y, 50, 20, hwnd, NULL, NULL, NULL);
        y += 22;

        for (int i = 0; i < g_numCations; i++) {
            CreateWindow("STATIC", g_cationNames[i], WS_VISIBLE | WS_CHILD, 15, y, 40, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 80, y, 70, 20, hwnd, (HMENU)(IDC_CATION_TOTAL_BASE + i), NULL, NULL);
            CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 160, y, 80, 20, hwnd, (HMENU)(IDC_CATION_FREE_BASE + i), NULL, NULL);
            
            HWND unitCombo = CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 250, y, 50, 100, hwnd, (HMENU)(IDC_CATION_UNIT_BASE + i), NULL, NULL);
            SendMessage(unitCombo, CB_ADDSTRING, 0, (LPARAM)"nM");
            SendMessage(unitCombo, CB_ADDSTRING, 0, (LPARAM)"uM");
            SendMessage(unitCombo, CB_ADDSTRING, 0, (LPARAM)"mM");
            SendMessage(unitCombo, CB_SETCURSEL, 1, 0);
            
            y += 22;
        }

        y += 10;

        // Calculate and Reset buttons
        CreateWindow("BUTTON", "Calculate", WS_VISIBLE | WS_CHILD, 20, y, 100, 30, hwnd, (HMENU)IDC_CALCULATE_BTN, NULL, NULL);
        CreateWindow("BUTTON", "Reset", WS_VISIBLE | WS_CHILD, 130, y, 80, 30, hwnd, (HMENU)IDC_RESET_BTN, NULL, NULL);
        y += 45;

        // Output text
        CreateWindow("STATIC", "Results:", WS_VISIBLE | WS_CHILD, 10, y, 100, 20, hwnd, NULL, NULL, NULL);
        y += 25;
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 10, y, 400, 150, hwnd, (HMENU)IDC_OUTPUT_TEXT, NULL, NULL);

        InitializeAppData();
        
        // Populate ligand combos
        std::vector<std::string> ligands = g_solver.GetAvailableLigands();
        for (int i = 0; i < g_numLigands; i++) {
            PopulateCombo(hwnd, IDC_LIGAND_COMBO_BASE + i, ligands);
        }

        SetOutputText(hwnd, "Ready. Configure ligands and solution parameters, then click Calculate.");
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_EXIT:
            PostQuitMessage(0);
            return 0;
        case IDM_LIGAND_EDITOR:
            MessageBox(hwnd, "Ligand Editor feature coming in next update.\nCurrently, ligand data is read from ligands.csv.", "Info", MB_ICONINFORMATION);
            return 0;
        case IDM_HELP_ABOUT:
            MessageBox(hwnd, "Cation-Ligand Equilibrium Engine v1.0\n\nCalculates free and total ion concentrations\nin complexing solutions.", "About", MB_ICONINFORMATION);
            return 0;
        case IDC_CALCULATE_BTN: {
            // Get solution parameters
            double temp = GetDoubleFromCtrl(hwnd, IDC_TEMP_INPUT);
            double ionic = GetDoubleFromCtrl(hwnd, IDC_IONIC_INPUT);
            double pH = GetDoubleFromCtrl(hwnd, IDC_PH_INPUT);

            SystemParameters params;
            params.temperature = temp;
            params.ionicStrength = ionic;
            params.pH = pH;
            params.volume = 1.0;
            g_solver.SetParameters(params);

            // Build result output
            std::string output = "=== Equilibrium Calculation Results ===\n";
            output += "\n";
            output += "Solution Parameters:\n";
            output += "────────────────────────\n";
            char buf[128];
            sprintf(buf, "  Temperature: %.1f °C\n", temp);
            output += buf;
            sprintf(buf, "  Ionic Strength: %.1f mM\n", ionic);
            output += buf;
            sprintf(buf, "  pH: %.2f\n\n", pH);
            output += buf;

            output += "Selected Ligands:\n";
            output += "────────────────────────\n";
            for (int i = 0; i < g_numLigands; i++) {
                int ligandIdx = SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO_BASE + i, CB_GETCURSEL, 0, 0);
                if (ligandIdx > 0) { // Skip <None>
                    char ligandName[128] = {0};
                    SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO_BASE + i, CB_GETLBTEXT, ligandIdx, (LPARAM)ligandName);
                    double conc = GetDoubleFromCtrl(hwnd, IDC_LIGAND_CONC_BASE + i);
                    sprintf(buf, "  %s: %.6f M\n", ligandName, conc / 1000.0);
                    output += buf;
                }
            }
            output += "\n";

            output += "Cation Calculations:\n";
            output += "────────────────────────\n\n";

            for (int i = 0; i < g_numCations; i++) {
                double free = GetDoubleFromCtrl(hwnd, IDC_CATION_FREE_BASE + i);
                double total = GetDoubleFromCtrl(hwnd, IDC_CATION_TOTAL_BASE + i);
                int unit = SendDlgItemMessage(hwnd, IDC_CATION_UNIT_BASE + i, CB_GETCURSEL, 0, 0);

                // Convert free to M
                double freeMolar = free;
                switch (unit) {
                case 0: freeMolar = free * 1e-9; break; // nM
                case 1: freeMolar = free * 1e-6; break; // uM
                case 2: freeMolar = free * 1e-3; break; // mM
                }

                // If free is provided, calculate total
                if (free > 0 && total == 0) {
                    // Get first non-None ligand to test with
                    int ligandIdx = SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO_BASE, CB_GETCURSEL, 0, 0);
                    if (ligandIdx > 0) {
                        char ligandName[128] = {0};
                        SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO_BASE, CB_GETLBTEXT, ligandIdx, (LPARAM)ligandName);
                        double ligandConc = GetDoubleFromCtrl(hwnd, IDC_LIGAND_CONC_BASE) / 1000.0;
                        
                        auto res = g_solver.CalculateFreeToTotal(ligandConc, freeMolar, ligandName, g_cationNames[i]);
                        
                        // Populate result back to total box with 2 decimals
                        char totalStr[64];
                        sprintf(totalStr, "%.2f", res.totalMetal * 1000.0);
                        SetDlgItemText(hwnd, IDC_CATION_TOTAL_BASE + i, totalStr);
                        
                        // Update tracked unit (free was unchanged, but total is set)
                        g_previousUnits[i] = unit;
                        
                        sprintf(buf, "%s:\n", g_cationNames[i]);
                        output += buf;
                        sprintf(buf, "  Free:    %.3e M\n", res.freeMetal);
                        output += buf;
                        sprintf(buf, "  Total:   %.3e M (%.2f mM)\n", res.totalMetal, res.totalMetal * 1000.0);
                        output += buf;
                        sprintf(buf, "  Complex: %.3e M\n\n", res.complex);
                        output += buf;
                    }
                } else if (total > 0) {
                    // Get first non-None ligand to test with
                    int ligandIdx = SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO_BASE, CB_GETCURSEL, 0, 0);
                    if (ligandIdx > 0) {
                        char ligandName[128] = {0};
                        SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO_BASE, CB_GETLBTEXT, ligandIdx, (LPARAM)ligandName);
                        double ligandConc = GetDoubleFromCtrl(hwnd, IDC_LIGAND_CONC_BASE) / 1000.0;
                        
                        auto res = g_solver.CalculateTotalToFree(ligandConc, total / 1000.0, ligandName, g_cationNames[i]);
                        
                        // Determine best unit for display
                        int bestUnit = DetermineBestUnit(res.freeMetal);
                        double freeValue = ConvertConcentration(res.freeMetal, bestUnit);
                        
                        // Populate result back to free box with best unit
                        char freeStr[64];
                        sprintf(freeStr, "%.6f", freeValue);
                        SetDlgItemText(hwnd, IDC_CATION_FREE_BASE + i, freeStr);
                        
                        // Update unit selector to match best unit
                        SendDlgItemMessage(hwnd, IDC_CATION_UNIT_BASE + i, CB_SETCURSEL, bestUnit, 0);
                        
                        // Update tracked unit to the new best unit
                        g_previousUnits[i] = bestUnit;
                        
                        sprintf(buf, "%s:\n", g_cationNames[i]);
                        output += buf;
                        sprintf(buf, "  Free:    %.3e M\n", res.freeMetal);
                        output += buf;
                        sprintf(buf, "  Total:   %.3e M (%.2f mM)\n", res.totalMetal, res.totalMetal * 1000.0);
                        output += buf;
                        sprintf(buf, "  Complex: %.3e M\n\n", res.complex);
                        output += buf;
                    }
                }
            }

            SetOutputText(hwnd, output);
            return 0;
        }
        // Handle unit combo box changes
        default: {
            int ctrlId = LOWORD(wParam);
            int notifyCode = HIWORD(wParam);
            
            // Check if this is a unit combo box selection change
            if (notifyCode == CBN_SELCHANGE && ctrlId >= IDC_CATION_UNIT_BASE && ctrlId < IDC_CATION_UNIT_BASE + g_numCations) {
                int cationIndex = ctrlId - IDC_CATION_UNIT_BASE;
                int newUnit = SendDlgItemMessage(hwnd, IDC_CATION_UNIT_BASE + cationIndex, CB_GETCURSEL, 0, 0);
                int oldUnit = g_previousUnits[cationIndex];
                
                // Get the free concentration value in the box (in the old unit)
                double freeValue = GetDoubleFromCtrl(hwnd, IDC_CATION_FREE_BASE + cationIndex);
                
                if (freeValue > 0 && newUnit != oldUnit) {
                    // Convert from old unit to Molar
                    double freeMolar = 0.0;
                    switch (oldUnit) {
                    case 0: freeMolar = freeValue * 1e-9; break; // nM to M
                    case 1: freeMolar = freeValue * 1e-6; break; // uM to M
                    case 2: freeMolar = freeValue * 1e-3; break; // mM to M
                    }
                    
                    // Now convert to new unit
                    double newValue = ConvertConcentration(freeMolar, newUnit);
                    
                    // Update the free concentration box
                    char buf[64];
                    sprintf(buf, "%.6f", newValue);
                    SetDlgItemText(hwnd, IDC_CATION_FREE_BASE + cationIndex, buf);
                }
                
                // Update the tracked previous unit
                g_previousUnits[cationIndex] = newUnit;
                return 0;
            }
            break;
        }
        case IDC_RESET_BTN: {
            // Clear all cation free and total boxes
            for (int i = 0; i < g_numCations; i++) {
                SetDlgItemText(hwnd, IDC_CATION_FREE_BASE + i, "");
                SetDlgItemText(hwnd, IDC_CATION_TOTAL_BASE + i, "");
            }
            // Clear output
            SetOutputText(hwnd, "");
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

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Cation-Ligand Equilibrium Engine v1.0", 
                              WS_OVERLAPPEDWINDOW | WS_VSCROLL,
                              CW_USEDEFAULT, CW_USEDEFAULT, 500, 800, NULL, NULL, hInstance, NULL);
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
    std::cout << "Cation-Ligand Equilibrium Solver (Console Mode)" << std::endl;
    std::cout << "===============================================" << std::endl;
    InitializeLigandData();
    CationSystem solver;
    for (auto& ligand : GetAllLigands()) solver.AddLigand(ligand);
    for (auto& metal : GetAllMetals()) solver.AddMetal(metal);
    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0;
    params.pH = 7.0;
    params.volume = 1.0;
    solver.SetParameters(params);
    auto result = solver.CalculateTotalToFree(1.0e-3, 1.0e-4, "EDTA", "Ca2");
    std::cout << "EDTA-Ca2+ test: Free = " << result.freeMetal << " M\n";
    return 0;
}

#endif
