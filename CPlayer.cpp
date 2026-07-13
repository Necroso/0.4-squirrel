#include "CObject.h"
#include "CPlayer.h"
#include "CVehicle.h"
#include "CCore.h"
#include "main.h"
#include <cmath>
#include <cwchar>

inline bool Boolify(unsigned int n) { return (n ? true : false); }

// Core instance reference
extern CCore* pCore;

void CPlayer::SetPosition(Vector pos) { functions->SetPlayerPosition(this->nPlayerId, pos.x, pos.y, pos.z); }
void CPlayer::SetHealth(float health) { functions->SetPlayerHealth(this->nPlayerId, health); }
void CPlayer::SetArmour(float armour) { functions->SetPlayerArmour(this->nPlayerId, armour); }
void CPlayer::PlaySound(int sound) { functions->PlaySound(GetUniqueWorld(), sound, NAN, NAN, NAN); }

bool CPlayer::StreamedToPlayer(CPlayer* player)
{
	if (player != nullptr) {
		return Boolify(functions->IsPlayerStreamedForPlayer(player->nPlayerId, this->nPlayerId));
	}
	return false;
}

void CPlayer::SetAdmin(bool toSetAdmin) { functions->SetPlayerAdmin(this->nPlayerId, toSetAdmin); }
void CPlayer::SetWorld(int world) { functions->SetPlayerWorld(this->nPlayerId, world); }
void CPlayer::SetSecWorld(int world) { functions->SetPlayerSecondaryWorld(this->nPlayerId, world); }
void CPlayer::SetTeam(int team) { functions->SetPlayerTeam(this->nPlayerId, team); }
void CPlayer::SetSkin(int skin) { functions->SetPlayerSkin(this->nPlayerId, skin); }
void CPlayer::SetColour(cRGB colour) { functions->SetPlayerColour(this->nPlayerId, colour.toUInt()); }

void CPlayer::SetMoney(int money)
{
	int oldCash = functions->GetPlayerMoney(this->nPlayerId);

	Function callback = RootTable().GetFunction(_SC("onPlayerCashChange"));
	if (!callback.IsNull()) {
		callback(pCore->RetrievePlayer(this->nPlayerId), oldCash, money);
	}

	functions->SetPlayerMoney(this->nPlayerId, money);
}

void CPlayer::SetScore(int score)
{
	int oldScore = functions->GetPlayerScore(this->nPlayerId);

	Function callback = RootTable().GetFunction(_SC("onPlayerScoreChange"));
	if (!callback.IsNull()) {
		callback(pCore->RetrievePlayer(this->nPlayerId), oldScore, score);
	}

	functions->SetPlayerScore(this->nPlayerId, score);
}

void CPlayer::SetImmunity(unsigned int immunity) { functions->SetPlayerImmunityFlags(this->nPlayerId, immunity); }
void CPlayer::SetHeading(float heading) { functions->SetPlayerHeading(this->nPlayerId, heading); }
void CPlayer::SetAlpha(int alpha, int fadeTime) { functions->SetPlayerAlpha(this->nPlayerId, alpha, fadeTime); }

void CPlayer::SetVehicle(CVehicle* vehiclePointer)
{
	if (vehiclePointer != nullptr)
	{
		Function callback = RootTable().GetFunction(_SC("onPlayerEnterVehicle"));
		if (!callback.IsNull()) {
			callback(pCore->RetrievePlayer(this->nPlayerId), vehiclePointer);
		}

		functions->PutPlayerInVehicle(this->nPlayerId, vehiclePointer->nVehicleId, 0, 0, 1);
	}
}

void CPlayer::SetVehicleSlot(CVehicle* vehiclePointer, int slot)
{
	if (vehiclePointer != nullptr)
	{
		Function callback = RootTable().GetFunction(_SC("onPlayerEnterVehicle"));
		if (!callback.IsNull()) {
			callback(pCore->RetrievePlayer(this->nPlayerId), vehiclePointer);
		}

		functions->PutPlayerInVehicle(this->nPlayerId, vehiclePointer->nVehicleId, slot, 1, 0);
	}
}

