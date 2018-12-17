#include <iostream>
#include "Memory.h"

using namespace std;

PModule bClient;
memory mem;
#define oLocalPlayer 0x00AA9AB4
#define oHealth		0xFC
#define oTeamNum	0xF0

int main()
{

	while (!mem.Attach("csgo.exe", PROCESS_ALL_ACCESS)) {}
	bClient = mem.GetModule("client.dll");
	while (1)
	{
		system("cls");

		DWORD pLocal = mem.Read<DWORD>(bClient.dwBase + oLocalPlayer);
		DWORD pLocalTeam = mem.Read<DWORD>(pLocal + oTeamNum);
		DWORD pLocalHealth = mem.Read<DWORD>(pLocal + oHealth);

		printf("LocalBase: 0x%X\n", pLocal);
		printf("Team: %d\n", pLocalTeam);
		printf("HP: %d\n", pLocalHealth);
		
		system("pause>nul");
	}
	return 0;
}