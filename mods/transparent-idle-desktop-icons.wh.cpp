// ==WindhawkMod==
// @id              transparent-idle-desktop-icons
// @name            Transparent Idle Desktop Icons
// @description     Make desktop icons transparent when idle and restore opacity on hover
// @version         0.1-beta11
// @author          Kitsune
// @github          https://github.com/AromaKitsune
// @include         explorer.exe
// @compilerOptions -lcomctl32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Transparent Idle Desktop Icons
Makes desktop icons semi-transparent when they are not actively being interacted
with. When the mouse hovers over the desktop area, the icons will instantly
restore to full opacity. After the cursor leaves the desktop or remains idle
for the configured duration, the icons fade back to the custom opacity level.

![](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/transparent-idle-desktop-icons.png)

## ⚠ Important note ⚠
This mod has a known incompatibility with the
"[Desktop Live Overlay](https://windhawk.net/mods/desktop-live-overlay)" mod.
Running both mods simultaneously will cause the wallpaper to turn completely
black when interacting with the desktop, or artificially darken when idle. For
the best experience, it is highly recommended to use only one of these mods at a
time.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- idleOpacity: 128
  $name: Idle Opacity
  $description: Opacity level when idle (0 = invisible, 255 = fully visible)
- idleDelay: 2000
  $name: Idle Timeout (ms)
  $description: Time in milliseconds before icons become transparent (0 = disable timer, transparent instantly on mouse leave)
- opaqueCondition: anyInteraction
  $name: Opaque Condition
  $description: When should the icons become fully opaque?
  $options:
    - anyInteraction: Any Interaction
    - hovered: Hovered (Mouse over an icon)
    - selected: Selected (An icon is selected)
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <algorithm>
#include <commctrl.h>

struct
{
    int idleOpacity;
    int idleDelay;
    int opaqueCondition;
} settings;

constexpr UINT_PTR IDT_IDLE_TIMER = 1001;
constexpr UINT_PTR IDT_STATE_TIMER = 1003;
constexpr UINT WM_APP_APPLY_SUBCLASS = WM_APP + 1;
constexpr UINT WM_APP_REMOVE_SUBCLASS = WM_APP + 2;

HWND g_hFolderView = nullptr;
HHOOK g_hMouseHook = nullptr;
bool g_bIsIdle = false;
bool g_bConditionMet = false;
ULONGLONG g_lastWakeTime = 0;

HWND GetFolderViewWnd()
{
    HWND hDefView = FindWindowExW(GetShellWindow(), nullptr,
        L"SHELLDLL_DefView", nullptr);

    if (!hDefView)
    {
        HWND hWorkerW = nullptr;
        while ((hWorkerW = FindWindowExW(nullptr, hWorkerW, L"WorkerW",
            nullptr)) != nullptr)
        {
            hDefView = FindWindowExW(hWorkerW, nullptr,
                L"SHELLDLL_DefView", nullptr);
            if (hDefView)
            {
                break;
            }
        }
    }

    if (!hDefView)
    {
        return nullptr;
    }

    return FindWindowExW(hDefView, nullptr, L"SysListView32", L"FolderView");
}

bool IsFolderViewWnd(HWND hWnd)
{
    WCHAR szBuffer[64];

    if (!GetClassNameW(hWnd, szBuffer, ARRAYSIZE(szBuffer)) ||
        _wcsicmp(szBuffer, L"SysListView32") != 0)
    {
        return false;
    }

    if (!GetWindowTextW(hWnd, szBuffer, ARRAYSIZE(szBuffer)) ||
        _wcsicmp(szBuffer, L"FolderView") != 0)
    {
        return false;
    }

    HWND hParent = GetAncestor(hWnd, GA_PARENT);
    if (!hParent)
    {
        return false;
    }

    if (!GetClassNameW(hParent, szBuffer, ARRAYSIZE(szBuffer)) ||
        _wcsicmp(szBuffer, L"SHELLDLL_DefView") != 0)
    {
        return false;
    }

    return true;
}

