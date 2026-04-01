#include "Solver.h"
#include "Ligand.h"
#include "LigandCSVLoader.h"
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")

// Add manifest for modern visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define IDM_FILE_EXIT 40001
#define IDM_LIGAND_EDITOR 40002
#define IDM_HELP_ABOUT 40003
#define IDM_FILE_NEW 40005
#define IDM_HELP_CONSTANTS 40006
#define IDM_MODE_WEBMAXC 40007
#define IDM_MODE_STANDARD 40008

#define IDC_CALCULATE_BTN 50100
#define IDC_RESET_BTN 50104
#define IDC_TEMP_INPUT 50101
#define IDC_IONIC_INPUT 50102
#define IDC_PH_INPUT 50103

// Ligand section: 10 ligands
#define IDC_LIGAND_COMBO_BASE 51000
#define IDC_LIGAND_CONC_BASE 51100

// Cation section: 9 cations
#define IDC_CATION_FREE_BASE 52000
#define IDC_CATION_UNIT_BASE 52100
#define IDC_CATION_TOTAL_BASE 52200
#define IDC_CATION_RADIO_FREE_BASE 52300
#define IDC_CATION_RADIO_TOTAL_BASE 52400

#define IDC_OUTPUT_TEXT 53000

// Cation names (9 total)
static const char* g_cationNames[] = {"Ca2", "Mg2", "Ba2", "Cd2", "Sr2", "Mn2", "Fe2", "Cu2", "Zn2"};
static const char* g_cationLabels[] = {"Ca2+", "Mg2+", "Ba2+", "Cd2+", "Sr2+", "Mn2+", "Fe2+", "Cu2+", "Zn2+"};
static const int g_numCations = 9;
static const int g_numLigands = 10;

static CationSystem g_solver;
static int g_previousUnits[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1}; // Track previous unit for each cation (default uM)
static bool g_calculationDone = false; // Track if calculation has been performed

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

static double ConvertConcentration(double valueMolar, int targetUnit) {
    switch (targetUnit) {
    case 0: return valueMolar * 1e9;
    case 1: return valueMolar * 1e6;
    case 2: return valueMolar * 1e3;
    }
    return valueMolar;
}

