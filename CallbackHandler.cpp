#include "CallbackHandler.h"
#include "Exports.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#ifdef _MSC_VER
#define strdup _strdup
#endif

// Console stuff
#include "ConsoleUtils.h"
// Script and VM core
#include "CCore.h"
#include "SQModule.h"
#include "CStream.h"

#include "VCMP.h"

#define ANSI_RESET      "\x1b[0m"
#define ANSI_RED        "\x1b[1;31m"
#define ANSI_BOLD       "\x1b[1m"
#define ANSI_YELLOW     "\x1b[1;33m"
#define ANSI_CYAN       "\x1b[1;36m"
#define ANSI_MAGENTA    "\x1b[1;35m"

static bool gServerInitPending = true;
extern HSQUIRRELVM v;
extern HSQUIRRELVM v;

// Create arrays for several structures.
savedVehicleData lastVehInfo[MAX_VEHICLES];
savedPlayerData  lastPlrInfo[MAX_PLAYERS];
savedServerData  lastSrvInfo;

// Externalize the core instance and other critical variables
extern CCore* pCore;
extern PluginInfo* information;
extern PluginCallbacks* callbacks;
extern HSQAPI sq;

// Helper to safely convert const char* to the format expected by Squirrel (SQChar)
#ifdef SQUNICODE
std::wstring ToSQString(const char* str) {
	if (!str) return _SC("");
	size_t len = strlen(str) + 1;
	std::vector<wchar_t> buf(len);
	mbstowcs(buf.data(), str, len);
	return std::wstring(buf.data());
}
#else
std::string ToSQString(const char* str) {
	return str ? str : "";
}
#endif

// Utility helper to execute global callbacks in a clean and protected manner
template<typename... Args>
void ExecuteCallback(const SQChar* callbackName, Args... args) {
	Function callback = RootTable(v).GetFunction(callbackName);
	if (!callback.IsNull()) {
		try {
			callback.Execute(args...);
		}
		catch (const Sqrat::Exception&)
		{
			char errorMsg[256];
			snprintf(errorMsg, sizeof(errorMsg), "Callback '%s' failed to execute -- check console.", callbackName);
			OutputWarning(errorMsg);
		}
	}
	callback.Release();
}

uint8_t OnInitServer()
{
	printf("\n");
	OutputMessage("--------------------------------------------------------------------");
	OutputMessage("* SqVCMP v1.1 (" ANSI_BOLD ANSI_YELLOW "Release 0.4.7" ANSI_RESET ")");
	OutputMessage("* Updated by: " ANSI_RED "[LBR]Necroso" ANSI_RESET " | Original Dev: " ANSI_MAGENTA "Stormeus" ANSI_RESET);
	OutputMessage("* Features/Changelog: " ANSI_CYAN "https://forum.vc-mp.org/index.php?topic=9438.0" ANSI_RESET);
	OutputMessage("* Source: " ANSI_CYAN "https://github.com/Necroso/0.4-squirrel" ANSI_RESET);

	pCore->LoadVM();

	if (functions && functions->SendPluginCommand)
	{
		functions->SendPluginCommand(0x7D6E22D8, "");
	}

	gServerInitPending = true;
	return 1;
}

void OnFrame(float fElapsedTime)
{
	if (gServerInitPending)
	{
		gServerInitPending = false;

		OutputMessage("--------------------------------------------------------------------");
		OutputMessage("   |----- SqVCMP: Gamemode initialization started");

		pCore->LoadScript();

		pCore->ChangeReloadPermission(false);

		ExecuteCallback(_SC("onServerStart"));

		pCore->ChangeReloadPermission(true);

		
	}

	pCore->ProcessTimers(fElapsedTime);

	int lastHour = lastSrvInfo.lastHour;
	int lastMinute = lastSrvInfo.lastMinute;
	int hour = functions->GetHour();
	int minute = functions->GetMinute();

	if (lastHour != hour || lastMinute != minute)
	{
		Function callback = RootTable(v).GetFunction(_SC("onTimeChange"));
		if (!callback.IsNull())
		{
			callback(lastHour, lastMinute, hour, minute);
		}
		callback.Release();

		lastSrvInfo.lastHour = hour;
		lastSrvInfo.lastMinute = minute;
	}
}

