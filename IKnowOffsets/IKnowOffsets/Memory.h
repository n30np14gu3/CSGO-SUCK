#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <comdef.h>

struct PModule
{
	DWORD dwBase;
	DWORD dwSize;
};

class memory
{
public:
	bool Attach(const char* ProcessName, DWORD rights)
	{
		HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);

		do
			if (!strcmp(_bstr_t(entry.szExeFile), ProcessName)) {
				pID = entry.th32ProcessID;
				CloseHandle(handle);
				_process = OpenProcess(rights, false, pID);
				return true;
			}
		while (Process32Next(handle, &entry));
		return false;
	}

	void CloseProcess(char* ProcessName)
	{

		HANDLE hProcess;
		DWORD pid;
		pid = GetPIDbyName(ProcessName);
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		TerminateProcess(hProcess, -1);
		CloseHandle(hProcess);
	}

	PModule GetModule(const char* moduleName)
	{
		HANDLE module = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);
		MODULEENTRY32 mEntry;
		mEntry.dwSize = sizeof(mEntry);

		do {
			if (!strcmp(_bstr_t(mEntry.szModule), moduleName)) {
				CloseHandle(module);

				PModule mod = { reinterpret_cast<DWORD>(mEntry.hModule), mEntry.modBaseSize };
				return mod;
			}
		} while (Module32Next(module, &mEntry));

		PModule mod = { static_cast<DWORD>(false), static_cast<DWORD>(false) };
		return mod;
	}

	template <class T>
	T Read(const DWORD addr) {
		T _read;
		ReadProcessMemory(_process, reinterpret_cast<LPVOID>(addr), &_read, sizeof(T), NULL);
		return _read;
	}

	template <class T>
	void Write(const DWORD addr, T val) {
		WriteProcessMemory(_process, reinterpret_cast<LPVOID>(addr), &val, sizeof(T), NULL);
	}

	DWORD FindPattern(const DWORD start, const DWORD size, const char* sig, const char* mask) {
		BYTE* data = new BYTE[size];

		unsigned long bytesRead;
		if (!ReadProcessMemory(_process, reinterpret_cast<LPVOID>(start), data, size, &bytesRead)) {
			return NULL;
		}

		for (DWORD i = 0; i < size; i++) {
			if (DataCompare(static_cast<const BYTE*>(data + i), reinterpret_cast<const BYTE*>(sig), mask)) {
				return start + i;
			}
		}
		return NULL;
	}
	DWORD FindPatternArray(const DWORD start, const DWORD size, const char* mask, int count, ...) {
		char* sig = new char[count + 1];
		va_list ap;
		va_start(ap, count);
		for (int i = 0; i < count; i++) {
			char read = va_arg(ap, char);
			sig[i] = read;
		}
		va_end(ap);
		sig[count] = '\0';
		return FindPattern(start, size, sig, mask);
	}

	void Exit()
	{
		CloseHandle(_process);
	}
private:
	bool DataCompare(const BYTE* pData, const BYTE* pMask, const char* pszMask) {
		for (; *pszMask; ++pszMask, ++pData, ++pMask) {
			if (*pszMask == 'x' && *pData != *pMask) {
				return false;
			}
		}
		return (*pszMask == NULL);
	}
	DWORD GetPIDbyName(LPTSTR szProcessName) {
		HANDLE hSnapshot;
		PROCESSENTRY32 pe = { sizeof(pe) };
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE)
			return 0;
		if (!Process32First(hSnapshot, &pe))
			return 0;
		do {
			if (!lstrcmpi(pe.szExeFile, szProcessName))
				return pe.th32ProcessID;
		} while (Process32Next(hSnapshot, &pe));
		return 0;
	}
	HANDLE _process;
	DWORD pID;
};