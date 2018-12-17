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
	inline bool Attach(const char* pName, DWORD dwAccess)
	{
		HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);

		do
			if (!strcmp(_bstr_t(entry.szExeFile), pName)) 
			{
				_pId = entry.th32ProcessID;
				CloseHandle(handle);
				_process = OpenProcess(dwAccess, false, _pId);
				return true;
			}
		while (Process32Next(handle, &entry));
		return false;
	}

	inline PModule GetModule(const char* pModule)
	{
		HANDLE module = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, _pId);
		MODULEENTRY32 entry;
		entry.dwSize = sizeof(entry);

		do
			if (!strcmp(_bstr_t(entry.szModule), pModule))
			{
				CloseHandle(module);
				return PModule{ reinterpret_cast<DWORD>(entry.hModule), entry.modBaseSize };
			}
		while (Module32Next(module, &entry));

		return PModule{ 0, 0 };
	}

	template <class T>
	T Read(const DWORD dwAddress)
	{
		T _read;
		ReadProcessMemory(_process, LPVOID(dwAddress), &_read, sizeof(T), NULL);
		return _read;
	}

	template <class T>
	void Write(const DWORD dwAdress, const T value)
	{
		WriteProcessMemory(_process, LPVOID(dwAdress), &value, sizeof(T), NULL);
	}

	void Exit()
	{
		CloseHandle(_process);
	}
private:
	HANDLE _process;
	DWORD _pId;
};