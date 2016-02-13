#pragma once


typedef int AC_STATUS;

#define AC_OFF 0
#define AC_REPLAY 1
#define AC_LIVE 2
#define AC_PAUSE 3

typedef int AC_SESSION_TYPE;

#define AC_UNKNOWN -1
#define AC_PRACTICE 0
#define AC_QUALIFY 1
#define AC_RACE 2
#define AC_HOTLAP 3
#define AC_TIME_ATTACK 4
#define AC_DRIFT 5
#define AC_DRAG 6


#pragma pack(push)
#pragma pack(4)

typedef struct SPageFilePhysics
{
	int packetId;
	float gas;
	float brake;
	float fuel;
	int gear;
	int rpms;
	float steerAngle;
	float speedKmh;
	float velocity[3];
	float accG[3];
	float wheelSlip[4];
	float wheelLoad[4];
	float wheelsPressure[4];
	float wheelAngularSpeed[4];
	float tyreWear[4];
	float tyreDirtyLevel[4];
	float tyreCoreTemperature[4];
	float camberRAD[4];
	float suspensionTravel[4];
	float drs;
	float tc;
	float heading;
	float pitch;
	float roll;
	float cgHeight;
	float carDamage[5];
	int numberOfTyresOut;
	int pitLimiterOn;
	float abs;
} SPageFilePhysics;


typedef struct SPageFileGraphic
{
	int packetId;
	AC_STATUS status;
	AC_SESSION_TYPE session;
	wchar_t currentTime[15];
	wchar_t lastTime[15];
	wchar_t bestTime[15];
	wchar_t split[15];
	int completedLaps;
	int position;
	int iCurrentTime;
	int iLastTime;
	int iBestTime;
	float sessionTimeLeft;
	float distanceTraveled;
	int isInPit;
	int currentSectorIndex;
	int lastSectorTime;
	int numberOfLaps;
	wchar_t tyreCompound[33];

	float replayTimeMultiplier;
	float normalizedCarPosition;
	float carCoordinates[3];
} SPageFileGraphic;


typedef struct SPageFileStatic
{
	wchar_t smVersion[15];
	wchar_t acVersion[15];
	// session static info
	int numberOfSessions;
	int numCars;
	wchar_t carModel[33];
	wchar_t track[33];
	wchar_t playerName[33];
	wchar_t playerSurname[33];
	wchar_t playerNick[33];
	int sectorCount;

	// car static info
	float maxTorque;
	float maxPower;
	int	maxRpm;
	float maxFuel;
	float suspensionMaxTravel[4];
	float tyreRadius[4];
} SPageFileStatic;


#pragma pack(pop)
