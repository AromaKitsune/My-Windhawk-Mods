// ==WindhawkMod==
// @id              fix-darkmode-listviews
// @name            Fix Darkmode ListViews
// @description     Fixes ListViews in dark mode
// @version         1.0-beta8
// @author          Kitsune
// @github          https://github.com/AromaKitsune
// @include         *
// @compilerOptions -luxtheme
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Fix Darkmode ListViews

Fixes hardcoded text color in ListViews when using a system-wide dark theme such
as "Rectify11 dark theme".

| Before | After |
| :----: | :---: |
| ![](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/fix-darkmode-listviews_before.png) | ![](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/fix-darkmode-listviews_after.png) |

[Original code](https://windhawk.net/mods/fix-darkmode-listviews) by
**Reabstraction**.
*/
// ==/WindhawkModReadme==

#include <commctrl.h>
#include <uxtheme.h>

using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_orig;
using DefDlgProcW_t = decltype(&DefDlgProcW);
DefDlgProcW_t DefDlgProcW_orig;

bool IsDefaultAeroThemeActive()
{
    WCHAR szThemeFileName[MAX_PATH];
    if (SUCCEEDED(GetCurrentThemeName(szThemeFileName, MAX_PATH, NULL, 0, NULL, 0)))
    {
        WCHAR szLower[MAX_PATH];
        wcscpy_s(szLower, MAX_PATH, szThemeFileName);
        _wcslwr_s(szLower, MAX_PATH);

        // Check specifically for \aero\ folder to avoid catching custom \dark\aero.msstyles
        if (wcsstr(szLower, L"\\aero\\aero.msstyles") != NULL)
        {
            return true;
        }
    }
    return false;
}

void ApplyTheme(HWND hListView)
{
    // Do not force text colors if the default Aero theme is active.
    // This prevents breaking apps that implement their own custom dark modes.
    if (IsDefaultAeroThemeActive())
    {
        return;
    }

    // Dynamically match text color to system theme
    ListView_SetTextColor(hListView, GetSysColor(COLOR_WINDOWTEXT));
}

BOOL ShouldApply(
    HWND hWndParent,
    LPCWSTR lpClassName
)
{
    return (hWndParent != NULL
    &&      lpClassName != NULL
    &&      (((ULONG_PTR)lpClassName & ~(ULONG_PTR)0xffff) != 0)
    &&      0 == wcscmp(lpClassName, L"SysListView32"));
}

HWND WINAPI CreateWindowExW_hook(
    DWORD     dwExStyle,
    LPCWSTR   lpClassName,
    LPCWSTR   lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
)
{
    HWND hRes = CreateWindowExW_orig(
        dwExStyle, lpClassName, lpWindowName, dwStyle,
        X, Y, nWidth, nHeight, hWndParent, hMenu,
        hInstance, lpParam
    );

    if (ShouldApply(hWndParent, lpClassName))
    {
        ApplyTheme(hRes);
    }

    return hRes;
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
    WCHAR lpClassName[256];
    if (GetClassNameW(hWnd, lpClassName, ARRAYSIZE(lpClassName)))
    {
        if (wcscmp(lpClassName, L"SysListView32") == 0)
        {
            ApplyTheme(hWnd);
        }
    }
    return TRUE;
}

LRESULT WINAPI DefDlgProcW_hook(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT res = DefDlgProcW_orig(hDlg, Msg, wParam, lParam);

    if (Msg == WM_INITDIALOG)
    {
        EnumChildWindows(hDlg, EnumChildProc, 0);
    }

    return res;
}

BOOL Wh_ModInit(void)
{
    Wh_Log(L"Mod loaded");

    if (!Wh_SetFunctionHook(
        (void *)CreateWindowExW,
        (void *)CreateWindowExW_hook,
        (void **)&CreateWindowExW_orig
    ))
    {
        return FALSE;
    }

    if (!Wh_SetFunctionHook(
        (void *)DefDlgProcW,
        (void *)DefDlgProcW_hook,
        (void **)&DefDlgProcW_orig
    ))
    {
        Wh_Log(L"Failed to hook DefDlgProcW");
    }

    return TRUE;
}
