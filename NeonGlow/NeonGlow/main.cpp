#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <iostream>

#include "Memory.h"

using namespace std;

#define dwLocalPlayer				0xAAAAB4
#define dwEntityList				0x4A8574C
#define dwGlowObject				0x4FB2540

#define m_iTeamNum					0xF0
#define m_iHealth 					0xFC
#define m_iGlowIndex                0xA310

typedef struct GlowObjectDefinition_t {
	float r;
	float g;
	float b;
	float a;
	uint8_t unk1[16];
	bool m_bRenderWhenOccluded;
	bool m_bRenderWhenUnoccluded;
	bool m_bFullBloom;
	int GlowStyle;
	uint8_t unk2[10];
} GlowObjectDefinition_t;

memory mem;
PModule bClient;

int main()
{
	while (!mem.Attach("csgo.exe", PROCESS_ALL_ACCESS)) { }
	bClient = mem.GetModule("client.dll");

	cout << "Glow started..." << endl;

	while (1)
	{
		DWORD playerBase = mem.Read<DWORD>(bClient.dwBase + dwLocalPlayer);
		if (!playerBase)
			continue;

		int playerHp = mem.Read<int>(playerBase + m_iHealth);
		if (!playerHp)
			continue;

		int playerTeam = mem.Read<int>(playerBase + m_iTeamNum);

		int glowArray = mem.Read<int>(bClient.dwBase + dwGlowObject);
		int glowCount = mem.Read<int>(bClient.dwBase + dwGlowObject + 0x4);

		for (int i = 0; i < glowCount; i++)
		{
			DWORD gEntity = mem.Read<DWORD>(glowArray + 0x38 * i);
			if (!gEntity) 
				continue;

			int entHp = mem.Read<int>(gEntity + m_iHealth);
			if (!entHp)
				continue;

			int enTeam = mem.Read<int>(gEntity + m_iTeamNum);

			if (enTeam == playerTeam)
				continue;

			static GlowObjectDefinition_t Glow = mem.Read<GlowObjectDefinition_t>(glowArray + (i * 0x38) + 0x4);
			if (enTeam == 3)
			{
				Glow.r = 0.447058827f;
				Glow.g = 0.607843161f;
				Glow.b = 0.8666667f;
				Glow.a = 0.5f;
				Glow.m_bRenderWhenOccluded = true;
				Glow.m_bRenderWhenUnoccluded = false;
			}

			if (enTeam == 2)
			{
				Glow.r = 0.8784314f;
				Glow.g = 0.6862745f;
				Glow.b = 0.3372549f;
				Glow.a = 0.5f;
				Glow.m_bRenderWhenOccluded = true;
				Glow.m_bRenderWhenUnoccluded = false;
			}
			mem.Write<GlowObjectDefinition_t>(glowArray + (i * 0x38) + 0x4, Glow);
		}
	}

	return 0;
}