void OnShutdownServer()
{
	if (pCore != nullptr)
	{
		ExecuteCallback(_SC("onServerStop"));
		ExecuteCallback(_SC("onScriptUnload"));
		pCore->Release();
	}
}

void OnPlayerConnect(int nPlayerId)
{
	if (pCore == nullptr || nPlayerId < 0 || nPlayerId >= MAX_PLAYERS)
	{
		OutputWarning("OnPlayerConnect aborted: pCore is null or nPlayerId out of bounds.");
		return;
	}

	try
	{
		lastPlrInfo[nPlayerId].lastX = 0.0f;
		lastPlrInfo[nPlayerId].lastY = 0.0f;
		lastPlrInfo[nPlayerId].lastZ = 0.0f;
		lastPlrInfo[nPlayerId].lastHP = 100.0f;
		lastPlrInfo[nPlayerId].lastArmour = 0.0f;
		lastPlrInfo[nPlayerId].lastWep = 0;

		CPlayer* newPlayer = pCore->AllocatePlayer(nPlayerId);

		if (newPlayer == nullptr)
		{
			OutputWarning("Critical failure allocating CPlayer in OnPlayerConnect.");
			return;
		}

		ExecuteCallback(_SC("onPlayerJoin"), newPlayer);
	}
	catch (const std::exception& e)
	{
		char errorLog[512];
		snprintf(errorLog, sizeof(errorLog), "Crash prevented in OnPlayerConnect: %s", e.what());
		OutputWarning(errorLog);
	}
	catch (...)
	{
		OutputWarning("Unknown crash prevented in OnPlayerConnect.");
	}
}

void OnPlayerDisconnect(int nPlayerId, vcmpDisconnectReason nReason)
{
	if (pCore == nullptr || nPlayerId < 0 || nPlayerId >= MAX_PLAYERS)
	{
		OutputWarning("OnPlayerDisconnect aborted: pCore is null or nPlayerId out of bounds.");
		return;
	}

	// 1. ANTES de executar o callback ou dereferenciar, cancelamos os timers associados a este ID.
	// Isso impede que o Timer execute no próximo ciclo e gere logs inválidos.
	pCore->CancelTimersForPlayer(nPlayerId);

	try
	{
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);

		if (playerInstance != nullptr)
		{
			ExecuteCallback(_SC("onPlayerPart"), playerInstance, static_cast<int>(nReason));
		}
		else
		{
			char warnLog[128];
			snprintf(warnLog, sizeof(warnLog), "Warning: Attempted to disconnect Player ID %d, but instance was null.", nPlayerId);
			OutputWarning(warnLog);
		}
	}
	catch (const std::exception& e)
	{
		char errorLog[512];
		snprintf(errorLog, sizeof(errorLog), "Exception caught in OnPlayerDisconnect: %s", e.what());
		OutputWarning(errorLog);
	}
	catch (...)
	{
		OutputWarning("Unknown crash prevented during disconnection callback.");
	}

	try
	{
		pCore->DereferencePlayer(nPlayerId);
		memset(&lastPlrInfo[nPlayerId], 0, sizeof(savedPlayerData));
	}
	catch (...)
	{
		OutputWarning("Critical failure inside DereferencePlayer.");
	}
}

uint8_t OnPlayerRequestClass(int nPlayerId, int nOffset)
{
	if (pCore != nullptr)
	{
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		if (playerInstance == nullptr) return 1;

		Function callback = RootTable(v).GetFunction(_SC("onPlayerRequestClass"));
		int returnValue = 1;

		if (!callback.IsNull()) {
			try {
				returnValue = callback.Evaluate<int, CPlayer*, int, int, int>(
					playerInstance, nOffset, playerInstance->GetTeam(), playerInstance->GetSkin());
			}
			catch (const Sqrat::Exception&) {
				OutputWarning("onPlayerRequestClass failed to execute.");
			}
		}
		callback.Release();
		return returnValue;
	}
	return 1;
}

