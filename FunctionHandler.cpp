#include "FunctionHandler.h"
#include "Functions.h"
#include "ConsoleUtils.h"
#include "CCore.h"
#include "main.h"
#include "SQModule.h"

#ifndef _SQUIRREL_H_
#include "squirrel.h"
#endif

#ifndef SQCHAR
#ifdef UNICODE
typedef wchar_t SQChar;
#else
typedef char SQChar;
#endif
#endif

#ifdef _WIN32
    #define sq_strdup _strdup
#else
    #define sq_strdup strdup
#endif

#ifdef _WIN32
    #define sq_strtok strtok_s
#else
    #define sq_strtok strtok_r
#endif

#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>

inline void szlower(char* string)
{
	for (int i = 0; string[i]; i++)
		string[i] = (char)tolower(string[i]);
}

namespace
{
	std::string ConvertSQString(const SQChar* source)
	{
		if (!source) return std::string("");
		return std::string(source);
	}

	const SQChar* ConvertToSQStringStatic(const char* source, size_t maxLen)
	{
		if (!source) return _SC("");
		return (const SQChar*)source;
	}
}

// Tha core
extern CCore * pCore;

void ShutdownServer(void) { functions->ShutdownServer(); }
void ClientMessage(const SQChar * message, CPlayer * player, int r, int g, int b)
{
	ClientMessageWithAlpha(message, player, r, g, b, 255);
}

void ClientMessageWithAlpha(const SQChar * message, CPlayer * player, int r, int g, int b, int a)
{
	if (player != NULL)
	{
		RGBa colour(r, g, b, a);
		functions->SendClientMessage(player->nPlayerId, colour.toUInt(), "%s", message);
	}
}

void ClientMessageToAll(const SQChar* message, int r, int g, int b)
{
	ClientMessageToAllWithAlpha(message, r, g, b, 255);
}

void ClientMessageToAllWithAlpha(const SQChar* message, int r, int g, int b, int a)
{
	const int maxPlayers = functions->GetMaxPlayers();

	for (int i = 0; i < maxPlayers; i++)
	{
		if (functions->IsPlayerConnected(i))
			ClientMessageWithAlpha(message, pCore->RetrievePlayer(i), r, g, b, a);
	}
}

void GameMessage(const SQChar* message, CPlayer* player, int type)
{
	if (player != nullptr)
	{
		std::string converted = ConvertSQString(message);
		functions->SendGameMessage(player->nPlayerId, type, "%s", converted.c_str());
	}
}

void GameMessageAlternate(const SQChar* message, CPlayer* player)
{
	if (player != nullptr)
	{
		std::string converted = ConvertSQString(message);
		functions->SendGameMessage(player->nPlayerId, 1, "%s", converted.c_str());
	}
}

void GameMessageToAll(const SQChar* message, int type)
{
	std::string converted = ConvertSQString(message);
	functions->SendGameMessage(-1, type, "%s", converted.c_str());
}

void GameMessageToAllAlternate(const SQChar* message)
{
	std::string converted = ConvertSQString(message);
	functions->SendGameMessage(-1, 1, "%s", converted.c_str());
}

void SetServerName(const SQChar* message)
{
	std::string converted = ConvertSQString(message);
	functions->SetServerName(converted.c_str());
}

void SetMaxPlayers(int newMaxPlayers)
{
	functions->SetMaxPlayers(newMaxPlayers);
}

void SetServerPassword(const SQChar* password)
{
	std::string converted = ConvertSQString(password);
	functions->SetServerPassword(converted.c_str());
}

void SetGameModeText(const SQChar* gamemodeName)
{
	std::string converted = ConvertSQString(gamemodeName);
	functions->SetGameModeText(converted.c_str());
}

void SetTimeRate( unsigned int timeRate ) { functions->SetTimeRate( timeRate ); }
void SetHour( int hour )                  { functions->SetHour( hour ); }
void SetMinute( int minute )              { functions->SetMinute( minute ); }
void SetTime( int hour, int minute )      { SetHour(hour); SetMinute(minute); }
void SetWeather( int weather )            { functions->SetWeather( weather ); }
void SetGravity( float gravity )          { functions->SetGravity( gravity ); }
void SetGamespeed( float speed )          { functions->SetGameSpeed( speed ); }
void SetWaterLevel( float waterLevel )    { functions->SetWaterLevel( waterLevel ); }
void SetMaxHeight( float maxHeight )      { functions->SetMaximumFlightAltitude( maxHeight ); }

const SQChar* GetServerName()
{
	static char serverName[128];
	functions->GetServerName(serverName, 128);

	return ConvertToSQStringStatic(serverName, 128);
}

int GetMaxPlayers()
{
	return functions->GetMaxPlayers();
}

const SQChar* GetServerPassword()
{
	static char password[128];
	functions->GetServerPassword(password, 128);

	return ConvertToSQStringStatic(password, 128);
}

const SQChar* GetGameModeText()
{
	static char gamemode[96];
	functions->GetGameModeText(gamemode, 96);

	return ConvertToSQStringStatic(gamemode, 96);
}

unsigned int GetTimeRate()   { return ( functions->GetTimeRate() );   }
int          GetHour()       { return ( functions->GetHour() );       }
int          GetMinute()     { return ( functions->GetMinute() );     }
int          GetWeather()    { return ( functions->GetWeather() );    }
float        GetGravity()    { return ( functions->GetGravity() );    }
float        GetGamespeed()  { return ( functions->GetGameSpeed() );  }
float        GetWaterLevel() { return ( functions->GetWaterLevel() ); }
float        GetMaxHeight()  { return ( functions->GetMaximumFlightAltitude() );  }

void ToggleSyncFrameLimiter ( bool toggle ) { functions->SetServerOption( vcmpServerOptionSyncFrameLimiter, ( toggle ? 1 : 0 ) ); }
void ToggleFrameLimiter     ( bool toggle ) { functions->SetServerOption( vcmpServerOptionFrameLimiter, ( toggle ? 1 : 0 ) );     }
void ToggleTaxiBoostJump    ( bool toggle ) { functions->SetServerOption( vcmpServerOptionTaxiBoostJump, ( toggle ? 1 : 0 ) );    }
void ToggleDriveOnWater     ( bool toggle ) { functions->SetServerOption( vcmpServerOptionDriveOnWater, ( toggle ? 1 : 0 ) );     }
void ToggleFastSwitch       ( bool toggle ) { functions->SetServerOption( vcmpServerOptionFastSwitch, ( toggle ? 1 : 0 ) );       }
void ToggleFriendlyFire     ( bool toggle ) { functions->SetServerOption( vcmpServerOptionFriendlyFire, ( toggle ? 1 : 0 ) );     }
void ToggleDisableDriveby   ( bool toggle ) { functions->SetServerOption( vcmpServerOptionDisableDriveBy, ( toggle ? 0 : 1 ) );   }
void TogglePerfectHandling  ( bool toggle ) { functions->SetServerOption( vcmpServerOptionPerfectHandling, ( toggle ? 1 : 0 ) );  }
void ToggleFlyingCars       ( bool toggle ) { functions->SetServerOption( vcmpServerOptionFlyingCars, ( toggle ? 1 : 0 ) );       }
void ToggleJumpSwitch       ( bool toggle ) { functions->SetServerOption( vcmpServerOptionJumpSwitch, ( toggle ? 1 : 0 ) );       }
void ToggleShowOnRadar      ( bool toggle ) { functions->SetServerOption( vcmpServerOptionShowMarkers, ( toggle ? 1 : 0 ) );      }
void ToggleStuntBike        ( bool toggle ) { functions->SetServerOption( vcmpServerOptionStuntBike, ( toggle ? 1 : 0 ) );        }
void ToggleShootInAir       ( bool toggle ) { functions->SetServerOption( vcmpServerOptionShootInAir, ( toggle ? 1 : 0 ) );       }
void ToggleShowNametags     ( bool toggle ) { functions->SetServerOption( vcmpServerOptionShowNameTags, ( toggle ? 1 : 0 ) );     }
void ToggleJoinMessages     ( bool toggle ) { functions->SetServerOption( vcmpServerOptionJoinMessages, ( toggle ? 1 : 0 ) );     }
void ToggleDeathMessages    ( bool toggle ) { functions->SetServerOption( vcmpServerOptionDeathMessages, ( toggle ? 1 : 0 ) );    }
void ToggleChatTagDefault   ( bool toggle ) { functions->SetServerOption( vcmpServerOptionChatTagsEnabled, ( toggle ? 1 : 0 ) );  }
void ToggleShowOnlyTeamMarkers(bool toggle) { functions->SetServerOption( vcmpServerOptionOnlyShowTeamMarkers,  ( toggle ? 1 : 0 ) ); }
void ToggleWallglitch       ( bool toggle ) { functions->SetServerOption( vcmpServerOptionWallGlitch,  ( toggle ? 1 : 0 ) );      }
void ToggleDisableBackfaceCulling( bool toggle ) { functions->SetServerOption( vcmpServerOptionDisableBackfaceCulling,  ( toggle ? 1 : 0 ) ); }
void ToggleDisableHeliBladeDamage( bool toggle ) { functions->SetServerOption( vcmpServerOptionDisableHeliBladeDamage,  ( toggle ? 1 : 0 ) ); }

bool EnabledSyncFrameLimiter() { return ( functions->GetServerOption(vcmpServerOptionSyncFrameLimiter) != 0 );  }
bool EnabledFrameLimiter()     { return ( functions->GetServerOption(vcmpServerOptionFrameLimiter) != 0 );      }
bool EnabledTaxiBoostJump()    { return ( functions->GetServerOption(vcmpServerOptionTaxiBoostJump) != 0 );     }
bool EnabledDriveOnWater()     { return ( functions->GetServerOption(vcmpServerOptionDriveOnWater) != 0 );      }
bool EnabledFastSwitch()       { return ( functions->GetServerOption(vcmpServerOptionFastSwitch) != 0 );        }
bool EnabledFriendlyFire()     { return ( functions->GetServerOption(vcmpServerOptionFriendlyFire) != 0 );      }
bool EnabledDisableDriveby()   { return ( functions->GetServerOption(vcmpServerOptionDisableDriveBy) == 0 );    }
bool EnabledPerfectHandling()  { return ( functions->GetServerOption(vcmpServerOptionPerfectHandling) != 0 );   }
bool EnabledFlyingCars()       { return ( functions->GetServerOption(vcmpServerOptionFlyingCars) != 0 );        }
bool EnabledJumpSwitch()       { return ( functions->GetServerOption(vcmpServerOptionJumpSwitch) != 0 );        }
bool EnabledShowOnRadar()      { return ( functions->GetServerOption(vcmpServerOptionShowMarkers) != 0 );       }
bool EnabledStuntBike()        { return ( functions->GetServerOption(vcmpServerOptionStuntBike) != 0 );         }
bool EnabledShootInAir()       { return ( functions->GetServerOption(vcmpServerOptionShootInAir) != 0 );        }
bool EnabledShowNametags()     { return ( functions->GetServerOption(vcmpServerOptionShowNameTags) != 0 );      }
bool EnabledJoinMessages()     { return ( functions->GetServerOption(vcmpServerOptionJoinMessages) != 0 );      }
bool EnabledDeathMessages()    { return ( functions->GetServerOption(vcmpServerOptionDeathMessages) != 0 );     }
bool EnabledChatTagDefault()   { return ( functions->GetServerOption(vcmpServerOptionChatTagsEnabled) != 0 );   }
bool EnabledShowOnlyTeamMarkers() { return functions->GetServerOption(vcmpServerOptionOnlyShowTeamMarkers) != 0;}
bool EnabledWallglitch()       { return functions->GetServerOption(vcmpServerOptionWallGlitch) != 0;            }
bool EnabledDisableBackfaceCulling() { return functions->GetServerOption(vcmpServerOptionDisableBackfaceCulling) != 0; }
bool EnabledDisableHeliBladeDamage() { return functions->GetServerOption(vcmpServerOptionDisableHeliBladeDamage) != 0; }

void SetCrouchEnabled(bool enable) { functions->SetServerOption(vcmpServerOptionDisableCrouch, !enable); }
bool GetCrouchEnabled(void) { return !functions->GetServerOption(vcmpServerOptionDisableCrouch); }

void CreateExplosion( int world, int type, Vector * pos, int playerCaused, bool onGround )
{
	functions->CreateExplosion( world, type, pos->x, pos->y, pos->z, playerCaused, onGround );
}

void CreateExplosionExpanded( int world, int type, float x, float y, float z, int playerCaused, bool onGround )
{
	Vector pos = Vector( x, y, z );
	CreateExplosion( world, type, &pos, playerCaused, onGround );
}

void PlayGameSound ( int world, int sound, Vector * pos ) { functions->PlaySound( world, sound, pos->x, pos->y, pos->z ); }
void PlayGameSoundForPlayer(CPlayer* pPlayer, int sound) {
	functions->PlaySound(pPlayer->GetUniqueWorld(), sound, NAN, NAN, NAN);
}

