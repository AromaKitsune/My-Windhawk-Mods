// ==WindhawkMod==
// @id              fix-darkmode-listviews
// @name            Fix Darkmode ListViews
// @description     Fixes ListViews in dark mode
// @version         1.0-beta30
// @author          Kitsune
// @github          https://github.com/AromaKitsune
// @include         *
// @compilerOptions -lcomctl32 -lgdi32 -luxtheme
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
**Translucent Windows compatibility:** Fixes unreadable dark text in the
Explorer address bar drop-down menu when using the
"[Translucent Windows](https://windhawk.net/mods/translucent-windows)" mod.

---

[Original code](https://windhawk.net/mods/fix-darkmode-listviews) by
**Reabstraction**. This forked mod adds compatibility with dialogs and
"Translucent Windows" mod.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- translucentWindowsCompatibility: false
  $name: Translucent Windows compatibility
  $description: >-
    Fixes unreadable dark text in the Explorer address bar drop-down menu when
    using the "Translucent Windows" mod
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>

struct
{
    bool translucentWindowsCompatibility;
} settings;

// Define OCM_NOTIFY for message reflection
constexpr UINT OCM_NOTIFY = 0x204E;

// Thread-local flag to track when a DropDown's ListView is actively painting.
thread_local bool g_bInsideDropDownPaint = false;

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
    if (!settings.translucentWindowsCompatibility &&
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
            ~static_cast<ULONG_PTR>(0xFFFF)) != 0) &&
        wcscmp(lpClassName, L"SysListView32") == 0);
}

// Subclass procedure specifically for SysListView32 inside DropDown windows
LRESULT CALLBACK DropDownListViewSubclassProc(HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam, DWORD_PTR dwRefData)
{
    // Track paint and reflected custom draw messages
    if (uMsg == WM_PAINT || uMsg == OCM_NOTIFY)
    {
        bool bPreviousPaintState = g_bInsideDropDownPaint;
        g_bInsideDropDownPaint = true;

        LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);

        g_bInsideDropDownPaint = bPreviousPaintState;
        return lResult;
    }
    else if (uMsg == WM_NCDESTROY)
    {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd,
            DropDownListViewSubclassProc);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EnumDropDownChildProc(HWND hWnd, LPARAM lParam)
{
    WCHAR lpClassName[256];
    if (GetClassNameW(hWnd, lpClassName, ARRAYSIZE(lpClassName)) &&
        wcscmp(lpClassName, L"SysListView32") == 0)
    {
        WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd,
            DropDownListViewSubclassProc, 0);
    }
    return TRUE;
}

// Subclass procedure for Explorer DropDown windows
LRESULT CALLBACK DropDownSubclassProc(HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam, DWORD_PTR dwRefData)
{
    if (uMsg == WM_SHOWWINDOW && wParam == TRUE)
    {
        EnumChildWindows(hWnd, EnumDropDownChildProc, 0);
    }
    else if (uMsg == WM_NCDESTROY)
    {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd,
            DropDownSubclassProc);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
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

// Enumeration callback: Attach the subclass to existing DropDown windows
BOOL CALLBACK EnumWindows_AttachDropDowns(HWND hWnd, LPARAM lParam)
{
    WCHAR szClassName[256];
    if (GetClassNameW(hWnd, szClassName, ARRAYSIZE(szClassName)) &&
        wcscmp(szClassName, L"DropDown") == 0)
    {
        WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd,
            DropDownSubclassProc, 0);
    }
    return TRUE;
}

// Enumeration callback: Detach the subclass from existing DropDown ListViews
BOOL CALLBACK EnumDropDownChildProc_Detach(HWND hWnd, LPARAM lParam)
{
    WCHAR lpClassName[256];
    if (GetClassNameW(hWnd, lpClassName, ARRAYSIZE(lpClassName)) &&
        wcscmp(lpClassName, L"SysListView32") == 0)
    {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd,
            DropDownListViewSubclassProc);
    }
    return TRUE;
}