uint8_t OnPlayerRequestSpawn(int nPlayerId)
{
	if (pCore != nullptr)
	{
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		if (playerInstance == nullptr) return 1;

		Function callback = RootTable(v).GetFunction(_SC("onPlayerRequestSpawn"));
		int returnValue = 1;

		if (!callback.IsNull()) {
			try {
				returnValue = callback.Evaluate<int, CPlayer*>(playerInstance);
			}
			catch (const Sqrat::Exception&) {
				OutputWarning("onPlayerRequestSpawn failed to execute.");
			}
		}
		callback.Release();
		return returnValue;
	}
	return 1;
}

void OnPlayerSpawn(int nPlayerId)
{
	if (pCore != nullptr) {
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		if (playerInstance != nullptr) {
			ExecuteCallback(_SC("onPlayerSpawn"), playerInstance);
		}
	}
}

void OnPlayerDeath(int nPlayerId, int nKillerId, int nReason, vcmpBodyPart nBodyPart)
{
	if (pCore == nullptr) return;

	CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
	if (playerInstance == nullptr) return;

	if (!functions->IsPlayerConnected(nKillerId))
	{
		if (nReason == 43 || nReason == 50)      nReason = 43; // drowned
		else if (nReason == 39 && nBodyPart == 7) nReason = 39; // car crash
		else if (nReason == 39 || nReason == 40 || nReason == 44) nReason = 44; // fell

		ExecuteCallback(_SC("onPlayerDeath"), playerInstance, nReason);
	}
	else
	{
		CPlayer* killerInstance = pCore->RetrievePlayer(nKillerId);
		if (killerInstance == nullptr) return;

		const SQChar* callbackName = (functions->GetPlayerTeam(nPlayerId) == functions->GetPlayerTeam(nKillerId))
			? _SC("onPlayerTeamKill")
			: _SC("onPlayerKill");

		ExecuteCallback(callbackName, killerInstance, playerInstance, nReason, static_cast<int>(nBodyPart));
	}
}

uint8_t OnPlayerRequestEnter(int nPlayerId, int nVehicleId, int nSlotId)
{
	if (pCore != nullptr)
	{
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		CVehicle* vehicleInstance = pCore->RetrieveVehicle(nVehicleId);

		if (playerInstance == nullptr || vehicleInstance == nullptr) return 1;

		Function callback = RootTable(v).GetFunction(_SC("onPlayerEnteringVehicle"));
		int returnValue = 1;
		if (!callback.IsNull()) {
			try {
				returnValue = callback.Evaluate<int, CPlayer*, CVehicle*, int>(playerInstance, vehicleInstance, nSlotId);
			}
			catch (const Sqrat::Exception&) {
				OutputWarning("onPlayerEnteringVehicle failed to execute.");
			}
		}
		callback.Release();
		return returnValue;
	}
	return 1;
}

void OnPlayerEnterVehicle(int nPlayerId, int nVehicleId, int nSlotId)
{
	if (pCore != nullptr) {
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		CVehicle* vehicleInstance = pCore->RetrieveVehicle(nVehicleId);

		if (playerInstance != nullptr && vehicleInstance != nullptr) {
			ExecuteCallback(_SC("onPlayerEnterVehicle"), playerInstance, vehicleInstance, nSlotId);
		}
	}
}

void OnPlayerExitVehicle(int nPlayerId, int nVehicleId)
{
	if (pCore != nullptr) {
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		CVehicle* vehicleInstance = pCore->RetrieveVehicle(nVehicleId);

		if (playerInstance != nullptr && vehicleInstance != nullptr) {
			ExecuteCallback(_SC("onPlayerExitVehicle"), playerInstance, vehicleInstance);
		}
	}
}

