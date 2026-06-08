// ==WindhawkMod==
// @id              fix-darkmode-listviews
// @name            Fix Darkmode ListViews
// @description     Fixes ListViews in dark mode
// @version         1.0-beta40
// @author          Kitsune
// @github          https://github.com/AromaKitsune
// @include         *
// @compilerOptions -lcomctl32 -lgdi32 -luxtheme -lntdll
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Fix Darkmode ListViews
Fixes hardcoded text colors in ListViews when using a system-wide dark theme such
as "Rectify11 dark theme".

| Before | After |
| :----: | :---: |
| ![](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/fix-darkmode-listviews_before.png) | ![](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/fix-darkmode-listviews_after.png) |
| ![](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/fix-darkmode-listviews_altColors_before.png) | ![](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/fix-darkmode-listviews_altColors_after.png) |

## Extended features
The [original mod](https://windhawk.net/mods/fix-darkmode-listviews) was created
by **Reabstraction**. This forked mod significantly expands the scope of the
original fix with the following improvements:
* Added compatibility with dialogs
* Added compatibility with  "Translucent Windows" mod.
* Fixed unreadable hardcoded blue and green text colors for compressed and
  encrypted files/folders.

## Configuration
**Translucent Windows compatibility:** Fixes unreadable dark text in the
Explorer address bar drop-down menu when using the
"[Translucent Windows](https://windhawk.net/mods/translucent-windows)" mod.
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
#include <string_view>

struct
{
    bool translucentWindowsCompatibility;
} settings;

// Define OCM_NOTIFY for message reflection
constexpr UINT OCM_NOTIFY = 0x204E;

// Theme Part and State Definitions for ItemsView
namespace ThemeProps
{
    constexpr int PART_PROPERTY = 4;
    constexpr int STATE_FILENAMECOMPRESSEDPROPERTY = 5;
    constexpr int STATE_FILENAMEENCRYPTEDPROPERTY = 7;
    constexpr int TMT_TEXTCOLOR = 3803;
}

// Dark-Mode Friendly Replacement Colors for the Registry Spoof Fallback
constexpr COLORREF COLOR_COMPRESSED_DARK = RGB(86, 156, 214);
constexpr COLORREF COLOR_ENCRYPTED_DARK  = RGB(78, 201, 176);

// NTDLL Constants for NtQueryKey
constexpr NTSTATUS STATUS_SUCCESS = 0x00000000;
constexpr NTSTATUS STATUS_BUFFER_TOO_SMALL = static_cast<NTSTATUS>(0xC0000023);

// Thread-local flags
thread_local bool g_bInsideDropDownPaint = false;
thread_local bool g_bInsideRegistrySpoof = false;

enum KEY_INFORMATION_CLASS
{
    KeyNameInformation = 3
};

struct KEY_NAME_INFORMATION
{
    ULONG NameLength;
    WCHAR Name[1];
};

EXTERN_C NTSYSAPI NTSTATUS NTAPI NtQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
);

bool EndsWith(std::wstring_view str, std::wstring_view suffix)
{
    if (str.length() < suffix.length())
    {
        return false;
    }
    return (_wcsnicmp(str.data() + str.length() - suffix.length(),
        suffix.data(), suffix.length()) == 0);
}

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

// Dynamically fetch the text color from the active .msstyles file
COLORREF GetThemeAltColor(bool bIsCompressed)
{
    // Set our dark pastel colors as the default safety fallback
    COLORREF color = bIsCompressed ? COLOR_COMPRESSED_DARK : COLOR_ENCRYPTED_DARK;

    // Open the specific DarkMode ItemsView class
    HTHEME hTheme = OpenThemeData(nullptr, L"DarkMode::ItemsView");
    if (hTheme != nullptr)
    {
        COLORREF themeColor;
        int iStateId = bIsCompressed ? ThemeProps::STATE_FILENAMECOMPRESSEDPROPERTY : ThemeProps::STATE_FILENAMEENCRYPTEDPROPERTY;

        // Query the active theme for the TEXTCOLOR property
        HRESULT hr = GetThemeColor(hTheme, ThemeProps::PART_PROPERTY, iStateId, ThemeProps::TMT_TEXTCOLOR, &themeColor);
        if (SUCCEEDED(hr))
        {
            // If the theme author explicitly defined it, override our fallback!
            color = themeColor;
        }
        CloseThemeData(hTheme);
    }

    return color;
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

BOOL CALLBACK InitDropDownListViewEnumProc(HWND hWnd, LPARAM lParam)
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
        EnumChildWindows(hWnd, InitDropDownListViewEnumProc, 0);
    }
    else if (uMsg == WM_NCDESTROY)
    {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd,
            DropDownSubclassProc);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK InitDialogListViewEnumProc(HWND hWnd, LPARAM lParam)
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
BOOL CALLBACK InitEnumDropDownWindowsProc(HWND hWnd, LPARAM lParam)
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
BOOL CALLBACK UninitDropDownListViewEnumProc(HWND hWnd, LPARAM lParam)
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
BOOL CALLBACK UninitEnumDropDownWindowsProc(HWND hWnd, LPARAM lParam)
{
    WCHAR szClassName[256];
    if (GetClassNameW(hWnd, szClassName, ARRAYSIZE(szClassName)) &&
        wcscmp(szClassName, L"DropDown") == 0)
    {
        // Detach inner ListView subclasses first
        EnumChildWindows(hWnd, UninitDropDownListViewEnumProc, 0);

        // Detach the parent DropDown subclass
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd,
            DropDownSubclassProc);
    }
    return TRUE;
}

