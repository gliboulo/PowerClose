// PowerClose — kill la fenêtre au premier plan via un raccourci global.
// - Raccourci par défaut : Ctrl+Q (remappable via systray)
// - Icône systray, "Lancer au démarrage" (cochable), "Changer le raccourci…", "Quitter"
// - Localisation auto : FR / EN / ES / DE / ZH-CN
// - Sauvegarde du raccourci dans powerclose.ini (même dossier)
// Build (MSYS2 UCRT64) : voir commande en bas.

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <objbase.h>
#include <shobjidl.h>
#include <stdio.h>

#define WM_TRAYICON       (WM_APP + 1)
#define ID_TRAY_AUTOSTART 1001
#define ID_TRAY_REMAP     1002
#define ID_TRAY_EXIT      1003
#define IDI_APPICON       101

/* ==================== Localisation ==================== */
typedef struct {
    const wchar_t* AppName;
    const wchar_t* TipFmt;
    const wchar_t* Menu_AutoStart;
    const wchar_t* Menu_Remap;
    const wchar_t* Menu_Quit;
    const wchar_t* Err_CreateHidden;
    const wchar_t* Err_Hotkey;
    const wchar_t* Remap_Title;
    const wchar_t* Remap_Prompt;
    const wchar_t* Remap_Conflict;
    const wchar_t* Remap_Toast;
    const wchar_t* Key_Ctrl;
    const wchar_t* Key_Alt;
    const wchar_t* Key_Shift;
    const wchar_t* Key_Win;
    const wchar_t* Key_Or;
    const wchar_t* Hotkey_DefaultHint;
} STRINGS;

static STRINGS gS;

