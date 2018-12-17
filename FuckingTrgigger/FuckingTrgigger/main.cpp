#include <iostream>
#include "Memory.h"

using namespace std;

#define dwLocalPlayer				0x00AAAAB4
#define dwEntityList				0x04A8574C

#define m_iTeamNum					0xF0
#define m_iHealth 					0xFC
#define m_iCrossHairID				0xB2A4

memory mem;

PModule bClient;

int main()
{
	while (!mem.Attach("csgo.exe", PROCESS_ALL_ACCESS)) {}
	bClient = mem.GetModule("client.dll");
	cout << "Trigger started!" << endl;
	while (1)
	{
		DWORD pLocal = mem.Read<DWORD>(bClient.dwBase + dwLocalPlayer);
		DWORD lHealth = mem.Read<DWORD>(pLocal + m_iHealth);

		if (!lHealth)
			continue;

		DWORD lTeamNum = mem.Read<DWORD>(pLocal + m_iTeamNum);

		//player id in crosshair
		DWORD lCrosshairId = mem.Read<DWORD>(pLocal + m_iCrossHairID);
		if (!lCrosshairId || lCrosshairId > 64)
			continue;

		//Base adress of player in crosshair
		DWORD entityBase = mem.Read<int>(bClient.dwBase + dwEntityList /*offset to entity list*/ + (lCrosshairId - 1) * 0x10);
		if (!entityBase)
			continue;

		DWORD entityHp = mem.Read<DWORD>(entityBase + m_iHealth);
		DWORD entityTeam = mem.Read<DWORD>(entityBase + m_iTeamNum);

		if (!entityHp || entityTeam == lTeamNum)
			continue;

		/*SHOOT*/
		Sleep(5);
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		Sleep(15);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}

	return 0;
}