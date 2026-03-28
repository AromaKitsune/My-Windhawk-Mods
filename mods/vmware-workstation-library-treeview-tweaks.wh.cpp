// ==WindhawkMod==
// @id              vmware-workstation-library-treeview-tweaks
// @name            VMware Workstation Library TreeView Tweaks
// @description     Increases item height, applies modern Explorer themes, and customizes indentation in the Library tree view in VMware Workstation.
// @version         0.1-beta39
// @author          Kitsune
// @github          https://github.com/AromaKitsune
// @include         vmware.exe
// @architecture    x86
// @compilerOptions -lcomctl32 -luxtheme -lgdi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# VMware Workstation Library TreeView Tweaks
This mod modernizes and customizes the "Library" tree view sidebar in VMware Workstation. By default, VMware uses an older, cramped tree view style with classic 3D borders. This mod allows you to inject modern Windows UI elements, improve spacing, and cleanly format the list to your liking.

![Preview](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/vmware-workstation-library-treeview-tweaks.png)

## Features & Configuration
All visual tweaks dynamically scale with your monitor's DPI settings. You can mix and match the following options in the settings tab:

* **Item Height**: Adjust the vertical spacing of the virtual machines in the list (Default: 18px).
* **Themed TreeView**: Applies the modern Windows File Explorer visual style (fading selection boxes, updated expand/collapse arrows). Automatically and instantly syncs with VMware's Dark Mode / System Theme settings!
* **Full-Row Selection**: Expands the highlight selection box across the entire width of the sidebar.
* **Tree Indentation**: Controls the horizontal spacing/indentation of child VMs and folders (Default: 18px). Lower this to push VMs closer to the left edge.
* **Modern Flat Border**: Replaces the dated 3D sunken border (`WS_EX_CLIENTEDGE`) with a clean, flat 1px line (`WS_BORDER`).
* **Remove Expando Buttons**: Hides the expand/collapse arrows completely for a minimalist look. (You can still expand folders by double-clicking them).
* **Disable ToolTips**: Hides the native popup tooltips that appear when hovering over truncated virtual machine names.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- itemHeight: 18
  $name: Item Height (Base)
  $description: The height of the tree-view items in pixels at 100% scaling. (Default is 18).
- themed: true
  $name: Themed TreeView
  $description: Applies the modern Explorer visual style to the tree view.
- fullRowSelect: true
  $name: Full-Row Selection
  $description: Makes the selection highlight span the entire width of the tree view.
- treeIndent: 18
  $name: Tree Indentation (Base)
  $description: The horizontal spacing (in pixels) for child items at 100% scaling. Lower this to push VMs to the left. (Default is 18).
- replaceClientEdge: true
  $name: Modern Flat Border
  $description: Replaces the old 3D sunken border (WS_EX_CLIENTEDGE) with a modern flat 1px border (WS_BORDER).
- removeExpandos: false
  $name: Remove Expando Buttons
  $description: Hides the expand/collapse arrows next to parent items. (You can still double-click parents to expand them).
- disableToolTips: false
  $name: Disable ToolTips
  $description: Prevents the tree view from showing popup tooltips when hovering over truncated items.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <string>
#include <fstream>

#define IDT_THEME_SYNC 1

struct {
    int itemHeight;
    bool themed;
    bool fullRowSelect;
    int treeIndent;
    bool replaceClientEdge;
    bool removeExpandos;
    bool disableToolTips;
} g_settings;

// Helper function to dynamically read the theme state with heavy caching to prevent disk-thrashing
bool IsVMwareDarkMode()
{
    static FILETIME s_ftLastWriteTime = {0};
    static int s_iThemePref = -1; // -1 = system, 0 = light, 1 = dark

    WCHAR szAppData[MAX_PATH];
    if (GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH))
    {
        std::wstring strPrefPath = std::wstring(szAppData) + L"\\VMware\\preferences.ini";
        WIN32_FILE_ATTRIBUTE_DATA fileAttributes;

        // Grab the file's basic attributes. This doesn't actually read the file, so it's lightning fast.
        if (GetFileAttributesExW(strPrefPath.c_str(), GetFileExInfoStandard, &fileAttributes))
        {
            // If the file was modified since our last check, open it and read the preference
            if (CompareFileTime(&fileAttributes.ftLastWriteTime, &s_ftLastWriteTime) != 0)
            {
                s_ftLastWriteTime = fileAttributes.ftLastWriteTime;
                s_iThemePref = -1; // Default to system if string is missing

                std::ifstream inFile(strPrefPath.c_str());
                if (inFile.is_open())
                {
                    std::string strLine;
                    while (std::getline(inFile, strLine))
                    {
                        if (strLine.find("pref.ui.theme") != std::string::npos)
                        {
                            if (strLine.find("\"dark\"") != std::string::npos) s_iThemePref = 1;
                            else if (strLine.find("\"light\"") != std::string::npos) s_iThemePref = 0;
                            else if (strLine.find("\"system\"") != std::string::npos) s_iThemePref = -1;
                            break;
                        }
                    }
                }
            }
        }
    }

    // If explicitly set to dark or light in VMware, prioritize that
    if (s_iThemePref == 1) return true;
    if (s_iThemePref == 0) return false;

    // Otherwise (System fallback), query the Windows OS theme registry
    HKEY hThemeKey;
    DWORD dwUseLightTheme = 1; // Default to light
    DWORD cbDataSize = sizeof(dwUseLightTheme);
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hThemeKey) == ERROR_SUCCESS)
    {
        RegQueryValueExW(hThemeKey, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&dwUseLightTheme, &cbDataSize);
        RegCloseKey(hThemeKey);
    }

    return dwUseLightTheme == 0;
}