static void InitLocalization(void) {
    LANGID lid = GetUserDefaultUILanguage();
    WORD primary = PRIMARYLANGID(lid);
    WORD sub     = SUBLANGID(lid);

    if (primary == LANG_FRENCH) {
        gS.AppName         = L"PowerClose";
        gS.TipFmt          = L"PowerClose — %s : tuer la fenêtre active";
        gS.Menu_AutoStart  = L"Lancer au démarrage";
        gS.Menu_Remap      = L"Changer le raccourci…";
        gS.Menu_Quit       = L"Quitter PowerClose";
        gS.Err_CreateHidden= L"Échec de création de la fenêtre cachée.";
        gS.Err_Hotkey      = L"Impossible de s'enregistrer sur ce raccourci.";
        gS.Remap_Title     = L"Nouveau raccourci";
        gS.Remap_Prompt    = L"Appuie sur la nouvelle combinaison… (Échap pour annuler)";
        gS.Remap_Conflict  = L"Échec d'enregistrement du raccourci (en conflit ?) : %s";
        gS.Remap_Toast     = L"Raccourci défini : %s";
        gS.Key_Ctrl        = L"Ctrl";
        gS.Key_Alt         = L"Alt";
        gS.Key_Shift       = L"Shift";
        gS.Key_Win         = L"Win";
        gS.Key_Or          = L"ou";
        gS.Hotkey_DefaultHint = L"Ctrl+Q";
    }
    else if (primary == LANG_SPANISH) {
        gS.AppName         = L"PowerClose";
        gS.TipFmt          = L"PowerClose — %s: cerrar la ventana en primer plano";
        gS.Menu_AutoStart  = L"Iniciar con Windows";
        gS.Menu_Remap      = L"Cambiar atajo…";
        gS.Menu_Quit       = L"Salir de PowerClose";
        gS.Err_CreateHidden= L"Error al crear la ventana oculta.";
        gS.Err_Hotkey      = L"No se pudo registrar este atajo.";
        gS.Remap_Title     = L"Nuevo atajo";
        gS.Remap_Prompt    = L"Pulsa la nueva combinación… (Esc para cancelar)";
        gS.Remap_Conflict  = L"No se pudo registrar el atajo (¿conflicto?): %s";
        gS.Remap_Toast     = L"Atajo establecido: %s";
        gS.Key_Ctrl        = L"Ctrl";
        gS.Key_Alt         = L"Alt";
        gS.Key_Shift       = L"Shift";
        gS.Key_Win         = L"Win";
        gS.Key_Or          = L"o";
        gS.Hotkey_DefaultHint = L"Ctrl+Q";
    }
    else if (primary == LANG_GERMAN) {
        gS.AppName         = L"PowerClose";
        gS.TipFmt          = L"PowerClose — %s: Vordergrundfenster beenden";
        gS.Menu_AutoStart  = L"Beim Start ausführen";
        gS.Menu_Remap      = L"Tastenkürzel ändern…";
        gS.Menu_Quit       = L"PowerClose beenden";
        gS.Err_CreateHidden= L"Verstecktes Fenster konnte nicht erstellt werden.";
        gS.Err_Hotkey      = L"Dieses Tastenkürzel konnte nicht registriert werden.";
        gS.Remap_Title     = L"Neues Tastenkürzel";
        gS.Remap_Prompt    = L"Drücke die neue Kombination… (Esc zum Abbrechen)";
        gS.Remap_Conflict  = L"Kürzel konnte nicht registriert werden (Konflikt?): %s";
        gS.Remap_Toast     = L"Kürzel gesetzt: %s";
        gS.Key_Ctrl        = L"Strg";
        gS.Key_Alt         = L"Alt";
        gS.Key_Shift       = L"Umsch";
        gS.Key_Win         = L"Win";
        gS.Key_Or          = L"oder";
        gS.Hotkey_DefaultHint = L"Strg+Q";
    }
    else if (primary == LANG_CHINESE &&
            (sub == SUBLANG_CHINESE_SIMPLIFIED || sub == SUBLANG_CHINESE_SINGAPORE)) {
        gS.AppName         = L"PowerClose";
        gS.TipFmt          = L"PowerClose — %s：强制关闭前台窗口";
        gS.Menu_AutoStart  = L"开机自启动";
        gS.Menu_Remap      = L"更改快捷键…";
        gS.Menu_Quit       = L"退出 PowerClose";
        gS.Err_CreateHidden= L"创建隐藏窗口失败。";
        gS.Err_Hotkey      = L"无法注册此快捷键。";
        gS.Remap_Title     = L"新的快捷键";
        gS.Remap_Prompt    = L"请按下新的组合键…（Esc 取消）";
        gS.Remap_Conflict  = L"注册快捷键失败（冲突？）：%s";
        gS.Remap_Toast     = L"已设置快捷键：%s";
        gS.Key_Ctrl        = L"Ctrl";
        gS.Key_Alt         = L"Alt";
        gS.Key_Shift       = L"Shift";
        gS.Key_Win         = L"Win";
        gS.Key_Or          = L"或";
        gS.Hotkey_DefaultHint = L"Ctrl+Q";
    }
    else { // English default
        gS.AppName         = L"PowerClose";
        gS.TipFmt          = L"PowerClose — %s: kill foreground window";
        gS.Menu_AutoStart  = L"Launch at startup";
        gS.Menu_Remap      = L"Change shortcut…";
        gS.Menu_Quit       = L"Quit PowerClose";
        gS.Err_CreateHidden= L"Failed to create hidden window.";
        gS.Err_Hotkey      = L"Unable to register this hotkey.";
        gS.Remap_Title     = L"New shortcut";
        gS.Remap_Prompt    = L"Press the new combination… (Esc to cancel)";
        gS.Remap_Conflict  = L"Failed to register hotkey (conflict?): %s";
        gS.Remap_Toast     = L"Shortcut set to: %s";
        gS.Key_Ctrl        = L"Ctrl";
        gS.Key_Alt         = L"Alt";
        gS.Key_Shift       = L"Shift";
        gS.Key_Win         = L"Win";
        gS.Key_Or          = L"or";
        gS.Hotkey_DefaultHint = L"Ctrl+Q";
    }
}

/* ============ util INI / chemins ============ */
static void GetExeDir(wchar_t* buf, size_t n) {
    DWORD len = GetModuleFileNameW(NULL, buf, (DWORD)n);
    for (DWORD i=len; i>0; --i) if (buf[i-1] == L'\\') { buf[i] = 0; break; }
}
static void GetIniPath(wchar_t* buf, size_t n) {
    GetExeDir(buf, n);
    lstrcatW(buf, L"powerclose.ini");
}

