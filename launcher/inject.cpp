// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#include "inject.h"
#include <tlhelp32.h>

bool IsModuleLoaded(DWORD pid, const wchar_t *moduleName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

    // fails with ERROR_BAD_LENGTH while ARK is in the middle of loading a dll, msdn says to just retry?
    for (int retry = 0; retry < 20 && snapshot == INVALID_HANDLE_VALUE && GetLastError() == ERROR_BAD_LENGTH; ++retry) {
        snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    }

    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    MODULEENTRY32W entry = {sizeof(entry)};

    bool found = false;

    for (BOOL ok = Module32FirstW(snapshot, &entry); ok && !found; ok = Module32NextW(snapshot, &entry)) {
        found = _wcsicmp(entry.szModule, moduleName) == 0;
    }

    CloseHandle(snapshot);

    return found;
}

bool InjectDll(DWORD pid, const std::wstring &dllPath, std::wstring &error) {
    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        error = L"cant find " + dllPath;

        return false;
    }

    HANDLE process = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, pid);

    if (!process) {
        error = L"cant open ARK (error " + std::to_wstring(GetLastError()) + L")";
        return false;
    }

    size_t bytes = (dllPath.size() + 1) * sizeof(wchar_t);
    void *remotePath = VirtualAllocEx(process, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    HANDLE thread = nullptr;

    if (remotePath && WriteProcessMemory(process, remotePath, dllPath.c_str(), bytes, nullptr)) {
        // kernel32 is at the same address in every process so our LoadLibraryW is ARKs LoadLibraryW
        auto loadLibrary = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(
            GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"));
        thread = CreateRemoteThread(process, nullptr, 0, loadLibrary, remotePath, 0, nullptr);
    }

    if (thread) {
        WaitForSingleObject(thread, 10000);
        CloseHandle(thread);
    } else {
        error = L"couldnt start the loader thread in ARK (error " + std::to_wstring(GetLastError()) + L")";
    }
    if (remotePath) {
        VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
    }

    CloseHandle(process);

    std::wstring fileName = dllPath.substr(dllPath.find_last_of(L'\\') + 1);

    if (thread && !IsModuleLoaded(pid, fileName.c_str())) {
        error = L"the mod didn't stay loaded.";
    }

    return error.empty();
}
