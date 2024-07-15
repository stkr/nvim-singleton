// nvim-dispatch.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//

#include <string>
#include <tchar.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

const string relative_config_path = ".config\\nvim-singleton\\config.txt";
const string relative_pid_path = ".nvim-singleton-pid.txt";

const string default_alacritty_path = "C:\\Program Files\\Alacritty\\alacritty.exe";
const string default_nvim_path = "c:\\Program Files\\Neovim\\bin\\nvim.exe";
const string default_nvim_pipe_path = "\\\\.\\pipe\\nvim-singleton";

typedef struct _config_params_t {
  string alacritty_path;
  string nvim_path;
  string nvim_pipe_path;
  string additional_nvim_args;
} config_params_t;

static string get_env_as_string(string varname) {
  char *value;
  size_t len;
  errno_t err = _dupenv_s(&value, &len, varname.c_str());
  if (err || value == NULL) {
    return string();
  }
  return string(value);
}

#if NDEBUG
#define LOG_DEBUG(MSG, ...)
#else
#define LOG_DEBUG(MSG, ...)                           \
  do {                                                \
    printf(MSG "\n", __VA_ARGS__); /* Sleep(1000); */ \
  } while (false)
#endif


static string get_config_path() {
  return get_env_as_string("USERPROFILE") + "\\" + relative_config_path;
}

static config_params_t parse_config_file(string config_path) {
    LOG_DEBUG("Attempting to read config file [%s]", config_path.c_str());
    config_params_t config_params = {
        default_alacritty_path,
        default_nvim_path,
    };
    ifstream file(config_path);
    if (file.is_open()) {
        std::getline(file, config_params.alacritty_path);
        std::getline(file, config_params.nvim_path);
        std::getline(file, config_params.nvim_pipe_path);
        std::getline(file, config_params.additional_nvim_args);
        file.close();
        LOG_DEBUG("    Read config:");
        LOG_DEBUG("        alacritty_path:            [%s]", config_params.alacritty_path.c_str());
        LOG_DEBUG("        nvim_path:                 [%s]", config_params.nvim_path.c_str());
        LOG_DEBUG("        nvim_pipe_path:            [%s]", config_params.nvim_pipe_path.c_str());
        LOG_DEBUG("        additional_nvim_args:      [%s]", config_params.additional_nvim_args.c_str());
    }
    else {
        LOG_DEBUG("    Unable to open config file.");
    }
    return config_params;
}

static string get_pid_path() {
  return get_env_as_string("TEMP") + "\\" + relative_pid_path;
}

static DWORD read_pid_from_file(string pid_path) {
    LOG_DEBUG("Attempting to read pid file [%s]", pid_path.c_str());
    ifstream file(pid_path);
    if (file.is_open()) {
        DWORD pid;
        file >> pid;
        file.close();
        LOG_DEBUG("    Read pid: [%lu]", pid);
        return pid;
    }
    else {
        LOG_DEBUG("    Unable to open pid file.");
        return 0;
    }
}

static void write_pid_to_file(string pid_path, DWORD pid) {
    LOG_DEBUG("Attempting to write pid to file [%s]", pid_path.c_str());
    ofstream file(pid_path);
    if (file.is_open()) {
        file << pid;
        file.close();
        LOG_DEBUG("    Written pid: [%lu]", pid);
    }
    else {
        LOG_DEBUG("    Unable to open pid file.");
    }
}

// narrow and widen taken from:
// https://stackoverflow.com/questions/51313560/how-to-widen-a-standard-string-while-maintaining-the-characters

/// Convert a windows UTF-16 string to a UTF-8 string
///
/// @param s[in] the UTF-16 string
/// @return std::string UTF-8 string
inline std::string narrow(std::wstring_view wstr) {
    if (wstr.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr.size(), nullptr, 0,
        nullptr, nullptr);
    std::string out(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr.size(), &out[0], len,
        nullptr, nullptr);
    return out;
}

/// Convert a UTF-8 string to a windows UTF-16 string
///
/// @param s[in] the UTF-8 string
/// @param n[in] the UTF-8 string's length, or -1 if string is null-terminated
/// @return std::wstring UTF-16 string
static inline std::wstring widen(std::string_view str) {
    if (str.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, &str[0], str.size(), NULL, 0);
    std::wstring out(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], str.size(), &out[0], len);
    return out;
}