static int DetermineBestUnit(double valueMolar) {
    if (valueMolar <= 0) return 1;
    double nM = valueMolar * 1e9;
    double uM = valueMolar * 1e6;
    if (nM >= 1 && nM < 1000) return 0;
    if (uM >= 1 && uM < 1000) return 1;
    return 2;
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

#define IDC_LIGAND_LIST 60001
#define IDC_EDIT_NAME 60002
#define IDC_EDIT_VALENCE 60003
#define IDC_EDIT_H1 60004
#define IDC_EDIT_H2 60005
#define IDC_EDIT_H3 60006
#define IDC_EDIT_H4 60007
#define IDC_EDIT_CA1 60008
#define IDC_EDIT_MG1 60009
#define IDC_EDIT_BA1 60010
#define IDC_EDIT_CD1 60011
#define IDC_EDIT_SR1 60012
#define IDC_EDIT_MN1 60013
#define IDC_EDIT_FE1 60014
#define IDC_EDIT_CU1 60015
#define IDC_EDIT_ZN1 60016
#define IDC_EDIT_DH1 60017
#define IDC_EDIT_DH2 60018
#define IDC_EDIT_DH3 60019
#define IDC_EDIT_DH4 60020
#define IDC_EDIT_DCA1 60021
#define IDC_EDIT_DMG1 60022
#define IDC_EDIT_DBA1 60023
#define IDC_EDIT_DCD1 60024
#define IDC_EDIT_DSR1 60025
#define IDC_EDIT_DMN1 60026
#define IDC_EDIT_DFE1 60027
#define IDC_EDIT_DCU1 60028
#define IDC_EDIT_DZN1 60029
#define IDC_SAVE_BTN 60030
#define IDC_CANCEL_BTN 60031

static std::vector<Ligand> g_editorLigands;

static void PopulateLigandList(HWND dlg) {
    HWND list = GetDlgItem(dlg, IDC_LIGAND_LIST);
    if (!list) return;
    SendMessage(list, LB_RESETCONTENT, 0, 0);
    for (const auto& ligand : g_editorLigands) {
        SendMessage(list, LB_ADDSTRING, 0, (LPARAM)ligand.name.c_str());
    }
}

static void LoadLigandData(HWND dlg, int index) {
    if (index < 0 || index >= (int)g_editorLigands.size()) return;
    const Ligand& ligand = g_editorLigands[index];
    SetTextInCtrl(dlg, IDC_EDIT_NAME, ligand.name.c_str());
    char buf[32];
    sprintf(buf, "%d", ligand.valence); SetTextInCtrl(dlg, IDC_EDIT_VALENCE, buf);
    sprintf(buf, "%.2f", ligand.constants.log_K1); SetTextInCtrl(dlg, IDC_EDIT_H1, buf);
    sprintf(buf, "%.2f", ligand.constants.log_K2); SetTextInCtrl(dlg, IDC_EDIT_H2, buf);
    sprintf(buf, "%.2f", ligand.constants.log_K3); SetTextInCtrl(dlg, IDC_EDIT_H3, buf);
    sprintf(buf, "%.2f", ligand.constants.log_K4); SetTextInCtrl(dlg, IDC_EDIT_H4, buf);
    sprintf(buf, "%.2f", ligand.constants.Ca1); SetTextInCtrl(dlg, IDC_EDIT_CA1, buf);
    sprintf(buf, "%.2f", ligand.constants.Mg1); SetTextInCtrl(dlg, IDC_EDIT_MG1, buf);
    sprintf(buf, "%.2f", ligand.constants.Ba1); SetTextInCtrl(dlg, IDC_EDIT_BA1, buf);
    sprintf(buf, "%.2f", ligand.constants.Cd1); SetTextInCtrl(dlg, IDC_EDIT_CD1, buf);
    sprintf(buf, "%.2f", ligand.constants.Sr1); SetTextInCtrl(dlg, IDC_EDIT_SR1, buf);
    sprintf(buf, "%.2f", ligand.constants.Mn1); SetTextInCtrl(dlg, IDC_EDIT_MN1, buf);
    sprintf(buf, "%.2f", ligand.constants.X1); SetTextInCtrl(dlg, IDC_EDIT_FE1, buf);
    sprintf(buf, "%.2f", ligand.constants.Cu1); SetTextInCtrl(dlg, IDC_EDIT_CU1, buf);
    sprintf(buf, "%.2f", ligand.constants.Zn1); SetTextInCtrl(dlg, IDC_EDIT_ZN1, buf);
    sprintf(buf, "%.1f", ligand.constants.dlog_K1); SetTextInCtrl(dlg, IDC_EDIT_DH1, buf);
    sprintf(buf, "%.1f", ligand.constants.dlog_K2); SetTextInCtrl(dlg, IDC_EDIT_DH2, buf);
    sprintf(buf, "%.1f", ligand.constants.dlog_K3); SetTextInCtrl(dlg, IDC_EDIT_DH3, buf);
    sprintf(buf, "%.1f", ligand.constants.dlog_K4); SetTextInCtrl(dlg, IDC_EDIT_DH4, buf);
    sprintf(buf, "%.1f", ligand.constants.dCa1); SetTextInCtrl(dlg, IDC_EDIT_DCA1, buf);
    sprintf(buf, "%.1f", ligand.constants.dMg1); SetTextInCtrl(dlg, IDC_EDIT_DMG1, buf);
    sprintf(buf, "%.1f", ligand.constants.dBa1); SetTextInCtrl(dlg, IDC_EDIT_DBA1, buf);
    sprintf(buf, "%.1f", ligand.constants.dCd1); SetTextInCtrl(dlg, IDC_EDIT_DCD1, buf);
    sprintf(buf, "%.1f", ligand.constants.dSr1); SetTextInCtrl(dlg, IDC_EDIT_DSR1, buf);
    sprintf(buf, "%.1f", ligand.constants.dMn1); SetTextInCtrl(dlg, IDC_EDIT_DMN1, buf);
    sprintf(buf, "%.1f", ligand.constants.dX1); SetTextInCtrl(dlg, IDC_EDIT_DFE1, buf);
    sprintf(buf, "%.1f", ligand.constants.dCu1); SetTextInCtrl(dlg, IDC_EDIT_DCU1, buf);
    sprintf(buf, "%.1f", ligand.constants.dZn1); SetTextInCtrl(dlg, IDC_EDIT_DZN1, buf);
}

static void SaveLigandData(HWND dlg, int index) {
    if (index < 0 || index >= (int)g_editorLigands.size()) return;
    Ligand& ligand = g_editorLigands[index];
    ligand.valence = atoi(GetTextFromCtrl(dlg, IDC_EDIT_VALENCE).c_str());
    ligand.constants.log_K1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_H1).c_str());
    ligand.constants.log_K2 = atof(GetTextFromCtrl(dlg, IDC_EDIT_H2).c_str());
    ligand.constants.log_K3 = atof(GetTextFromCtrl(dlg, IDC_EDIT_H3).c_str());
    ligand.constants.log_K4 = atof(GetTextFromCtrl(dlg, IDC_EDIT_H4).c_str());
    ligand.constants.Ca1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_CA1).c_str());
    ligand.constants.Mg1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_MG1).c_str());
    ligand.constants.Ba1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_BA1).c_str());
    ligand.constants.Cd1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_CD1).c_str());
    ligand.constants.Sr1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_SR1).c_str());
    ligand.constants.Mn1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_MN1).c_str());
    ligand.constants.X1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_FE1).c_str());
    ligand.constants.Cu1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_CU1).c_str());
    ligand.constants.Zn1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_ZN1).c_str());
    ligand.constants.dlog_K1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DH1).c_str());
    ligand.constants.dlog_K2 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DH2).c_str());
    ligand.constants.dlog_K3 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DH3).c_str());
    ligand.constants.dlog_K4 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DH4).c_str());
    ligand.constants.dCa1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DCA1).c_str());
    ligand.constants.dMg1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DMG1).c_str());
    ligand.constants.dBa1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DBA1).c_str());
    ligand.constants.dCd1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DCD1).c_str());
    ligand.constants.dSr1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DSR1).c_str());
    ligand.constants.dMn1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DMN1).c_str());
    ligand.constants.dX1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DFE1).c_str());
    ligand.constants.dCu1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DCU1).c_str());
    ligand.constants.dZn1 = atof(GetTextFromCtrl(dlg, IDC_EDIT_DZN1).c_str());
}