void CPlayer::SetFrozen(bool toFreeze) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionControllable, !toFreeze); }
void CPlayer::SetDrivebyEnabled(bool canDb) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionDriveBy, canDb); }
void CPlayer::SetWhiteScanlines(bool hasWhiteScan) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionWhiteScanlines, hasWhiteScan); }
void CPlayer::SetGreenScanlines(bool hasGreenScan) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionGreenScanlines, hasGreenScan); }
void CPlayer::SetWidescreen(bool hasWidescreen) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionWidescreen, hasWidescreen); }
void CPlayer::SetOnRadar(bool showOnRadar) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionHasMarker, showOnRadar); }
void CPlayer::SetCanAttack(bool canAttack) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionCanAttack, canAttack); }
void CPlayer::SetWeaponSlot(int slot) { functions->SetPlayerWeaponSlot(this->nPlayerId, slot); }

EntityVector CPlayer::GetPosition()
{
	float x, y, z;
	functions->GetPlayerPosition(this->nPlayerId, &x, &y, &z);
	return EntityVector(this->nPlayerId, ENTITY_PLAYER, PLRVECTOR_POS, x, y, z);
}

int CPlayer::GetClass() { return functions->GetPlayerClass(this->nPlayerId); }
bool CPlayer::GetAdmin() { return Boolify(functions->IsPlayerAdmin(this->nPlayerId) == 1); }
bool CPlayer::Typing() { return Boolify(functions->IsPlayerTyping(this->nPlayerId)); }

SQChar* CPlayer::GetIP()
{
	// Extract ANSI IP address from VC:MP API
	functions->GetPlayerIP(this->nPlayerId, this->m_ip, 17);

	// Static Wide buffer to output Unicode to the Squirrel Engine safely
	static SQChar wIp[17];
#ifdef SQUNICODE
	mbstowcs(wIp, this->m_ip, 17);
	return wIp;
#else
	return this->m_ip;
#endif
}

bool CPlayer::GetSpawned() { return Boolify(functions->IsPlayerSpawned(this->nPlayerId) == 1); }
int CPlayer::GetWorld() { return functions->GetPlayerWorld(this->nPlayerId); }
int CPlayer::GetSecWorld() { return functions->GetPlayerSecondaryWorld(this->nPlayerId); }
int CPlayer::GetUniqueWorld() { return functions->GetPlayerUniqueWorld(this->nPlayerId); }
int CPlayer::GetState() { return functions->GetPlayerState(this->nPlayerId); }

Sqrat::string CPlayer::GetName()
{
	memset(this->m_name, '\0', sizeof(this->m_name));
	functions->GetPlayerName(this->nPlayerId, this->m_name, 64);

#ifdef SQUNICODE
	wchar_t wName[64];
	mbstowcs(wName, this->m_name, 64);
	return Sqrat::string(wName);
#else
	return Sqrat::string(this->m_name);
#endif
}

void CPlayer::SetName(Sqrat::string newName)
{
#ifdef SQUNICODE
	// Convert inbound Wide-char string back to raw char array for the plugin API
	char szName[64];
	wcstombs(szName, newName.c_str(), 64);
	functions->SetPlayerName(this->nPlayerId, szName);
#else
	functions->SetPlayerName(this->nPlayerId, newName.c_str());
#endif
}

int CPlayer::GetTeam() { return functions->GetPlayerTeam(this->nPlayerId); }
int CPlayer::GetSkin() { return functions->GetPlayerSkin(this->nPlayerId); }

EntityRGB CPlayer::GetColour()
{
	unsigned int colour = functions->GetPlayerColour(this->nPlayerId);
	unsigned char r = (colour >> 16) & 0xff;
	unsigned char g = (colour >> 8) & 0xff;
	unsigned char b = colour & 0xff;

	return EntityRGB(this->nPlayerId, ENTITY_PLAYER, 0, r, g, b);
}

int CPlayer::GetMoney() { return functions->GetPlayerMoney(this->nPlayerId); }
int CPlayer::GetScore() { return functions->GetPlayerScore(this->nPlayerId); }
int CPlayer::GetPing() { return functions->GetPlayerPing(this->nPlayerId); }
float CPlayer::GetHealth() { return functions->GetPlayerHealth(this->nPlayerId); }
float CPlayer::GetArmour() { return functions->GetPlayerArmour(this->nPlayerId); }
unsigned int CPlayer::GetImmunity() { return functions->GetPlayerImmunityFlags(this->nPlayerId); }
float CPlayer::GetHeading() { return functions->GetPlayerHeading(this->nPlayerId); }
int CPlayer::GetAlpha() { return functions->GetPlayerAlpha(this->nPlayerId); }