// Enumeration callback: Detach the subclass from existing DropDown windows
BOOL CALLBACK EnumWindows_DetachDropDowns(HWND hWnd, LPARAM lParam)
{
    WCHAR szClassName[256];
    if (GetClassNameW(hWnd, szClassName, ARRAYSIZE(szClassName)) &&
        wcscmp(szClassName, L"DropDown") == 0)
    {
        // Detach inner ListView subclasses first
        EnumChildWindows(hWnd, EnumDropDownChildProc_Detach, 0);

        // Detach the parent DropDown subclass
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd,
            DropDownSubclassProc);
    }
    return TRUE;
}

// Hook for GDI SetTextColor
using SetTextColor_t = decltype(&SetTextColor);
SetTextColor_t SetTextColor_Original;
COLORREF WINAPI SetTextColor_Hook(HDC hdc, COLORREF color)
{
    // Apply strict GDI overrides exclusively during a DropDown paint state
    if (g_bInsideDropDownPaint && settings.translucentWindowsCompatibility)
    {
        return SetTextColor_Original(hdc, GetSysColor(COLOR_WINDOWTEXT));
    }
    return SetTextColor_Original(hdc, color);
}

// Hook for UxTheme DrawThemeTextEx
using DrawThemeTextEx_t = decltype(&DrawThemeTextEx);
DrawThemeTextEx_t DrawThemeTextEx_Original;
HRESULT WINAPI DrawThemeTextEx_Hook(HTHEME hTheme, HDC hdc, int iPartId,
    int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect,
    const DTTOPTS *pOptions)
{
    // Apply strict UxTheme overrides exclusively during a DropDown paint state
    if (g_bInsideDropDownPaint && settings.translucentWindowsCompatibility)
    {
        DTTOPTS newOptions;
        if (pOptions)
        {
            newOptions = *pOptions;
        }
        else
        {
            newOptions.dwSize = sizeof(DTTOPTS);
        }

        newOptions.dwFlags |= DTT_TEXTCOLOR;
        newOptions.crText = GetSysColor(COLOR_WINDOWTEXT);

        return DrawThemeTextEx_Original(hTheme, hdc, iPartId, iStateId,
            pszText, cchText, dwTextFlags, pRect, &newOptions);
    }
    return DrawThemeTextEx_Original(hTheme, hdc, iPartId, iStateId, pszText,
        cchText, dwTextFlags, pRect, pOptions);
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

    if (hWnd != nullptr && lpClassName != nullptr &&
        ((reinterpret_cast<ULONG_PTR>(lpClassName) &
            ~static_cast<ULONG_PTR>(0xFFFF)) != 0) &&
        wcscmp(lpClassName, L"DropDown") == 0)
    {
        WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd,
            DropDownSubclassProc, 0);
    }

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
    settings.translucentWindowsCompatibility =
        Wh_GetIntSetting(L"translucentWindowsCompatibility");
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

    HMODULE hGdi32 = GetModuleHandleW(L"gdi32.dll");
    if (hGdi32)
    {
        WindhawkUtils::SetFunctionHook(
            reinterpret_cast<void*>(GetProcAddress(hGdi32, "SetTextColor")),
            reinterpret_cast<void*>(SetTextColor_Hook),
            reinterpret_cast<void**>(&SetTextColor_Original)
        );
    }

    HMODULE hUxTheme = GetModuleHandleW(L"uxtheme.dll");
    if (hUxTheme)
    {
        WindhawkUtils::SetFunctionHook(
            reinterpret_cast<void*>(
                GetProcAddress(hUxTheme, "DrawThemeTextEx")),
            reinterpret_cast<void*>(DrawThemeTextEx_Hook),
            reinterpret_cast<void**>(&DrawThemeTextEx_Original)
        );
    }

    EnumWindows(EnumWindows_AttachDropDowns, 0);

    return TRUE;
}

void Wh_ModUninit()
{
    Wh_Log(L"Uninit");

    EnumWindows(EnumWindows_DetachDropDowns, 0);
}

void Wh_ModSettingsChanged()
{
    LoadSettings();
}