void PlayGameSoundForWorld(int world, int sound) {
	functions->PlaySound(world, sound, NAN, NAN, NAN);
}

void PlayGameSoundExpanded( int world, int sound, float x, float y, float z )
{
	Vector pos = Vector( x, y, z );
	PlayGameSound( world, sound, &pos );
}

void SetUseClasses ( bool toUse )                         { functions->SetServerOption( vcmpServerOptionUseClasses, toUse ); }
bool UsingClasses  ()                                     { return ( functions->GetServerOption( vcmpServerOptionUseClasses ) != 0 ); }

void AddClass( int team, cRGB * colour, int skin, Vector * spawnPos, float spawnAngle, int wep1, int ammo1, int wep2, int ammo2, int wep3, int ammo3 )
{
	functions->AddPlayerClass( team, colour->toUInt(), skin, spawnPos->x, spawnPos->y, spawnPos->z, spawnAngle, wep1, ammo1, wep2, ammo2, wep3, ammo3 );
}

void AddClassExpanded ( int team, int r, int g, int b, int skin, float x, float y, float z, float spawnAngle, int wep1, int ammo1, int wep2, int ammo2, int wep3, int ammo3 )
{
	cRGB col = cRGB( r, g, b );
	Vector spawn = Vector( x, y, z );

	AddClass( team, &col, skin, &spawn, spawnAngle, wep1, ammo1, wep2, ammo2, wep3, ammo3 );
}

void SetSpawnPlayerPos( Vector * pos )  { functions->SetSpawnPlayerPosition( pos->x, pos->y, pos->z ); }
void SetSpawnCameraPos( Vector * pos )  { functions->SetSpawnCameraPosition( pos->x, pos->y, pos->z ); }
void SetSpawnCameraLook( Vector * pos ) { functions->SetSpawnCameraLookAt( pos->x, pos->y, pos->z ); }

void SetSpawnPlayerPosExpanded  ( float x, float y, float z ) { Vector p = Vector( x, y, z ); SetSpawnPlayerPos( &p ); }
void SetSpawnCameraPosExpanded  ( float x, float y, float z ) { Vector p = Vector( x, y, z ); SetSpawnCameraPos( &p ); }
void SetSpawnCameraLookExpanded ( float x, float y, float z ) { Vector p = Vector( x, y, z ); SetSpawnCameraLook( &p ); }

void BanIP(const SQChar* ip)
{
	std::string converted = ConvertSQString(ip);
	functions->BanIP(const_cast<char*>(converted.c_str()));
}

void UnbanIP(const SQChar* ip)
{
	std::string converted = ConvertSQString(ip);
	functions->UnbanIP(const_cast<char*>(converted.c_str()));
}

bool IsIPBanned(const SQChar* ip)
{
	std::string converted = ConvertSQString(ip);
	return (functions->IsIPBanned(const_cast<char*>(converted.c_str())) != 0);
}

bool IsWorldCompatibleWithPlayer( CPlayer * cPlayer, int world )
{
	if( cPlayer != NULL )
		return ( functions->IsPlayerWorldCompatible( cPlayer->nPlayerId, world ) != 0 );

	return false;
}

int GetPlayerIDFromName(const SQChar* name)
{
	std::string converted = ConvertSQString(name);
	return functions->GetPlayerIdFromName(const_cast<char*>(converted.c_str()));
}

CVehicle * CreateVehicle( int model, int world, Vector * pos, float angle, int col1, int col2 )
{
	int vId = functions->CreateVehicle( model, world, pos->x, pos->y, pos->z, angle, col1, col2 );
	if (vId < 1)
		return NULL;
	else
		return pCore->AllocateVehicle(vId);
}

CPickup * CreatePickup( int model, int world, int quantity, Vector * pos, int alpha, bool isAuto )
{
	int pId = functions->CreatePickup(model, world, quantity, pos->x, pos->y, pos->z, alpha, isAuto);
	if (pId < 0)
		return NULL;
	else
		return pCore->AllocatePickup(pId);
}

CPickup * CreatePickupCompat(int model, Vector * pos)
{
	int pId = functions->CreatePickup(model, 1, 0, pos->x, pos->y, pos->z, 255, 0);
	if (pId < 0)
		return NULL;
	else
		return pCore->AllocatePickup(pId);
}

CVehicle * CreateVehicleCompat(int model, Vector * pos, float angle, int col1, int col2)
{
	int vId = functions->CreateVehicle(model, 1, pos->x, pos->y, pos->z, angle, col1, col2);
	if (vId < 1)
		return NULL;
	else
		return pCore->AllocateVehicle(vId);
}

CObject * CreateObject( int model, int world, Vector * pos, int alpha )
{
	int oId = functions->CreateObject( model, world, pos->x, pos->y, pos->z, alpha );
	if (oId < 0)
		return NULL;
	else
		return pCore->AllocateObject(oId);
}

CCheckpoint * CreateCheckpoint(CPlayer* pPlayer, int world, bool isSphere, Vector * pos, ARGB * color, float radius)
{
	if (!pos || !color) {
		return NULL;
	}

	int id = (pPlayer ? pPlayer->GetID() : -1);

	int cId = functions->CreateCheckPoint(id, world, isSphere, pos->x, pos->y, pos->z, color->r, color->g, color->b, color->a, radius);
	if (cId < 0)
		return NULL;
	else {
		CCheckpoint * pCheckpoint = pCore->AllocateCheckpoint(cId);
		pCheckpoint->nOwnerId = id;

		return pCheckpoint;
	}
}

CVehicle * CreateVehicleExpanded( int model, int world, float x, float y, float z, float angle, int col1, int col2 )
{
	Vector pos = Vector( x, y, z );
	return CreateVehicle( model, world, &pos, angle, col1, col2 );
}

CPickup * CreatePickupExpanded( int model, int world, int quantity, float x, float y, float z, int alpha, bool isAuto )
{
	Vector pos = Vector( x, y, z );
	return CreatePickup( model, world, quantity, &pos, alpha, isAuto );
}

CObject * CreateObjectExpanded( int model, int world, float x, float y, float z, int alpha )
{
	Vector pos = Vector( x, y, z );
	return CreateObject( model, world, &pos, alpha );
}

CPickup * FindPickup(int id) { return pCore->RetrievePickup(id); }
CObject * FindObject(int id) { return pCore->RetrieveObject(id); }
CVehicle * FindVehicle(int id) { return pCore->RetrieveVehicle(id); }
CCheckpoint * FindCheckpoint(int id) { return pCore->RetrieveCheckpoint(id); }

void SetWorldBounds( float maxX, float minX, float maxY, float minY )
{
	functions->SetWorldBounds( maxX, minX, maxY, minY );
}

Bounds GetWorldBounds()
{
	float maxX, minX, maxY, minY;
	functions->GetWorldBounds( &maxX, &minX, &maxY, &minY );

	return Bounds( maxX, minX, maxY, minY );
}

void SetWastedSettings( int deathTime, int fadeTime, float fadeInSpeed, float fadeOutSpeed, cRGB colour, int corpseFadeDelay, int corpseFadeTime )
{
	unsigned int rDeathTime, rFadeTime, rCorpseFadeDelay, rCorpseFadeTime;

	rDeathTime       = static_cast<unsigned int>(deathTime);
	rFadeTime        = static_cast<unsigned int>(fadeTime);
	rCorpseFadeDelay = static_cast<unsigned int>(corpseFadeDelay);
	rCorpseFadeTime  = static_cast<unsigned int>(corpseFadeTime);

	functions->SetWastedSettings
	(
		rDeathTime,
		rFadeTime,
		fadeInSpeed,
		fadeOutSpeed,
		colour.toUInt(),
		rCorpseFadeDelay,
		rCorpseFadeTime
	);
}

WastedSettings GetWastedSettings()
{
	unsigned int deathTime, fadeTime, corpseFadeDelay, corpseFadeTime, dwColour;
	float fadeInSpeed, fadeOutSpeed;

	functions->GetWastedSettings
	(
		&deathTime,
		&fadeTime,
		&fadeInSpeed,
		&fadeOutSpeed,
		&dwColour,
		&corpseFadeDelay,
		&corpseFadeTime
	);

	int rDeathTime, rFadeTime, rCorpseFadeDelay, rCorpseFadeTime;
	cRGB rColour;

	rDeathTime       = static_cast<int>(deathTime);
	rFadeTime        = static_cast<int>(fadeTime);
	rCorpseFadeDelay = static_cast<int>(corpseFadeDelay);
	rCorpseFadeTime  = static_cast<int>(corpseFadeTime);

	rColour.r = (dwColour >> 16) & 255;
	rColour.g = (dwColour >> 8) & 255;
	rColour.b = dwColour & 255;

	return WastedSettings
	(
		rDeathTime,
		rFadeTime,
		fadeInSpeed,
		fadeOutSpeed,
		rColour,
		rCorpseFadeDelay,
		rCorpseFadeTime
	);
}

void RawHideMapObject( int nModelId, int x, int y, int z )
{
	functions->HideMapObject( nModelId, x, y, z );
}

void HideMapObject( int nModelId, float x, float y, float z )
{
	int x2 = (int)(floor( x * 10.0f ) + 0.5f);
	int y2 = (int)(floor( y * 10.0f ) + 0.5f);
	int z2 = (int)(floor( z * 10.0f ) + 0.5f);

	functions->HideMapObject( nModelId, x2, y2, z2 );
}

void ShowMapObject( int nModelId, float x, float y, float z )
{
	int x2 = (int)(floor( x * 10.0f ) + 0.5f);
	int y2 = (int)(floor( y * 10.0f ) + 0.5f);
	int z2 = (int)(floor( z * 10.0f ) + 0.5f);

	functions->ShowMapObject( nModelId, x2, y2, z2 );
}

void ShowAllMapObjects() { functions->ShowAllMapObjects(); }
void ForceAllSelect() { functions->ForceAllSelect(); }

void ResetAllVehicleHandling() { functions->ResetAllVehicleHandlings(); }
bool IsHandlingRuleSet( int model, int rule ) { return ( functions->ExistsHandlingRule( model, rule ) != 0 ); }
void SetHandlingRule( int model, int rule, float value ) { functions->SetHandlingRule( model, rule, value ); }
double GetHandlingRule( int model, int rule ) { return functions->GetHandlingRule( model, rule ); }
void ResetHandlingRule( int model, int rule ) { functions->ResetHandlingRule( model, rule ); }
void ResetVehicleHandling( int model ) { functions->ResetHandling( model ); }

// All of these functions exist for compatibility
bool GetCinematicBorder ( CPlayer * player )
{
	if( player != NULL )
		return ( functions->GetPlayerOption( player->nPlayerId, vcmpPlayerOptionWidescreen ) != 0 );

	return false;
}

bool GetGreenScanLines  ( CPlayer * player )
{
	if( player != NULL )
		return ( functions->GetPlayerOption( player->nPlayerId, vcmpPlayerOptionGreenScanlines ) != 0 );

	return false;
}

bool GetWhiteScanLines  ( CPlayer * player )
{
	if( player != NULL )
		return ( functions->GetPlayerOption( player->nPlayerId, vcmpPlayerOptionWhiteScanlines ) != 0 );

	return false;
}

void SetCinematicBorder ( CPlayer * player, bool toEnable )
{
	if( player != NULL )
		functions->SetPlayerOption( player->nPlayerId, vcmpPlayerOptionWidescreen, toEnable );
}

void SetGreenScanLines  ( CPlayer * player, bool toEnable )
{
	if( player != NULL )
		functions->SetPlayerOption( player->nPlayerId, vcmpPlayerOptionGreenScanlines, toEnable );
}

void SetWhiteScanLines  ( CPlayer * player, bool toEnable )
{
	if( player != NULL )
		functions->SetPlayerOption( player->nPlayerId, vcmpPlayerOptionWhiteScanlines, toEnable );
}

void KickPlayer         ( CPlayer * player )
{
	if( player != NULL )
		functions->KickPlayer( player->nPlayerId );
}

void BanPlayer          ( CPlayer * player )
{
	if( player != NULL )
		functions->BanPlayer( player->nPlayerId );
}

void Message            ( const SQChar * message ) { functions->SendClientMessage( -1, 0x0b5fa5ff, "%s", message ); }
void MessagePlayer      ( const SQChar * message, CPlayer * player )
{
	if( player != NULL )
		functions->SendClientMessage( player->nPlayerId, 0x0b5fa5ff, "%s", message );
}

void MessageAllExcept   ( const SQChar * message, CPlayer * player )
{
	if( player != NULL )
	{
		for( int i = 0; i < MAX_PLAYERS; i++ )
		{
			if( functions->IsPlayerConnected( i ) && i != player->GetID() )
				functions->SendClientMessage( i, 0x0b5fa5ff, "%s", message );
		}
	}
}