/* ============ privileges / sécurité ============ */
static BOOL EnableDebugPrivilege(void) {
    HANDLE tok = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tok))
        return FALSE;
    LUID luid;
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
        CloseHandle(tok); return FALSE;
    }
    TOKEN_PRIVILEGES tp = {0};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(tok, FALSE, &tp, sizeof(tp), NULL, NULL);
    CloseHandle(tok);
    return GetLastError() == ERROR_SUCCESS;
}

static BOOL IsCriticalName(const wchar_t* name) {
    if (!name) return FALSE;
    return (_wcsicmp(name, L"csrss.exe")==0) ||
           (_wcsicmp(name, L"wininit.exe")==0) ||
           (_wcsicmp(name, L"winlogon.exe")==0);
}

/* ============ Startup (raccourci .lnk) ============ */
static void GetStartupLinkPath(wchar_t *linkPath, size_t size) {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, path))) {
        swprintf(linkPath, size, L"%s\\PowerClose.lnk", path);
    } else {
        linkPath[0] = 0;
    }
}
static BOOL IsInStartup(void) {
    wchar_t linkPath[MAX_PATH];
    GetStartupLinkPath(linkPath, MAX_PATH);
    return GetFileAttributesW(linkPath) != INVALID_FILE_ATTRIBUTES;
}
static void AddToStartup(void) {
    wchar_t linkPath[MAX_PATH];
    GetStartupLinkPath(linkPath, MAX_PATH);
    if (!linkPath[0]) return;

    IShellLinkW *psl;
    if (SUCCEEDED(CoInitialize(NULL)) &&
        SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                   &IID_IShellLinkW, (LPVOID*)&psl))) {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        psl->lpVtbl->SetPath(psl, exePath);
        psl->lpVtbl->SetDescription(psl, L"PowerClose AutoStart");
        IPersistFile *ppf;
        if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (LPVOID*)&ppf))) {
            ppf->lpVtbl->Save(ppf, linkPath, TRUE);
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }
    CoUninitialize();
}
static void RemoveFromStartup(void) {
    wchar_t linkPath[MAX_PATH];
    GetStartupLinkPath(linkPath, MAX_PATH);
    if (linkPath[0]) DeleteFileW(linkPath);
}

/* ============ Hotkey: état + ini ============ */
static UINT g_hotkeyMods = MOD_CONTROL;  // défaut: Ctrl
static UINT g_hotkeyVK   = 'Q';          // défaut: Q
static BOOL g_autostart  = FALSE;        // état du menu autostart

static void FormatHotkey(UINT mods, UINT vk, wchar_t* out, size_t n) {
    out[0] = 0;
    BOOL first = TRUE;
    if (mods & MOD_CONTROL) { lstrcatW(out, gS.Key_Ctrl); first = FALSE; }
    if (mods & MOD_ALT)     { if (!first) lstrcatW(out, L"+"); lstrcatW(out, gS.Key_Alt); first = FALSE; }
    if (mods & MOD_SHIFT)   { if (!first) lstrcatW(out, L"+"); lstrcatW(out, gS.Key_Shift); first = FALSE; }
    if (mods & MOD_WIN)     { if (!first) lstrcatW(out, L"+"); lstrcatW(out, gS.Key_Win); first = FALSE; }
    if (vk) {
        if (!first) lstrcatW(out, L"+");
        wchar_t key[16]; key[0]=0;
        if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) {
            swprintf(key, 16, L"%c", (wchar_t)vk);
        } else {
            swprintf(key, 16, L"VK%02X", vk);
        }
        lstrcatW(out, key);
    }
}

static void LoadHotkeyFromIni(void) {
    wchar_t ini[MAX_PATH]; GetIniPath(ini, MAX_PATH);
    wchar_t buf[64]; buf[0]=0;
    GetPrivateProfileStringW(L"Hotkey", L"Mods", L"", buf, 64, ini);
    if (buf[0]) g_hotkeyMods = (UINT)wcstoul(buf, NULL, 10);
    GetPrivateProfileStringW(L"Hotkey", L"VK", L"", buf, 64, ini);
    if (buf[0]) g_hotkeyVK = (UINT)wcstoul(buf, NULL, 10);
}
static void SaveHotkeyToIni(void) {
    wchar_t ini[MAX_PATH]; GetIniPath(ini, MAX_PATH);
    wchar_t v[32];
    swprintf(v, 32, L"%u", g_hotkeyMods); WritePrivateProfileStringW(L"Hotkey", L"Mods", v, ini);
    swprintf(v, 32, L"%u", g_hotkeyVK);   WritePrivateProfileStringW(L"Hotkey", L"VK",   v, ini);
}

