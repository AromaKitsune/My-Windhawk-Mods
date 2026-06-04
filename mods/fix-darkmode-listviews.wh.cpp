// ==WindhawkMod==
// @id              fix-darkmode-listviews
// @name            Fix Darkmode ListViews
// @description     Fixes ListViews in dark mode
// @version         1.0-beta17
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
* **Ignore Aero visual style check:** Disables the `Aero.msstyles` check,
  making ListViews always use system-colored text.
  * Without this check, apps with built-in dark modes would render unreadable
    dark text on dark ListViews because the mod applies the system text color
    while the default Aero visual style is active.
  * Enable this option if you use the
    "[Translucent Windows](https://windhawk.net/mods/translucent-windows)"
    mod while the default Aero visual style is active. That mod already handles
    light text color properly within apps featuring built-in dark modes.

---

[Original code](https://windhawk.net/mods/fix-darkmode-listviews) by
**Reabstraction**. This forked mod adds compatibility with dialogs.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- ignoreAeroVisualStyleCheck: false
  $name: Ignore Aero visual style check
  $description: >-
    Disables the Aero.msstyles check, making ListViews always use system-colored
    text. Enable this option if you use the "Translucent Windows" mod while the
    default Aero visual style is active.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>

struct
{
    bool ignoreAeroVisualStyleCheck;
} settings;

bool IsDefaultAeroVisualStyleActive()
{
    WCHAR szVisualStyleFileName[MAX_PATH];
    if (SUCCEEDED(GetCurrentThemeName(szVisualStyleFileName, MAX_PATH, nullptr,
            0, nullptr, 0)))
    {
        WCHAR szLower[MAX_PATH];
        wcscpy_s(szLower, ARRAYSIZE(szLower), szVisualStyleFileName);
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

void SetListViewTextColor(HWND hListView)
{
    // Do not convert text colors if the default Aero visual style is active.
    // This prevents breaking apps that implement their own custom dark modes.
    if (!settings.ignoreAeroVisualStyleCheck &&
        IsDefaultAeroVisualStyleActive())
    {
        return;
    }

    // Dynamically match text color to system color
    ListView_SetTextColor(hListView, GetSysColor(COLOR_WINDOWTEXT));
}

BOOL ShouldApply(HWND hWndParent, LPCWSTR lpClassName)
{
    return (hWndParent != nullptr &&
        lpClassName != nullptr &&
        ((reinterpret_cast<ULONG_PTR>(lpClassName) &
            ~static_cast<ULONG_PTR>(0xffff)) != 0) &&
        wcscmp(lpClassName, L"SysListView32") == 0);
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
    WCHAR lpClassName[256];
    if (GetClassNameW(hWnd, lpClassName, ARRAYSIZE(lpClassName)) &&
        wcscmp(lpClassName, L"SysListView32") == 0)
    {
        SetListViewTextColor(hWnd);
    }
    return TRUE;
}

// Hook for CreateWindowExW
using CreateWindowExW_t = decltype(&CreateWindowExW);
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
    LPVOID lpParam
)
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

    if (ShouldApply(hWndParent, lpClassName))
    {
        SetListViewTextColor(hWnd);
    }

    return hWnd;
}

// Hook for DefDlgProcW
using DefDlgProcW_t = decltype(&DefDlgProcW);
DefDlgProcW_t DefDlgProcW_Original;
LRESULT WINAPI DefDlgProcW_Hook(HWND hDlg, UINT Msg, WPARAM wParam,
    LPARAM lParam)
{
    LRESULT lResult = DefDlgProcW_Original(hDlg, Msg, wParam, lParam);

    if (Msg == WM_INITDIALOG)
    {
        EnumChildWindows(hDlg, EnumChildProc, 0);
    }

    return lResult;
}

void LoadSettings()
{
    settings.ignoreAeroVisualStyleCheck =
        Wh_GetIntSetting(L"ignoreAeroVisualStyleCheck");
}

BOOL Wh_ModInit()
{
    Wh_Log(L"Init");

    LoadSettings();

    WindhawkUtils::SetFunctionHook(
        CreateWindowExW,
        CreateWindowExW_Hook,
        &CreateWindowExW_Original
    );

    WindhawkUtils::SetFunctionHook(
        DefDlgProcW,
        DefDlgProcW_Hook,
        &DefDlgProcW_Original
    );

    return TRUE;
}

void Wh_ModSettingsChanged()
{
    LoadSettings();
}
