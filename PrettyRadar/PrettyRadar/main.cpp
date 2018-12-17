#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "Memory.h"

using namespace std;

#define dwLocalPlayer	0xAAAAB4
#define dwEntityList	0x4A8574C

#define m_bSpotted		0x939

#define m_iHealth		0xFC
#define m_iTeamNum		0xF0


memory mem;
PModule bClient;

int main()
{
	while (!mem.Attach("csgo.exe", PROCESS_ALL_ACCESS)) {}
	bClient = mem.GetModule("client.dll");

	cout << "RadarHack started..." << endl;
	while (1)
	{
		DWORD pLocal = mem.Read<DWORD>(bClient.dwBase + dwLocalPlayer);
		DWORD localHp = mem.Read<DWORD>(pLocal + m_iHealth);
		if (!localHp)
			continue;

		DWORD localTeam = mem.Read<DWORD>(pLocal + m_iTeamNum);
		for (DWORD i = 0; i < 64; i++)
		{
			DWORD pEnt = mem.Read<DWORD>(bClient.dwBase + dwEntityList /*offset to EntityList*/ + (i - 1) * 0x10 /*size of player Data*/);
			DWORD entHp = mem.Read<DWORD>(pEnt + m_iHealth);
			if (!entHp)
				continue;

			DWORD entTeam = mem.Read<DWORD>(pEnt + m_iTeamNum);
			if (entTeam == localTeam)
				continue;

			BOOL entSpotted = mem.Read<BOOL>(pEnt + m_bSpotted);
			if (entSpotted)
				continue;

			mem.Write(pEnt + m_bSpotted, TRUE);
		}
	}
	return 0;
}