uint8_t OnPickupClaimPicked(int nPickupId, int nPlayerId)
{
	if (pCore != nullptr)
	{
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		CPickup* pickupInstance = pCore->RetrievePickup(nPickupId);
		if (playerInstance == nullptr || pickupInstance == nullptr) return 1;

		Function callback = RootTable(v).GetFunction(_SC("onPickupClaimPicked"));
		int returnValue = 1;
		if (!callback.IsNull()) {
			try {
				returnValue = callback.Evaluate<int, CPlayer*, CPickup*>(playerInstance, pickupInstance);
			}
			catch (const Sqrat::Exception&) {
				OutputWarning("onPickupClaimPicked failed to execute.");
			}
		}
		callback.Release();
		return returnValue;
	}
	return 1;
}

void OnPickupPickedUp(int nPickupId, int nPlayerId)
{
	if (pCore != nullptr) {
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		CPickup* pickupInstance = pCore->RetrievePickup(nPickupId);

		if (playerInstance != nullptr && pickupInstance != nullptr) {
			ExecuteCallback(_SC("onPickupPickedUp"), playerInstance, pickupInstance);
		}
	}
}

void OnPickupRespawn(int nPickupId)
{
	if (pCore != nullptr) {
		CPickup* pickupInstance = pCore->RetrievePickup(nPickupId);
		if (pickupInstance != nullptr) {
			ExecuteCallback(_SC("onPickupRespawn"), pickupInstance);
		}
	}
}

void OnVehicleExplode(int nVehicleId)
{
	if (pCore != nullptr) {
		CVehicle* vehicleInstance = pCore->RetrieveVehicle(nVehicleId);
		if (vehicleInstance != nullptr) {
			ExecuteCallback(_SC("onVehicleExplode"), vehicleInstance);
		}
	}
}

void OnVehicleRespawn(int nVehicleId)
{
	if (pCore != nullptr) {
		CVehicle* vehicleInstance = pCore->RetrieveVehicle(nVehicleId);
		if (vehicleInstance != nullptr) {
			ExecuteCallback(_SC("onVehicleRespawn"), vehicleInstance);
		}
	}
}

uint8_t OnPublicMessage(int nPlayerId, const char* pszText)
{
	if (pCore == nullptr || pszText == nullptr || nPlayerId < 0 || nPlayerId >= MAX_PLAYERS)
	{
		OutputWarning("OnPublicMessage aborted: Invalid parameters or pCore is null.");
		return 1;
	}

	CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
	if (playerInstance == nullptr)
	{
		return 1;
	}

	Function callback = RootTable(v).GetFunction(_SC("onPlayerChat"));
	int returnValue = 1;

	if (!callback.IsNull())
	{
		try
		{
			// Safe conversion to normalize types across Sqrat's template compiler
			auto textStr = ToSQString(pszText);
			const SQChar* pText = textStr.c_str();

			returnValue = callback.Evaluate<int>(playerInstance, pText);
		}
		catch (const Sqrat::Exception& e)
		{
			char errorLog[512];
#ifdef SQUNICODE
			char msgBuffer[256];
			wcstombs(msgBuffer, e.Message().c_str(), sizeof(msgBuffer));
			snprintf(errorLog, sizeof(errorLog), "Sqrat exception caught in OnPublicMessage: %s", msgBuffer);
#else
			snprintf(errorLog, sizeof(errorLog), "Sqrat exception caught in OnPublicMessage: %s", e.Message().c_str());
#endif
			OutputWarning(errorLog);
		}
		catch (const std::exception& e)
		{
			char errorLog[512];
			snprintf(errorLog, sizeof(errorLog), "Standard exception caught in OnPublicMessage: %s", e.what());
			OutputWarning(errorLog);
		}
		catch (...)
		{
			OutputWarning("Unknown crash prevented in OnPublicMessage.");
		}
	}

	callback.Release();
	return static_cast<uint8_t>(returnValue);
}

