#include <iostream>
#include "Memory.h"

using namespace std;

memory mem;
PModule bClient;

int main()
{
	while (!mem.Attach("csgo.exe", PROCESS_ALL_ACCESS)) {}
	bClient = mem.GetModule("client.dll");

	cout << "dwBase: " << bClient.dwBase << endl;
	cout << "dwSize: " << bClient.dwSize << endl;
	cout << "some value" << mem.Read<DWORD>(bClient.dwBase + 0x1488) << endl;
	mem.Exit();
	system("pause");

	return 0;
}