static void SaveLigandsToFile() {
    std::string backupName = "ligands.csv.bak";
    CopyFile("ligands.csv", backupName.c_str(), FALSE);
    time_t now = time(NULL);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", localtime(&now));
    std::string newFileName = std::string("ligands_") + timestamp + ".csv";
    FILE* f = fopen(newFileName.c_str(), "w");
    if (!f) return;
    fprintf(f, "Ligand,Valence,H1,H2,H3,H4,Ca1,Mg1,Ba1,Cd1,Sr1,Mn1,Fe(II)1,Cu1,Zn1,dH1,dH2,dH3,dH4,dCa1,dMg1,dBa1,dCd1,dSr1,dMn1,dFe(II)1,dCu1,dZn1\n");
    for (const auto& ligand : g_editorLigands) {
        fprintf(f, "%s,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
                ligand.name.c_str(), ligand.valence,
                ligand.constants.log_K1, ligand.constants.log_K2, ligand.constants.log_K3, ligand.constants.log_K4,
                ligand.constants.Ca1, ligand.constants.Mg1, ligand.constants.Ba1, ligand.constants.Cd1,
                ligand.constants.Sr1, ligand.constants.Mn1, ligand.constants.X1, ligand.constants.Cu1, ligand.constants.Zn1,
                ligand.constants.dlog_K1, ligand.constants.dlog_K2, ligand.constants.dlog_K3, ligand.constants.dlog_K4,
                ligand.constants.dCa1, ligand.constants.dMg1, ligand.constants.dBa1, ligand.constants.dCd1,
                ligand.constants.dSr1, ligand.constants.dMn1, ligand.constants.dX1, ligand.constants.dCu1, ligand.constants.dZn1);
    }
    fclose(f);
    MessageBox(NULL, ("Ligands saved to " + newFileName + "\nOriginal backed up as ligands.csv.bak").c_str(), "Save Complete", MB_ICONINFORMATION);
}

