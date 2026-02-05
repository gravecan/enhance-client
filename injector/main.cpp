#ifndef WINVER
#define WINVER 0x0601
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <conio.h>
#include "bytes.hpp"


HANDLE g_hConsole = NULL;

void SetConsoleColor(WORD color) {
    if (!g_hConsole) {
        g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    SetConsoleTextAttribute(g_hConsole, color);
}

void WriteLine(const std::string& text) {
    if (!g_hConsole) {
        g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    DWORD written = 0;
    std::string line = text + "\r\n";
    WriteConsoleA(g_hConsole, line.c_str(), (DWORD)line.length(), &written, NULL);
}

void PrintASCII() {
    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("");
    WriteLine("  ______       _                          ");
    WriteLine(" |  ____|     | |                         ");
    WriteLine(" | |__   _ __ | |__   __ _ _ __   ___ ___ ");
    WriteLine(" |  __| | '_ \\| '_ \\ / _` | '_ \\ / __/ _ \\");
    WriteLine(" | |____| | | | | | | (_| | | | | (_|  __/");
    WriteLine(" |______|_| |_|_| |_|\\__,_|_| |_|\\___\\___|");
    WriteLine("                                          ");
    WriteLine("                                          ");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}


void PrintTips() {
    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    WriteLine("");
    WriteLine("tips & hints:");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    std::vector<std::string> tips = {
        "• make sure minecraft is running before injecting",
        "• keep this window open until injection completes",
        "• if injection fails, restart minecraft and try again",
        "• use the latest version of minecraft for best results"
    };
    
    for (const auto& tip : tips) {
        WriteLine(tip);
        Sleep(50);
    }
    
    WriteLine("");
}

bool WriteDLLToFile(const std::string& outputPath) {
    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("preparing dll data...");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(300);

    // Write embedded bytes to file
    std::ofstream file(outputPath, std::ios::binary);
    if (!file.is_open()) {
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("failed to create output file");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return false;
    }

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("writing dll to disk...");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    file.write(reinterpret_cast<const char*>(dll_bytes), dll_size);
    file.close();

    // Verify file exists and has content
    std::ifstream verify(outputPath, std::ios::binary | std::ios::ate);
    if (!verify.is_open() || verify.tellg() == 0) {
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("written file is empty or invalid");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return false;
    }
    verify.close();
    return true;
}

DWORD GetProcessIDByName(const char* processName) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32First(hSnap, &pe32)) {
        do {
            if (strcmp(pe32.szExeFile, processName) == 0) {
                CloseHandle(hSnap);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnap, &pe32));
    }

    CloseHandle(hSnap);
    return 0;
}

int main() {
    // Show console window
    AllocConsole();
    FILE* pCout;
    FILE* pCin;
    FILE* pCerr;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    freopen_s(&pCin, "CONIN$", "r", stdin);
    freopen_s(&pCerr, "CONOUT$", "w", stderr);
    
    // Initialize console handle
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Set console title
    SetConsoleTitleA("enhance injector");
    
    // Set console output mode
    DWORD dwMode = 0;
    if (GetConsoleMode(g_hConsole, &dwMode)) {
        dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
        SetConsoleMode(g_hConsole, dwMode);
    }
    
    // Set console buffer size for proper display
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(g_hConsole, &csbi)) {
        COORD newSize;
        newSize.X = 120;
        newSize.Y = 3000;
        SetConsoleScreenBufferSize(g_hConsole, newSize);
    }
    
    // Clear screen
    COORD topLeft = { 0, 0 };
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(g_hConsole, &info);
    FillConsoleOutputCharacterA(g_hConsole, ' ', info.dwSize.X * info.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(g_hConsole, topLeft);
    
    // Print ASCII art
    PrintASCII();
    
    // Print tips
    PrintTips();
    
    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("initializing...");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    // Get Windows temp folder
    char tempPath[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath) == 0) {
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("failed to get temp path");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(3000);
        return 1;
    }
    
    std::string dllPath = std::string(tempPath) + "bg1750fhq2.dll";
    
    if (!WriteDLLToFile(dllPath)) {
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("failed to write dll. please try again.");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(5000);
        return 1;
    }

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("loading..");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    char m_dll[260];
    DWORD pathLen = GetFullPathNameA(dllPath.c_str(), 260, m_dll, 0);
    if (pathLen == 0) {
        DeleteFileA(dllPath.c_str());
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("failed to resolve dll path");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(3000);
        return 1;
    }

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("searching for minecraft process..");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(500);

    DWORD m_pid = GetProcessIDByName("javaw.exe");
    if (!m_pid || m_pid == 0) {
        DeleteFileA(dllPath.c_str());
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("minecraft not found. please open minecraft before injecting.");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(5000);
        return 1;
    }

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    std::string pidMsg = "minecraft process found (pid: " + std::to_string(m_pid) + ")";
    WriteLine(pidMsg);
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    void* m_loadlibrary = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!m_loadlibrary) {
        DeleteFileA(dllPath.c_str());
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("failed to get loadlibrary address");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(3000);
        return 1;
    }

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("opening process..");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(300);

    HANDLE m_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_pid);
    if (m_handle == nullptr) {
        DeleteFileA(dllPath.c_str());
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("failed to open process. try running as administrator.");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(5000);
        return 1;
    }

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("allocating memory..");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(300);

    void* m_write = VirtualAllocEx(m_handle, nullptr, strlen(m_dll) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (m_write == nullptr) {
        CloseHandle(m_handle);
        DeleteFileA(dllPath.c_str());
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("failed to allocate memory");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(3000);
        return 1;
    }

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("writing to memory..");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(300);

    SIZE_T bytesWritten = 0;
    if (!WriteProcessMemory(m_handle, m_write, m_dll, strlen(m_dll) + 1, &bytesWritten)) {
        CloseHandle(m_handle);
        DeleteFileA(dllPath.c_str());
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("failed to write to memory");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(3000);
        return 1;
    }

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("injecting...");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(500);

    HANDLE hThread = CreateRemoteThread(m_handle, nullptr, 0, (LPTHREAD_START_ROUTINE)m_loadlibrary, m_write, 0, nullptr);
    if (hThread == nullptr) {
        CloseHandle(m_handle);
        DeleteFileA(dllPath.c_str());
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
        WriteLine("failed to create remote thread");
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(3000);
        return 1;
    }
    
    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);
    CloseHandle(m_handle);
    
    SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    WriteLine("");
    WriteLine("injection successful!");
    WriteLine("enhance is now loaded in minecraft.");
    WriteLine("");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    WriteLine("cleaning up temporary files..");
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    // Try to delete the temporary DLL file
    int attempts = 0;
    const int maxAttempts = 3;
    
    while (attempts < maxAttempts) {
        Sleep(1000); 
        attempts++;

        if (DeleteFileA(dllPath.c_str())) {
            break;
        }
        
        DWORD error = GetLastError();
        if (error != ERROR_SHARING_VIOLATION && error != ERROR_ACCESS_DENIED) {
            break;
        }
    }

    return 0;
}