// Hook for RegQueryValueExW via kernelbase.dll to spoof Explorer colors
using RegQueryValueExW_t = decltype(&RegQueryValueExW);
RegQueryValueExW_t RegQueryValueExW_Original;
LSTATUS WINAPI RegQueryValueExW_Hook(HKEY hKey, LPCWSTR lpValueName,
    LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    if (!g_bInsideRegistrySpoof && lpValueName != nullptr)
    {
        bool bIsAltColor = (_wcsicmp(lpValueName, L"AltColor") == 0);
        bool bIsAltEncryption = !bIsAltColor &&
            (_wcsicmp(lpValueName, L"AltEncryptionColor") == 0);

        if (bIsAltColor || bIsAltEncryption)
        {
            // Only spoof the colors if a custom visual style is active
            if (!IsDefaultAeroVisualStyleActive())
            {
                bool bIsExplorerKey = false;
                ULONG ulAlloc = 0;
                NTSTATUS status = NtQueryKey(hKey, KeyNameInformation, nullptr,
                    0, &ulAlloc);

                if (status == STATUS_BUFFER_TOO_SMALL)
                {
                    ulAlloc += sizeof(WCHAR);
                    auto* pNameInfo = reinterpret_cast<KEY_NAME_INFORMATION*>(
                        LocalAlloc(LPTR, ulAlloc));

                    if (pNameInfo != nullptr)
                    {
                        status = NtQueryKey(hKey, KeyNameInformation, pNameInfo,
                            ulAlloc, &ulAlloc);

                        if (status == STATUS_SUCCESS)
                        {
                            pNameInfo->Name[pNameInfo->NameLength / sizeof(WCHAR)] = L'\0';
                            if (EndsWith(pNameInfo->Name,
                                    L"\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer"))
                            {
                                bIsExplorerKey = true;
                            }
                        }
                        LocalFree(pNameInfo);
                    }
                }

                if (bIsExplorerKey)
                {
                    // Engage recursion guard because GetThemeAltColor triggers API calls
                    g_bInsideRegistrySpoof = true;

                    if (lpType != nullptr)
                    {
                        *lpType = REG_DWORD;
                    }

                    // Pass 1: Sizing query
                    if (lpData == nullptr && lpcbData != nullptr)
                    {
                        *lpcbData = sizeof(DWORD);
                        g_bInsideRegistrySpoof = false;
                        return ERROR_SUCCESS;
                    }

                    // Pass 2: Data query
                    if (lpData != nullptr && lpcbData != nullptr)
                    {
                        if (*lpcbData >= sizeof(DWORD))
                        {
                            // Fetch the visual color
                            COLORREF themeColor = GetThemeAltColor(bIsAltColor);

                            // Write it into the registry buffer as a standard DWORD
                            *reinterpret_cast<DWORD*>(lpData) = static_cast<DWORD>(themeColor);
                            *lpcbData = sizeof(DWORD);
                            g_bInsideRegistrySpoof = false;
                            return ERROR_SUCCESS;
                        }
                        else
                        {
                            *lpcbData = sizeof(DWORD);
                            g_bInsideRegistrySpoof = false;
                            return ERROR_MORE_DATA;
                        }
                    }

                    g_bInsideRegistrySpoof = false;
                }
            }
        }
    }

    return RegQueryValueExW_Original(hKey, lpValueName, lpReserved, lpType,
        lpData, lpcbData);
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
        EnumChildWindows(hDlg, InitDialogListViewEnumProc, 0);
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

    // Deep hook into kernelbase.dll to ensure early registry spoofing succeeds
    HMODULE hKernelBase = GetModuleHandleW(L"kernelbase.dll");
    if (!hKernelBase)
    {
        hKernelBase = LoadLibraryExW(L"kernelbase.dll", nullptr,
            LOAD_LIBRARY_SEARCH_SYSTEM32);
    }
    if (hKernelBase)
    {
        auto pfnRegQueryValueExW = reinterpret_cast<void*>(
            GetProcAddress(hKernelBase, "RegQueryValueExW"));

        if (pfnRegQueryValueExW)
        {
            WindhawkUtils::SetFunctionHook(
                pfnRegQueryValueExW,
                reinterpret_cast<void*>(RegQueryValueExW_Hook),
                reinterpret_cast<void**>(&RegQueryValueExW_Original)
            );
        }
    }

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

    EnumWindows(InitEnumDropDownWindowsProc, 0);

    return TRUE;
}

void Wh_ModUninit()
{
    Wh_Log(L"Uninit");

    EnumWindows(UninitEnumDropDownWindowsProc, 0);
}

void Wh_ModSettingsChanged()
{
    LoadSettings();
}