void EvaluateOpaqueCondition()
{
    if (!g_hFolderView || settings.opaqueCondition == 0)
    {
        return;
    }

    bool wantsOpaque = false;
    if (settings.opaqueCondition == 1)
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(g_hFolderView, &pt);
        LVHITTESTINFO ht{};
        ht.pt = pt;
        ListView_HitTest(g_hFolderView, &ht);
        wantsOpaque = (ht.flags & LVHT_ONITEM) != 0;
    }
    else if (settings.opaqueCondition == 2)
    {
        wantsOpaque = (ListView_GetSelectedCount(g_hFolderView) > 0);
    }

    if (wantsOpaque)
    {
        // Stay completely awake and kill the timer so it never expires while hovering
        if (g_bIsIdle || !g_bConditionMet)
        {
            SetLayeredWindowAttributes(g_hFolderView, 0, 255, LWA_ALPHA);
            g_bIsIdle = false;
            g_bConditionMet = true;
            KillTimer(g_hFolderView, IDT_IDLE_TIMER);
        }
    }
    else
    {
        // Start the countdown strictly when the condition STOPS being met
        if (g_bConditionMet)
        {
            g_bConditionMet = false;
            if (settings.idleDelay > 0)
            {
                SetTimer(g_hFolderView, IDT_IDLE_TIMER, settings.idleDelay,
                    nullptr);
            }
            else
            {
                g_bIsIdle = true;
                SetLayeredWindowAttributes(g_hFolderView, 0, settings.idleOpacity,
                    LWA_ALPHA);
            }
        }
    }
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && g_hFolderView)
    {
        if (wParam == WM_MOUSEMOVE || wParam == WM_LBUTTONDOWN ||
            wParam == WM_RBUTTONDOWN || wParam == WM_LBUTTONUP ||
            wParam == WM_RBUTTONUP)
        {
            if (settings.opaqueCondition != 0)
            {
                EvaluateOpaqueCondition();
            }
            else
            {
                ULONGLONG now = GetTickCount64();
                if (g_bIsIdle)
                {
                    SetLayeredWindowAttributes(g_hFolderView, 0, 255,
                        LWA_ALPHA);
                    g_bIsIdle = false;

                    if (settings.idleDelay > 0)
                    {
                        SetTimer(g_hFolderView, IDT_IDLE_TIMER,
                            settings.idleDelay, nullptr);
                    }
                    g_lastWakeTime = now;
                }
                else if (settings.idleDelay > 0 && now - g_lastWakeTime > 200)
                {
                    SetTimer(g_hFolderView, IDT_IDLE_TIMER, settings.idleDelay,
                        nullptr);
                    g_lastWakeTime = now;
                }
            }
        }
    }

    return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK FolderViewSubclassProc(HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
        case WM_MOUSEMOVE:
        {
            if (settings.opaqueCondition == 0)
            {
                TRACKMOUSEEVENT tme{};
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);
            }
            break;
        }
        case WM_MOUSELEAVE:
        {
            if (settings.opaqueCondition == 0)
            {
                KillTimer(hWnd, IDT_IDLE_TIMER);
                g_bIsIdle = true;
                SetLayeredWindowAttributes(hWnd, 0, settings.idleOpacity,
                    LWA_ALPHA);
            }
            break;
        }
        case WM_TIMER:
        {
            if (wParam == IDT_IDLE_TIMER)
            {
                KillTimer(hWnd, IDT_IDLE_TIMER);
                g_bIsIdle = true;
                SetLayeredWindowAttributes(hWnd, 0, settings.idleOpacity,
                    LWA_ALPHA);
            }
            else if (wParam == IDT_STATE_TIMER)
            {
                if (settings.opaqueCondition != 0)
                {
                    EvaluateOpaqueCondition();
                }
            }
            break;
        }
        case WM_DESTROY:
        {
            if (g_hMouseHook)
            {
                UnhookWindowsHookEx(g_hMouseHook);
                g_hMouseHook = nullptr;
            }
            g_hFolderView = nullptr;
            RemoveWindowSubclass(hWnd, FolderViewSubclassProc, uIdSubclass);
            break;
        }
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void ApplySubclass(HWND hWnd)
{
    if (!hWnd)
    {
        return;
    }

    g_hFolderView = hWnd;

    LONG_PTR exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    if ((exStyle & WS_EX_LAYERED) == 0)
    {
        SetWindowLongPtrW(hWnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    }

    SetWindowSubclass(hWnd, FolderViewSubclassProc, 1, 0);

    if (!g_hMouseHook)
    {
        DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
        g_hMouseHook = SetWindowsHookExW(WH_MOUSE, MouseHookProc, nullptr,
            dwThreadId);
    }

    if (settings.opaqueCondition != 0)
    {
        // Force the evaluation to start the countdown if the condition is not already met
        g_bConditionMet = true;
        g_bIsIdle = false;
        SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
        SetTimer(hWnd, IDT_STATE_TIMER, 100, nullptr);
        EvaluateOpaqueCondition();
    }
    else
    {
        if (settings.idleDelay > 0)
        {
            g_bIsIdle = false;
            g_lastWakeTime = GetTickCount64();
            SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
            SetTimer(hWnd, IDT_IDLE_TIMER, settings.idleDelay, nullptr);
        }
        else
        {
            g_bIsIdle = true;
            SetLayeredWindowAttributes(hWnd, 0, settings.idleOpacity, LWA_ALPHA);
        }
    }
}

void RemoveSubclass(HWND hWnd)
{
    if (!hWnd)
    {
        return;
    }

    if (g_hMouseHook)
    {
        UnhookWindowsHookEx(g_hMouseHook);
        g_hMouseHook = nullptr;
    }

    KillTimer(hWnd, IDT_IDLE_TIMER);
    KillTimer(hWnd, IDT_STATE_TIMER);

    RemoveWindowSubclass(hWnd, FolderViewSubclassProc, 1);
    SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);

    LONG_PTR exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    SetWindowLongPtrW(hWnd, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);

    g_hFolderView = nullptr;
}