uint8_t OnCommandMessage(int nPlayerId, const char* pszText)
{
	if (pCore != nullptr && pszText != nullptr)
	{
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		if (playerInstance == nullptr) return 1;

		SQInteger top = sq->gettop(v);
		sq->pushroottable(v);
		sq->pushstring(v, _SC("onPlayerCommand"), -1);

		if (SQ_SUCCEEDED(sq->get(v, -2)))
		{
			char* szTextCopy = strdup(pszText);
			if (szTextCopy != nullptr)
			{
				char* szSpacePos = strchr(szTextCopy, ' ');
				if (szSpacePos != nullptr) {
					szSpacePos[0] = '\0';
				}

				char* szArguments = (szSpacePos != nullptr) ? &szSpacePos[1] : nullptr;

				sq->pushroottable(v);
				Var<CPlayer*>::push(v, playerInstance);

				// Safe use of the String handler, dependent on the VM encoding
				auto cmdStr = ToSQString(szTextCopy);
				sq->pushstring(v, cmdStr.c_str(), -1);

				if (szArguments == nullptr || strlen(szArguments) == 0) {
					sq->pushnull(v);
				}
				else {
					auto argStr = ToSQString(szArguments);
					sq->pushstring(v, argStr.c_str(), -1);
				}

				sq->call(v, 4, 0, 1);
				free(szTextCopy);
			}
		}
		sq->settop(v, top);
	}
	return 1;
}

uint8_t OnPrivateMessage(int nPlayerId, int nTargetId, const char* pszText)
{
	if (pCore == nullptr || pszText == nullptr || nPlayerId < 0 || nPlayerId >= MAX_PLAYERS || nTargetId < 0 || nTargetId >= MAX_PLAYERS)
	{
		OutputWarning("OnPrivateMessage aborted: Invalid parameters or pCore is null.");
		return 1;
	}

	CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
	CPlayer* targetInstance = pCore->RetrievePlayer(nTargetId);
	if (playerInstance == nullptr || targetInstance == nullptr)
	{
		return 1;
	}

	Function callback = RootTable(v).GetFunction(_SC("onPlayerPM"));
	int returnValue = 1;

	if (!callback.IsNull())
	{
		try
		{
			// Safely normalize string data types for Sqrat
			auto textStr = ToSQString(pszText);
			const SQChar* pText = textStr.c_str();

			returnValue = callback.Evaluate<int>(playerInstance, targetInstance, pText);
		}
		catch (const Sqrat::Exception& e)
		{
			char errorLog[512];
#ifdef SQUNICODE
			char msgBuffer[256];
			wcstombs(msgBuffer, e.Message().c_str(), sizeof(msgBuffer));
			snprintf(errorLog, sizeof(errorLog), "Sqrat exception caught in OnPrivateMessage: %s", msgBuffer);
#else
			snprintf(errorLog, sizeof(errorLog), "Sqrat exception caught in OnPrivateMessage: %s", e.Message().c_str());
#endif
			OutputWarning(errorLog);
		}
		catch (const std::exception& e)
		{
			char errorLog[512];
			snprintf(errorLog, sizeof(errorLog), "Standard exception caught in OnPrivateMessage: %s", e.what());
			OutputWarning(errorLog);
		}
		catch (...)
		{
			OutputWarning("Unknown crash prevented in OnPrivateMessage.");
		}
	}

	callback.Release();
	return static_cast<uint8_t>(returnValue);
}

void OnPlayerBeginTyping(int nPlayerId)
{
	if (pCore != nullptr) {
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		if (playerInstance != nullptr) {
			ExecuteCallback(_SC("onPlayerBeginTyping"), playerInstance);
		}
	}
}

void OnPlayerEndTyping(int nPlayerId)
{
	if (pCore != nullptr) {
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		if (playerInstance != nullptr) {
			ExecuteCallback(_SC("onPlayerEndTyping"), playerInstance);
		}
	}
}