void PrivMessage        ( CPlayer * player, const SQChar * message )
{
	if( player != NULL )
		functions->SendClientMessage( player->nPlayerId, 0x007f16ff, "** pm >> %s", message );
}

void PrivMessageAll     ( const SQChar * message )
{
	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		if( functions->IsPlayerConnected( i ) )
			functions->SendClientMessage( i, 0x007f16ff, "** pm >> %s", message );
	}
}

void SendPlayerMessage  ( CPlayer * playerToFake, CPlayer * playerTo, const SQChar * message )
{
	if( playerToFake != NULL && playerTo != NULL )
		functions->SendClientMessage( playerTo->nPlayerId, 0x007f16ff, "** pm from %s >> %s", playerToFake->GetName().c_str(), message );
}

const SQChar* GetWeaponName(int weaponID)
{
	switch (weaponID)
	{
	case 0:  return _SC("Unarmed");
	case 1:  return _SC("Brass Knuckles");
	case 2:  return _SC("Screwdriver");
	case 3:  return _SC("Golf Club");
	case 4:  return _SC("Nightstick");
	case 5:  return _SC("Knife");
	case 6:  return _SC("Baseball Bat");
	case 7:  return _SC("Hammer");
	case 8:  return _SC("Meat Cleaver");
	case 9:  return _SC("Machete");
	case 10: return _SC("Katana");
	case 11: return _SC("Chainsaw");
	case 12: return _SC("Grenade");
	case 13: return _SC("Remote Detonation Grenade");
	case 14: return _SC("Tear Gas");
	case 15: return _SC("Molotov Cocktails");
	case 16: return _SC("Rocket");
	case 17: return _SC("Colt .45");
	case 18: return _SC("Python");
	case 19: return _SC("Pump-Action Shotgun");
	case 20: return _SC("SPAS-12 Shotgun");
	case 21: return _SC("Stubby Shotgun");
	case 22: return _SC("TEC-9");
	case 23: return _SC("Uzi");
	case 24: return _SC("Silenced Ingram");
	case 25: return _SC("MP5");
	case 26: return _SC("M4");
	case 27: return _SC("Ruger");
	case 28: return _SC("Sniper Rifle");
	case 29: return _SC("Laserscope Sniper Rifle");
	case 30: return _SC("Rocket Launcher");
	case 31: return _SC("Flamethrower");
	case 32: return _SC("M60");
	case 33: return _SC("Minigun");
	case 34: return _SC("Explosion");
	case 35: return _SC("Helicannon");
	case 36: return _SC("Camera");
	case 39: return _SC("Vehicle");
	case 41: return _SC("Explosion");
	case 42: return _SC("Driveby");
	case 43: return _SC("Drowned");
	case 44: return _SC("Fall");
	case 51: return _SC("Explosion");
	case 70: return _SC("Suicide");
	default: return _SC("Unknown");
	}
}

const SQChar* GetDistrictName(float x, float y)
{
	if (x > -1613.03f && y > 413.218f && x < -213.73f && y < 1677.32f)
		return _SC("Downtown Vice City");
	else if (x > 163.656f && y > -351.153f && x < 1246.03f && y < 1398.85f)
		return _SC("Vice Point");
	else if (x > -103.97f && y > -930.526f && x < 1246.03f && y < -351.153f)
		return _SC("Washington Beach");
	else if (x > -253.206f && y > -1805.37f && x < 1254.9f && y < -930.526f)
		return _SC("Ocean Beach");
	else if (x > -1888.21f && y > -1779.61f && x < -1208.21f && y < 230.39f)
		return _SC("Escobar International Airport");
	else if (x > -748.206f && y > -818.266f && x < -104.505f && y < -241.467f)
		return _SC("Starfish Island");
	else if (x > -213.73f && y > 797.605f && x < 163.656f && y < 1243.47f)
		return _SC("Prawn Island");
	else if (x > -213.73f && y > -241.429f && x < 163.656f && y < 797.605f)
		return _SC("Leaf Links");
	else if (x > -1396.76f && y > -42.9113f && x < -1208.21f && y < 230.39f)
		return _SC("Junkyard");
	else if (x > -1208.21f && y > -1779.61f && x < -253.206f && y < -898.738f)
		return _SC("Viceport");
	else if (x > -1208.21f && y > -898.738f && x < -748.206f && y < -241.467f)
		return _SC("Little Havana");
	else if (x > -1208.21f && y > -241.467f && x < -578.289f && y < 412.66f)
		return _SC("Little Haiti");
	else
		return _SC("Vice City");
}

const SQChar* GetSkinName(int skinID)
{
	switch (skinID)
	{
	case 0:   return _SC("Tommy Vercetti");
	case 1:   return _SC("Cop");
	case 2:   return _SC("SWAT");
	case 3:   return _SC("FBI");
	case 4:   return _SC("Army");
	case 5:   return _SC("Paramedic");
	case 6:   return _SC("Firefighter");
	case 7:   return _SC("Golf Guy #1");
	case 8:   return NULL;
	case 9:   return _SC("Bum Lady #1");
	case 10:  return _SC("Bum Lady #2");
	case 11:  return _SC("Punk #1");
	case 12:  return _SC("Lawyer");
	case 13:  return _SC("Spanish Lady #1");
	case 14:  return _SC("Spanish Lady #2");
	case 15:  return _SC("Cool Guy #1");
	case 16:  return _SC("Arabic Guy");
	case 17:  return _SC("Beach Lady #1");
	case 18:  return _SC("Beach Lady #2");
	case 19:  return _SC("Beach Guy #1");
	case 20:  return _SC("Beach Guy #2");
	case 21:  return _SC("Office Lady #1");
	case 22:  return _SC("Waitress #1");
	case 23:  return _SC("Food Lady");
	case 24:  return _SC("Prostitute #1");
	case 25:  return _SC("Bum Lady #3");
	case 26:  return _SC("Bum Guy #1");
	case 27:  return _SC("Garbageman #1");
	case 28:  return _SC("Taxi Driver #1");
	case 29:  return _SC("Haitian #1");
	case 30:  return _SC("Criminal #1");
	case 31:  return _SC("Hood Lady");
	case 32:  return _SC("Granny #1");
	case 33:  return _SC("Businessman #1");
	case 34:  return _SC("Church Guy");
	case 35:  return _SC("Club Lady");
	case 36:  return _SC("Church Lady");
	case 37:  return _SC("Pimp");
	case 38:  return _SC("Beach Lady #3");
	case 39:  return _SC("Beach Guy #3");
	case 40:  return _SC("Beach Lady #4");
	case 41:  return _SC("Beach Guy #4");
	case 42:  return _SC("Businessman #2");
	case 43:  return _SC("Prostitute #2");
	case 44:  return _SC("Bum Lady #4");
	case 45:  return _SC("Bum Guy #2");
	case 46:  return _SC("Haitian #2");
	case 47:  return _SC("Construction Worker #1");
	case 48:  return _SC("Punk #2");
	case 49:  return _SC("Prostitute #3");
	case 50:  return _SC("Granny #2");
	case 51:  return _SC("Punk #3");
	case 52:  return _SC("Businessman #3");
	case 53:  return _SC("Spanish Lady #3");
	case 54:  return _SC("Spanish Lady #4");
	case 55:  return _SC("Cool Guy #2");
	case 56:  return _SC("Businessman #4");
	case 57:  return _SC("Beach Lady #5");
	case 58:  return _SC("Beach Guy #5");
	case 59:  return _SC("Beach Lady #6");
	case 60:  return _SC("Beach Guy #6");
	case 61:  return _SC("Construction Worker #2");
	case 62:  return _SC("Golf Guy #2");
	case 63:  return _SC("Golf Lady");
	case 64:  return _SC("Golf Guy #3");
	case 65:  return _SC("Beach Lady #7");
	case 66:  return _SC("Beach Guy #7");
	case 67:  return _SC("Office Lady #2");
	case 68:  return _SC("Businessman #5");
	case 69:  return _SC("Businessman #6");
	case 70:  return _SC("Prostitute #2");
	case 71:  return _SC("Bum Lady #4");
	case 72:  return _SC("Bum Guy #3");
	case 73:  return _SC("Spanish Guy");
	case 74:  return _SC("Taxi Driver #2");
	case 75:  return _SC("Gym Lady");
	case 76:  return _SC("Gym Guy");
	case 77:  return _SC("Skate Lady");
	case 78:  return _SC("Skate Guy");
	case 79:  return _SC("Shopper #1");
	case 80:  return _SC("Shopper #2");
	case 81:  return _SC("Tourist #1");
	case 82:  return _SC("Tourist #2");
	case 83:  return _SC("Cuban #1");
	case 84:  return _SC("Cuban #2");
	case 85:  return _SC("Haitian #3");
	case 86:  return _SC("Haitian #4");
	case 87:  return _SC("Shark #1");
	case 88:  return _SC("Shark #2");
	case 89:  return _SC("Diaz Guy #1");
	case 90:  return _SC("Diaz Guy #2");
	case 91:  return _SC("DBP Security #1");
	case 92:  return _SC("DBP Security #2");
	case 93:  return _SC("Biker #1");
	case 94:  return _SC("Biker #2");
	case 95:  return _SC("Vercetti Guy #1");
	case 96:  return _SC("Vercetti Guy #2");
	case 97:  return _SC("Undercover Cop #1");
	case 98:  return _SC("Undercover Cop #2");
	case 99:  return _SC("Undercover Cop #3");
	case 100: return _SC("Undercover Cop #4");
	case 101: return _SC("Undercover Cop #5");
	case 102: return _SC("Undercover Cop #6");
	case 103: return _SC("Rich Guy");
	case 104: return _SC("Cool Guy #3");
	case 105: return _SC("Prostitute #3");
	case 106: return _SC("Prostitute #4");
	case 107: return _SC("Love Fist #1");
	case 108: return _SC("Ken Rosenburg");
	case 109: return _SC("Candy Suxx");
	case 110: return _SC("Hilary");
	case 111: return _SC("Love Fist #2");
	case 112: return _SC("Phil");
	case 113: return _SC("Rockstar Guy");
	case 114: return _SC("Sonny");
	case 115: return _SC("Lance");
	case 116: return _SC("Mercedes");
	case 117: return _SC("Love Fist #3");
	case 118: return _SC("Alex Shrub");
	case 119: return _SC("Lance (Cop)");
	case 120: return _SC("Lance");
	case 121: return _SC("Cortez");
	case 122: return _SC("Love Fist #4");
	case 123: return _SC("Columbian Guy #1");
	case 124: return _SC("Hilary (Robber)");
	case 125: return _SC("Mercedes");
	case 126: return _SC("Cam");
	case 127: return _SC("Cam (Robber)");
	case 128: return _SC("Phil (One Arm)");
	case 129: return _SC("Phil (Robber)");
	case 130: return _SC("Cool Guy #4");
	case 131: return _SC("Pizza Man");
	case 132: return _SC("Taxi Driver #1");
	case 133: return _SC("Taxi Driver #2");
	case 134: return _SC("Sailor #1");
	case 135: return _SC("Sailor #2");
	case 136: return _SC("Sailor #3");
	case 137: return _SC("Chef");
	case 138: return _SC("Criminal #2");
	case 139: return _SC("French Guy");
	case 140: return _SC("Garbageman #2");
	case 141: return _SC("Haitian #5");
	case 142: return _SC("Waitress #2");
	case 143: return _SC("Sonny Guy #1");
	case 144: return _SC("Sonny Guy #2");
	case 145: return _SC("Sonny Guy #3");
	case 146: return _SC("Columbian Guy #2");
	case 147: return _SC("Haitian #6");
	case 148: return _SC("Beach Guy #8");
	case 149: return _SC("Garbageman #3");
	case 150: return _SC("Garbageman #4");
	case 151: return _SC("Garbageman #5");
	case 152: return _SC("Tranny");
	case 153: return _SC("Thug #5");
	case 154: return _SC("SpandEx Guy #1");
	case 155: return _SC("SpandEx Guy #2");
	case 156: return _SC("Stripper #1");
	case 157: return _SC("Stripper #2");
	case 158: return _SC("Stripper #3");
	case 159: return _SC("Store Clerk");
	case 160: return NULL;
	case 161: return _SC("Tommy with Suit");
	case 162: return _SC("Worker Tommy");
	case 163: return _SC("Golfer Tommy");
	case 164: return _SC("Cuban Tommy");
	case 165: return _SC("VCPD Tommy");
	case 166: return _SC("Bank Robber Tommy");
	case 167: return _SC("Street Tommy");
	case 168: return _SC("Mafia Tommy");
	case 169: return _SC("Jogger Tommy #1");
	case 170: return _SC("Jogger Tommy #2");
	case 171: return _SC("Guy With Suit #1");
	case 172: return _SC("Guy With Suit #3");
	case 173: return _SC("Prostitute #5");
	case 174: return _SC("Rico");
	case 175: return _SC("Prostitute #3");
	case 176: return _SC("Club Lady");
	case 177: return _SC("Prostitute #2");
	case 178: return _SC("Skull T-Shirt Guy");
	case 179: return _SC("Easter Egg Tommy");
	case 180: return _SC("Diaz Gangster #1");
	case 181: return _SC("Diaz Gangster #2");
	case 182: return _SC("Hood Lady");
	case 183: return _SC("Punk #1");
	case 184: return _SC("Tray Lady");
	case 185: return _SC("Kent Paul");
	case 186: return _SC("Taxi Driver #1");
	case 187: return _SC("Deformed Ken Rosenberg");
	case 188: return _SC("Deformed Woman");
	case 189: return _SC("Deformed Man");
	case 190: return _SC("Deformed Cortez");
	case 191: return _SC("Deformed Lance Vance");
	case 192: return _SC("Thief #1");
	case 193: return _SC("Thief #2");
	case 194: return _SC("Thief #3");
	default:  return NULL;
	}
}