//static HANDLE get_process_handle(DWORD wanted_pid) {
//    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, wanted_pid);
//    if (hProcess)
//    {
//        TCHAR path[MAX_PATH];
//        if (GetModuleFileNameEx(hProcess, NULL, path, sizeof(path)))
//        {
//            MessageBox(0, path, "The path", MB_ICONINFORMATION);
//        }
//        CloseHandle(hProcess);
//    }
//}

static string get_executable_of_pid(DWORD wanted_pid) {
    LOG_DEBUG("Querying executable for pid %lu", wanted_pid);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, wanted_pid);
    if (hProcess)
    {
        WCHAR path[MAX_PATH];
        DWORD len = sizeof(path) / sizeof(path[0]);
        if (::QueryFullProcessImageNameW(hProcess, NULL, path, &len)) {
            string path_str = narrow(path);
            LOG_DEBUG("    Found executable [%s]", path_str.c_str());
            return path_str;
        }
        CloseHandle(hProcess);
    }
    LOG_DEBUG("    No executable found.");
    return string();
}

HWND get_handle_for_pid(DWORD wanted_pid) {
  HWND h = GetTopWindow(0);
  while (h) {
    DWORD pid;
    DWORD dwTheardId = GetWindowThreadProcessId(h, &pid);
    if (pid == wanted_pid) {
      // here h is the handle to the window
      break;
    }
    h = GetNextWindow(h, GW_HWNDNEXT);
  }
  return h;
}