LRESULT CALLBACK LigandEditorProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        int y = 10;
        CreateWindow("STATIC", "Ligands (Database):", WS_VISIBLE | WS_CHILD, 10, y, 150, 20, dlg, NULL, NULL, NULL);
        y += 25;
        CreateWindow("LISTBOX", "", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY | WS_VSCROLL, 10, y, 150, 450, dlg, (HMENU)IDC_LIGAND_LIST, NULL, NULL);
        int x = 180; y = 10;
        CreateWindow("STATIC", "Name:", WS_VISIBLE | WS_CHILD, x, y, 50, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY, x+60, y, 150, 20, dlg, (HMENU)IDC_EDIT_NAME, NULL, NULL);
        y += 30;
        CreateWindow("STATIC", "Valence:", WS_VISIBLE | WS_CHILD, x, y, 60, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+70, y, 60, 20, dlg, (HMENU)IDC_EDIT_VALENCE, NULL, NULL);
        y += 30;
        CreateWindow("STATIC", "H1 (pKa1):", WS_VISIBLE | WS_CHILD, x, y, 70, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+80, y, 60, 20, dlg, (HMENU)IDC_EDIT_H1, NULL, NULL);
        CreateWindow("STATIC", "H2 (pKa2):", WS_VISIBLE | WS_CHILD, x+160, y, 70, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+240, y, 60, 20, dlg, (HMENU)IDC_EDIT_H2, NULL, NULL);
        y += 25;
        CreateWindow("STATIC", "H3 (pKa3):", WS_VISIBLE | WS_CHILD, x, y, 70, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+80, y, 60, 20, dlg, (HMENU)IDC_EDIT_H3, NULL, NULL);
        CreateWindow("STATIC", "H4 (pKa4):", WS_VISIBLE | WS_CHILD, x+160, y, 70, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+240, y, 60, 20, dlg, (HMENU)IDC_EDIT_H4, NULL, NULL);
        y += 30;
        CreateWindow("STATIC", "Ca1:", WS_VISIBLE | WS_CHILD, x, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+45, y, 60, 20, dlg, (HMENU)IDC_EDIT_CA1, NULL, NULL);
        CreateWindow("STATIC", "Mg1:", WS_VISIBLE | WS_CHILD, x+120, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+165, y, 60, 20, dlg, (HMENU)IDC_EDIT_MG1, NULL, NULL);
        CreateWindow("STATIC", "Ba1:", WS_VISIBLE | WS_CHILD, x+240, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+285, y, 60, 20, dlg, (HMENU)IDC_EDIT_BA1, NULL, NULL);
        y += 25;
        CreateWindow("STATIC", "Cd1:", WS_VISIBLE | WS_CHILD, x, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+45, y, 60, 20, dlg, (HMENU)IDC_EDIT_CD1, NULL, NULL);
        CreateWindow("STATIC", "Sr1:", WS_VISIBLE | WS_CHILD, x+120, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+165, y, 60, 20, dlg, (HMENU)IDC_EDIT_SR1, NULL, NULL);
        CreateWindow("STATIC", "Mn1:", WS_VISIBLE | WS_CHILD, x+240, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+285, y, 60, 20, dlg, (HMENU)IDC_EDIT_DMN1, NULL, NULL);
        y += 25;
        CreateWindow("STATIC", "Fe1:", WS_VISIBLE | WS_CHILD, x, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+45, y, 60, 20, dlg, (HMENU)IDC_EDIT_FE1, NULL, NULL);
        CreateWindow("STATIC", "Cu1:", WS_VISIBLE | WS_CHILD, x+120, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+165, y, 60, 20, dlg, (HMENU)IDC_EDIT_CU1, NULL, NULL);
        CreateWindow("STATIC", "Zn1:", WS_VISIBLE | WS_CHILD, x+240, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+285, y, 60, 20, dlg, (HMENU)IDC_EDIT_ZN1, NULL, NULL);
        y += 35;
        CreateWindow("STATIC", "ΔH1:", WS_VISIBLE | WS_CHILD, x, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+45, y, 60, 20, dlg, (HMENU)IDC_EDIT_DH1, NULL, NULL);
        CreateWindow("STATIC", "ΔH2:", WS_VISIBLE | WS_CHILD, x+120, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+165, y, 60, 20, dlg, (HMENU)IDC_EDIT_DH2, NULL, NULL);
        CreateWindow("STATIC", "ΔH3:", WS_VISIBLE | WS_CHILD, x+240, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+285, y, 60, 20, dlg, (HMENU)IDC_EDIT_DH3, NULL, NULL);
        y += 25;
        CreateWindow("STATIC", "ΔH4:", WS_VISIBLE | WS_CHILD, x, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+45, y, 60, 20, dlg, (HMENU)IDC_EDIT_DH4, NULL, NULL);
        y += 35;
        CreateWindow("STATIC", "ΔCa:", WS_VISIBLE | WS_CHILD, x, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+45, y, 60, 20, dlg, (HMENU)IDC_EDIT_DCA1, NULL, NULL);
        CreateWindow("STATIC", "ΔMg:", WS_VISIBLE | WS_CHILD, x+120, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+165, y, 60, 20, dlg, (HMENU)IDC_EDIT_DMG1, NULL, NULL);
        CreateWindow("STATIC", "ΔBa:", WS_VISIBLE | WS_CHILD, x+240, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+285, y, 60, 20, dlg, (HMENU)IDC_EDIT_DBA1, NULL, NULL);
        y += 25;
        CreateWindow("STATIC", "ΔCd:", WS_VISIBLE | WS_CHILD, x, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+45, y, 60, 20, dlg, (HMENU)IDC_EDIT_DCD1, NULL, NULL);
        CreateWindow("STATIC", "ΔSr:", WS_VISIBLE | WS_CHILD, x+120, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+165, y, 60, 20, dlg, (HMENU)IDC_EDIT_DSR1, NULL, NULL);
        CreateWindow("STATIC", "ΔMn:", WS_VISIBLE | WS_CHILD, x+240, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+285, y, 60, 20, dlg, (HMENU)IDC_EDIT_DMN1, NULL, NULL);
        y += 25;
        CreateWindow("STATIC", "ΔFe:", WS_VISIBLE | WS_CHILD, x, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+45, y, 60, 20, dlg, (HMENU)IDC_EDIT_DFE1, NULL, NULL);
        CreateWindow("STATIC", "ΔCu:", WS_VISIBLE | WS_CHILD, x+120, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+165, y, 60, 20, dlg, (HMENU)IDC_EDIT_DCU1, NULL, NULL);
        CreateWindow("STATIC", "ΔZn:", WS_VISIBLE | WS_CHILD, x+240, y, 40, 20, dlg, NULL, NULL, NULL);
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, x+285, y, 60, 20, dlg, (HMENU)IDC_EDIT_DZN1, NULL, NULL);
        y += 45;
        CreateWindow("BUTTON", "Save Changes", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 200, y, 120, 30, dlg, (HMENU)IDC_SAVE_BTN, NULL, NULL);
        CreateWindow("BUTTON", "Close", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 340, y, 100, 30, dlg, (HMENU)IDC_CANCEL_BTN, NULL, NULL);
        g_editorLigands = GetAllLigands();
        PopulateLigandList(dlg);
        if (!g_editorLigands.empty()) { SendMessage(GetDlgItem(dlg, IDC_LIGAND_LIST), LB_SETCURSEL, 0, 0); LoadLigandData(dlg, 0); }
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_LIGAND_LIST:
            if (HIWORD(wParam) == LBN_SELCHANGE) { int index = SendMessage(GetDlgItem(dlg, IDC_LIGAND_LIST), LB_GETCURSEL, 0, 0); LoadLigandData(dlg, index); }
            break;
        case IDC_SAVE_BTN: {
            int index = SendMessage(GetDlgItem(dlg, IDC_LIGAND_LIST), LB_GETCURSEL, 0, 0);
            if (index >= 0) { SaveLigandData(dlg, index); SaveLigandsToFile(); InitializeAppData(); MessageBox(dlg, "Saved stability constants to database.", "Success", MB_OK); }
            break;
        }
        case IDC_CANCEL_BTN: SendMessage(dlg, WM_CLOSE, 0, 0); break;
        }
        break;
    case WM_CLOSE: DestroyWindow(dlg); break;
    default: return DefWindowProc(dlg, msg, wParam, lParam);
    }
    return 0;
}