/* —— Protos utilisés plus bas —— */
static BOOL ApplyHotkey(HWND hwndTipUpdate);
static void Tray_UpdateTip(void);

/* ============ Kill ============ */
static void KillForeground(void) {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return;

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid <= 4) return;

    // Filtre nom de binaire
    wchar_t nameBuf[MAX_PATH] = L"";
    HANDLE hQuery = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hQuery) {
        DWORD len = MAX_PATH;
        QueryFullProcessImageNameW(hQuery, 0, nameBuf, &len);
        const wchar_t* base = wcsrchr(nameBuf, L'\\');
        const wchar_t* fname = base ? base+1 : nameBuf;
        if (IsCriticalName(fname)) { CloseHandle(hQuery); return; }
        CloseHandle(hQuery);
    }

    EnableDebugPrivilege();

    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProc) return;
    TerminateProcess(hProc, 0xC0000005);
    CloseHandle(hProc);
}

/* —— Hook clavier bas-niveau pour remap —— */
static HHOOK g_hHook = NULL;
static UINT g_tmpMods = 0, g_tmpVK = 0;

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* k = (KBDLLHOOKSTRUCT*)lParam;
        g_tmpMods = 0;
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000) g_tmpMods |= MOD_CONTROL;
        if (GetAsyncKeyState(VK_MENU)    & 0x8000) g_tmpMods |= MOD_ALT;
        if (GetAsyncKeyState(VK_SHIFT)   & 0x8000) g_tmpMods |= MOD_SHIFT;
        if ((GetAsyncKeyState(VK_LWIN)   & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000)) g_tmpMods |= MOD_WIN;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            UINT vk = (UINT)k->vkCode;
            if (vk==VK_CONTROL || vk==VK_LCONTROL || vk==VK_RCONTROL ||
                vk==VK_MENU    || vk==VK_LMENU    || vk==VK_RMENU    ||
                vk==VK_SHIFT   || vk==VK_LSHIFT   || vk==VK_RSHIFT   ||
                vk==VK_LWIN    || vk==VK_RWIN) {
                // ignore
            } else {
                g_tmpVK = vk;
                PostQuitMessage(0);
            }
        } else if (wParam == WM_KEYUP && k->vkCode == VK_ESCAPE) {
            g_tmpVK = 0; g_tmpMods = 0;
            PostQuitMessage(0);
        }
    }
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

static BOOL CaptureNewHotkey(HWND owner) {
    MessageBoxW(owner, gS.Remap_Prompt, gS.Remap_Title, MB_OK | MB_ICONINFORMATION);
    g_tmpMods = 0; g_tmpVK = 0;
    g_hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!g_hHook) return FALSE;

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    UnhookWindowsHookEx(g_hHook); g_hHook = NULL;

    if (g_tmpVK == 0) return FALSE; // annulé
    g_hotkeyMods = g_tmpMods ? g_tmpMods : 0;
    g_hotkeyVK   = g_tmpVK;
    return TRUE;
}

/* ==================== Systray ==================== */
static NOTIFYICONDATAW g_nid = {0};

static void Tray_UpdateTip(void) {
    wchar_t combo[64]; FormatHotkey(g_hotkeyMods, g_hotkeyVK, combo, 64);
    wchar_t tip[128];  swprintf(tip, 128, gS.TipFmt, combo);
    g_nid.uFlags = NIF_TIP | NIF_INFO;
    lstrcpynW(g_nid.szTip, tip, ARRAYSIZE(g_nid.szTip));
    lstrcpynW(g_nid.szInfoTitle, gS.AppName, ARRAYSIZE(g_nid.szInfoTitle));
    wchar_t toast[128]; swprintf(toast, 128, gS.Remap_Toast, combo);
    lstrcpynW(g_nid.szInfo, toast, ARRAYSIZE(g_nid.szInfo));
    g_nid.dwInfoFlags = NIIF_NONE;
    Shell_NotifyIconW(NIM_MODIFY, &g_nid);
}