CVehicle* CPlayer::GetVehicle()
{
	int vehicleId = functions->GetPlayerVehicleId(this->nPlayerId);
	return pCore->RetrieveVehicle(vehicleId);
}

bool CPlayer::GetFrozen() { return !functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionControllable); }
bool CPlayer::GetDrivebyEnabled() { return Boolify(functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionDriveBy)); }
bool CPlayer::GetWhiteScanlines() { return Boolify(functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionWhiteScanlines)); }
bool CPlayer::GetGreenScanlines() { return Boolify(functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionGreenScanlines)); }
bool CPlayer::GetWidescreen() { return Boolify(functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionWidescreen)); }
bool CPlayer::GetOnRadar() { return Boolify(functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionHasMarker)); }
bool CPlayer::GetCanAttack() { return Boolify(functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionCanAttack)); }
int CPlayer::GetWeaponSlot() { return functions->GetPlayerWeaponSlot(this->nPlayerId); }
int CPlayer::GetWeapon() { return functions->GetPlayerWeapon(this->nPlayerId); }
int CPlayer::GetWeaponAmmo() { return functions->GetPlayerWeaponAmmo(this->nPlayerId); }

int CPlayer::GetWantedLevel(void) { return functions->GetPlayerWantedLevel(this->nPlayerId); }
bool CPlayer::GetPlayerOnFireStatus(void) { return Boolify(functions->IsPlayerOnFire(this->nPlayerId)); }
bool CPlayer::GetPlayerCrouchStatus(void) { return Boolify(functions->IsPlayerCrouching(this->nPlayerId)); }
int CPlayer::GetPlayerAction(void) { return functions->GetPlayerAction(this->nPlayerId); }
int CPlayer::GetPlayerGameKeys(void) { return functions->GetPlayerGameKeys(this->nPlayerId); }

Vector CPlayer::GetPlayerAimPos(void)
{
	float x, y, z;
	functions->GetPlayerAimPosition(this->nPlayerId, &x, &y, &z);
	return Vector(x, y, z);
}

Vector CPlayer::GetPlayerAimDir(void)
{
	float x, y, z;
	functions->GetPlayerAimDirection(this->nPlayerId, &x, &y, &z);
	return Vector(x, y, z);
}

void CPlayer::Kick() { functions->KickPlayer(this->nPlayerId); }
void CPlayer::Ban() { functions->BanPlayer(this->nPlayerId); }
void CPlayer::Spawn() { functions->ForcePlayerSpawn(this->nPlayerId); }
void CPlayer::GiveMoney(int money) { functions->GivePlayerMoney(this->nPlayerId, money); }

void CPlayer::ShowMarkers(bool setMarkers) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionShowMarkers, setMarkers); }
bool CPlayer::ShowingMarkers() { return Boolify(functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionShowMarkers)); }

void CPlayer::SetSpectateTarget(CPlayer* pTarget)
{
	if (pTarget == nullptr) {
		functions->SetPlayerSpectateTarget(this->nPlayerId, -1);
	}
	else {
		functions->SetPlayerSpectateTarget(this->nPlayerId, pTarget->nPlayerId);
	}
}

CPlayer* CPlayer::GetSpectateTarget()
{
	int target = functions->GetPlayerSpectateTarget(this->nPlayerId);
	return pCore->RetrievePlayer(target);
}

EntityVector CPlayer::GetSpeed()
{
	float x, y, z;
	functions->GetPlayerSpeed(this->nPlayerId, &x, &y, &z);
	return EntityVector(this->nPlayerId, ENTITY_PLAYER, PLRVECTOR_SPEED, x, y, z);
}

void CPlayer::AddSpeed(Vector speed)
{
	functions->AddPlayerSpeed(this->nPlayerId, speed.x, speed.y, speed.z);
}

void CPlayer::SetSpeed(Vector speed) { functions->SetPlayerSpeed(this->nPlayerId, speed.x, speed.y, speed.z); }

void CPlayer::SetInterior(int interior)
{
	// OutputWarning accepts a direct ANSI string literal execution context
	OutputWarning("Player.SetInterior is deprecated. Teleport them to the interior location instead.");
}

