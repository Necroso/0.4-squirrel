#include "CCore.h"
#include "Functions.h"
#include "ConsoleUtils.h"
#include "CStream.h"
#include <cassert>
#include <cwchar>
#include <vector>
#include <memory>

// Static members
unsigned short CCore::refCount = 0;
CCore* CCore::pCoreInstance = nullptr;
extern CCore* pCore;

// Global VM variable required by Sqrat design
HSQUIRRELVM v = nullptr;

CCore::CCore()
{
	script = nullptr;
	pLogFile = nullptr;

	this->playerMap.fill(nullptr);
	this->pickupMap.fill(nullptr);
	this->objectMap.fill(nullptr);
	this->vehicleMap.fill(nullptr);
	this->checkpointMap.fill(nullptr);

	canReload = false;
	pLogFile = fopen("server_log.txt", "a");
}

CCore::~CCore()
{
	DropAllTimers();
	ProcessTimers(0.0f);

	if (script != nullptr)
	{
		delete script;
		script = nullptr;
	}

	// Release Squirrel Virtual Machine Engine context
	if (v != nullptr)
	{
		sq_close(v);
		v = nullptr;
	}

	// Clean tracking maps to avoid memory leaks on server shutdown/reload
	for (unsigned int i = 0; i < MAX_PLAYERS; ++i) {
		if (this->playerMap[i] != nullptr) { delete this->playerMap[i]; this->playerMap[i] = nullptr; }
	}
	for (unsigned int i = 0; i < MAX_OBJECTS; ++i) {
		if (this->objectMap[i] != nullptr) { delete this->objectMap[i]; this->objectMap[i] = nullptr; }
	}
	for (unsigned int i = 0; i < MAX_PICKUPS; ++i) {
		if (this->pickupMap[i] != nullptr) { delete this->pickupMap[i]; this->pickupMap[i] = nullptr; }
	}
	for (unsigned int i = 0; i < MAX_VEHICLES; ++i) {
		if (this->vehicleMap[i] != nullptr) { delete this->vehicleMap[i]; this->vehicleMap[i] = nullptr; }
	}
	for (unsigned int i = 0; i < MAX_CHECKPOINTS; ++i) {
		if (this->checkpointMap[i] != nullptr) { delete this->checkpointMap[i]; this->checkpointMap[i] = nullptr; }
	}

	if (pLogFile != nullptr)
	{
		fclose(pLogFile);
		pLogFile = nullptr;
	}
}

void CCore::LoadVM()
{
	v = sq_open(2048);
	sq_setprintfunc(v, printfunc, errorfunc);

	DefaultVM::Set(v);
	Sqrat::ErrorHandling::Enable(true);
	sq_pushroottable(v);

	this->RegisterEntities();

	// Signal external plugins to hook registration
	// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA functions->SendPluginCommand(0x7D6E22D8, "");
}

void CCore::ScanForEntities()
{
	for (unsigned int i = 0; i < MAX_VEHICLES; ++i) {
		if (functions->CheckEntityExists(vcmpEntityPoolVehicle, i)) this->AllocateVehicle(i, false);
	}
	for (unsigned int i = 0; i < MAX_PICKUPS; ++i) {
		if (functions->CheckEntityExists(vcmpEntityPoolPickup, i)) this->AllocatePickup(i, false);
	}
	for (unsigned int i = 0; i < MAX_OBJECTS; ++i) {
		if (functions->CheckEntityExists(vcmpEntityPoolObject, i)) this->AllocateObject(i, false);
	}
	for (unsigned int i = 0; i < MAX_PLAYERS; ++i) {
		if (functions->IsPlayerConnected(i)) this->AllocatePlayer(i);
	}
	for (unsigned int i = 0; i < MAX_CHECKPOINTS; ++i) {
		if (functions->CheckEntityExists(vcmpEntityPoolCheckPoint, i)) this->AllocateCheckpoint(i, false);
	}
}

void CCore::AddTimer(CTimer* pTimer, Sqrat::Object timerHandle)
{
	this->timers.push_back(TimerEntry{ pTimer, std::move(timerHandle) });

	char buf[256];
	snprintf(buf, sizeof(buf), "[Timer] Created: '%s'. Active: %zu", pTimer->functionName.c_str(), this->timers.size());
	OutputMessage(buf);
}