void LoadVCMPModule( const SQChar * name )
{
	OutputWarning( "LoadModule() cannot be used by scripts. Specify plugins in server.cfg" );
}

int GetWeaponID(const SQChar* name)
{
	if (name == NULL || strlen(name) < 1)
		return 0;

	char char1, char2, char3;
	char1 = 0;
	char2 = 0;
	char3 = 0;

	char1 = (char)tolower(name[0]);

	if (strlen(name) >= 3)
	{
		char3 = (char)tolower(name[2]);
		char2 = (char)tolower(name[1]);
	}
	else if (strlen(name) >= 2)
	{
		char2 = (char)tolower(name[1]);
	}

	switch (char1)
	{
		// [F]ists, [F]lamethrower
	case 'f':
	{
		// [Fi]sts
		if (char2 && char2 == 'i')
			return 0;

		// Default to flamethrower
		else
			return 31;

		break;
	}

	case 'b':
		// [Br]ass Knuckles
		if (char2 && char2 == 'r')
			return 1;

		// [Ba]seball Bat
		return 6;

		// [S]crewdriver, [S]hotgun, [S]PAS-12 Shotgun, [S]tubby/[S]awnoff Shotgun, [Si]lenced Ingram [S]niper Rifle
	case 's':
	{
		switch (char2)
		{
			// [Sc]rewdriver
		case 'c':
			return 2;

			// [Sh]otgun
		case 'h':
			return 19;

			// [SP]AS-12 / [Sp]az Shotgun
		case 'p':
			return 20;

			// [St]ubby / [Sa]wnoff Shotgun
		case 't':
		case 'a':
			return 21;

			// [Si]lenced Ingram
		case 'i':
			return 24;

			// [Sn]iper
		case 'n':
			return 28;

			// Default to screwdriver
		default:
			return 2;
		}
	}

	// [G]olf Club, [G]renade
	case 'g':
	{
		// [Go]lf Club
		if (char2 && char2 == 'o')
			return 3;

		// Grenades being more popular in servers, default to grenade
		else
			return 12;

		break;
	}

	// [N]ightstick
	case 'n':
		return 4;

		// [K]nife, [K]atana
	case 'k':
	{
		if (char2 && char2 == 'n')
		{
			// [Kn]ife
			if (char3 == 'i')
				return 5;
			// [Knu]ckles
			else if (char3 == 'u')
				return 1;
		}

		// Default to katana
		else
			return 10;

		break;
	}

	// [H]ammer
	case 'h':
		return 7;

		// [M]eat Cleaver, [M]achete, [M]olotov Cocktail, [M]P5, [M]4, [M]60, [M]inigun
	case 'm':
	{
		switch (char2)
		{
			// [Me]at Cleaver
		case 'e':
			return 8;

			// [Ma]chete
		case 'a':
			return 9;

			// [Mo]lotov Cocktail
		case 'o':
			return 15;

			// [MP]5
		case 'p':
			return 25;

			// [M4]
		case '4':
			return 26;

			// [M6]0
		case '6':
			return 32;

			// [Mi]inigun
		case 'i':
			return 33;

			// Default to M4
		default:
			return 26;
		}

		break;
	}

	// [C]leaver, [C]hainsaw, [C]olt .45
	case 'c':
	{
		switch (char2)
		{
			// [Cl]eaver
		case 'l':
			return 8;

			// [Ch]ainsaw
		case 'h':
			return 11;

			// Default to Colt .45
		default:
			return 17;
		}

		break;
	}

	// [R]emote Detonation Grenade, [R]uger, [R]ocket Launcher / [R]PG
	case 'r':
	{
		switch (char2)
		{
			// [Re]mote Detonation Grenade
		case 'e':
			return 13;

			// [Ro]cket Launcher, [RP]G
		case 'o':
		case 'p':
			return 30;

			// [Ru]ger
		case 'u':
			return 27;

			// Default to ruger
		default:
			return 27;
		}
	}

	// [T]ear Gas, [T]EC-9
	case 't':
	{
		// Both of them have E as a second character anyways.
		if (char2)
		{
			// [Tea]r Gas
			if (char3 && char3 == 'a')
				return 14;

			// Default to TEC-9
			else
				return 22;
		}

		// Default to TEC-9 if no second character exists.
		else
			return 22;

		break;
	}

	// [P]ython
	case 'p':
		return 18;

		// [U]zi
	case 'u':
		return 23;

		// [I]ngram
	case 'i':
		return 24;

		// [L]aserscope Sniper
	case 'l':
		return 29;

		// Default to fists
	default:
		return 255;
	}

	return 255;
}

#if defined(_WIN32) || defined(WIN32)
extern "C" __declspec(dllimport) unsigned long __stdcall GetTickCount(void);
#endif

DWORD SQGetTickCount(void)
{
	#if defined(_WIN32) || defined(WIN32)
		return GetTickCount();
	#else
		struct timespec now;

		if (clock_gettime(CLOCK_MONOTONIC, &now))
			return 0;

		return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
	#endif
}

float DistanceFromPoint(float x1, float y1, float x2, float y2)
{
	float matrixX = powf((x2 - x1), 2.0f);
	float matrixY = powf((y2 - y1), 2.0f);
	float matrixF = sqrtf(matrixX + matrixY);
	return matrixF;
}

// This is a crude implementation to be replaced later.
int BindKey( bool onKeyDown, int key, int key2, int key3 )
{
	int keyslot = functions->GetKeyBindUnusedSlot();
	functions->RegisterKeyBind( keyslot, onKeyDown, key, key2, key3 );

	return keyslot;
}

bool RemoveKeybind( int nKeybindId ) { return (functions->RemoveKeyBind( nKeybindId ) ? true : false); }
void RemoveAllKeybinds( void ) { functions->RemoveAllKeyBinds(); }

void ReloadScripts( void )
{
	// Are we allowed to reload?
	if( pCore->IsReloadingAllowed() )
	{
		// Let's not reload the scripts for now
		pCore->ChangeReloadPermission(false);

		// Get rid of ALL declared timers
		pCore->DropAllTimers();

		// Clean the world up
		pCore->CleanWorld();

		// Release the old root table to free old binds
		Sqrat::RootTable(v).Release();

		// Reload the VM and all entities
		pCore->LoadVM();

		// Re-parse the config and reload the script
		pCore->LoadScript();

		// And now we dance
		OutputMessage( "Scripts reloaded." );
	}
	else
		OutputError( "The server refused to reload. Is this an infinite loop?" );
}

int GetVehicleModelFromName(SQChar* name)
{
	if (!name) return -1;

	// Convertido para std::string (ANSI)
	std::string lowername(name);

	for (auto& ch : lowername) {
		ch = (char)tolower(ch);
	}

	int res = -1;
	size_t len = lowername.length();

	if (len == 0) return -1;

	switch (lowername[0])
	{
	case 'l':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'a') res = 130; // landstalker
			else if (lowername[1] == 'i') res = 133; // linerunner
			else if (lowername[1] == 'o') res = 201; // love fist
		}
		break;
	}
	case 'i':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'd') res = 131; // idaho
			else if (lowername[1] == 'n') res = 141; // infernus
		}
		break;
	}
	case 's':
	{
		if (len >= 3)
		{
			if (lowername[1] == 't')
			{
				if (lowername[2] == 'i') res = 132; // stinger
				else if (lowername[2] == 'r') res = 139; // stretch
				else if (lowername[2] == 'a') res = 169; // stallion
			}
			else if (lowername[1] == 'e')
			{
				if (lowername[2] == 'c') res = 158; // securicar
				else if (lowername[2] == 'a') res = 177; // sea sparrow
				else if (len > 8) res = 174; // sentinel xs
				else res = 135; // sentinel
			}
			else if (lowername[1] == 'q') res = 176; // squalo
			else if (lowername[1] == 'p')
			{
				if (lowername[2] == 'e') res = 182; // speeder
				else if (lowername[2] == 'a')
				{
					if (len >= 4)
					{
						if (lowername[3] == 'r') res = 199; // sparrow
						else if (lowername[3] == 'n') res = 213; // spand express
					}
				}
			}
			else if (lowername[1] == 'k') res = 190; // skimmer
			else if (lowername[1] == 'a')
			{
				if (lowername[2] == 'n')
				{
					if (len >= 4)
					{
						if (lowername[3] == 'c') res = 198; // sanchez
						else if (lowername[3] == 'd') res = 225; // sandking
					}
				}
				else if (lowername[2] == 'b' && len > 5) res = 206; // sabre turbo
				else if (lowername[2] == 'b') res = 205; // sabre
			}
		}
		break;
	}
	case 'p':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'e') res = 134; // perennial
			else if (lowername[1] == 'o')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'n') res = 143; // pony
					else if (len > 6) res = 227; // police maverick
					else res = 156; // police
				}
			}
			else if (lowername[1] == 'r') res = 160; // predator
			else if (lowername[1] == 'a')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'c') res = 173; // packer
					else if (lowername[2] == 't') res = 200; // patriot
				}
			}
			else if (lowername[1] == 'i') res = 178; // pizza boy
			else if (lowername[1] == 'c') res = 191; // pcj-600
			else if (lowername[1] == 'h') res = 207; // phoenix
		}
		break;
	}
	case 'r':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'i') res = 136; // rio
			else if (lowername[1] == 'h') res = 162; // rhino
			else if (lowername[1] == 'u') res = 170; // rumpo
			else if (lowername[1] == 'c')
			{
				if (len >= 3 && lowername[2] == 'b')
				{
					if (len >= 5)
					{
						if (lowername[4] == 'n') res = 171; // rcbandit
						else if (lowername[4] == 'r') res = 194; // rcbaron
					}
				}
				else if (len >= 3 && lowername[2] == 'r') res = 195; // rcraider
				else if (len >= 3 && lowername[2] == 'g') res = 231; // rctiger
			}
			else if (lowername[1] == 'o') res = 172; // romero's hearse
			else if (lowername[1] == 'e')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'e') res = 183; // reefer
					else if (lowername[2] == 'g') res = 209; // regina
				}
			}
			else if (lowername[1] == 'a') res = 219; // rancher
		}
		break;
	}
	case 'f':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'i') res = 137; // firetruck
			else if (lowername[1] == 'b')
			{
				if (len >= 5)
				{
					if (lowername[4] == 'w') res = 147; // fbi washington
					else if (lowername[4] == 'r') res = 220; // fbi rancher
				}
			}
			else if (lowername[1] == 'l') res = 185; // flatbed
			else if (lowername[1] == 'a') res = 192; // faggio
			else if (lowername[1] == 'r') res = 193; // freeway
		}
		break;
	}
	case 't':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'r')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'a') res = 138; // trashmaster
					else if (lowername[2] == 'o') res = 184; // tropic
				}
			}
			else if (lowername[1] == 'o') res = 189; // top fun
			else if (lowername[1] == 'a') res = 150; // taxi
		}
		break;
	}
	case 'm':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'a')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'n') res = 140; // manana
					else if (lowername[2] == 'r') res = 214; // marquis
					else if (lowername[2] == 'v') res = 217; // maverick
				}
			}
			else if (lowername[1] == 'u') res = 144; // mule
			else if (lowername[1] == 'r') res = 153; // mr. whoopee
			else if (lowername[1] == 'e') res = 230; // mesa grande
			else if (lowername[1] == 'o') res = 148; // moonbeam
		}
		break;
	}
	case 'c':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'h') res = 145; // cheetah
			else if (lowername[1] == 'u')
			{
				if (len >= 7)
				{
					if (lowername[6] == 'h') res = 164; // cuban hermes
					else if (lowername[6] == 'j') res = 223; // cuban jetmax
				}
			}
			else if (lowername[1] == 'o')
			{
				if (len >= 3 && lowername[2] == 'm') res = 210; // comet
				else if (len >= 4)
				{
					if (lowername[3] == 'c') res = 167; // coach
					else if (lowername[3] == 's') res = 202; // coast guard
				}
			}
			else if (lowername[1] == 'a')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'b') res = 168; // cabbie
					else if (lowername[2] == 'd') res = 187; // caddy
				}
			}
		}
		break;
	}
	case 'a':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'm') res = 146; // ambulance
			else if (lowername[1] == 'n') res = 166; // angel
			else if (lowername[1] == 'd') res = 175; // admiral
			else if (lowername[1] == 'i') res = 180; // airtrain
		}
		break;
	}
	case 'e':
	{
		if (len >= 2)
		{
			if (lowername[1] == 's') res = 149; // esperanto
			else if (lowername[1] == 'n') res = 157; // enforcer
		}
		break;
	}
	case 'w':
	{
		if (len >= 3)
		{
			if (lowername[2] == 's') res = 151; // washington
			else if (lowername[2] == 'l') res = 208; // walton
		}
		break;
	}
	case 'b':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'o')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'b') res = 152; // bobcat
					else if (lowername[2] == 'x') res = 228; // boxville
				}
			}
			else if (lowername[1] == 'a')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'r') res = 163; // barracks ol
					else if (lowername[2] == 'g') res = 215; // baggage handler
					else if (lowername[2] == 'n') res = 159; // banshee
				}
			}
			else if (lowername[1] == 'u')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'r') res = 212; // burrito
					else if (lowername[2] == 's') res = 161; // bus
				}
			}
			else if (lowername[1] == 'l')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'i') res = 226; // blista compact
					else if (lowername[2] == 'o')
					{
						res = 234; // bloodra
						if (lowername == "bloodrb") res = 235; // bloodrb
					}
				}
			}
			else if (lowername[1] == 'e') res = 229; // benson
			else if (lowername[1] == 'f') res = 154; // bf injection
		}
		break;
	}
	case 'h':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'u') res = 155; // hunter
			else if (lowername[1] == 'e')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'r') res = 204; // hermes
					else if (lowername[2] == 'l') res = 165; // helicopter
				}
			}
			else if (lowername[1] == 'o')
			{
				res = 224; // hotring racer
				if (lowername == "hotrina") res = 232; // hotrina
				else if (lowername == "hotrinb") res = 233; // hotrinb
			}
		}
		break;
	}
	case 'g':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'a') res = 179; // gang burrito
			else if (lowername[1] == 'l') res = 196; // glendale
			else if (lowername[1] == 'r') res = 222; // greenwood
		}
		break;
	}
	case 'd':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'e') res = 211; // deluxo
			else if (lowername[1] == 'i') res = 203; // dinghy
		}
		break;
	}
	case 'y': res = 186; break; // yankee
	case 'z': res = 188; break; // zebra cab
	case 'k': res = 216; break; // kaufman cab
	case 'o': res = 197; break; // oceanic
	case 'j': res = 223; break; // cuban jetmax
	case 'v':
	{
		if (len >= 2)
		{
			if (lowername[1] == 'c') res = 218; // vcn maverick
			else if (lowername[1] == 'i')
			{
				if (len >= 3)
				{
					if (lowername[2] == 'r') res = 221; // virgo
					else if (lowername[2] == 'c') res = 236; // vicechee
				}
			}
			else if (lowername[1] == 'o') res = 142; // voodoo
		}
		break;
	}
	}

	return res;
}

