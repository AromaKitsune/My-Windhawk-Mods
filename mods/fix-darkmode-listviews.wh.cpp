// ==WindhawkMod==
// @id              fix-darkmode-listviews
// @name            Fix Darkmode ListViews
// @description     Fixes ListViews in dark mode
// @version         1.0-beta9
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

## Configuration
**Ignore Aero Theme Check:** Disables the `Aero.msstyles` check, forcing system
colored text. Enable this option if you use the
"[Translucent Windows](https://windhawk.net/mods/translucent-windows)"
mod that forces a dark theme despite the default Aero theme being active.

[Original code](https://windhawk.net/mods/fix-darkmode-listviews) by
**Reabstraction**. This forked mod adds compatibility with dialogs.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- ignoreAeroThemeCheck: false
  $name: Ignore Aero Theme Check
  $description: >-
    Disables the Aero.msstyles check, forcing system colored text. Enable this
    option if you use the "Translucent Windows" mod that forces a dark theme
    despite the default Aero theme being active.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>

struct {
    bool ignoreAeroThemeCheck;
} settings;

bool IsDefaultAeroThemeActive()
{
    WCHAR szThemeFileName[MAX_PATH];
    if (SUCCEEDED(GetCurrentThemeName(szThemeFileName, MAX_PATH, nullptr, 0,
            nullptr, 0)))
    {
        WCHAR szLower[MAX_PATH];
        wcscpy_s(szLower, ARRAYSIZE(szLower), szThemeFileName);
        _wcslwr_s(szLower, ARRAYSIZE(szLower));

        // Check specifically for \aero\ folder to avoid catching custom
        // \dark\aero.msstyles
        if (wcsstr(szLower, L"\\aero\\aero.msstyles") != nullptr)
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
    if (!settings.ignoreAeroThemeCheck && IsDefaultAeroThemeActive())
    {
        return;
    }

    // Dynamically match text color to system theme
    ListView_SetTextColor(hListView, GetSysColor(COLOR_WINDOWTEXT));
}

BOOL ShouldApply(HWND hWndParent, LPCWSTR lpClassName)
{
    return (hWndParent != nullptr &&
        lpClassName != nullptr &&
        (((ULONG_PTR)lpClassName & ~(ULONG_PTR)0xffff) != 0) &&
        wcscmp(lpClassName, L"SysListView32") == 0);
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

using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_orig;
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

using DefDlgProcW_t = decltype(&DefDlgProcW);
DefDlgProcW_t DefDlgProcW_orig;
LRESULT WINAPI DefDlgProcW_hook(HWND hDlg, UINT Msg, WPARAM wParam,
    LPARAM lParam)
{
    LRESULT res = DefDlgProcW_orig(hDlg, Msg, wParam, lParam);

    if (Msg == WM_INITDIALOG)
    {
        EnumChildWindows(hDlg, EnumChildProc, 0);
    }

    return res;
}

void LoadSettings()
{
    settings.ignoreAeroThemeCheck = Wh_GetIntSetting(L"ignoreAeroThemeCheck");
}

BOOL Wh_ModInit(void)
{
    Wh_Log(L"Mod loaded");
    LoadSettings();

    if (!WindhawkUtils::SetFunctionHook(
        CreateWindowExW,
        CreateWindowExW_hook,
        &CreateWindowExW_orig
    ))
    {
        return FALSE;
    }

    if (!WindhawkUtils::SetFunctionHook(
        DefDlgProcW,
        DefDlgProcW_hook,
        &DefDlgProcW_orig
    ))
    {
        Wh_Log(L"Failed to hook DefDlgProcW");
    }

    return TRUE;
}

void Wh_ModSettingsChanged()
{
    LoadSettings();
}