void CCore::ProcessTimers(float elapsedTime)
{
	float deltaMs = elapsedTime * 1000.0f;

	for (size_t i = 0; i < this->timers.size(); )
	{
		CTimer* pTimer = this->timers[i].ptr;

		bool shouldDie = pTimer->isPaused
			? pTimer->committingSeppuku
			: pTimer->Pulse(deltaMs);

		if (shouldDie)
		{
			size_t lastIndex = this->timers.size() - 1;
			if (i != lastIndex)
			{
				this->timers[i] = std::move(this->timers[lastIndex]);
			}
			this->timers.pop_back();
			continue;
		}
		++i;
	}
}

void CCore::DropAllTimers()
{
	for (auto& entry : this->timers)
		entry.ptr->committingSeppuku = true;

	char buf[128];
	snprintf(buf, sizeof(buf), "[Timer] Marked %zu timer(s) for shutdown.", this->timers.size());
	OutputMessage(buf);
}

void CCore::CleanWorld()
{
	for (unsigned int i = 0; i < MAX_VEHICLES; ++i) {
		if (vehicleMap[i] != nullptr && vehicleMap[i]->isOurs) { vehicleMap[i]->Delete(); vehicleMap[i] = nullptr; }
	}
	for (unsigned int i = 0; i < MAX_PICKUPS; ++i) {
		if (pickupMap[i] != nullptr && pickupMap[i]->isOurs) { pickupMap[i]->Delete(); pickupMap[i] = nullptr; }
	}
	for (unsigned int i = 0; i < MAX_OBJECTS; ++i) {
		if (objectMap[i] != nullptr && objectMap[i]->isOurs) { objectMap[i]->Delete(); objectMap[i] = nullptr; }
	}
	for (unsigned int i = 0; i < MAX_CHECKPOINTS; ++i) {
		if (checkpointMap[i] != nullptr && checkpointMap[i]->isOurs) { checkpointMap[i]->Delete(); checkpointMap[i] = nullptr; }
	}
}

void CCore::RegisterEntities()
{
	if (SQ_FAILED(sqstd_register_bloblib(v)))   OutputWarning("sqstd_bloblib failed to load.");
	if (SQ_FAILED(sqstd_register_iolib(v)))     OutputWarning("sqstd_iolib failed to load.");
	if (SQ_FAILED(sqstd_register_mathlib(v)))   OutputWarning("sqstd_mathlib failed to load.");
	if (SQ_FAILED(sqstd_register_stringlib(v))) OutputWarning("sqstd_stringlib failed to load.");
	if (SQ_FAILED(sqstd_register_systemlib(v))) OutputWarning("sqstd_systemlib failed to load.");

	RegisterStructures();
	RegisterGlobals();
	RegisterConstants();

	RegisterObject();
	RegisterPickup();
	RegisterPlayer();
	RegisterTimer();
	RegisterVehicle();
	RegisterCheckpoint();

	CStream::RegisterStream();
	sqstd_seterrorhandlers(v);
}

void CCore::LoadScript()
{
	this->ScanForEntities();
	this->canReload = false;

	FILE* file = fopen("server.cfg", "r");
	if (file == nullptr)
	{
		OutputError("SqVCMP could not read server.cfg");
		this->canReload = true;
		return;
	}

	bool scriptFound = false;
	char lineBuffer[256];

	// Streamlined line-by-line configuration scanning
	while (fgets(lineBuffer, sizeof(lineBuffer), file) != nullptr)
	{
		// Strip newlines or carriage returns quickly
		size_t len = strlen(lineBuffer);
		while (len > 0 && (lineBuffer[len - 1] == '\n' || lineBuffer[len - 1] == '\r')) {
			lineBuffer[--len] = '\0';
		}

		if (this->ParseConfigLine(lineBuffer))
		{
			scriptFound = true;
			break;
		}
	}

	fclose(file);
	this->canReload = true;

	if (!scriptFound) {
		OutputError("No Squirrel gamemode was specified.");
	}
}