static void Tray_Add(HWND hwnd) {
    HINSTANCE hInst = GetModuleHandleW(NULL);
    g_nid.cbSize = sizeof(NOTIFYICONDATAW);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_APPICON));
    if (!g_nid.hIcon) g_nid.hIcon = LoadIconW(NULL, IDI_APPLICATION);

    wchar_t combo[64]; FormatHotkey(g_hotkeyMods, g_hotkeyVK, combo, 64);
    wchar_t tip[128];  swprintf(tip, 128, gS.TipFmt, combo);
    lstrcpynW(g_nid.szTip, tip, ARRAYSIZE(g_nid.szTip));
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}

static void Tray_Delete(void) {
    if (g_nid.cbSize) {
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        if (g_nid.hIcon) DestroyIcon(g_nid.hIcon);
        ZeroMemory(&g_nid, sizeof(g_nid));
    }
}

/* ==================== Menu & fenêtre ==================== */
static void Tray_ShowMenu(HWND hwnd) {
    POINT pt; GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    UINT checkFlag = g_autostart ? MF_CHECKED : MF_UNCHECKED;
    AppendMenuW(menu, MF_STRING | checkFlag, ID_TRAY_AUTOSTART, gS.Menu_AutoStart);
    AppendMenuW(menu, MF_STRING, ID_TRAY_REMAP,     gS.Menu_Remap);
    AppendMenuW(menu, MF_STRING, ID_TRAY_EXIT,      gS.Menu_Quit);
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(menu);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
            Tray_ShowMenu(hwnd);
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT) {
            PostQuitMessage(0);
            return 0;
        } else if (LOWORD(wParam) == ID_TRAY_AUTOSTART) {
            if (g_autostart) { RemoveFromStartup(); g_autostart = FALSE; }
            else             { AddToStartup();      g_autostart = TRUE;  }
        } else if (LOWORD(wParam) == ID_TRAY_REMAP) {
            if (CaptureNewHotkey(hwnd)) {
                if (ApplyHotkey(hwnd)) {
                    SaveHotkeyToIni();
                    Tray_UpdateTip();
                }
            }
        }
        break;

    case WM_DESTROY:
        Tray_Delete();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static HWND CreateHiddenWindow(HINSTANCE hInst) {
    const wchar_t *CLASS_NAME = L"PowerCloseHiddenWindow";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon         = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_APPICON));
    if (!RegisterClassW(&wc)) return NULL;

    return CreateWindowExW(0, CLASS_NAME, gS.AppName,
                           WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                           NULL, NULL, hInst, NULL);
}

/* ==================== Hotkey apply ==================== */
static BOOL ApplyHotkey(HWND hwndTipUpdate) {
    UnregisterHotKey(NULL, 1);
    UINT mods = g_hotkeyMods | 0x4000; // MOD_NOREPEAT
    if (!RegisterHotKey(NULL, 1, mods, g_hotkeyVK)) {
        wchar_t combo[64]; FormatHotkey(g_hotkeyMods, g_hotkeyVK, combo, 64);
        wchar_t msg[128]; swprintf(msg, 128, gS.Remap_Conflict, combo);
        MessageBoxW(hwndTipUpdate, msg, gS.AppName, MB_ICONWARNING);
        return FALSE;
    }
    return TRUE;
}

/* ==================== Entry point ==================== */
int wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    InitLocalization();
    LoadHotkeyFromIni();

    HWND hwnd = CreateHiddenWindow(hInst);
    if (!hwnd) {
        MessageBoxW(NULL, gS.Err_CreateHidden, gS.AppName, MB_ICONERROR);
        return 1;
    }
    Tray_Add(hwnd);

    g_autostart = IsInStartup();

    if (!ApplyHotkey(hwnd)) {
        MessageBoxW(NULL, gS.Err_Hotkey, gS.AppName, MB_ICONERROR);
        DestroyWindow(hwnd);
        return 1;
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_HOTKEY && msg.wParam == 1) {
            KillForeground();
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnregisterHotKey(NULL, 1);
    DestroyWindow(hwnd);
    return 0;
}