uint8_t OnLoginAttempt(char* playerName, size_t size, const char* password, const char* pszIpAddress)
{
	if (pCore == nullptr || playerName == nullptr || password == nullptr || pszIpAddress == nullptr)
	{
		OutputWarning("OnLoginAttempt aborted: pCore or required parameters are null.");
		return 1;
	}

	Function callback = RootTable(v).GetFunction(_SC("onLoginAttempt"));
	int returnValue = 1;

	if (!callback.IsNull())
	{
		try
		{
			auto nameStr = ToSQString(playerName);
			auto passStr = ToSQString(password);
			auto ipStr = ToSQString(pszIpAddress);

			// Explicitly forcing Sqrat to accept these as standard Squirrel string pointers
			const SQChar* pName = nameStr.c_str();
			const SQChar* pPass = passStr.c_str();
			const SQChar* pIp = ipStr.c_str();

			returnValue = callback.Evaluate<int>(pName, pPass, pIp);
		}
		catch (const Sqrat::Exception& e)
		{
			char errorLog[512];
#ifdef SQUNICODE
			char msgBuffer[256];
			wcstombs(msgBuffer, e.Message().c_str(), sizeof(msgBuffer));
			snprintf(errorLog, sizeof(errorLog), "Sqrat exception caught in OnLoginAttempt: %s", msgBuffer);
#else
			snprintf(errorLog, sizeof(errorLog), "Sqrat exception caught in OnLoginAttempt: %s", e.Message().c_str());
#endif
			OutputWarning(errorLog);
		}
		catch (const std::exception& e)
		{
			char errorLog[512];
			snprintf(errorLog, sizeof(errorLog), "Standard exception caught in OnLoginAttempt: %s", e.what());
			OutputWarning(errorLog);
		}
		catch (...)
		{
			OutputWarning("Unknown crash prevented in OnLoginAttempt.");
		}
	}

	callback.Release();
	return static_cast<uint8_t>(returnValue);
}

void OnClientScriptData(int playerId, const uint8_t* data, size_t size) {
	if (data == nullptr || size == 0) return;

	CStream::LoadInput(data, size);

	if (pCore != nullptr) {
		CPlayer* playerInstance = pCore->RetrievePlayer(playerId);
		if (playerInstance != nullptr) {
			Function callback = RootTable(v).GetFunction(_SC("onClientScriptData"));
			if (!callback.IsNull()) {
				try {
					callback.Evaluate<CPlayer*>(playerInstance);
				}
				catch (const Sqrat::Exception&) {
					OutputWarning("onClientScriptData failed to execute.");
				}
			}
			callback.Release();
		}
	}
}

void OnNameChangeable(char* playerName, char** namePtr) {}

void OnVehicleUpdate(int nVehicleId, vcmpVehicleUpdate nUpdateType)
{
	if (pCore != nullptr && nVehicleId >= 0 && nVehicleId < MAX_VEHICLES)
	{
		savedVehicleData* vehInfo = &lastVehInfo[nVehicleId];
		Vector lastPos(vehInfo->lastX, vehInfo->lastY, vehInfo->lastZ);
		float lastHP = vehInfo->lastHP;

		float x, y, z;
		functions->GetVehiclePosition(nVehicleId, &x, &y, &z);
		float hp = functions->GetVehicleHealth(nVehicleId);

		if (lastHP != hp)
		{
			CVehicle* vehInst = pCore->RetrieveVehicle(nVehicleId);
			if (vehInst != nullptr) {
				ExecuteCallback(_SC("onVehicleHealthChange"), vehInst, lastHP, hp);
			}
			vehInfo->lastHP = hp;
		}

		if (lastPos.x != x || lastPos.y != y || lastPos.z != z)
		{
			CVehicle* vehInst = pCore->RetrieveVehicle(nVehicleId);
			if (vehInst != nullptr) {
				ExecuteCallback(_SC("onVehicleMove"), vehInst, lastPos.x, lastPos.y, lastPos.z, x, y, z);
			}
			vehInfo->lastX = x;
			vehInfo->lastY = y;
			vehInfo->lastZ = z;
		}
	}
}