const SQChar* GetVehicleNameFromModel(int model)
{
	switch (model)
	{
	case 130: return _SC("Landstalker");
	case 131: return _SC("Idaho");
	case 132: return _SC("Stinger");
	case 133: return _SC("Linerunner");
	case 134: return _SC("Perennial");
	case 135: return _SC("Sentinel");
	case 136: return _SC("Rio");
	case 137: return _SC("Firetruck");
	case 138: return _SC("Trashmaster");
	case 139: return _SC("Stretch");
	case 140: return _SC("Manana");
	case 141: return _SC("Infernus");
	case 142: return _SC("Voodoo");
	case 143: return _SC("Pony");
	case 144: return _SC("Mule");
	case 145: return _SC("Cheetah");
	case 146: return _SC("Ambulance");
	case 147: return _SC("FBI Washington");
	case 148: return _SC("Moonbeam");
	case 149: return _SC("Esperanto");
	case 150: return _SC("Taxi");
	case 151: return _SC("Washington");
	case 152: return _SC("Bobcat");
	case 153: return _SC("Mr. Whoopee");
	case 154: return _SC("BF Injection");
	case 155: return _SC("Hunter");
	case 156: return _SC("Police");
	case 157: return _SC("Enforcer");
	case 158: return _SC("Securicar");
	case 159: return _SC("Banshee");
	case 160: return _SC("Predator");
	case 161: return _SC("Bus");
	case 162: return _SC("Rhino");
	case 163: return _SC("Barracks OL");
	case 164: return _SC("Cuban Hermes");
	case 165: return _SC("Helicopter");
	case 166: return _SC("Angel");
	case 167: return _SC("Coach");
	case 168: return _SC("Cabbie");
	case 169: return _SC("Stallion");
	case 170: return _SC("Rumpo");
	case 171: return _SC("RC Bandit");
	case 172: return _SC("Romero's Hearse");
	case 173: return _SC("Packer");
	case 174: return _SC("Sentinel XS");
	case 175: return _SC("Admiral");
	case 176: return _SC("Squalo");
	case 177: return _SC("Sea Sparrow");
	case 178: return _SC("Pizza Boy");
	case 179: return _SC("Gang Burrito");
	case 180: return _SC("Airtrain");
	case 181: return _SC("Deaddodo");
	case 182: return _SC("Speeder");
	case 183: return _SC("Reefer");
	case 184: return _SC("Tropic");
	case 185: return _SC("Flatbed");
	case 186: return _SC("Yankee");
	case 187: return _SC("Caddy");
	case 188: return _SC("Zebra Cab");
	case 189: return _SC("Top Fun");
	case 190: return _SC("Skimmer");
	case 191: return _SC("PCJ-600");
	case 192: return _SC("Faggio");
	case 193: return _SC("Freeway");
	case 194: return _SC("RC Baron");
	case 195: return _SC("RC Raider");
	case 196: return _SC("Glendale");
	case 197: return _SC("Oceanic");
	case 198: return _SC("Sanchez");
	case 199: return _SC("Sparrow");
	case 200: return _SC("Patriot");
	case 201: return _SC("Love Fist");
	case 202: return _SC("Coast Guard");
	case 203: return _SC("Dinghy");
	case 204: return _SC("Hermes");
	case 205: return _SC("Sabre");
	case 206: return _SC("Sabre Turbo");
	case 207: return _SC("Phoenix");
	case 208: return _SC("Walton");
	case 209: return _SC("Regina");
	case 210: return _SC("Comet");
	case 211: return _SC("Deluxo");
	case 212: return _SC("Burrito");
	case 213: return _SC("Spand Express");
	case 214: return _SC("Marquis");
	case 215: return _SC("Baggage Handler");
	case 216: return _SC("Kaufman Cab");
	case 217: return _SC("Maverick");
	case 218: return _SC("VCN Maverick");
	case 219: return _SC("Rancher");
	case 220: return _SC("FBI Rancher");
	case 221: return _SC("Virgo");
	case 222: return _SC("Greenwood");
	case 223: return _SC("Cuban Jetmax");
	case 224: return _SC("Hotring Racer #1");
	case 225: return _SC("Sandking");
	case 226: return _SC("Blista Compact");
	case 227: return _SC("Police Maverick");
	case 228: return _SC("Boxville");
	case 229: return _SC("Benson");
	case 230: return _SC("Mesa Grande");
	case 231: return _SC("RC Goblin");
	case 232: return _SC("Hotring Racer #2");
	case 233: return _SC("Hotring Racer #3");
	case 234: return _SC("Bloodring Banger #1");
	case 235: return _SC("Bloodring Banger #2");
	case 236: return _SC("Vice Squad Cheetah");
	default:  return NULL;
	}
}

void SetKillDelay( int delay ) { functions->SetKillCommandDelay( delay ); }
int GetKillDelay( void ) { return functions->GetKillCommandDelay(); }

void DestroyBlip( int blipID ) { functions->DestroyCoordBlip( blipID ); }
int CreateBlip( int world, Vector * pos, int scale, RGBa color, int nSpriteId )
{
	return functions->CreateCoordBlip( -1, world, pos->x, pos->y, pos->z, scale, color.toUInt(), nSpriteId );
}

void DestroyRadioStream(int radioID) { functions->RemoveRadioStream(radioID); }
int CreateRadioStream(const char * radioName, const char * radioURL, bool bCanSelect)
{
	return functions->AddRadioStream(-1, radioName, radioURL, bCanSelect);
}
void CreateRadioStreamWithID(int streamID, const char * radioName, const char * radioURL, bool bCanSelect)
{
	functions->AddRadioStream(streamID, radioName, radioURL, bCanSelect);
}

bool SetWeaponDataValue   ( int weaponID, int fieldID, double value ) { return (functions->SetWeaponDataValue( weaponID, fieldID, value ) ? true : false); }
double GetWeaponDataValue ( int weaponID, int fieldID ) { return functions->GetWeaponDataValue( weaponID, fieldID ); }
bool ResetWeaponDataValue ( int weaponID, int fieldID ) { return (functions->ResetWeaponDataValue( weaponID, fieldID ) ? true : false); }
bool IsWeaponDataModified ( int weaponID, int fieldID ) { return (functions->IsWeaponDataValueModified( weaponID, fieldID ) ? true : false); }
bool ResetWeaponData      ( int weaponID ) { return (functions->ResetWeaponData( weaponID ) ? true : false); }
void ResetAllWeaponData   () { return functions->ResetAllWeaponData(); }

bool IsNum(const SQChar* line)
{
	if (!line || line[0] == '\0')
	{
		return false;
	}

	while (std::isspace(static_cast<unsigned char>(*line)))
	{
		line++;
	}

	if (*line == '\0')
	{
		return false;
	}

	SQChar* pEnd = nullptr;

	(void)std::strtol(line, &pEnd, 10);

	if (pEnd == line)
	{
		return false;
	}

	while (pEnd && std::isspace(static_cast<unsigned char>(*pEnd)))
	{
		pEnd++;
	}

	return pEnd && *pEnd == '\0';
}

int GetVehicleCount()
{
	int count = 0;
	for (int i = 0; i < MAX_VEHICLES; i++)
	if (functions->CheckEntityExists(vcmpEntityPoolVehicle, i) > 0)
		count++;

	return count;
}

int GetPickupCount()
{
	int count = 0;
	for (int i = 0; i < MAX_PICKUPS; i++)
	if (functions->CheckEntityExists(vcmpEntityPoolPickup, i) > 0)
		count++;

	return count;
}

int GetObjectCount()
{
	int count = 0;
	for (int i = 0; i < MAX_OBJECTS; i++)
	if (functions->CheckEntityExists(vcmpEntityPoolObject, i) > 0)
		count++;

	return count;
}

int GetPlayers()
{
	int count = 0;
	const int maxPlayers = functions->GetMaxPlayers();

	for (int i = 0; i < maxPlayers; i++)
	{
		if (functions->IsPlayerConnected(i))
		{
			count++;
		}
	}

	return count;
}

SQInteger release_hook( SQUserPointer p, SQInteger size ) { return 1; }

SQInteger FindPlayer(HSQUIRRELVM v)
{
	const SQInteger top = sq_gettop(v);
	if (top < 2)
	{
		return sq_throwerror(v, _SC("Unexpected number of parameters for FindPlayer [integer or string]"));
	}

	CPlayer* pPlayer = nullptr;
	const SQInteger argType = sq_gettype(v, 2);

	if (argType == OT_INTEGER)
	{
		SQInteger playerID = -1;
		sq_getinteger(v, 2, &playerID);
		if (playerID >= 0 && playerID < MAX_PLAYERS)
		{
			pPlayer = pCore->RetrievePlayer(static_cast<int>(playerID));
		}
	}
	else if (argType == OT_STRING)
	{
		const SQChar* pName = nullptr;
		if (SQ_SUCCEEDED(sq_getstring(v, 2, &pName)) && pName)
		{
			std::string searchName(pName);
			std::transform(searchName.begin(), searchName.end(), searchName.begin(),
				[](unsigned char c) { return std::tolower(c); });

			const int maxPlayers = static_cast<int>(functions->GetMaxPlayers());
			char ansiNameBuf[64];

			for (int i = 0; i < maxPlayers; ++i)
			{
				if (functions->IsPlayerConnected(i))
				{
					functions->GetPlayerName(i, ansiNameBuf, 64);
					ansiNameBuf[63] = '\0';

					std::string currentName(ansiNameBuf);
					std::transform(currentName.begin(), currentName.end(), currentName.begin(),
						[](unsigned char c) { return std::tolower(c); });

					if (currentName.find(searchName) != std::string::npos)
					{
						pPlayer = pCore->RetrievePlayer(i);
						break;
					}
				}
			}
		}
	}
	else
	{
		return sq_throwerror(v, _SC("Unexpected argument in FindPlayer: must be integer or string"));
	}

	if (pPlayer == nullptr)
		sq_pushnull(v);
	else
		Sqrat::ClassType<CPlayer>().PushInstance(v, pPlayer);

	return 1;
}