void CPlayer::Eject() { functions->RemovePlayerFromVehicle(this->nPlayerId); }
void CPlayer::SetWantedLevel(int wantedLevel) { functions->SetPlayerWantedLevel(this->nPlayerId, wantedLevel); }

// NEW FEATURES 0.4.7 ======================================================================================================
bool CPlayer::GetBleeding() { return functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionBleeding) == 1; }
void CPlayer::SetBleeding(bool b) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionBleeding, b); }

int CPlayer::GetDrunkVisuals() { return functions->GetPlayerDrunkVisuals(this->nPlayerId); }
void CPlayer::SetDrunkVisuals(int v) { functions->SetPlayerDrunkVisuals(this->nPlayerId, (uint8_t)v); }

int CPlayer::GetDrunkHandling() { return functions->GetPlayerDrunkHandling(this->nPlayerId); }
void CPlayer::SetDrunkHandling(int h) { functions->SetPlayerDrunkHandling(this->nPlayerId, (uint32_t)h); }


void CPlayer::Kill() { functions->KillPlayer(this->nPlayerId); }

void CPlayer::SetCameraPos(Vector pos, Vector look, int interpTimeMS) {
	if (interpTimeMS > 0) {
		functions->InterpolateCameraLookAt(this->nPlayerId, look.x, look.y, look.z, interpTimeMS);
	}
	functions->SetCameraPosition(this->nPlayerId, pos.x, pos.y, pos.z, look.x, look.y, look.z);
}
// NEW FEATURES 0.4.7 ======================================================================================================
void CPlayer::Set3DArrowForPlayer(CPlayer* target, bool enable) {
	functions->SetPlayer3DArrowForPlayer(this->nPlayerId, target->nPlayerId, enable);
}

bool CPlayer::Get3DArrowForPlayer(CPlayer* target) {
	return functions->GetPlayer3DArrowForPlayer(this->nPlayerId, target->nPlayerId) == 1;
}
// ========================================================================================================================
void CPlayer::Disarm() { functions->RemoveAllWeapons(this->nPlayerId); }
void CPlayer::CompatAnimation(int anim) { functions->SetPlayerAnimation(this->nPlayerId, 0, anim); }
void CPlayer::Animation(int group, int anim) { functions->SetPlayerAnimation(this->nPlayerId, group, anim); }

CVehicle* CPlayer::StandingOnVehicle()
{
	int veh = functions->GetPlayerStandingOnVehicle(this->nPlayerId);
	return pCore->RetrieveVehicle(veh);
}

CObject* CPlayer::StandingOnObject()
{
	int obj = functions->GetPlayerStandingOnObject(this->nPlayerId);
	if (obj < 1) {
		return nullptr;
	}
	else {
		this->m_standingOn.nObjectId = obj;
		return &this->m_standingOn;
	}
}

void CPlayer::GiveWeapon(int weapon, int ammo) { functions->GivePlayerWeapon(this->nPlayerId, weapon, ammo); }
void CPlayer::SetWeapon(int weapon, int ammo) { functions->SetPlayerWeapon(this->nPlayerId, weapon, ammo); }
void CPlayer::RemoveWeapon(int weapon) { functions->RemovePlayerWeapon(this->nPlayerId, weapon); }
int CPlayer::GetWeaponAtSlot(int slot) { return functions->GetPlayerWeaponAtSlot(this->nPlayerId, slot); }
int CPlayer::GetAmmoAtSlot(int slot) { return functions->GetPlayerAmmoAtSlot(this->nPlayerId, slot); }
int CPlayer::GetID() { return this->nPlayerId; }
void CPlayer::Select() { functions->ForcePlayerSelect(this->nPlayerId); }
bool CPlayer::GetCameraLocked() { return Boolify(functions->IsCameraLocked(this->nPlayerId)); }
void CPlayer::RestoreCamera() { functions->RestoreCamera(this->nPlayerId); }
int CPlayer::GetKey() { return functions->GetPlayerKey(this->nPlayerId); }
double CPlayer::GetFPS() { return functions->GetPlayerFPS(this->nPlayerId); }