void OnPlayerUpdate(int nPlayerId, vcmpPlayerUpdate nUpdateType)
{
	if (pCore != nullptr && nPlayerId >= 0 && nPlayerId < MAX_PLAYERS)
	{
		CPlayer* playerInstance = pCore->RetrievePlayer(nPlayerId);
		if (playerInstance == nullptr) return;

		savedPlayerData* plrInfo = &lastPlrInfo[nPlayerId];
		Vector lastPos(plrInfo->lastX, plrInfo->lastY, plrInfo->lastZ);
		float lastArmour = plrInfo->lastArmour;
		float lastHP = plrInfo->lastHP;
		int lastWep = plrInfo->lastWep;

		float x, y, z;
		functions->GetPlayerPosition(nPlayerId, &x, &y, &z);
		float hp = functions->GetPlayerHealth(nPlayerId);
		float armour = functions->GetPlayerArmour(nPlayerId);
		int wep = functions->GetPlayerWeapon(nPlayerId);

		if (lastPos.x != x || lastPos.y != y || lastPos.z != z)
		{
			ExecuteCallback(_SC("onPlayerMove"), playerInstance, lastPos.x, lastPos.y, lastPos.z, x, y, z);
			plrInfo->lastX = x; plrInfo->lastY = y; plrInfo->lastZ = z;
		}

		if (lastHP != hp)
		{
			ExecuteCallback(_SC("onPlayerHealthChange"), playerInstance, lastHP, hp);
			plrInfo->lastHP = hp;
		}

		if (lastArmour != armour)
		{
			ExecuteCallback(_SC("onPlayerArmourChange"), playerInstance, lastArmour, armour);
			plrInfo->lastArmour = armour;
		}

		if (lastWep != wep)
		{
			ExecuteCallback(_SC("onPlayerWeaponChange"), playerInstance, lastWep, wep);
			plrInfo->lastWep = wep;
		}
	}
}

void OnObjectShot(int nObjectId, int nPlayerId, int nWeapon)
{
	if (pCore != nullptr) {
		CObject* obj = pCore->RetrieveObject(nObjectId);
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (obj != nullptr && ply != nullptr) {
			ExecuteCallback(_SC("onObjectShot"), obj, ply, nWeapon);
		}
	}
}

void OnObjectBump(int nObjectId, int nPlayerId)
{
	if (pCore != nullptr) {
		CObject* obj = pCore->RetrieveObject(nObjectId);
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (obj != nullptr && ply != nullptr) {
			ExecuteCallback(_SC("onObjectBump"), obj, ply);
		}
	}
}

// Substitua a sua função atual por esta no CallbackHandler.cpp
uint8_t OnInternalCommand(uint32_t uCmdType, const char* pszText)
{
	if (uCmdType == 0x7D6E22D8)
	{
		extern SquirrelExports* pExp;
		extern HSQUIRRELVM v;

		if (pExp) {
			pExp->GetSquirrelVM = pfGetSquirrelVM;
		}
	}

	return 1;
}

void OnEntityPoolChange(vcmpEntityPool nEntityType, int nEntityId, uint8_t bDeleted)
{
	if (pCore == nullptr) return;

	switch (nEntityType)
	{
	case vcmpEntityPoolVehicle:
		if (!bDeleted) pCore->AllocateVehicle(nEntityId, false);
		else pCore->DereferenceVehicle(nEntityId);
		break;
	case vcmpEntityPoolObject:
		if (!bDeleted) pCore->AllocateObject(nEntityId, false);
		else pCore->DereferenceObject(nEntityId);
		break;
	case vcmpEntityPoolPickup:
		if (!bDeleted) pCore->AllocatePickup(nEntityId, false);
		else pCore->DereferencePickup(nEntityId);
		break;
	case vcmpEntityPoolCheckPoint:
		if (!bDeleted) pCore->AllocateCheckpoint(nEntityId, false);
		else pCore->DereferenceCheckpoint(nEntityId);
		break;
	default:
		break;
	}
}

