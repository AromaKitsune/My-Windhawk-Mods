// ==WindhawkMod==
// @id              notepad-plus-plus-ui-tweaks
// @name            Notepad++ UI Tweaks
// @description     Applies UI tweaks to Notepad++ to improve usability
// @version         1.0
// @author          Kitsune
// @include         notepad++.exe
// @compilerOptions -lcomctl32 -luser32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Notepad++ UI Tweaks
This mod applies UI tweaks to Notepad++ to improve usability.

## Remove Border from Scintilla
Removes the border from the main text editing area (Scintilla control).

**Why?**: When the border is visible, a 1px gap exists between the vertical scroll bar and the edge of the screen in a maximized window.
This prevents you from simply flicking your mouse cursor to the screen edge to grab the scroll bar.
Removing the border eliminates this gap, making the scroll bar easier to grab.

## Remove InfoTip from Document List
Removes the tooltip (InfoTip) that appears when hovering over files in the "Document List" panel.

**Why?**: The default InfoTip can be intrusive; if you hover over file item 1, the tooltip often appears over file item 2, obscuring it.

Additionally, on Windows 11, this prevents the mouse hover effect from triggering on the obscured item, making the list feel unresponsive to cursor movement.
Curiously, this issue also affects several items below it (e.g., items 3, 4, 5), even though the tooltip is only obscuring item 2.
This is a system-wide bug (or "feature"?) that affects `SysListView32` controls (not just in Notepad++) and does not occur in Windows 10 or earlier.
Removing the InfoTip solves this obstruction.
*/
// ==/WindhawkModReadme==

#include <windhawk_utils.h>
#include <windows.h>
#include <commctrl.h>

// Helper: Force remove the border & client edge from Scintilla
void ForceRemoveScintillaBorder(HWND hWnd) {
    BOOL bChanged = FALSE;

    // Border for dark theme
    LONG_PTR style = GetWindowLongPtrW(hWnd, GWL_STYLE);
    if (style & WS_BORDER) {
        SetWindowLongPtrW(hWnd, GWL_STYLE, style & ~WS_BORDER);
        bChanged = TRUE;
    }

    // Extended client edge (3D border) for light theme
    LONG_PTR exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_CLIENTEDGE) {
        SetWindowLongPtrW(hWnd, GWL_EXSTYLE, exStyle & ~WS_EX_CLIENTEDGE);
        bChanged = TRUE;
    }

    // Only force a frame redraw if we actually changed something
    if (bChanged) {
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
    }
}

// Helper: Force remove the InfoTip from ListView
void ForceRemoveListViewInfoTip(HWND hWnd) {
    // LVM_SETEXTENDEDLISTVIEWSTYLE = 0x1036
    // Mask = LVS_EX_INFOTIP (0x400), Style = 0
    SendMessageW(hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_INFOTIP, 0);
}

// Subclass procedure to intercept messages sent to Scintilla
LRESULT CALLBACK ScintillaSubclassProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    DWORD_PTR dwRefData
) {
    switch (uMsg) {
        // Prevent WS_BORDER or WS_EX_CLIENTEDGE from ever being added back
        case WM_STYLECHANGING: {
            STYLESTRUCT* ss = (STYLESTRUCT*)lParam;

            // Check Standard Styles
            if (wParam == (WPARAM)GWL_STYLE) {
                if (ss->styleNew & WS_BORDER) {
                    ss->styleNew &= ~WS_BORDER;
                }
            }

            // Check Extended Styles
            else if (wParam == (WPARAM)GWL_EXSTYLE) {
                if (ss->styleNew & WS_EX_CLIENTEDGE) {
                    ss->styleNew &= ~WS_EX_CLIENTEDGE;
                }
            }
            break;
        }

        case WM_NCDESTROY:
            // Remove the subclass when the Notepad++ window is destroyed
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd, ScintillaSubclassProc);
            break;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Subclass procedure to intercept messages sent to ListView
LRESULT CALLBACK ListViewSubclassProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    DWORD_PTR dwRefData
) {
    switch (uMsg) {
        case LVM_SETEXTENDEDLISTVIEWSTYLE: {
            // wParam is the mask (if 0, it sets all). lParam is the style.
            // We want to ensure LVS_EX_INFOTIP is NOT set in the resulting style.
            // By masking it out of lParam, we ensure it isn't enabled.
            lParam &= ~LVS_EX_INFOTIP;
            break;
        }

        case WM_NCDESTROY:
            // Remove the subclass when the Notepad++ window is destroyed
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd, ListViewSubclassProc);
            break;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Hook logic to attach the subclass to Notepad++ windows
void AttachSubclass(HWND hWnd) {
    WCHAR className[256];
    if (GetClassNameW(hWnd, className, ARRAYSIZE(className))) {

        // Handle Scintilla controls
        if (wcsstr(className, L"Scintilla")) {
            ForceRemoveScintillaBorder(hWnd);
            WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd, ScintillaSubclassProc, 0);
        }

        // Handle SysListView32 controls
        else if (wcsstr(className, L"SysListView32")) {
            ForceRemoveListViewInfoTip(hWnd);
            WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd, ListViewSubclassProc, 0);
        }
    }
}

// Hook logic to detach the subclass
void DetachSubclass(HWND hWnd) {
    // Attempt to remove both subclasses.
    // It is safe to call remove if the subclass isn't attached.
    WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd, ScintillaSubclassProc);
    WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd, ListViewSubclassProc);
}

// Hook CreateWindowExW to catch new Notepad++ windows
using CreateWindowExW_t = HWND(WINAPI*)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
CreateWindowExW_t CreateWindowExW_Original;

HWND WINAPI CreateWindowExW_Hook(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam)
{
    HWND hWnd = CreateWindowExW_Original(
        dwExStyle,
        lpClassName,
        lpWindowName,
        dwStyle,
        X,
        Y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,
        hInstance,
        lpParam
    );

    if (hWnd) {
        AttachSubclass(hWnd);
    }

    return hWnd;
}

// Enumeration callback for existing Notepad++ windows
BOOL CALLBACK EnumChildWindowsCallback(HWND hWnd, LPARAM lParam) {
    AttachSubclass(hWnd);
    return TRUE;
}

BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam) {
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hWnd, &lpdwProcessId);
    if (lpdwProcessId == GetCurrentProcessId()) {
        EnumChildWindows(hWnd, EnumChildWindowsCallback, 0);
    }
    return TRUE;
}

// Enumeration callback for removing subclass
BOOL CALLBACK EnumChildWindowsRemoveCallback(HWND hWnd, LPARAM lParam) {
    DetachSubclass(hWnd);
    return TRUE;
}

BOOL CALLBACK EnumWindowsRemoveCallback(HWND hWnd, LPARAM lParam) {
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hWnd, &lpdwProcessId);
    if (lpdwProcessId == GetCurrentProcessId()) {
        EnumChildWindows(hWnd, EnumChildWindowsRemoveCallback, 0);
    }
    return TRUE;
}

// Mod initialization
BOOL Wh_ModInit() {
    Wh_Log(L"Init");

    // Hook creation of new Notepad++ windows
    Wh_SetFunctionHook((void*)CreateWindowExW, (void*)CreateWindowExW_Hook, (void**)&CreateWindowExW_Original);

    // Attach to any currently open Notepad++ windows
    EnumWindows(EnumWindowsCallback, 0);

    return TRUE;
}

// Mod uninitialization
void Wh_ModUninit() {
    Wh_Log(L"Uninit");

    // Clean-up: Remove subclass from all Notepad++ windows to prevent dangling pointers (aka crash)
    EnumWindows(EnumWindowsRemoveCallback, 0);
}