struct AreaPoints
{
	float x, y;

	AreaPoints(void) : x(0.0f), y(0.0f) {};
	AreaPoints(float fX, float fY) : x(fX), y(fY) {};
};

inline size_t Internal_NumTok(const char* szText, const char cDelimiter)
{
	if (!*szText) return 0;

	size_t iCounter = 1;
	char* p = (char*)szText;

	if (*p == cDelimiter)
		iCounter = 0;

	while (*p)
	{
		if (*p == cDelimiter)
		{
			if ((*++p) && (*p != cDelimiter)) iCounter++;
		}
		else p++;
	}

	return iCounter;
}

inline bool Internal_InPoly(float fX, float fY,
	const unsigned int uiPoints, const AreaPoints fPoints[])
{
	// http://sidvind.com/wiki/Point-in-polygon:_Jordan_Curve_Theorem

	// The points creating the polygon
	float x1, x2;

	// How many times the ray crosses a line segment
	int crossings = 0;

	// Iterate through each line
	for (unsigned int i = 0; i < uiPoints; i++)
	{
		// This is done to ensure that we get the same result when
		// the line goes from left to right and right to left.
		if (fPoints[i].x < fPoints[(i + 1) % uiPoints].x)
		{
			x1 = fPoints[i].x;
			x2 = fPoints[(i + 1) % uiPoints].x;
		}
		else
		{
			x1 = fPoints[(i + 1) % uiPoints].x;
			x2 = fPoints[i].x;
		}

		// First check if the ray is able to cross the line
		if (fX > x1 && fX <= x2 && (fY < fPoints[i].y || fY <= fPoints[(i + 1) % uiPoints].y))
		{
			static const float eps = 0.000001f;

			// Calculate the equation of the line
			float dx = fPoints[(i + 1) % uiPoints].x - fPoints[i].x;
			float dy = fPoints[(i + 1) % uiPoints].y - fPoints[i].y;
			float k;

			if (fabs(dx) < eps)
				k = static_cast<float>(0xffffffff);
			else
				k = dy / dx;

			float m = fPoints[i].y - k * fPoints[i].x;

			// Find if the ray crosses the line
			float y2 = k * fX + m;
			if (fY <= y2)
				crossings++;
		}
	}

	if (crossings % 2 == 1)
		return true;

	return false;
}

SQInteger InPoly(HSQUIRRELVM v)
{
	const SQInteger iArgs = sq_gettop(v);

	if (iArgs >= 9 && (iArgs - 1) % 2 == 0)
	{
		SQFloat fX = 0.0f, fY = 0.0f;
		sq_getfloat(v, 2, &fX);
		sq_getfloat(v, 3, &fY);

		AreaPoints areaPoints[128];
		const unsigned int uiPoints = static_cast<unsigned int>((iArgs - 3) / 2);
		const unsigned int maxPoints = (uiPoints > 128) ? 128 : uiPoints;

		SQFloat fPointX = 0.0f, fPointY = 0.0f;
		for (unsigned int ui = 0; ui < maxPoints; ui++)
		{
			sq_getfloat(v, 4 + (ui * 2), &fPointX);
			sq_getfloat(v, 5 + (ui * 2), &fPointY);

			areaPoints[ui].x = (float)fPointX;
			areaPoints[ui].y = (float)fPointY;
		}

		bool bRet = Internal_InPoly((float)fX, (float)fY, maxPoints, areaPoints);
		sq_pushbool(v, bRet);
		return 1;
	}
	else if (iArgs == 4)
	{
		const SQInteger type4 = sq_gettype(v, 4);

		if (type4 == OT_STRING)
		{
			SQFloat fX = 0.0f, fY = 0.0f;
			sq_getfloat(v, 2, &fX);
			sq_getfloat(v, 3, &fY);

			const SQChar* szSqParams = nullptr;
			sq_getstring(v, 4, &szSqParams);

			if (!szSqParams) return 0;

			char* pBuffer = sq_strdup(szSqParams);
			if (!pBuffer) return 0;

			AreaPoints areaPoints[128];
			unsigned int uiPoints = 0;

			char* nextToken = nullptr;
			char* token = sq_strtok(pBuffer, ",", &nextToken);

			while (token != nullptr && uiPoints < 128)
			{
				areaPoints[uiPoints].x = static_cast<float>(std::strtod(token, nullptr));

				token = sq_strtok(nullptr, ",", &nextToken);
				if (!token) break;

				areaPoints[uiPoints].y = static_cast<float>(std::strtod(token, nullptr));
				uiPoints++;

				token = sq_strtok(nullptr, ",", &nextToken);
			}

			free(pBuffer);

			if (uiPoints >= 3)
			{
				bool bRet = Internal_InPoly((float)fX, (float)fY, uiPoints, areaPoints);
				sq_pushbool(v, bRet);
				return 1;
			}
		}
		else if (type4 == OT_ARRAY)
		{
			SQFloat fX = 0.0f, fY = 0.0f;
			sq_getfloat(v, 2, &fX);
			sq_getfloat(v, 3, &fY);

			SQObject obj;
			sq_getstackobj(v, 4, &obj);

			Sqrat::Array arr(Sqrat::Object(obj, v));
			AreaPoints areaPoints[128];

			const size_t totalElements = arr.GetSize() / 2;
			const size_t uiPoints = totalElements > 128 ? 128 : totalElements;

			if (uiPoints >= 3)
			{
				for (unsigned int ui = 0; ui < uiPoints; ui++)
				{
					areaPoints[ui].x = arr[2 * ui].Cast<float>();
					areaPoints[ui].y = arr[2 * ui + 1].Cast<float>();
				}

				bool bRet = Internal_InPoly(
					static_cast<float>(fX),
					static_cast<float>(fY),
					static_cast<unsigned int>(uiPoints),
					areaPoints
				);
				sq_pushbool(v, bRet);
				return 1;
			}
		}
	}

	return sq_throwerror(v, _SC("Unexpected number or types of parameters for InPoly"));
}

SQInteger NewTimer(HSQUIRRELVM v)
{
	const SQInteger top = sq_gettop(v);
	if (top < 4)
		return sq_throwerror(v, _SC("Usage: NewTimer(string funcName, float/int interval, int repeats, ...)"));

	const SQChar* pFuncName = nullptr;
	if (sq_gettype(v, 2) != OT_STRING || SQ_FAILED(sq_getstring(v, 2, &pFuncName)))
		return sq_throwerror(v, _SC("Argument 1: Function name must be a string."));

	SQFloat fInterval = 0.0f;
	SQObjectType intervalType = sq_gettype(v, 3);
	if (intervalType == OT_INTEGER) {
		SQInteger i; sq_getinteger(v, 3, &i);
		fInterval = static_cast<SQFloat>(i);
	}
	else if (intervalType == OT_FLOAT) {
		sq_getfloat(v, 3, &fInterval);
	}
	else {
		return sq_throwerror(v, _SC("Argument 2: Interval must be an integer or float."));
	}

	SQInteger maxPulses = 0;
	if (sq_gettype(v, 4) != OT_INTEGER)
		return sq_throwerror(v, _SC("Argument 3: Repeats must be an integer."));

	sq_getinteger(v, 4, &maxPulses);
	if (maxPulses < 0)
		return sq_throwerror(v, _SC("Argument 3: Repeats must be >= 0 (0 for infinite)."));

	Sqrat::Object funcObj = Sqrat::RootTable(v).GetSlot(pFuncName);
	if (funcObj.IsNull())
		return sq_throwerror(v, _SC("The given timer callback function does not exist in RootTable."));

	CTimer* pTimer = new CTimer();
	pTimer->functionName = pFuncName;
	pTimer->funcObj = funcObj;
	pTimer->intervalInTicks = fInterval;
	pTimer->maxNumberOfPulses = static_cast<unsigned int>(maxPulses);

	for (SQInteger i = 5; i <= top; ++i) {
		HSQOBJECT raw;
		sq_resetobject(&raw);
		if (SQ_SUCCEEDED(sq_getstackobj(v, i, &raw))) {
			pTimer->params.emplace_back(raw, v);
		}
	}

	Sqrat::ClassType<CTimer>::PushInstance(v, pTimer);
	sq_setreleasehook(v, -1, &CTimer::ReleaseHook);

	HSQOBJECT instObj;
	sq_resetobject(&instObj);
	sq_getstackobj(v, -1, &instObj);
	Sqrat::Object timerHandle(instObj, v);

	pCore->AddTimer(pTimer, timerHandle);

	return 1;
}

void SetVehiclesForcedRespawnHeight(SQFloat height) { functions->SetVehiclesForcedRespawnHeight(height); }
SQFloat GetVehiclesForcedRespawnHeight(void) { return functions->GetVehiclesForcedRespawnHeight(); }

// These functions are for compatibility, but will be deprecated
DWORD GetTime( void )
{
	OutputWarning( "GetTime is deprecated and may be removed in the future.\n"
		"          Please use Squirrel's time() function instead." );

	return static_cast<DWORD>( time( NULL ) );
}

const SQChar* GetFullTime(void)
{
	OutputWarning("GetFullTime is deprecated and may be removed in the future.\n"
		"          Please use Squirrel's date() function instead.");

	static SQChar date[96];
	time_t rawtime;
	std::time(&rawtime);

	struct tm timeinfo;
	#if defined(_WIN32) || defined(_WIN64)
		localtime_s(&timeinfo, &rawtime);
	#else
		localtime_r(&rawtime, &timeinfo);
	#endif

	std::strftime(date, 96, "%A, %B %d, %Y, %H:%M:%S %Z", &timeinfo);

	return date;
}

// These functions do nothing and exist for the sole purpose of avoiding compiler errors
extern HSQAPI sq;
SQInteger SetAmmuWeapon( HSQUIRRELVM v ) { OutputWarning( "SetAmmuWeapon does not exist in 0.4. Ammunations must be scripted." ); return 0; }
SQInteger IsAmmuWeaponEnabled(HSQUIRRELVM v) { OutputWarning("IsAmmuWeaponEnabled does not exist in 0.4. Ammunations must be scripted."); sq->pushbool(v, false);  return 1; }
SQInteger SetAmmuWeaponEnabled( HSQUIRRELVM v ) { OutputWarning( "SetAmmuWeaponEnabled does not exist in 0.4. Ammunations must be scripted." ); return 0; }

bool GetFallEnabled(void)
{
	OutputWarning("GetFallEnabled has no effect.");
	return false;
}

void SetFallEnabled(bool unused) { OutputWarning("SetFallEnabled has no effect."); }
bool GetDeathmatchScoreboard(void)
{
	OutputWarning("GetDeathmatchScoreboard has no effect. Use scripts to implement it.");
	return false;
}

void SetDeathmatchScoreboard(bool isDmScoreboard) { OutputWarning("GetDeathmatchScoreboard has no effect. Use scripts to implement it."); }
bool GetWeaponSync(int weapon)
{
	OutputWarning("GetWeaponSync does not exist in 0.4. Rely on the server's anti-hack system and callbacks instead.");
	return false;
}

void SetWeaponSync(int weapon, bool isSynced) { OutputWarning("GetWeaponSync does not exist in 0.4. Rely on the server's anti-hack system and callbacks instead."); }
bool GetWeatherLock(void)
{
	OutputWarning("GetWeatherLock has no effect.");
	return true;
}

void SetWeatherLock(bool isLocked) { OutputWarning("SetWeatherLock has no effect."); }
const SQChar * GetMapName(void)
{
	OutputWarning("GetMapName has no effect.");
	return _SC("Vice-City");
}

void SetMapName(const SQChar * mapName) { OutputWarning("SetMapName has no effect."); }	

