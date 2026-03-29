# Complete Fix for Ligand Editor Window Creation

The issue with the ligand editor window creation stems from mixing two incompatible approaches:
1. Manual window class registration with CreateWindowExA
2. Manual control creation on top of a dialog

## Root Cause Analysis

Looking at the ShowLigandEditorDialog function, there are two major issues:
1. It registers a window class but then tries to create a dialog using CreateDialogParamA
2. It manually creates controls using CreateWindow instead of using dialog template resources

## Solution

The ligand editor should be completely restructured as a proper dialog. Here's the correct approach:

1. **Option 1: Use Dialog Resource (Recommended)**
   - Create a dialog resource in the resource file (.rc)
   - Use DialogBox or CreateDialogParam with the resource ID
   - Handle WM_INITDIALOG to populate controls

2. **Option 2: Fix the Current Manual Approach**
   - Remove the window class registration (since it's not needed for this approach)
   - Create the dialog properly with proper flags
   - Move all control creation to WM_INITDIALOG

## Recommended Fix for Current Code

Since the code is already trying to create a dialog but with manual controls, the best fix is to:

1. Remove the window class registration from ShowLigandEditorDialog
2. Use proper dialog creation with dialog template approach
3. Move control creation to WM_INITDIALOG handler

## Specific Code Changes Needed

In the ShowLigandEditorDialog function:
1. Remove the entire window class registration section (lines 439-455)
2. Change the CreateDialogParamA call to properly handle the dialog
3. Move all control creation to the WM_INITDIALOG handler in LigandEditorProc

## Alternative Quick Fix

If you want a quick fix that maintains the current structure:
1. Remove the window class registration entirely
2. Keep the CreateDialogParamA call as is
3. Move all the control creation code from ShowLigandEditorDialog to WM_INITDIALOG in LigandEditorProc

This approach would require moving lines 472-520 (the control creation) to be inside the WM_INITDIALOG case in LigandEditorProc.

The error "Failed to create Ligand Editor window (error 0)" occurs because the dialog creation is not matching the expected dialog template or the window class registration is conflicting with the dialog creation.