void CPlayer::SetMarker(int nDummy) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionHasMarker, 1); }
void CPlayer::RemoveMarker() { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionHasMarker, 0); }
bool CPlayer::GetMarkerVisible() { return functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionHasMarker) == 1; }
void CPlayer::SetMarkerVisible(bool isVisible) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionHasMarker, isVisible); }

bool CPlayer::GetAwayStatus() { return functions->IsPlayerAway(this->nPlayerId) == 1; }
bool CPlayer::GetCanUseColors() { return functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionChatTagsEnabled) == 1; }
void CPlayer::SetCanUseColors(bool canUse) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionChatTagsEnabled, canUse); }

bool CPlayer::GetDrunkStatus() { return functions->GetPlayerOption(this->nPlayerId, vcmpPlayerOptionDrunkEffectsDeprecated) == 1; }
void CPlayer::SetDrunkStatus(bool isDrunk) { functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionDrunkEffectsDeprecated, isDrunk); }

void CPlayer::SetDrunkLevel(int visuals, int handling)
{
	if (visuals <= 0 && handling <= 0) {
		functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionDrunkEffectsDeprecated, 0);
	}
	else {
		functions->SetPlayerOption(this->nPlayerId, vcmpPlayerOptionDrunkEffectsDeprecated, 1);
	}
}

SQChar* CPlayer::GetUniqueID()
{
	functions->GetPlayerUID(this->nPlayerId, this->m_uid, 41);

	static SQChar wUid[41];
#ifdef SQUNICODE
	mbstowcs(wUid, this->m_uid, 41);
	return wUid;
#else
	return this->m_uid;
#endif
}

SQChar* CPlayer::GetUniqueID2()
{
	functions->GetPlayerUID2(this->nPlayerId, this->m_uid2, 41);

	static SQChar wUid2[41];
#ifdef SQUNICODE
	mbstowcs(wUid2, this->m_uid2, 41);
	return wUid2;
#else
	return this->m_uid2;
#endif
}

int CPlayer::GetVehicleSlot() { return functions->GetPlayerInVehicleSlot(this->nPlayerId); }
int CPlayer::GetVehicleStatus() { return functions->GetPlayerInVehicleStatus(this->nPlayerId); }

bool CPlayer::RedirectPlayerToServer(const char* szIP, unsigned int usPort, const char* szNickname, const char* szServerPass, const char* szUserPass)
{
	return Boolify(functions->RedirectPlayerToServer(this->nPlayerId, szIP, usPort, szNickname, szServerPass, szUserPass));
}

void CPlayer::RequestModuleList(void) { functions->GetPlayerModuleList(this->nPlayerId); }

Sqrat::string PlayerToString(CPlayer* p)
{
	return p->GetName();
}