static bool ShowLigandEditorDialog(HWND parent) {
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSEXA wc; ZeroMemory(&wc, sizeof(WNDCLASSEXA));
        wc.cbSize = sizeof(WNDCLASSEXA); wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = LigandEditorProc; wc.hInstance = GetModuleHandleA(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = "LigandEditorDialog"; RegisterClassExA(&wc); classRegistered = true;
    }
    HWND dlg = CreateWindowExA(WS_EX_DLGMODALFRAME, "LigandEditorDialog", "Ligand Stability Constants Editor", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 650, 650, parent, NULL, GetModuleHandleA(NULL), NULL);
    ShowWindow(dlg, SW_SHOW); UpdateWindow(dlg); EnableWindow(parent, FALSE);
    MSG msg; while (IsWindow(dlg) && GetMessage(&msg, NULL, 0, 0)) { if (!IsDialogMessage(dlg, &msg)) { TranslateMessage(&msg); DispatchMessage(&msg); } }
    EnableWindow(parent, TRUE); SetForegroundWindow(parent); return true;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HMENU hMenu = CreateMenu();
        HMENU hFile = CreatePopupMenu(); AppendMenu(hFile, MF_STRING, IDM_FILE_NEW, "&New Calculation\tCtrl+N"); AppendMenu(hFile, MF_SEPARATOR, 0, NULL); AppendMenu(hFile, MF_STRING, IDM_FILE_EXIT, "E&xit"); AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, "&File");
        HMENU hEdit = CreatePopupMenu(); AppendMenu(hEdit, MF_STRING, IDM_LIGAND_EDITOR, "&Ligand Editor..."); AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEdit, "&Edit");
        HMENU hOptions = CreatePopupMenu(); AppendMenu(hOptions, MF_STRING, IDM_MODE_WEBMAXC, "WebMaxC Legacy Mode"); AppendMenu(hOptions, MF_STRING, IDM_MODE_STANDARD, "Industry Standard Mode (Davies)"); CheckMenuItem(hOptions, IDM_MODE_WEBMAXC, MF_BYCOMMAND | MF_CHECKED); AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hOptions, "&Options");
        HMENU hHelp = CreatePopupMenu(); AppendMenu(hHelp, MF_STRING, IDM_HELP_CONSTANTS, "&Lookup Stability Constants..."); AppendMenu(hHelp, MF_STRING, IDM_HELP_ABOUT, "&About"); AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, "&Help");
        SetMenu(hwnd, hMenu);

        int y = 10;
        CreateWindow("STATIC", "Solution Parameters:", WS_VISIBLE | WS_CHILD, 10, y, 150, 20, hwnd, NULL, NULL, NULL); y += 25;
        CreateWindow("STATIC", "Temperature (C):", WS_VISIBLE | WS_CHILD, 20, y, 120, 20, hwnd, NULL, NULL, NULL); CreateWindow("EDIT", "25.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, y, 80, 20, hwnd, (HMENU)(INT_PTR)IDC_TEMP_INPUT, NULL, NULL); y += 25;
        CreateWindow("STATIC", "Ionic Strength (mM):", WS_VISIBLE | WS_CHILD, 20, y, 130, 20, hwnd, NULL, NULL, NULL); CreateWindow("EDIT", "100.0", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, y, 80, 20, hwnd, (HMENU)(INT_PTR)IDC_IONIC_INPUT, NULL, NULL); y += 25;
        CreateWindow("STATIC", "pH:", WS_VISIBLE | WS_CHILD, 20, y, 120, 20, hwnd, NULL, NULL, NULL); CreateWindow("EDIT", "7.2", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, y, 80, 20, hwnd, (HMENU)(INT_PTR)IDC_PH_INPUT, NULL, NULL); y += 30;
        CreateWindow("STATIC", "Ligands (concentration in mM):", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, y, 300, 20, hwnd, NULL, NULL, NULL); y += 25;
        for (int i = 0; i < g_numLigands; i++) {
            CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 20, y, 120, 200, hwnd, (HMENU)(INT_PTR)(IDC_LIGAND_COMBO_BASE + i), NULL, NULL);
            CreateWindow("EDIT", "0", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, y, 60, 20, hwnd, (HMENU)(INT_PTR)(IDC_LIGAND_CONC_BASE + i), NULL, NULL);
            CreateWindow("STATIC", "mM", WS_VISIBLE | WS_CHILD, 220, y, 30, 20, hwnd, NULL, NULL, NULL); y += 22;
        }
        y += 10;
        CreateWindow("STATIC", "Cations:", WS_VISIBLE | WS_CHILD | WS_BORDER, 10, y, 350, 20, hwnd, NULL, NULL, NULL); y += 25;
        CreateWindow("STATIC", "Enter 'Free' to calc 'Total', or 'Total' to calc 'Free'.\nFocusing a box clears its counterpart.", WS_VISIBLE | WS_CHILD, 15, y, 350, 35, hwnd, NULL, NULL, NULL); y += 40;
        CreateWindow("STATIC", "Ion", WS_VISIBLE | WS_CHILD, 15, y, 40, 20, hwnd, NULL, NULL, NULL); CreateWindow("STATIC", "Total (mM)", WS_VISIBLE | WS_CHILD, 80, y, 70, 20, hwnd, NULL, NULL, NULL); CreateWindow("STATIC", "Free", WS_VISIBLE | WS_CHILD, 160, y, 80, 20, hwnd, NULL, NULL, NULL); CreateWindow("STATIC", "Unit", WS_VISIBLE | WS_CHILD, 250, y, 50, 20, hwnd, NULL, NULL, NULL); y += 22;
        for (int i = 0; i < g_numCations; i++) {
            CreateWindow("STATIC", g_cationLabels[i], WS_VISIBLE | WS_CHILD, 15, y, 40, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 80, y, 70, 20, hwnd, (HMENU)(INT_PTR)(IDC_CATION_TOTAL_BASE + i), NULL, NULL);
            CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 160, y, 80, 20, hwnd, (HMENU)(INT_PTR)(IDC_CATION_FREE_BASE + i), NULL, NULL);
            HWND unitCombo = CreateWindow("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 250, y, 50, 100, hwnd, (HMENU)(INT_PTR)(IDC_CATION_UNIT_BASE + i), NULL, NULL);
            SendMessage(unitCombo, CB_ADDSTRING, 0, (LPARAM)"nM"); SendMessage(unitCombo, CB_ADDSTRING, 0, (LPARAM)"uM"); SendMessage(unitCombo, CB_ADDSTRING, 0, (LPARAM)"mM"); SendMessage(unitCombo, CB_SETCURSEL, 1, 0); y += 22;
        }
        y += 10;
        CreateWindow("BUTTON", "Calculate", WS_VISIBLE | WS_CHILD, 20, y, 100, 30, hwnd, (HMENU)(INT_PTR)IDC_CALCULATE_BTN, NULL, NULL);
        CreateWindow("BUTTON", "Reset All", WS_VISIBLE | WS_CHILD, 130, y, 100, 30, hwnd, (HMENU)(INT_PTR)IDC_RESET_BTN, NULL, NULL); y += 45;
        CreateWindow("STATIC", "Results:", WS_VISIBLE | WS_CHILD, 10, y, 100, 20, hwnd, NULL, NULL, NULL); y += 25;
        CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 10, y, 450, 150, hwnd, (HMENU)(INT_PTR)IDC_OUTPUT_TEXT, NULL, NULL);
        InitializeAppData();
        std::vector<std::string> ligands = g_solver.GetAvailableLigands();
        for (int i = 0; i < g_numLigands; i++) { PopulateCombo(hwnd, IDC_LIGAND_COMBO_BASE + i, ligands); }
        SetOutputText(hwnd, "Ready. Configure ligands and solution parameters, then click Calculate.");
        return 0;
    }
    case WM_SIZE: {
        SCROLLINFO si; si.cbSize = sizeof(si); si.fMask = SIF_RANGE | SIF_PAGE; si.nMin = 0; si.nMax = 1050; si.nPage = HIWORD(lParam); SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        return 0;
    }
    case WM_VSCROLL: {
        SCROLLINFO si; si.cbSize = sizeof(si); si.fMask = SIF_ALL; GetScrollInfo(hwnd, SB_VERT, &si);
        int oldPos = si.nPos;
        switch (LOWORD(wParam)) {
            case SB_TOP: si.nPos = si.nMin; break; case SB_BOTTOM: si.nPos = si.nMax; break;
            case SB_LINEUP: si.nPos -= 20; break; case SB_LINEDOWN: si.nPos += 20; break;
            case SB_PAGEUP: si.nPos -= si.nPage; break; case SB_PAGEDOWN: si.nPos += si.nPage; break;
            case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
        }
        si.fMask = SIF_POS; SetScrollInfo(hwnd, SB_VERT, &si, TRUE); GetScrollInfo(hwnd, SB_VERT, &si);
        if (si.nPos != oldPos) { ScrollWindowEx(hwnd, 0, oldPos - si.nPos, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE); UpdateWindow(hwnd); }
        return 0;
    }
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        SendMessage(hwnd, WM_VSCROLL, delta > 0 ? SB_LINEUP : SB_LINEDOWN, 0);
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_NEW:
            for (int i = 0; i < g_numCations; i++) { SetDlgItemText(hwnd, IDC_CATION_FREE_BASE + i, ""); SetDlgItemText(hwnd, IDC_CATION_TOTAL_BASE + i, ""); SendDlgItemMessage(hwnd, IDC_CATION_UNIT_BASE + i, CB_SETCURSEL, 1, 0); g_previousUnits[i] = 1; }
            for (int i = 0; i < g_numLigands; i++) { SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO_BASE + i, CB_SETCURSEL, 0, 0); SetDlgItemText(hwnd, IDC_LIGAND_CONC_BASE + i, ""); }
            SetOutputText(hwnd, "Ready. Configure ligands and solution parameters, then click Calculate.");
            SetDlgItemText(hwnd, IDC_CALCULATE_BTN, "Calculate"); g_calculationDone = false; return 0;
        case IDM_FILE_EXIT: PostQuitMessage(0); return 0;
        case IDM_LIGAND_EDITOR: ShowLigandEditorDialog(hwnd); return 0;
        case IDM_HELP_CONSTANTS: ShellExecute(NULL, "open", "https://n-hatada.github.io/stability-constant-explorer/english.html", NULL, NULL, SW_SHOWNORMAL); return 0;
        case IDM_MODE_WEBMAXC: {
            SystemParameters params = g_solver.GetParameters(); params.mode = SystemParameters::SolverMode::WebMaxC; g_solver.SetParameters(params);
            HMENU hMenu = GetMenu(hwnd); CheckMenuItem(hMenu, IDM_MODE_WEBMAXC, MF_BYCOMMAND | MF_CHECKED); CheckMenuItem(hMenu, IDM_MODE_STANDARD, MF_BYCOMMAND | MF_UNCHECKED);
            SendMessage(hwnd, WM_COMMAND, IDC_CALCULATE_BTN, 0); return 0;
        }
        case IDM_MODE_STANDARD: {
            SystemParameters params = g_solver.GetParameters(); params.mode = SystemParameters::SolverMode::IndustryStandard; g_solver.SetParameters(params);
            HMENU hMenu = GetMenu(hwnd); CheckMenuItem(hMenu, IDM_MODE_WEBMAXC, MF_BYCOMMAND | MF_UNCHECKED); CheckMenuItem(hMenu, IDM_MODE_STANDARD, MF_BYCOMMAND | MF_CHECKED);
            SendMessage(hwnd, WM_COMMAND, IDC_CALCULATE_BTN, 0); return 0;
        }
        case IDM_HELP_ABOUT: MessageBox(hwnd, "Cation-Ligand Equilibrium Engine v1.1.1\n\nCalculates free and total ion concentrations\nin complexing solutions.", "About", MB_ICONINFORMATION); return 0;
        case IDC_RESET_BTN: SendMessage(hwnd, WM_COMMAND, IDM_FILE_NEW, 0); return 0;
        case IDC_CALCULATE_BTN: {
            double temp = GetDoubleFromCtrl(hwnd, IDC_TEMP_INPUT), ionic = GetDoubleFromCtrl(hwnd, IDC_IONIC_INPUT), pH = GetDoubleFromCtrl(hwnd, IDC_PH_INPUT);
            SystemParameters params = g_solver.GetParameters(); params.temperature = temp; params.ionicStrength = ionic; params.pH = pH; g_solver.SetParameters(params);
            std::string output = "=== Equilibrium Calculation Results ===\r\n";
            output += (params.mode == SystemParameters::SolverMode::IndustryStandard) ? "MODE: Industry Standard (Davies/Speciation)\r\n" : "MODE: WebMaxC Legacy (NIST v8)\r\n";
            output += "\r\nSolution Parameters:\r\n------------------------\r\n";
            char buf[256]; sprintf(buf, "  Temperature: %.1f C\r\n  Ionic Strength: %.1f mM\r\n  pH: %.2f\r\n\r\n", temp, ionic, pH); output += buf;
            output += "Selected Ligands:\r\n------------------------\r\n";
            std::vector<std::string> selectedLigands; std::vector<double> ligandTotals;
            for (int i = 0; i < g_numLigands; i++) {
                int ligandIdx = SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO_BASE + i, CB_GETCURSEL, 0, 0);
                if (ligandIdx > 0) {
                    char ligandName[128] = {0}; SendDlgItemMessage(hwnd, IDC_LIGAND_COMBO_BASE + i, CB_GETLBTEXT, ligandIdx, (LPARAM)ligandName);
                    double concMM = GetDoubleFromCtrl(hwnd, IDC_LIGAND_CONC_BASE + i); if (concMM <= 0) continue;
                    double concM = concMM / 1000.0; selectedLigands.push_back(ligandName); ligandTotals.push_back(concM);
                    sprintf(buf, "  %s: %.6f M\r\n", ligandName, concM); output += buf;
                }
            }
            output += "\r\nCation Calculations:\r\n------------------------\r\n\r\n";
            std::vector<std::string> selectedMetals; std::vector<double> metalInputs; std::vector<bool> isInputFree; std::vector<int> metalIndices;
            for (int i = 0; i < g_numCations; i++) {
                double freeVal = GetDoubleFromCtrl(hwnd, IDC_CATION_FREE_BASE + i), totalVal = GetDoubleFromCtrl(hwnd, IDC_CATION_TOTAL_BASE + i);
                if (totalVal > 0) { selectedMetals.push_back(g_cationNames[i]); metalInputs.push_back(totalVal / 1000.0); isInputFree.push_back(false); metalIndices.push_back(i); }
                else if (freeVal > 0) {
                    selectedMetals.push_back(g_cationNames[i]);
                    int unit = SendDlgItemMessage(hwnd, IDC_CATION_UNIT_BASE + i, CB_GETCURSEL, 0, 0);
                    double freeMolar = 0; switch (unit) { case 0: freeMolar = freeVal * 1e-9; break; case 1: freeMolar = freeVal * 1e-6; break; case 2: freeMolar = freeVal * 1e-3; break; }
                    metalInputs.push_back(freeMolar); isInputFree.push_back(true); metalIndices.push_back(i);
                }
            }
            if (selectedMetals.empty()) { SetOutputText(hwnd, "Error: No metal concentrations provided."); return 0; }
            auto resMulti = g_solver.CalculateSimultaneousEquilibrium(selectedLigands, ligandTotals, selectedMetals, metalInputs, isInputFree);
            if (resMulti.converged) {
                for (size_t j = 0; j < resMulti.metalNames.size(); ++j) {
                    int originalIdx = metalIndices[j]; double freeMolar = resMulti.freeMetals[j], totalMolar = resMulti.totalMetals[j];
                    if (isInputFree[j]) { char totalStr[64]; sprintf(totalStr, "%.4f", totalMolar * 1000.0); SetDlgItemText(hwnd, IDC_CATION_TOTAL_BASE + originalIdx, totalStr); }
                    else { int bestUnit = DetermineBestUnit(freeMolar); double freeValue = ConvertConcentration(freeMolar, bestUnit); char freeStr[64]; sprintf(freeStr, "%.6f", freeValue); SetDlgItemText(hwnd, IDC_CATION_FREE_BASE + originalIdx, freeStr); SendDlgItemMessage(hwnd, IDC_CATION_UNIT_BASE + originalIdx, CB_SETCURSEL, bestUnit, 0); g_previousUnits[originalIdx] = bestUnit; }
                    sprintf(buf, "%s:\r\n  Free:    %.3e M\r\n  Total:   %.3e M (%.4f mM)\r\n", resMulti.metalNames[j].c_str(), freeMolar, totalMolar, totalMolar * 1000.0); output += buf;
                    double totalComplex = 0; for (size_t i = 0; i < selectedLigands.size(); ++i) { totalComplex += resMulti.complexes[i][j]; }
                    sprintf(buf, "  Complex: %.3e M (Total bound)\r\n\r\n", totalComplex); output += buf;
                }
            } else { output += "Error: Solver failed to converge.\r\n"; }
            SetOutputText(hwnd, output); g_calculationDone = true; return 0;
        }
        default: {
            int ctrlId = LOWORD(wParam); int notifyCode = HIWORD(wParam);
            if (notifyCode == EN_SETFOCUS) { if (ctrlId >= IDC_CATION_FREE_BASE && ctrlId < IDC_CATION_FREE_BASE + g_numCations) SetDlgItemText(hwnd, IDC_CATION_TOTAL_BASE + (ctrlId - IDC_CATION_FREE_BASE), ""); else if (ctrlId >= IDC_CATION_TOTAL_BASE && ctrlId < IDC_CATION_TOTAL_BASE + g_numCations) SetDlgItemText(hwnd, IDC_CATION_FREE_BASE + (ctrlId - IDC_CATION_TOTAL_BASE), ""); }
            if (notifyCode == CBN_SELCHANGE && ctrlId >= IDC_CATION_UNIT_BASE && ctrlId < IDC_CATION_UNIT_BASE + g_numCations) {
                int cationIndex = ctrlId - IDC_CATION_UNIT_BASE, newUnit = SendDlgItemMessage(hwnd, IDC_CATION_UNIT_BASE + cationIndex, CB_GETCURSEL, 0, 0), oldUnit = g_previousUnits[cationIndex];
                double freeValue = GetDoubleFromCtrl(hwnd, IDC_CATION_FREE_BASE + cationIndex);
                if (freeValue > 0 && newUnit != oldUnit) {
                    double freeMolar = 0.0; switch (oldUnit) { case 0: freeMolar = freeValue * 1e-9; break; case 1: freeMolar = freeValue * 1e-6; break; case 2: freeMolar = freeValue * 1e-3; break; }
                    double newValue = ConvertConcentration(freeMolar, newUnit); char buf[64]; sprintf(buf, "%.6f", newValue); SetDlgItemText(hwnd, IDC_CATION_FREE_BASE + cationIndex, buf);
                }
                g_previousUnits[cationIndex] = newUnit; return 0;
            }
            break;
        }
        }
        break;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    INITCOMMONCONTROLSEX icex; icex.dwSize = sizeof(icex); icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES; InitCommonControlsEx(&icex);
    const char CLASS_NAME[] = "CationEngineMainWindow";
    WNDCLASSEX wc; ZeroMemory(&wc, sizeof(WNDCLASSEX)); wc.cbSize = sizeof(WNDCLASSEX); wc.style = CS_HREDRAW | CS_VREDRAW; wc.lpfnWndProc = MainWndProc; wc.hInstance = hInstance; wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); wc.lpszClassName = CLASS_NAME;
    if (!RegisterClassEx(&wc)) return 1;
    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Cation-Ligand Equilibrium Engine v1.1.1", WS_OVERLAPPEDWINDOW | WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, 500, 800, NULL, NULL, hInstance, NULL);
    if (!hwnd) return 1;
    ShowWindow(hwnd, nCmdShow); UpdateWindow(hwnd);
    MSG msg; while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    return (int)msg.wParam;
}
#else
int main() {
    std::cout << "Cation-Ligand Equilibrium Solver (Console Mode) v1.1.1" << std::endl;
    InitializeLigandData(); CationSystem solver; for (auto& ligand : GetAllLigands()) solver.AddLigand(ligand); for (auto& metal : GetAllMetals()) solver.AddMetal(metal);
    SystemParameters params; params.temperature = 25.0; params.ionicStrength = 100.0; params.pH = 7.0; solver.SetParameters(params);
    auto result = solver.CalculateTotalToFree(1.0e-3, 1.0e-4, "EDTA", "Ca2"); std::cout << "EDTA-Ca2+ test: Free = " << result.freeMetal << " M\n"; return 0;
}
#endif