// Function that actually applies the theme if a change is detected
void UpdateTreeViewTheme(HWND hTreeViewWnd, bool isForced = false)
{
    if (!g_settings.themed) return;

    bool isDarkMode = IsVMwareDarkMode();

    // We use a custom window property to track the last applied theme for this specific TreeView
    // 0 = Uninitialized, 1 = Light Mode applied, 2 = Dark Mode applied
    int iCurrentState = isDarkMode ? 2 : 1;
    int iLastState = (int)(intptr_t)GetPropW(hTreeViewWnd, L"TreeThemeState");

    if (isForced || (iCurrentState != iLastState))
    {
        SetPropW(hTreeViewWnd, L"TreeThemeState", (HANDLE)(intptr_t)iCurrentState);
        SetWindowTheme(hTreeViewWnd, isDarkMode ? L"DarkMode_Explorer" : L"Explorer", NULL);

        // Force the window to instantly redraw with the new colors
        RedrawWindow(hTreeViewWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME);
    }
}

// Helper function to scale a base value according to the window's current DPI
int ScaleForWindow(HWND hTreeViewWnd, int nBaseValue)
{
    UINT uDpi = 96; // Default 100% scaling

    // Dynamically load GetDpiForWindow to ensure compatibility
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32)
    {
        using GetDpiForWindow_t = UINT(WINAPI*)(HWND);
        auto pfnGetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(hUser32, "GetDpiForWindow");

        if (pfnGetDpiForWindow)
        {
            uDpi = pfnGetDpiForWindow(hTreeViewWnd);
        }
        else
        {
            // Fallback for much older Windows versions
            HDC hdc = GetDC(hTreeViewWnd);
            if (hdc)
            {
                uDpi = GetDeviceCaps(hdc, LOGPIXELSY);
                ReleaseDC(hTreeViewWnd, hdc);
            }
        }
    }

    // Multiply by the DPI and divide by the base 96 DPI
    return MulDiv(nBaseValue, uDpi, 96);
}

// Subclass procedure to enforce the styles persistently
LRESULT CALLBACK TreeViewSubclassProc(HWND hTreeViewWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
        case WM_SETFONT:
        case WM_DPICHANGED:
        {
            // Let the control process the layout change first
            LRESULT lResult = DefSubclassProc(hTreeViewWnd, uMsg, wParam, lParam);

            // Re-apply our custom height and indentation with DPI scaling
            int cyTreeItem = ScaleForWindow(hTreeViewWnd, g_settings.itemHeight);
            int cxTreeIndent = ScaleForWindow(hTreeViewWnd, g_settings.treeIndent);

            SendMessage(hTreeViewWnd, TVM_SETITEMHEIGHT, cyTreeItem, 0);
            SendMessage(hTreeViewWnd, TVM_SETINDENT, cxTreeIndent, 0);

            return lResult;
        }

        case WM_THEMECHANGED:
        {
            LRESULT lResult = DefSubclassProc(hTreeViewWnd, uMsg, wParam, lParam);
            UpdateTreeViewTheme(hTreeViewWnd, true); // Force reapply if OS forces a theme flush
            return lResult;
        }

        case WM_SETTINGCHANGE:
        {
            // Instantly triggered by the OS when you toggle Windows Dark Mode
            UpdateTreeViewTheme(hTreeViewWnd, false);
            break;
        }

        case WM_TIMER:
        {
            // 500ms heartbeat to catch VMware preferences.ini writes
            if (wParam == IDT_THEME_SYNC)
            {
                UpdateTreeViewTheme(hTreeViewWnd, false);
            }
            break;
        }

        case TVM_SETITEMHEIGHT:
        {
            // If VMware actively tries to set a different item height, override it with our scaled one
            int cyTreeItem = ScaleForWindow(hTreeViewWnd, g_settings.itemHeight);
            if (wParam != (WPARAM)cyTreeItem)
            {
                wParam = cyTreeItem;
            }
            break;
        }

        case TVM_SETINDENT:
        {
            // Lock the indentation to our scaled value if VMware tries to change it
            int cxTreeIndent = ScaleForWindow(hTreeViewWnd, g_settings.treeIndent);
            if (wParam != (WPARAM)cxTreeIndent)
            {
                wParam = cxTreeIndent;
            }
            break;
        }

        case WM_NCDESTROY:
            // Safely clean up timers and subclasses when the VMware window is destroyed
            KillTimer(hTreeViewWnd, IDT_THEME_SYNC);
            RemovePropW(hTreeViewWnd, L"TreeThemeState");
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(hTreeViewWnd, TreeViewSubclassProc);
            break;
    }
    return DefSubclassProc(hTreeViewWnd, uMsg, wParam, lParam);
}