void RegisterPlayer()
{
	Class<CPlayer> c(v, "CPlayer_INTERNAL");

	// Read-write properties
	c
		.Prop(_SC("Admin"), &CPlayer::GetAdmin, &CPlayer::SetAdmin)
		.Prop(_SC("Angle"), &CPlayer::GetHeading, &CPlayer::SetHeading)
		.Prop(_SC("Armor"), &CPlayer::GetArmour, &CPlayer::SetArmour)
		.Prop(_SC("Armour"), &CPlayer::GetArmour, &CPlayer::SetArmour)
		.Prop(_SC("Bleeding"), &CPlayer::GetBleeding, &CPlayer::SetBleeding)
		.Prop(_SC("CanAttack"), &CPlayer::GetCanAttack, &CPlayer::SetCanAttack)
		.Prop(_SC("CanDriveby"), &CPlayer::GetDrivebyEnabled, &CPlayer::SetDrivebyEnabled)
		.Prop(_SC("Cash"), &CPlayer::GetMoney, &CPlayer::SetMoney)
		.Prop(_SC("Color"), &CPlayer::GetColour, &CPlayer::SetColour)
		.Prop(_SC("Colour"), &CPlayer::GetColour, &CPlayer::SetColour)
		.Prop(_SC("DrivebyAbility"), &CPlayer::GetDrivebyEnabled, &CPlayer::SetDrivebyEnabled)
		.Prop(_SC("DrunkHandling"), &CPlayer::GetDrunkHandling, &CPlayer::SetDrunkHandling)
		.Prop(_SC("DrunkVisuals"), &CPlayer::GetDrunkVisuals, &CPlayer::SetDrunkVisuals)
		.Prop(_SC("Frozen"), &CPlayer::GetFrozen, &CPlayer::SetFrozen)
		.Prop(_SC("GreenScanlines"), &CPlayer::GetGreenScanlines, &CPlayer::SetGreenScanlines)
		.Prop(_SC("HasChatTags"), &CPlayer::GetCanUseColors, &CPlayer::SetCanUseColors)
		.Prop(_SC("HasMarker"), &CPlayer::GetMarkerVisible, &CPlayer::SetMarkerVisible)
		.Prop(_SC("Heading"), &CPlayer::GetHeading, &CPlayer::SetHeading)
		.Prop(_SC("Health"), &CPlayer::GetHealth, &CPlayer::SetHealth)
		.Prop(_SC("Immunity"), &CPlayer::GetImmunity, &CPlayer::SetImmunity)
		.Prop(_SC("IsAdmin"), &CPlayer::GetAdmin, &CPlayer::SetAdmin)
		.Prop(_SC("IsDrunk"), &CPlayer::GetDrunkStatus, &CPlayer::SetDrunkStatus)
		.Prop(_SC("IsFrozen"), &CPlayer::GetFrozen, &CPlayer::SetFrozen)
		.Prop(_SC("IsOnRadar"), &CPlayer::GetOnRadar, &CPlayer::SetOnRadar)
		.Prop(_SC("IsWeaponSyncBlocked"), &CPlayer::GetCanAttack, &CPlayer::SetCanAttack)
		.Prop(_SC("Money"), &CPlayer::GetMoney, &CPlayer::SetMoney)
		.Prop(_SC("Name"), &CPlayer::GetName, &CPlayer::SetName)
		.Prop(_SC("Pos"), &CPlayer::GetPosition, &CPlayer::SetPosition)
		.Prop(_SC("Score"), &CPlayer::GetScore, &CPlayer::SetScore)
		.Prop(_SC("SecWorld"), &CPlayer::GetSecWorld, &CPlayer::SetSecWorld)
		.Prop(_SC("ShowMarkers"), &CPlayer::ShowingMarkers, &CPlayer::ShowMarkers)
		.Prop(_SC("Skin"), &CPlayer::GetSkin, &CPlayer::SetSkin)
		.Prop(_SC("Slot"), &CPlayer::GetWeaponSlot, &CPlayer::SetWeaponSlot)
		.Prop(_SC("SpectateTarget"), &CPlayer::GetSpectateTarget, &CPlayer::SetSpectateTarget)
		.Prop(_SC("Speed"), &CPlayer::GetSpeed, &CPlayer::SetSpeed)
		.Prop(_SC("Team"), &CPlayer::GetTeam, &CPlayer::SetTeam)
		.Prop(_SC("Vehicle"), &CPlayer::GetVehicle, &CPlayer::SetVehicle)
		.Prop(_SC("WantedLevel"), &CPlayer::GetWantedLevel, &CPlayer::SetWantedLevel)
		.Prop(_SC("WhiteScanlines"), &CPlayer::GetWhiteScanlines, &CPlayer::SetWhiteScanlines)
		.Prop(_SC("Widescreen"), &CPlayer::GetWidescreen, &CPlayer::SetWidescreen)
		.Prop(_SC("World"), &CPlayer::GetWorld, &CPlayer::SetWorld);

	// Read-only properties
	c
		.Prop(_SC("Action"), &CPlayer::GetPlayerAction)
		.Prop(_SC("AimDir"), &CPlayer::GetPlayerAimDir)
		.Prop(_SC("AimPos"), &CPlayer::GetPlayerAimPos)
		.Prop(_SC("Alpha"), &CPlayer::GetAlpha)
		.Prop(_SC("Ammo"), &CPlayer::GetWeaponAmmo)
		.Prop(_SC("Away"), &CPlayer::GetAwayStatus)
		.Prop(_SC("CameraLocked"), &CPlayer::GetCameraLocked)
		.Prop(_SC("Class"), &CPlayer::GetClass)
		.Prop(_SC("FPS"), &CPlayer::GetFPS)
		.Prop(_SC("GameKeys"), &CPlayer::GetPlayerGameKeys)
		.Prop(_SC("ID"), &CPlayer::GetID)
		.Prop(_SC("IP"), &CPlayer::GetIP)
		.Prop(_SC("IsCrouching"), &CPlayer::GetPlayerCrouchStatus)
		.Prop(_SC("IsOnFire"), &CPlayer::GetPlayerOnFireStatus)
		.Prop(_SC("IsSpawned"), &CPlayer::GetSpawned)
		.Prop(_SC("Key"), &CPlayer::GetKey)
		.Prop(_SC("Ping"), &CPlayer::GetPing)
		.Prop(_SC("Spawned"), &CPlayer::GetSpawned)
		.Prop(_SC("StandingOnObject"), &CPlayer::StandingOnObject)
		.Prop(_SC("StandingOnVehicle"), &CPlayer::StandingOnVehicle)
		.Prop(_SC("State"), &CPlayer::GetState)
		.Prop(_SC("Typing"), &CPlayer::Typing)
		.Prop(_SC("UID"), &CPlayer::GetUniqueID)
		.Prop(_SC("UID2"), &CPlayer::GetUniqueID2)
		.Prop(_SC("UniqueID"), &CPlayer::GetUniqueID)
		.Prop(_SC("UniqueID2"), &CPlayer::GetUniqueID2)
		.Prop(_SC("UniqueWorld"), &CPlayer::GetUniqueWorld)
		.Prop(_SC("VehicleSlot"), &CPlayer::GetVehicleSlot)
		.Prop(_SC("VehicleStatus"), &CPlayer::GetVehicleStatus)
		.Prop(_SC("Weapon"), &CPlayer::GetWeapon);

	// Functions
	c.Func(_SC("AddSpeed"), &CPlayer::AddSpeed)
		.Func(_SC("Ban"), &CPlayer::Ban)
		.Func(_SC("Disarm"), &CPlayer::Disarm)
		.Func(_SC("Eject"), &CPlayer::Eject)
		.Func(_SC("Get3DArrowForPlayer"), &CPlayer::Get3DArrowForPlayer)
		.Func(_SC("GetAmmoAtSlot"), &CPlayer::GetAmmoAtSlot)
		.Func(_SC("GetWeaponAtSlot"), &CPlayer::GetWeaponAtSlot)
		.Func(_SC("GiveMoney"), &CPlayer::GiveMoney)
		.Func(_SC("GiveWeapon"), &CPlayer::GiveWeapon)
		.Func(_SC("Kick"), &CPlayer::Kick)
		.Func(_SC("Kill"), &CPlayer::Kill)
		.Func(_SC("PlaySound"), &CPlayer::PlaySound)
		.Func(_SC("PutInVehicleSlot"), &CPlayer::SetVehicleSlot)
		.Func(_SC("Redirect"), &CPlayer::RedirectPlayerToServer)
		.Func(_SC("RemoveMarker"), &CPlayer::RemoveMarker)
		.Func(_SC("RemoveWeapon"), &CPlayer::RemoveWeapon)
		.Func(_SC("RequestModuleList"), &CPlayer::RequestModuleList)
		.Func(_SC("RestoreCamera"), &CPlayer::RestoreCamera)
		.Func(_SC("Select"), &CPlayer::Select)
		.Func(_SC("Set3DArrowForPlayer"), &CPlayer::Set3DArrowForPlayer)
		.Func(_SC("SetAlpha"), &CPlayer::SetAlpha)
		.Overload(_SC("SetAnim"), &CPlayer::Animation)
		.Overload(_SC("SetAnim"), &CPlayer::CompatAnimation)
		.Overload(_SC("SetCameraPos"), &CPlayer::SetCameraPos)
		.Func(_SC("SetDrunkLevel"), &CPlayer::SetDrunkLevel)
		.Func(_SC("SetInterior"), &CPlayer::SetInterior)
		.Func(_SC("SetMarker"), &CPlayer::SetMarker)
		.Func(_SC("SetWantedLevel"), &CPlayer::SetWantedLevel)
		.Func(_SC("SetWeapon"), &CPlayer::SetWeapon)
		.Func(_SC("Spawn"), &CPlayer::Spawn)
		.Func(_SC("StreamedToPlayer"), &CPlayer::StreamedToPlayer);

	c.GlobalFunc(_SC("_tostring"), &PlayerToString);
	RootTable(v).Bind(_SC("CPlayer"), c);
}