LRESULT CALLBACK UIThreadHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        auto* pMsg = reinterpret_cast<CWPRETSTRUCT*>(lParam);
        if (pMsg->message == WM_APP_APPLY_SUBCLASS)
        {
            ApplySubclass(pMsg->hwnd);
        }
        else if (pMsg->message == WM_APP_REMOVE_SUBCLASS)
        {
            RemoveSubclass(pMsg->hwnd);
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void DispatchToDesktopThread(HWND hWnd, UINT uMsg)
{
    if (!hWnd)
    {
        return;
    }

    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (dwThreadId == GetCurrentThreadId())
    {
        if (uMsg == WM_APP_APPLY_SUBCLASS)
        {
            ApplySubclass(hWnd);
        }
        else if (uMsg == WM_APP_REMOVE_SUBCLASS)
        {
            RemoveSubclass(hWnd);
        }
    }
    else
    {
        HHOOK hHook = SetWindowsHookExW(WH_CALLWNDPROCRET, UIThreadHookProc,
            nullptr, dwThreadId);
        if (hHook)
        {
            SendMessageW(hWnd, uMsg, 0, 0);
            UnhookWindowsHookEx(hHook);
        }
    }
}

using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_Original;

HWND WINAPI CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName,
    LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, PVOID lpParam)
{
    HWND hWnd = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName,
        dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    if (hWnd && IsFolderViewWnd(hWnd))
    {
        static UINT_PTR s_timer = 0;
        s_timer = SetTimer(nullptr, s_timer, 1000,
            [](HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) WINAPI
            {
                KillTimer(nullptr, idEvent);
                DispatchToDesktopThread(GetFolderViewWnd(),
                    WM_APP_APPLY_SUBCLASS);
            });
    }

    return hWnd;
}

void LoadSettings()
{
    settings.idleOpacity = std::clamp(Wh_GetIntSetting(L"idleOpacity"), 0, 255);
    settings.idleDelay = std::max(Wh_GetIntSetting(L"idleDelay"), 0);

    PCWSTR opaqueConditionStr = Wh_GetStringSetting(L"opaqueCondition");
    if (_wcsicmp(opaqueConditionStr, L"hovered") == 0)
    {
        settings.opaqueCondition = 1;
    }
    else if (_wcsicmp(opaqueConditionStr, L"selected") == 0)
    {
        settings.opaqueCondition = 2;
    }
    else
    {
        settings.opaqueCondition = 0;
    }
    Wh_FreeStringSetting(opaqueConditionStr);
}

BOOL Wh_ModInit()
{
    LoadSettings();

    WindhawkUtils::SetFunctionHook(
        CreateWindowExW,
        CreateWindowExW_Hook,
        &CreateWindowExW_Original
    );

    DispatchToDesktopThread(GetFolderViewWnd(), WM_APP_APPLY_SUBCLASS);

    return TRUE;
}

void Wh_ModUninit()
{
    DispatchToDesktopThread(GetFolderViewWnd(), WM_APP_REMOVE_SUBCLASS);
}

BOOL Wh_ModSettingsChanged(BOOL* bReload)
{
    LoadSettings();

    if (g_hFolderView)
    {
        if (settings.opaqueCondition != 0)
        {
            g_bConditionMet = true; // Force the evaluation to start the countdown if the condition is not already met
            g_bIsIdle = false;
            SetLayeredWindowAttributes(g_hFolderView, 0, 255, LWA_ALPHA);
            SetTimer(g_hFolderView, IDT_STATE_TIMER, 100, nullptr);
            EvaluateOpaqueCondition();
        }
        else
        {
            KillTimer(g_hFolderView, IDT_STATE_TIMER);

            if (settings.idleDelay > 0)
            {
                g_bIsIdle = false;
                g_lastWakeTime = GetTickCount64();
                SetLayeredWindowAttributes(g_hFolderView, 0, 255, LWA_ALPHA);
                SetTimer(g_hFolderView, IDT_IDLE_TIMER, settings.idleDelay,
                    nullptr);
            }
            else
            {
                KillTimer(g_hFolderView, IDT_IDLE_TIMER);
                g_bIsIdle = true;
                SetLayeredWindowAttributes(g_hFolderView, 0,
                    settings.idleOpacity, LWA_ALPHA);
            }
        }
    }

    *bReload = FALSE;
    return TRUE;
}