#define SKIN_ID_UNKNOWN 				-1
#define SKIN_ID_TOMMY_VERCETTI 			0
#define SKIN_ID_COP 					1
#define SKIN_ID_SWAT 					2
#define SKIN_ID_FBI 					3
#define SKIN_ID_ARMY 					4
#define SKIN_ID_PARAMEDIC 				5
#define SKIN_ID_FIREMAN 				6
#define SKIN_ID_GOLF_GUY_A 				7
#define SKIN_ID_BUM_LADY_A 				9
#define SKIN_ID_BUM_LADY_B 				10
#define SKIN_ID_PUNK_A 					11
#define SKIN_ID_LAWYER 					12
#define SKIN_ID_SPANISH_LADY_A 			13
#define SKIN_ID_SPANISH_LADY_B 			14
#define SKIN_ID_COOL_GUY_A 				15
#define SKIN_ID_ARABIC_GUY 				16
#define SKIN_ID_BEACH_LADY_A 			17
#define SKIN_ID_BEACH_LADY_B 			18
#define SKIN_ID_BEACH_GUY_A 			19
#define SKIN_ID_BEACH_GUY_B 			20
#define SKIN_ID_OFFICE_LADY_A 			21
#define SKIN_ID_WAITRESS_A 				22
#define SKIN_ID_FOOD_LADY 				23
#define SKIN_ID_PROSTITUTE_A 			24
#define SKIN_ID_BUM_LADY_C 				25
#define SKIN_ID_BUM_GUY_A 				26
#define SKIN_ID_GARBAGEMAN_A 			27
#define SKIN_ID_TAXI_DRIVER_A 			28
#define SKIN_ID_HATIAN_A 				29
#define SKIN_ID_CRIMINAL_A 				30
#define SKIN_ID_HOOD_LADY 				31
#define SKIN_ID_GRANNY_A 				32
#define SKIN_ID_BUSINESS_MAN_A 			33
#define SKIN_ID_CHURCH_GUY 				34
#define SKIN_ID_CLUB_LADY 				35
#define SKIN_ID_CHURCH_LADY 			36
#define SKIN_ID_PIMP 					37
#define SKIN_ID_BEACH_LADY_C 			38
#define SKIN_ID_BEACH_GUY_C 			39
#define SKIN_ID_BEACH_LADY_D 			40
#define SKIN_ID_BEACH_GUY_D 			41
#define SKIN_ID_BUSINESS_MAN_B 			42
#define SKIN_ID_PROSTITUTE_B 			43
#define SKIN_ID_BUM_LADY_D 				44
#define SKIN_ID_BUM_GUY_B 				45
#define SKIN_ID_HATIAN_B 				46
#define SKIN_ID_CONSTRUCTION_WORKER_A 	47
#define SKIN_ID_PUNK_B 					48
#define SKIN_ID_PROSTITUTE_C 			49
#define SKIN_ID_GRANNY_B 				50
#define SKIN_ID_PUNK_C 					51
#define SKIN_ID_BUSINESS_MAN_C 			52
#define SKIN_ID_SPANISH_LADY_C 			53
#define SKIN_ID_SPANISH_LADY_D 			54
#define SKIN_ID_COOL_GUY_B 				55
#define SKIN_ID_BUSINESS_MAN_D 			56
#define SKIN_ID_BEACH_LADY_E 			57
#define SKIN_ID_BEACH_GUY_E 			58
#define SKIN_ID_BEACH_LADY_F 			59
#define SKIN_ID_BEACH_GUY_F 			60
#define SKIN_ID_CONSTRUCTION_WORKER_B 	61
#define SKIN_ID_GOLF_GUY_B 				62
#define SKIN_ID_GOLF_LADY 				63
#define SKIN_ID_GOLF_GUY_C 				64
#define SKIN_ID_BEACH_LADY_G 			65
#define SKIN_ID_BEACH_GUY_G 			66
#define SKIN_ID_OFFICE_LADY_B 			67
#define SKIN_ID_BUSINESS_MAN_E 			68
#define SKIN_ID_BUSINESS_MAN_F 			69
#define SKIN_ID_PROSTITUTE_D 			70
#define SKIN_ID_BUM_LADY_E 				71
#define SKIN_ID_BUM_GUY_C 				72
#define SKIN_ID_SPANISH_GUY 			73
#define SKIN_ID_TAXI_DRIVER_B 			74
#define SKIN_ID_GYM_LADY 				75
#define SKIN_ID_GYM_GUY 				76
#define SKIN_ID_SKATE_LADY 				77
#define SKIN_ID_SKATE_GUY 				78
#define SKIN_ID_SHOPPER_A 				79
#define SKIN_ID_SHOPPER_B 				80
#define SKIN_ID_TOURIST_A 				81
#define SKIN_ID_TOURIST_B 				82
#define SKIN_ID_CUBAN_A 				83
#define SKIN_ID_CUBAN_B 				84
#define SKIN_ID_HATIAN_C 				85
#define SKIN_ID_HATIAN_D 				86
#define SKIN_ID_SHARK_A 				87
#define SKIN_ID_SHARK_B 				88
#define SKIN_ID_DIAZ_GUY_A 				89
#define SKIN_ID_DIAZ_GUY_B 				90
#define SKIN_ID_DBP_SECURITY_A 			91
#define SKIN_ID_DBP_SECURITY_B 			92
#define SKIN_ID_BIKER_A 				93
#define SKIN_ID_BIKER_B 				94
#define SKIN_ID_VERCETTI_GUY_A 			95
#define SKIN_ID_VERCETTI_GUY_B 			96
#define SKIN_ID_UNDERCOVER_COP_A 		97
#define SKIN_ID_UNDERCOVER_COP_B 		98
#define SKIN_ID_UNDERCOVER_COP_C 		99
#define SKIN_ID_UNDERCOVER_COP_D 		100
#define SKIN_ID_UNDERCOVER_COP_E 		101
#define SKIN_ID_UNDERCOVER_COP_F 		102
#define SKIN_ID_RICH_GUY 				103
#define SKIN_ID_COOL_GUY_C 				104
#define SKIN_ID_PROSTITUTE_E 			105
#define SKIN_ID_PROSTITUTE_F 			106
#define SKIN_ID_LOVE_FIST_A 			107
#define SKIN_ID_KEN_ROSENBURG 			108
#define SKIN_ID_CANDY_SUXX 				109
#define SKIN_ID_HILARY 					110
#define SKIN_ID_LOVE_FIST_B 			111
#define SKIN_ID_PHIL 					112
#define SKIN_ID_ROCKSTAR_GUY 			113
#define SKIN_ID_SONNY 					114
#define SKIN_ID_LANCE_A 				115
#define SKIN_ID_MERCADES_A 				116
#define SKIN_ID_LOVE_FIST_C 			117
#define SKIN_ID_ALEX_SRUB 				118
#define SKIN_ID_LANCE_COP 				119
#define SKIN_ID_LANCE_B 				120
#define SKIN_ID_CORTEZ 					121
#define SKIN_ID_LOVE_FIST_D 			122
#define SKIN_ID_COLUMBIAN_GUY_A 		123
#define SKIN_ID_HILARY_ROBBER 			124
#define SKIN_ID_MERCADES_B 				125
#define SKIN_ID_CAM 					126
#define SKIN_ID_CAM_ROBBER 				127
#define SKIN_ID_PHIL_ONE_ARM 			128
#define SKIN_ID_PHIL_ROBBER 			129
#define SKIN_ID_COOL_GUY_D 				130
#define SKIN_ID_PIZZAMAN 				131
#define SKIN_ID_TAXI_DRIVER_C 			132
#define SKIN_ID_TAXI_DRIVER_D 			133
#define SKIN_ID_SAILOR_A 				134
#define SKIN_ID_SAILOR_B 				135
#define SKIN_ID_SAILOR_C 				136
#define SKIN_ID_CHEF 					137
#define SKIN_ID_CRIMINAL_B 				138
#define SKIN_ID_FRENCH_GUY 				139
#define SKIN_ID_GARBAGEMAN_B 			140
#define SKIN_ID_HATIAN_E 				141
#define SKIN_ID_WAITRESS_B 				142
#define SKIN_ID_SONNY_GUY_A 			143
#define SKIN_ID_SONNY_GUY_B 			144
#define SKIN_ID_SONNY_GUY_C 			145
#define SKIN_ID_COLUMBIAN_GUY_B 		146
#define SKIN_ID_THUG_A 					147
#define SKIN_ID_BEACH_GUY_H 			148
#define SKIN_ID_GARBAGEMAN_C 			149
#define SKIN_ID_GARBAGEMAN_D 			150
#define SKIN_ID_GARBAGEMAN_E 			151
#define SKIN_ID_TRANNY 					152
#define SKIN_ID_THUG_B 					153
#define SKIN_ID_SPANDEX_GUY_A 			154
#define SKIN_ID_SPANDEX_GUY_B 			155
#define SKIN_ID_STRIPPER_A 				156
#define SKIN_ID_STRIPPER_B 				157
#define SKIN_ID_STRIPPER_C 				158
#define SKIN_ID_STORE_CLERK 			159

#include <string>
#include <cwctype>