bool CCore::ParseConfigLine(char* lineBuffer)
{
	char* gamemodeSearch = strstr(lineBuffer, "sqgamemode ");
	if (gamemodeSearch == nullptr)
		return false;

	gamemodeSearch += 11;

	while (*gamemodeSearch == ' ' || *gamemodeSearch == '\t')
		gamemodeSearch++;

	size_t len = strlen(gamemodeSearch);
	while (len > 0 && (unsigned char)gamemodeSearch[len - 1] <= ' ') {
		gamemodeSearch[--len] = '\0';
	}

	if (len == 0)
		return false;

	char debugBuf[512];
	snprintf(debugBuf, sizeof(debugBuf), "   Loading script: '%s'", gamemodeSearch);
	OutputMessage(debugBuf);

	FILE* testFile = fopen(gamemodeSearch, "rb");
	if (testFile)
	{
		OutputMessage("   File detected and ready for reading.");
		fclose(testFile);
	}
	else
	{
		snprintf(debugBuf, sizeof(debugBuf), "Failed to open '%s': The file does not exist or is locked.", gamemodeSearch);
		OutputError(debugBuf);
	}

	this->script = new Script();

	try
	{
		this->script->CompileFile(gamemodeSearch);
		this->script->Run();
	}
	catch (Sqrat::Exception& e)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "Could not load script '%s'", gamemodeSearch);

		OutputWarning(buf);
		OutputWarning(e.Message().c_str());

		delete this->script;
		this->script = nullptr;
		return false;
	}

	try
	{
		Function callback = RootTable(v).GetFunction(_SC("onScriptLoad"));
		if (!callback.IsNull())
			callback();

		callback.Release();
	}
	catch (Sqrat::Exception&)
	{
		OutputWarning("onScriptLoad failed to execute -- check the console for more details.");
	}

	return true;
}

void CCore::printf(char* pszFormat, ...)
{
	char szInitBuffer[512];
	va_list va;

	va_start(va, pszFormat);
	int nChars = vsnprintf(szInitBuffer, sizeof(szInitBuffer), pszFormat, va);
	if (nChars >= static_cast<int>(sizeof(szInitBuffer)))
	{
		std::vector<char> szBuffer(nChars + 1);
		vsnprintf(szBuffer.data(), szBuffer.size(), pszFormat, va);
		this->rawprint(szBuffer.data());
	}
	else
	{
		this->rawprint(szInitBuffer);
	}
	va_end(va);
}

void CCore::rawprint(const char* pszOutput)
{
	fputs(pszOutput, stdout);
	if (this->pLogFile != nullptr)
	{
		fputs(pszOutput, this->pLogFile);
		fflush(this->pLogFile); // Guarantee real-time logs integrity
	}
}

void CCore::Release()
{
	if (refCount > 0) {
		refCount--;
	}

	if (refCount == 0 && pCoreInstance != nullptr)
	{
		delete pCoreInstance;
		pCoreInstance = nullptr;
		pCore = nullptr;
	}
}

FILE* CCore::GetLogInstance()
{
	return this->pLogFile;
}

CPlayer* CCore::AllocatePlayer(int gPlayerId)
{
	if (gPlayerId < 0 || gPlayerId >= MAX_PLAYERS) return nullptr;
	if (functions->IsPlayerConnected(gPlayerId) == 0) return nullptr;
	if (this->playerMap[gPlayerId] != nullptr) return this->playerMap[gPlayerId];

	CPlayer* pPlayer = new CPlayer();
	pPlayer->nPlayerId = gPlayerId;
	this->playerMap[gPlayerId] = pPlayer;
	return pPlayer;
}

CObject* CCore::AllocateObject(int gObjectId, bool isOurs)
{
	if (gObjectId < 0 || gObjectId >= MAX_OBJECTS) return nullptr;
	if (!functions->CheckEntityExists(vcmpEntityPoolObject, gObjectId)) return nullptr;
	if (this->objectMap[gObjectId] != nullptr) return this->objectMap[gObjectId];

	CObject* pObject = new CObject();
	pObject->Init(gObjectId, isOurs);
	this->objectMap[gObjectId] = pObject;
	return pObject;
}

CPickup* CCore::AllocatePickup(int gPickupId, bool isOurs)
{
	if (gPickupId < 0 || gPickupId >= MAX_PICKUPS) return nullptr;
	if (!functions->CheckEntityExists(vcmpEntityPoolPickup, gPickupId)) return nullptr;
	if (this->pickupMap[gPickupId] != nullptr) return this->pickupMap[gPickupId];

	CPickup* pPickup = new CPickup();
	pPickup->Init(gPickupId, isOurs);
	this->pickupMap[gPickupId] = pPickup;
	return pPickup;
}

CVehicle* CCore::AllocateVehicle(int gVehicleId, bool isOurs)
{
	if (gVehicleId <= 0 || gVehicleId > MAX_VEHICLES) return nullptr;
	if (!functions->CheckEntityExists(vcmpEntityPoolVehicle, gVehicleId)) return nullptr;
	if (this->vehicleMap[gVehicleId] != nullptr) return this->vehicleMap[gVehicleId];

	CVehicle* pVehicle = new CVehicle();
	pVehicle->Init(gVehicleId, isOurs);
	this->vehicleMap[gVehicleId] = pVehicle;
	return pVehicle;
}