void OnKeyBindDown(int nPlayerId, int nBindId)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) ExecuteCallback(_SC("onKeyDown"), ply, nBindId);
	}
}

void OnKeyBindUp(int nPlayerId, int nBindId)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) ExecuteCallback(_SC("onKeyUp"), ply, nBindId);
	}
}

void OnPlayerAwayChange(int nPlayerId, uint8_t bNewStatus)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) ExecuteCallback(_SC("onPlayerAwayChange"), ply, bNewStatus == 1);
	}
}

void OnPlayerSpectate(int nPlayerId, int nTargetId)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		CPlayer* tgt = pCore->RetrievePlayer(nTargetId);
		if (ply != nullptr && tgt != nullptr) ExecuteCallback(_SC("onPlayerSpectate"), ply, tgt);
	}
}

void OnPlayerCrashDump(int nPlayerId, const char* szCrashReport)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) {
			auto reportStr = ToSQString(szCrashReport);
			ExecuteCallback(_SC("onPlayerCrashDump"), ply, reportStr.c_str());
		}
	}
}

void OnPlayerModuleList(int nPlayerId, const char* szModuleList)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) {
			auto moduleStr = ToSQString(szModuleList);
			ExecuteCallback(_SC("onPlayerModuleList"), ply, moduleStr.c_str());
		}
	}
}

void OnPlayerNameChange(int nPlayerId, const char* oldName, const char* newName)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) {
			auto oName = ToSQString(oldName);
			auto nName = ToSQString(newName);
			ExecuteCallback(_SC("onPlayerNameChange"), ply, oName.c_str(), nName.c_str());
		}
	}
}

void OnPlayerActionChange(int nPlayerId, int nOldAction, int nNewAction)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) ExecuteCallback(_SC("onPlayerActionChange"), ply, nOldAction, nNewAction);
	}
}

void OnPlayerStateChange(int nPlayerId, vcmpPlayerState nOldState, vcmpPlayerState nNewState)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) ExecuteCallback(_SC("onPlayerStateChange"), ply, static_cast<int>(nOldState), static_cast<int>(nNewState));
	}
}

void OnPlayerOnFireChange(int nPlayerId, uint8_t bIsOnFireNow)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) ExecuteCallback(_SC("onPlayerOnFireChange"), ply, bIsOnFireNow == 1);
	}
}

void OnPlayerCrouchChange(int nPlayerId, uint8_t bIsCrouchingNow)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) {
			int isCrouching = functions->IsPlayerCrouching(nPlayerId);
			ExecuteCallback(_SC("onPlayerCrouchChange"), ply, isCrouching == 0);
		}
	}
}

void OnPlayerGameKeysChange(int nPlayerId, uint32_t nOldKeys, uint32_t nNewKeys)
{
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		if (ply != nullptr) ExecuteCallback(_SC("onPlayerGameKeysChange"), ply, static_cast<int>(nOldKeys), static_cast<int>(nNewKeys));
	}
}

void OnCheckpointEntered(int nCheckpointId, int nPlayerId) {
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		CCheckpoint* chk = pCore->RetrieveCheckpoint(nCheckpointId);
		if (ply != nullptr && chk != nullptr) ExecuteCallback(_SC("onCheckpointEntered"), ply, chk);
	}
}

void OnCheckpointExited(int nCheckpointId, int nPlayerId) {
	if (pCore != nullptr) {
		CPlayer* ply = pCore->RetrievePlayer(nPlayerId);
		CCheckpoint* chk = pCore->RetrieveCheckpoint(nCheckpointId);
		if (ply != nullptr && chk != nullptr) ExecuteCallback(_SC("onCheckpointExited"), ply, chk);
	}
}