// Function to apply tweaks and inject the subclass
void InjectTreeViewStyles(HWND hTreeViewWnd)
{
    // Apply scaled item height and indentation
    int cyTreeItem = ScaleForWindow(hTreeViewWnd, g_settings.itemHeight);
    int cxTreeIndent = ScaleForWindow(hTreeViewWnd, g_settings.treeIndent);

    SendMessage(hTreeViewWnd, TVM_SETITEMHEIGHT, cyTreeItem, 0);
    SendMessage(hTreeViewWnd, TVM_SETINDENT, cxTreeIndent, 0);

    // Apply window styles and extended styles
    LONG_PTR lStyle = GetWindowLongPtrW(hTreeViewWnd, GWL_STYLE);
    LONG_PTR lExStyle = GetWindowLongPtrW(hTreeViewWnd, GWL_EXSTYLE);

    // Full row select
    if (g_settings.fullRowSelect)
    {
        // Remove lines to prevent conflicts with full row selection
        lStyle &= ~TVS_HASLINES;
        lStyle |= TVS_FULLROWSELECT;
    }
    else
    {
        lStyle &= ~TVS_FULLROWSELECT;
        // Intentionally not restoring TVS_HASLINES here
    }

    // Expando Buttons
    if (g_settings.removeExpandos)
    {
        lStyle &= ~TVS_HASBUTTONS;
    }
    else
    {
        lStyle |= TVS_HASBUTTONS;
    }

    // ToolTips
    if (g_settings.disableToolTips)
    {
        lStyle |= TVS_NOTOOLTIPS;
    }
    else
    {
        lStyle &= ~TVS_NOTOOLTIPS;
    }

    // Border styles
    if (g_settings.replaceClientEdge)
    {
        lExStyle &= ~WS_EX_CLIENTEDGE;
        lStyle |= WS_BORDER;
    }
    else
    {
        lStyle &= ~WS_BORDER;
        lExStyle |= WS_EX_CLIENTEDGE;
    }

    SetWindowLongPtrW(hTreeViewWnd, GWL_STYLE, lStyle);
    SetWindowLongPtrW(hTreeViewWnd, GWL_EXSTYLE, lExStyle);

    // Force the window to recalculate its non-client borders and redraw
    SetWindowPos(hTreeViewWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    // Apply theme and start monitoring system changes
    if (g_settings.themed)
    {
        UpdateTreeViewTheme(hTreeViewWnd, true);
        TreeView_SetExtendedStyle(hTreeViewWnd, TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
        // Spin up the background 500ms file-check timer
        SetTimer(hTreeViewWnd, IDT_THEME_SYNC, 500, NULL);
    }
    else
    {
        KillTimer(hTreeViewWnd, IDT_THEME_SYNC);
        RemovePropW(hTreeViewWnd, L"TreeThemeState");
        SetWindowTheme(hTreeViewWnd, NULL, NULL);
    }

    WindhawkUtils::SetWindowSubclassFromAnyThread(hTreeViewWnd, TreeViewSubclassProc, 0);
}

// Hook CreateWindowExW to catch new VMware windows
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

    if (hWnd)
    {
        WCHAR szClassName[256];
        if (GetClassNameW(hWnd, szClassName, ARRAYSIZE(szClassName)))
        {
            if (wcscmp(szClassName, L"ATL:SysTreeView32") == 0)
            {
                InjectTreeViewStyles(hWnd);
            }
        }
    }

    return hWnd;
}

// Callback to find existing tree views and apply subclass
BOOL CALLBACK EnumChildWindows_AttachCallback(HWND hTreeViewWnd, LPARAM lParam)
{
    WCHAR szClassName[256];
    if (GetClassNameW(hTreeViewWnd, szClassName, ARRAYSIZE(szClassName)))
    {
        if (wcscmp(szClassName, L"ATL:SysTreeView32") == 0)
        {
            InjectTreeViewStyles(hTreeViewWnd);
        }
    }
    return TRUE;
}

// Callback for cleaning up subclasses on unload
BOOL CALLBACK EnumChildWindows_DetachCallback(HWND hTreeViewWnd, LPARAM lParam)
{
    WCHAR szClassName[256];
    if (GetClassNameW(hTreeViewWnd, szClassName, ARRAYSIZE(szClassName)))
    {
        if (wcscmp(szClassName, L"ATL:SysTreeView32") == 0)
        {
            // 1. Remove our subclass FIRST so it doesn't fight the style restorations
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(hTreeViewWnd, TreeViewSubclassProc);

            // 2. Stop our background theme-sync timer and clean up properties
            KillTimer(hTreeViewWnd, IDT_THEME_SYNC);
            RemovePropW(hTreeViewWnd, L"TreeThemeState");

            // 3. Strip the Explorer theme so it returns to normal
            SetWindowTheme(hTreeViewWnd, NULL, NULL);

            // 4. Remove extended styles (like double buffering)
            TreeView_SetExtendedStyle(hTreeViewWnd, 0, TVS_EX_DOUBLEBUFFER);

            // 5. Best-effort style restoration
            LONG_PTR lStyle = GetWindowLongPtrW(hTreeViewWnd, GWL_STYLE);
            LONG_PTR lExStyle = GetWindowLongPtrW(hTreeViewWnd, GWL_EXSTYLE);

            lStyle &= ~TVS_FULLROWSELECT;
            lStyle |= TVS_HASBUTTONS;
            lStyle &= ~TVS_NOTOOLTIPS; // Restore default tooltip behavior
            // Intentionally not restoring TVS_HASLINES here

            lStyle &= ~WS_BORDER;
            lExStyle |= WS_EX_CLIENTEDGE;

            SetWindowLongPtrW(hTreeViewWnd, GWL_STYLE, lStyle);
            SetWindowLongPtrW(hTreeViewWnd, GWL_EXSTYLE, lExStyle);

            // 6. Recalculate borders and force a complete visual refresh
            SetWindowPos(hTreeViewWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            RedrawWindow(hTreeViewWnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW);
        }
    }
    return TRUE;
}

// Callback to iterate through process windows for initialization
BOOL CALLBACK EnumWindows_AttachCallback(HWND hVMwareWnd, LPARAM lParam)
{
    DWORD dwProcessId;
    GetWindowThreadProcessId(hVMwareWnd, &dwProcessId);
    if (dwProcessId == GetCurrentProcessId())
    {
        EnumChildWindows(hVMwareWnd, EnumChildWindows_AttachCallback, 0);
    }
    return TRUE;
}

// Callback to iterate through process windows for cleanup
BOOL CALLBACK EnumWindows_DetachCallback(HWND hVMwareWnd, LPARAM lParam)
{
    DWORD dwProcessId;
    GetWindowThreadProcessId(hVMwareWnd, &dwProcessId);
    if (dwProcessId == GetCurrentProcessId())
    {
        EnumChildWindows(hVMwareWnd, EnumChildWindows_DetachCallback, 0);
    }
    return TRUE;
}

void LoadSettings()
{
    g_settings.itemHeight = Wh_GetIntSetting(L"itemHeight");
    g_settings.themed = Wh_GetIntSetting(L"themed");
    g_settings.fullRowSelect = Wh_GetIntSetting(L"fullRowSelect");
    g_settings.treeIndent = Wh_GetIntSetting(L"treeIndent");
    g_settings.replaceClientEdge = Wh_GetIntSetting(L"replaceClientEdge");
    g_settings.removeExpandos = Wh_GetIntSetting(L"removeExpandos");
    g_settings.disableToolTips = Wh_GetIntSetting(L"disableToolTips");
}

// Mod initialization
BOOL Wh_ModInit()
{
    LoadSettings();

    Wh_SetFunctionHook((void*)CreateWindowExW, (void*)CreateWindowExW_Hook, (void**)&CreateWindowExW_Original);

    EnumWindows(EnumWindows_AttachCallback, 0);

    return TRUE;
}

// Mod uninitialization
void Wh_ModUninit()
{
    EnumWindows(EnumWindows_DetachCallback, 0);
}

// Reload settings
void Wh_ModSettingsChanged()
{
    LoadSettings();

    EnumWindows(EnumWindows_AttachCallback, 0);
}
