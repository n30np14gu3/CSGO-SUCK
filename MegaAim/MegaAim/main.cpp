#include  "Memory.h"
#include  "MathTools.h"

#define dwLocalPlayer				0xC5B85C
#define dwEntityList				0x4C380DC
#define dwClientState 				0x586A74

#define m_iTeamNum					0xF0
#define m_iDormant					0xE9
#define m_vecView					0x104
#define m_iHealth 					0xFC

#define m_BoneMatrix				0x2698
#define m_vecOrigin					0x134
#define m_vecViewOffset				0x104
#define dwClientState_ViewAngles	0x4D10

memory mem;
PModule bClient;
PModule bEngine;

using namespace std;

/*Get Position of bone in the world*/
Vector getBonePos(int pTargetIn, int bone)
{
	const int matrix = mem.Read<int>(pTargetIn + m_BoneMatrix);
	return Vector(
		mem.Read<float>(matrix + 0x30 * bone + 0xC),
		mem.Read<float>(matrix + 0x30 * bone + 0x1C),
		mem.Read<float>(matrix + 0x30 * bone + 0x2C)
	);
}

/*Set new ViewAngle*/
void setViewAngle(DWORD EngineBase, Vector angle)
{

	ClampAngles(angle);
	NormalizeAngles(angle);

	mem.Write(EngineBase + dwClientState_ViewAngles, angle);
}

/*Calc angle to target bone*/
Vector AngelToTarget(int pLocal, int pTargetIn, int boneIndex)
{
	const Vector position = mem.Read<Vector>(pLocal + m_vecOrigin);
	const Vector vecView = mem.Read<Vector>(pLocal + m_vecViewOffset);

	const Vector myView = position + vecView;
	const Vector aimView = getBonePos(pTargetIn, boneIndex);

	Vector dst = CalcAngle(myView, aimView).ToVector();

	NormalizeAngles(dst);

	return dst;
}

/*get the best target*/
int getTarget(int pLocal, int clientState)
{
	int currentTarget = -1;
	float misDist = 360.f;

	Vector _viewAngels = mem.Read<Vector>(clientState + dwClientState_ViewAngles);

	const int playerTeam = mem.Read<int>(pLocal + m_iTeamNum);

	for (int i = 0; i <= 64; i++)
	{
		const int target = mem.Read<int>(bClient.dwBase + dwEntityList + (i - 1) * 0x10);
		if (!target || target < 0)
			continue;

		const int targetHealth = mem.Read<int>(target + m_iHealth);
		if (!targetHealth || targetHealth < 0)
			continue;

		const int targetTeam = mem.Read<int>(target + m_iTeamNum);

		if (!targetTeam || targetTeam == playerTeam)
			continue;

		const bool targetDormant = mem.Read<bool>(target + m_iDormant);
		if (targetDormant)
			continue;

		Vector angleToTarget = AngelToTarget(pLocal, target, 8/*head bone index head_0*/);
		float fov = GetFov(_viewAngels.ToQAngle(), angleToTarget.ToQAngle());
		if (fov < misDist && fov <= 20)/*sort by fov*/
		{
			misDist = fov;
			currentTarget = target;
		}
	}
	return currentTarget;
}

/*main func*/
int main()
{
	while(!mem.Attach("csgo.exe", PROCESS_ALL_ACCESS)){}
	bClient = mem.GetModule("client_panorama.dll");
	bEngine = mem.GetModule("engine.dll");

	cout << "My AIM IS ON!" << endl;
	while (1)
	{
		if (!GetAsyncKeyState(VK_SHIFT))
			continue;

		int pLocal = mem.Read<int>(bClient.dwBase + dwLocalPlayer);
		if (!pLocal)
			continue;

		if (!mem.Read<int>(pLocal + m_iHealth))
			continue;

		int pEngine = mem.Read<int>(bEngine.dwBase + dwClientState);
		if (!pEngine)
			continue;

		int target = getTarget(pLocal, pEngine);/*Get The best target addr*/
		if (target == -1)
			continue;

		setViewAngle(pEngine, AngelToTarget(pLocal, target, 8));/*Set new angle*/
	}
}