CCheckpoint* CCore::AllocateCheckpoint(int gCheckpointId, bool isOurs)
{
	if (gCheckpointId < 0 || gCheckpointId >= MAX_CHECKPOINTS) return nullptr;
	if (!functions->CheckEntityExists(vcmpEntityPoolCheckPoint, gCheckpointId)) return nullptr;
	if (this->checkpointMap[gCheckpointId] != nullptr) return this->checkpointMap[gCheckpointId];

	CCheckpoint* pCheckpoint = new CCheckpoint();
	pCheckpoint->Init(gCheckpointId, isOurs);
	this->checkpointMap[gCheckpointId] = pCheckpoint;
	return pCheckpoint;
}

void CCore::DereferenceObject(int gObjectId)
{
	if (gObjectId >= 0 && gObjectId < MAX_OBJECTS && this->objectMap[gObjectId] != nullptr)
	{
		delete this->objectMap[gObjectId];
		this->objectMap[gObjectId] = nullptr;
	}
}

void CCore::DereferencePickup(int gPickupId)
{
	if (gPickupId >= 0 && gPickupId < MAX_PICKUPS && this->pickupMap[gPickupId] != nullptr)
	{
		delete this->pickupMap[gPickupId];
		this->pickupMap[gPickupId] = nullptr;
	}
}

void CCore::DereferencePlayer(int gPlayerId)
{
	if (gPlayerId >= 0 && gPlayerId < MAX_PLAYERS && this->playerMap[gPlayerId] != nullptr)
	{
		delete this->playerMap[gPlayerId];
		this->playerMap[gPlayerId] = nullptr;
	}
}

void CCore::DereferenceVehicle(int gVehicleId)
{
	if (gVehicleId > 0 && gVehicleId <= MAX_VEHICLES && this->vehicleMap[gVehicleId] != nullptr)
	{
		delete this->vehicleMap[gVehicleId];
		this->vehicleMap[gVehicleId] = nullptr;
	}
}

void CCore::DereferenceCheckpoint(int gCheckpointId)
{
	if (gCheckpointId >= 0 && gCheckpointId < MAX_CHECKPOINTS && this->checkpointMap[gCheckpointId] != nullptr)
	{
		delete this->checkpointMap[gCheckpointId];
		this->checkpointMap[gCheckpointId] = nullptr;
	}
}

CObject* CCore::RetrieveObject(int gObjectId)
{
	return (gObjectId >= 0 && gObjectId < MAX_OBJECTS) ? this->objectMap[gObjectId] : nullptr;
}

CPickup* CCore::RetrievePickup(int gPickupId)
{
	return (gPickupId >= 0 && gPickupId < MAX_PICKUPS) ? this->pickupMap[gPickupId] : nullptr;
}

CPlayer* CCore::RetrievePlayer(int gPlayerId)
{
	return (gPlayerId >= 0 && gPlayerId < MAX_PLAYERS) ? this->playerMap[gPlayerId] : nullptr;
}

CVehicle* CCore::RetrieveVehicle(int gVehicleId)
{
	return (gVehicleId > 0 && gVehicleId <= MAX_VEHICLES) ? this->vehicleMap[gVehicleId] : nullptr;
}

CCheckpoint* CCore::RetrieveCheckpoint(int gCheckpointId)
{
	return (gCheckpointId >= 0 && gCheckpointId < MAX_CHECKPOINTS) ? this->checkpointMap[gCheckpointId] : nullptr;
}

void CCore::CancelTimersForPlayer(int playerId)
{
	for (auto& timerEntry : this->timers)
	{
		CTimer* pTimer = timerEntry.ptr;

		for (auto& param : pTimer->params)
		{
			if (param.GetType() == OT_INSTANCE)
			{
				sq_pushobject(v, param.GetObject());
				sq_pushstring(v, _SC("ID"), -1);
				if (SQ_SUCCEEDED(sq_get(v, -2)))
				{
					SQInteger id = 0;
					sq_getinteger(v, -1, &id);
					sq_pop(v, 2);

					if (id == playerId)
					{
						pTimer->committingSeppuku = true;
						break;
					}
				}
				else {
					sq_pop(v, 1);
				}
			}
		}
	}
}