int GetSkinID(const SQChar* name)
{
	if (!name || name[0] == L'\0') return SKIN_ID_UNKNOWN;

	std::wstring str;
	str.reserve(64);

	for (int i = 0; name[i] != L'\0'; ++i)
	{
		if (std::iswalnum(name[i]))
		{
			str.push_back(std::towlower(name[i]));
		}
	}

	size_t len = str.length();
	if (len < 1) return SKIN_ID_UNKNOWN;

	int id = SKIN_ID_UNKNOWN;

	wchar_t a = std::towlower(name[0]);
	wchar_t b = (name[0] != L'\0') ? std::towlower(name[1]) : L'\0';
	wchar_t c = (name[1] != L'\0') ? std::towlower(name[2]) : L'\0';
	wchar_t d = str[len - 1];

	switch (a)
	{
		case L'a':
			switch (b)
			{
				case L'l':
				case L's':
					id = SKIN_ID_ALEX_SRUB;
				break;
				case L'g':
					id = SKIN_ID_ARABIC_GUY;
				break;
				case L'r':
					if (c == L'a') id = SKIN_ID_ARABIC_GUY;
					else if (c == L'm') id = SKIN_ID_ARMY;
				break;
			}
		break;

		case L'b':
			if (b == L'e' && (c == L'g' || (len >= 6 && str[5] == L'g')))
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_BEACH_GUY_A; break;
					case L'2': case L'b': id = SKIN_ID_BEACH_GUY_B; break;
					case L'3': case L'c': id = SKIN_ID_BEACH_GUY_C; break;
					case L'4': case L'd': id = SKIN_ID_BEACH_GUY_D; break;
					case L'5': case L'e': id = SKIN_ID_BEACH_GUY_E; break;
					case L'6': case L'f': id = SKIN_ID_BEACH_GUY_F; break;
					case L'7': case L'g': id = SKIN_ID_BEACH_GUY_G; break;
					case L'8': case L'h': id = SKIN_ID_BEACH_GUY_H; break;
				}
			}
			else if (b == L'e' && (c == L'l' || (len >= 6 && str[5] == L'l')))
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_BEACH_LADY_A; break;
					case L'2': case L'b': id = SKIN_ID_BEACH_LADY_B; break;
					case L'3': case L'c': id = SKIN_ID_BEACH_LADY_C; break;
					case L'4': case L'd': id = SKIN_ID_BEACH_LADY_D; break;
					case L'5': case L'e': id = SKIN_ID_BEACH_LADY_E; break;
					case L'6': case L'f': id = SKIN_ID_BEACH_LADY_F; break;
					case L'7': case L'g': id = SKIN_ID_BEACH_LADY_G; break;
				}
			}
			else if (b == L'i' && (d == L'1' || d == L'a')) id = SKIN_ID_BIKER_A;
			else if (b == L'i' && (d == L'2' || d == L'b')) id = SKIN_ID_BIKER_B;
			else if (b == L'u' && (c == L'm' || c == L'g' || c == L'l'))
			{
				if (c == L'g' || (len >= 4 && str[3] == L'g'))
				{
					if (d == L'1' || d == L'a') id = SKIN_ID_BUM_GUY_A;
					else if (d == L'2' || d == L'b') id = SKIN_ID_BUM_GUY_B;
					else if (d == L'3' || d == L'c') id = SKIN_ID_BUM_GUY_C;
				}
				else if (c == L'l' || (len >= 4 && str[3] == L'l'))
				{
					if (d == L'1' || d == L'a') id = SKIN_ID_BUM_LADY_A;
					else if (d == L'2' || d == L'b') id = SKIN_ID_BUM_LADY_B;
					else if (d == L'2' || d == L'c') id = SKIN_ID_BUM_LADY_C;
					else if (d == L'3' || d == L'd') id = SKIN_ID_BUM_LADY_D;
					else if (d == L'4' || d == L'e') id = SKIN_ID_BUM_LADY_E;
				}
			}
			else if (b == L'u' && (c == L's' || (len >= 10 && str[9] == L'm')))
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_BUSINESS_MAN_A; break;
					case L'2': case L'b': id = SKIN_ID_BUSINESS_MAN_B; break;
					case L'3': case L'c': id = SKIN_ID_BUSINESS_MAN_C; break;
					case L'4': case L'd': id = SKIN_ID_BUSINESS_MAN_D; break;
					case L'5': case L'e': id = SKIN_ID_BUSINESS_MAN_E; break;
					case L'6': case L'f': id = SKIN_ID_BUSINESS_MAN_F; break;
				}
			}
		break;

		case L'c':
			if (b == L'a')
			{
				if ((c == L'm' || c == L'r') && d == L'r') id = SKIN_ID_CAM_ROBBER;
				else if (c == L'm') id = SKIN_ID_CAM;
				else if (c == L'n' || c == L's' || d == L'x') id = SKIN_ID_CANDY_SUXX;
			}
			else if (b == L'h')
			{
				if (c == L'e' || d == L'f') id = SKIN_ID_CHEF;
				else if ((c == L'u' && len >= 7 && str[6] == L'g') || (c == L'g')) id = SKIN_ID_CHURCH_GUY;
				else if ((c == L'u' && len >= 7 && str[6] == L'l') || (c == L'l')) id = SKIN_ID_CHURCH_LADY;
			}
			else if (b == L'l') id = SKIN_ID_CLUB_LADY;
			else if (b == L'o')
			{
				if ((c == L'l' && len >= 10 && str[9] == L'g') || (c == L'g'))
				{
					if (d == L'1' || d == L'a') id = SKIN_ID_COLUMBIAN_GUY_A;
					else if (d == L'2' || d == L'b') id = SKIN_ID_COLUMBIAN_GUY_B;
				}
				else if (c == L'n' || (len >= 13 && str[12] == L'g'))
				{
					if (d == L'1' || d == L'a') id = SKIN_ID_CONSTRUCTION_WORKER_A;
					else if (d == L'2' || d == L'b') id = SKIN_ID_CONSTRUCTION_WORKER_B;
				}
				else if (c == L'o')
				{
					switch (d)
					{
						case L'1': case L'a': id = SKIN_ID_COOL_GUY_A; break;
						case L'2': case L'b': id = SKIN_ID_COOL_GUY_B; break;
						case L'3': case L'c': id = SKIN_ID_COOL_GUY_C; break;
						case L'4': case L'd': id = SKIN_ID_COOL_GUY_D; break;
					}
				}
				else if (c == L'p') id = SKIN_ID_COP;
				else if (c == L'r' || c == L'z' || d == L'z') id = SKIN_ID_CORTEZ;
			}
			else if (b == L'r' && (d == L'1' || d == L'a')) id = SKIN_ID_CRIMINAL_A;
			else if (b == L'r' && (d == L'2' || d == L'b')) id = SKIN_ID_CRIMINAL_B;
			else if (b == L'u' && (d == L'1' || d == L'a')) id = SKIN_ID_CUBAN_A;
			else if (b == L'u' && (d == L'2' || d == L'b')) id = SKIN_ID_CUBAN_B;
		break;

		case L'd':
			switch (b)
			{
				case L'b':
				case L's':
					if (d == L'1' || d == L'a') id = SKIN_ID_DBP_SECURITY_A;
					else if (d == L'2' || d == L'b') id = SKIN_ID_DBP_SECURITY_B;
				break;
				case L'i':
				case L'z':
				case L'g':
					if (d == L'1' || d == L'a') id = SKIN_ID_DIAZ_GUY_A;
					else if (d == L'2' || d == L'b') id = SKIN_ID_DIAZ_GUY_B;
				break;
			}
		break;

		case L'f':
			switch (b)
			{
				case L'b': id = SKIN_ID_FBI; break;
				case L'i': case L'm': id = SKIN_ID_FIREMAN; break;
				case L'o': case L'l': id = SKIN_ID_FOOD_LADY; break;
				case L'r': case L'g': id = SKIN_ID_FRENCH_GUY; break;
			}
		break;

		case L'g':
			if (b == L'a' || b == L'm')
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_GARBAGEMAN_A; break;
					case L'2': case L'b': id = SKIN_ID_GARBAGEMAN_B; break;
					case L'3': case L'c': id = SKIN_ID_GARBAGEMAN_C; break;
					case L'4': case L'd': id = SKIN_ID_GARBAGEMAN_D; break;
					case L'5': case L'e': id = SKIN_ID_GARBAGEMAN_E; break;
				}
			}
			else if (b == L'o' && (c == L'g' || (len >= 5 && str[4] == L'g')))
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_GOLF_GUY_A; break;
					case L'2': case L'b': id = SKIN_ID_GOLF_GUY_B; break;
					case L'3': case L'c': id = SKIN_ID_GOLF_GUY_C; break;
				}
			}
			else if (b == L'o' && (c == L'l' || (len >= 5 && str[4] == L'l'))) id = SKIN_ID_GOLF_LADY;
			else if (b == L'r')
			{
				if (d == L'1' || d == L'a') id = SKIN_ID_GRANNY_A;
				else if (d == L'2' || d == L'b') id = SKIN_ID_GRANNY_B;
			}
			else if (b == L'g' || (b == L'y' && len >= 4 && str[3] == L'g')) id = SKIN_ID_GYM_GUY;
			else if (b == L'l' || (b == L'y' && len >= 4 && str[3] == L'l')) id = SKIN_ID_GYM_LADY;
		break;

		case L'h':
			if (b == L'a')
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_HATIAN_A; break;
					case L'2': case L'b': id = SKIN_ID_HATIAN_B; break;
					case L'3': case L'c': id = SKIN_ID_HATIAN_C; break;
					case L'4': case L'd': id = SKIN_ID_HATIAN_D; break;
					case L'5': case L'e': id = SKIN_ID_HATIAN_E; break;
				}
			}
			else if ((b == L'i' || b == L'r') && d == L'r') id = SKIN_ID_HILARY_ROBBER;
			else if (b == L'i') id = SKIN_ID_HILARY;
			
			if (b == L'o' || b == L'l') id = SKIN_ID_HOOD_LADY;
		break;

		case L'k':
			id = SKIN_ID_KEN_ROSENBURG;
		break;

		case L'l':
			if (b == L'a' && c == L'n' && ((len >= 6 && str[5] == L'c') || d == L'p')) id = SKIN_ID_LANCE_COP;
			else if (b == L'c' || (b == L'a' && c == L'n')) id = SKIN_ID_LANCE_COP;
			else if (b == L'a' && c == L'n')
			{
				if (d == L'1' || d == L'a') id = SKIN_ID_LANCE_A;
				else if (d == L'2' || d == L'b') id = SKIN_ID_LANCE_B;
			}
			else if (b == L'w' || (b == L'a' && c == L'w')) id = SKIN_ID_LAWYER;
			else if (b == L'o' || b == L'f')
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_LOVE_FIST_A; break;
					case L'2': case L'b': id = SKIN_ID_LOVE_FIST_B; break;
					case L'3': case L'c': id = SKIN_ID_LOVE_FIST_C; break;
					case L'd': id = SKIN_ID_LOVE_FIST_D; break;
				}
			}
		break;

		case L'm':
			id = (d == L'b') ? SKIN_ID_MERCADES_B : SKIN_ID_MERCADES_A;
		break;

		case L'o':
			id = (d == L'1' || d == L'a') ? SKIN_ID_OFFICE_LADY_A : SKIN_ID_OFFICE_LADY_B;
		break;

		case L'p':
			if (b == L'a') id = SKIN_ID_PARAMEDIC;
			else if (b == L'h')
			{
				if (b == L'o' || c == L'o' || (len >= 5 && str[4] == L'o') || d == L'm') id = SKIN_ID_PHIL_ONE_ARM;
				else if (c == L'r' || d == L'r' || (len >= 5 && str[4] == L'r')) id = SKIN_ID_PHIL_ROBBER;
				else if (c == L'i') id = SKIN_ID_PHIL;
			}
			else if (b == L'i' && (c == L'm' || d == L'p')) id = SKIN_ID_PIMP;
			else if (b == L'i' && (c == L'z' || d == L'n')) id = SKIN_ID_PIZZAMAN;
			else if (b == L'r')
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_PROSTITUTE_A; break;
					case L'2': case L'b': id = SKIN_ID_PROSTITUTE_B; break;
					case L'c': id = SKIN_ID_PROSTITUTE_C; break;
					case L'd': id = SKIN_ID_PROSTITUTE_D; break;
					case L'3': case L'e': id = SKIN_ID_PROSTITUTE_E; break;
					case L'4': case L'f': id = SKIN_ID_PROSTITUTE_F; break;
				}
			}
			else if (b == L'u')
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_PUNK_A; break;
					case L'2': case L'b': id = SKIN_ID_PUNK_B; break;
					case L'3': case L'c': id = SKIN_ID_PUNK_C; break;
				}
			}
		break;

		case L'r':
			if (b == L'i') id = SKIN_ID_RICH_GUY;
			else if (b == L'o') id = SKIN_ID_ROCKSTAR_GUY;
		break;

		case L's':
			if (b == L'a')
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_SAILOR_A; break;
					case L'2': case L'b': id = SKIN_ID_SAILOR_B; break;
					case L'3': case L'c': id = SKIN_ID_SAILOR_C; break;
				}
			}
			else if (b == L'h' && c == L'a')
			{
				id = (d == L'1' || d == L'a') ? SKIN_ID_SHARK_A : SKIN_ID_SHARK_B;
			}
			else if (b == L'h' && c == L'o')
			{
				id = (d == L'1' || d == L'a') ? SKIN_ID_SHOPPER_A : SKIN_ID_SHOPPER_B;
			}
			else if (b == L'k' && (c == L'g' || (len >= 6 && str[5] == L'g'))) id = SKIN_ID_SKATE_GUY;
			else if (b == L'k' && (c == L'l' || (len >= 6 && str[5] == L'l'))) id = SKIN_ID_SKATE_LADY;
			else if (b == L'o' || b == L'g')
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_SONNY_GUY_A; break;
					case L'2': case L'b': id = SKIN_ID_SONNY_GUY_B; break;
					case L'3': case L'c': id = SKIN_ID_SONNY_GUY_C; break;
				}
			}
			else if (b == L'p' && (c == L'x' || (len >= 7 && str[6] == L'x')))
			{
				id = (d == L'1' || d == L'a') ? SKIN_ID_SPANDEX_GUY_A : SKIN_ID_SPANDEX_GUY_B;
			}
			else if (b == L'p' && (c == L'g' || (len >= 8 && str[7] == L'g'))) id = SKIN_ID_SPANISH_GUY;
			else if (b == L'p' && (c == L'l' || (len >= 8 && str[7] == L'l')))
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_SPANISH_LADY_A; break;
					case L'2': case L'b': id = SKIN_ID_SPANISH_LADY_B; break;
					case L'3': case L'c': id = SKIN_ID_SPANISH_LADY_C; break;
					case L'4': case L'd': id = SKIN_ID_SPANISH_LADY_D; break;
				}
			}
			else if (b == L't' && c == L'o') id = SKIN_ID_STORE_CLERK;
			else if (b == L't' && c == L'r')
			{
				switch (d)
				{
					case L'1': case L'a': id = SKIN_ID_STRIPPER_A; break;
					case L'2': case L'b': id = SKIN_ID_STRIPPER_B; break;
					case L'3': case L'c': id = SKIN_ID_STRIPPER_C; break;
				}
			}
			else if (b == L'w') id = SKIN_ID_SWAT;
		break;

		case L't':
			switch (b)
			{
				case L'a':
					switch (d)
					{
						case L'1': case L'a': id = SKIN_ID_TAXI_DRIVER_A; break;
						case L'2': case L'b': id = SKIN_ID_TAXI_DRIVER_B; break;
						case L'c': id = SKIN_ID_TAXI_DRIVER_C; break;
						case L'd': id = SKIN_ID_TAXI_DRIVER_D; break;
					}
				break;
				case L'h':
					id = (d == L'5' || d == L'b') ? SKIN_ID_THUG_B : SKIN_ID_THUG_A;
				break;
				case L'v':
					id = SKIN_ID_TOMMY_VERCETTI;
				break;
				case L'o':
					if (c == L'm') id = SKIN_ID_TOMMY_VERCETTI;
					else if (c == L'u') id = (d == L'2' || d == L'b') ? SKIN_ID_TOURIST_B : SKIN_ID_TOURIST_A;
				break;
				case L'r':
					id = SKIN_ID_TRANNY;
				break;
			}
		break;

		case L'u':
			switch (d)
			{
				case L'1': case L'a': id = SKIN_ID_UNDERCOVER_COP_A; break;
				case L'2': case L'b': id = SKIN_ID_UNDERCOVER_COP_B; break;
				case L'3': case L'c': id = SKIN_ID_UNDERCOVER_COP_C; break;
				case L'4': case L'd': id = SKIN_ID_UNDERCOVER_COP_D; break;
				case L'5': case L'e': id = SKIN_ID_UNDERCOVER_COP_E; break;
				case L'6': case L'f': id = SKIN_ID_UNDERCOVER_COP_F; break;
			}
		break;

		case L'v':
			id = (d == L'2' || d == L'b') ? SKIN_ID_VERCETTI_GUY_B : SKIN_ID_VERCETTI_GUY_A;
		break;

		case L'w':
			id = (d == L'2' || d == L'b') ? SKIN_ID_WAITRESS_B : SKIN_ID_WAITRESS_A;
		break;
	}

	return id;
}

void SetFallTimer(int delay) {
	functions->SetFallTimer(delay);
}

int GetFallTimer(void) {
	return functions->GetFallTimer();
}