void simulate_mouseclick() {
  // https://social.msdn.microsoft.com/Forums/en-US/08e97a47-509c-41a1-99d3-7f3f5ea3d78d/how-do-i-set-the-focus-to-a-window-i-just-brought-to-the-top-using-setforegroundwindow?forum=vcgeneral
  // Simulate Mouse clock to make sure the windows has the focus
  POINT pt, cur;

  // Save current mouse position
  GetCursorPos(&cur);

  // For some odd reason GetWindowRect(hWndPopup, &r1) returns a zero filled
  // rect ! So, well just click the screen dead center.
  pt.x = GetSystemMetrics(SM_CXSCREEN) / 2;
  pt.y = GetSystemMetrics(SM_CYSCREEN) / 2;

  INPUT inp[3];
  inp[0].type = INPUT_MOUSE;
  inp[0].mi.dx = (pt.x * 65536) / GetSystemMetrics(SM_CXSCREEN);
  inp[0].mi.dy = (pt.y * 65536) / GetSystemMetrics(SM_CYSCREEN);
  inp[0].mi.mouseData = 0;
  inp[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
  inp[0].mi.time = 0;
  inp[0].mi.dwExtraInfo = 0;

  inp[1] = inp[0];
  inp[1].mi.dx = 0;
  inp[1].mi.dy = 0;
  inp[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

  inp[2] = inp[1];
  inp[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

  UINT nEvents = SendInput(3, &inp[0], sizeof(INPUT));

  // Restore mouse position
  SetCursorPos(cur.x, cur.y);
}

void focus_existing_window(HWND h) {
  if (::GetForegroundWindow() == h)
    return;

  HWND foregroundWindowHandle = GetForegroundWindow();
  DWORD currentThreadId = GetCurrentThreadId();
  DWORD foregroundThreadId =
      GetWindowThreadProcessId(foregroundWindowHandle, NULL);
  AttachThreadInput(currentThreadId, foregroundThreadId, true);
  SetForegroundWindow(h);
  simulate_mouseclick();
  AttachThreadInput(currentThreadId, foregroundThreadId, false);
}

void focus_existing_window(DWORD wanted_pid) {
  HWND h = get_handle_for_pid(wanted_pid);
  if (h != NULL) {
    LOG_DEBUG("Found handle 0x%p\n", h);
    focus_existing_window(h);
  }
}

DWORD launch_server_instance(const config_params_t* config_params) {
    LOG_DEBUG("Launching server instance.");
  STARTUPINFOW si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  string args = "\"" + config_params->alacritty_path + "\" -e \"\\\"" + config_params->nvim_path 
      + "\\\" --listen \\\"" + config_params->nvim_pipe_path + "\\\"\"";

  if (config_params->additional_nvim_args != "") {
      args += " " + config_params->additional_nvim_args;
  }

  LOG_DEBUG("    Commandline: [%s]", args.c_str());

  // Note: Even though this is supposed to not use the paren's (= this processes) console, 
  // there is some kind of link. If this processe's console is closed (even though the process 
  // is done), this also terminates the child. It is not clear to me why that is. Maybe it has 
  // to do sonething with input/output stream handles. However also experiments with setting 
  // those to NULL in the STARTUPINFO did not change this behaviour. So now, we help ourselves 
  // with a bit of a hack: after this application is done (at the end of main), in case a 
  // server instance was started we hide it's console window.
  CreateProcessW(NULL, widen(args).data(), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi);
  DWORD pid = pi.dwProcessId;
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  // Wait for the pipe to be created and ready. If we trigger the launch of a client before that 
  // point, there is no way to suncronize those two and weird things may happen.
  WIN32_FIND_DATA fd;
  HANDLE hFind = FindFirstFileA(config_params->nvim_pipe_path.c_str(), &fd);
  while (hFind == INVALID_HANDLE_VALUE) {
      LOG_DEBUG("    Pipe [%s] does not exist (yet?), retrying...", 
          config_params->nvim_pipe_path.c_str());
      Sleep(20);
      hFind = FindFirstFileA(config_params->nvim_pipe_path.c_str(), &fd);
  }

  BOOL pipe_available = WaitNamedPipe(config_params->nvim_pipe_path.c_str(), NMPWAIT_WAIT_FOREVER);
  if (!pipe_available) {
      LOG_DEBUG("    Error waiting for pipe at [%s]: Error %d", 
          config_params->nvim_pipe_path.c_str(), GetLastError());
  }
  return pid;
}

void launch_client_instance(const config_params_t* config_params, string file_path) {
    LOG_DEBUG("Launching client instance.");
  STARTUPINFOW si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  string args = "\"" + config_params->nvim_path + "\" --server \"" + config_params->nvim_pipe_path + "\" --remote \"" + file_path + "";
  LOG_DEBUG("    Commandline: [%s]", args.c_str());
  CreateProcessW(NULL, widen(args).data(), NULL, NULL, FALSE, 0, NULL, NULL,
                &si, &pi);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
}

void print_usage() {
    printf("Open file in a single nvim instance.                              \n");
    printf("                                                                  \n");
    printf("Usage:                                                            \n");
    printf("    nvim-singleton.exe PATH                                       \n");
    printf("    PATH: path to the file to open in the single nvim instance    \n");
    printf("                                                                  \n");
    printf("Configuration:                                                    \n");
    printf("    A configuration file is read from                             \n");
    printf("        %s.\n", get_config_path().c_str());
    printf("                                                                  \n");
    printf("    It needs to contain configuration parameters, one per line in \n");
    printf("    pre-defined order. The entries are as lollows:                \n");
    printf("        line0: path to alacritty                                  \n");
    printf("        line1: path to nvim                                       \n");
    printf("        line2: path to the named pipe to use for communication to \n");
    printf("            nvim server (see nvim --listen / nvim --server).      \n");
    printf("        line3: additional arguments to be passed to nvim.         \n");
    printf("                                                                  \n");
    printf("    In order to be able to reliably detect running instance of    \n");
    printf("    alacritty, the full path to the executable needs to be        \n");
    printf("    specified.\n");
    printf("                                                                  \n");
    printf("    Example config file:                                          \n");
    printf("        C:\\Program Files\\Alacritty\\alacritty.exe               \n");
    printf("        c:\\Program Files\\Neovim\\bin\\nvim.exe                  \n");
    printf("        \\\\.\\pipe\\nvim-singleton                               \n");
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        print_usage();
        exit(1);
    }
    
    string first_arg = string(argv[1]);
    if (first_arg == "-h" || first_arg == "--help") {
        print_usage();
        exit(1);
    }

  string config_path = get_config_path();
  config_params_t config_params = parse_config_file(config_path);
  string pid_path = get_pid_path();
  DWORD pid = read_pid_from_file(pid_path);
  bool server_is_running = false;
  if (pid != 0) {
      string executable_path = get_executable_of_pid(pid);
      std::transform(executable_path.begin(), executable_path.end(), executable_path.begin(),
          [](unsigned char c) { return std::tolower(c); });
      string alacritty_path_lc = config_params.alacritty_path;
      std::transform(alacritty_path_lc.begin(), alacritty_path_lc.end(), alacritty_path_lc.begin(),
          [](unsigned char c) { return std::tolower(c); });
      if (executable_path == alacritty_path_lc) {
          server_is_running = true;
          focus_existing_window(pid);
      }
  } 

  if (! server_is_running) {
      // Launch a new alacritty + nvim instance and record the pid.
      pid = launch_server_instance(&config_params);
      write_pid_to_file(pid_path, pid);
  }

  launch_client_instance(&config_params, string(argv[1]));

#if NDEBUG
  ShowWindow(::GetConsoleWindow(), SW_HIDE);
#endif
}
