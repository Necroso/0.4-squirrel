/**
 * @file PluginEntry.cpp
 * @brief Main entry point and callback registration for the VC-MP Squirrel host plugin.
 */

#ifndef WIN32_LEANANDMEAN
#define WIN32_LEANANDMEAN
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

// Standard Library
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cwchar>

// Project Headers
#include "main.h"
#include "CallbackHandler.h"
#include "ConsoleUtils.h"
#include "Exports.h"
#include "Functions.h"
#include "CCore.h"
#include "CObject.h"
#include "CPickup.h"
#include "CPlayer.h"
#include "CVehicle.h"

// --- Global Variables ---
SquirrelExports* pExp;
PluginFuncs*    functions;
PluginInfo*     information;
PluginCallbacks* callbacks;
CCore*          pCore;
HSQAPI          sq;

extern HSQUIRRELVM v;

// --- Helper Functions ---

/**
 * @brief Calculates the length of a formatted string before allocation.
 */
inline int vscprintf(const char* format, va_list args)
{
    va_list copy;
    va_copy(copy, args);
    int len = vsnprintf(nullptr, 0, format, copy);
    va_end(copy);
    return len;
}

/**
 * @brief Converts a wide character string to an ANSI string.
 */
std::string WideToAnsi(const wchar_t* wstr)
{
    if (!wstr) return {};

    size_t len = std::wcstombs(nullptr, wstr, 0);
    if (len == static_cast<size_t>(-1)) return {};

    std::string str(len, '\0');
    std::wcstombs(str.data(), wstr, len + 1);
    return str;
}

/**
 * @brief Outputs formatted script information to the console/server log.
 */
void printfunc(HSQUIRRELVM v, const SQChar* s, ...)
{
    va_list arglist;
    char szInitBuffer[1024];

    va_start(arglist, s);
    int nChars = vsnprintf(szInitBuffer, sizeof(szInitBuffer), s, arglist);

    if (nChars < 0 || nChars >= (int)sizeof(szInitBuffer))
    {
        if (nChars < 0) nChars = vscprintf(s, arglist);

        char* szBuffer = (char*)calloc(nChars + 1, sizeof(char));
        if (szBuffer == NULL) {
            pCore->rawprint("Error: failed to malloc buffer for printfunc.");
        } else {
            vsnprintf(szBuffer, nChars + 1, s, arglist);
            OutputScriptInfo(szBuffer);
            free(szBuffer);
        }
    } else {
        OutputScriptInfo(szInitBuffer);
    }
    va_end(arglist);
}

/**
 * @brief Outputs formatted error messages to the console/server log.
 */
void errorfunc(HSQUIRRELVM v, const SQChar* s, ...)
{
    va_list arglist;
    char szInitBuffer[1024];

    va_start(arglist, s);
    int nChars = vsnprintf(szInitBuffer, sizeof(szInitBuffer), s, arglist);

    if (nChars < 0 || nChars >= (int)sizeof(szInitBuffer))
    {
        if (nChars < 0) nChars = vscprintf(s, arglist);

        char* szBuffer = (char*)calloc(nChars + 1, sizeof(char));
        if (szBuffer == NULL) {
            pCore->rawprint("Error: failed to malloc buffer for errorfunc.");
        } else {
            vsnprintf(szBuffer, nChars + 1, s, arglist);
            pCore->rawprint(szBuffer);
            free(szBuffer);
        }
    } else {
        pCore->rawprint(szInitBuffer);
    }
    va_end(arglist);
}

// --- Plugin Export Interface ---

extern "C" EXPORT unsigned int Supports()
{
    return 0x00000FF0;
}

/**
 * @brief Initializes the plugin, registers callbacks, and sets up the Squirrel export table.
 */
extern "C" EXPORT unsigned int VcmpPluginInit(PluginFuncs* givenPluginFuncs, PluginCallbacks* givenPluginCalls, PluginInfo* givenPluginInfo)
{
    functions = givenPluginFuncs;
    callbacks = givenPluginCalls;
    information = givenPluginInfo;

    // Set Plugin metadata
    information->pluginVersion = 0x110;
    information->apiMajorVersion = PLUGIN_API_MAJOR;
    information->apiMinorVersion = PLUGIN_API_MINOR;
    snprintf(information->name, sizeof(information->name), "%s", "SQHost2");

    // Initialize Squirrel VM
    sq = NULL;
    InitSQAPI();

    // Configure Squirrel export structure for cross-plugin compatibility
    pExp = new SquirrelExports;
    pExp->GetSquirrelAPI = pfGetSquirrelAPI;
    pExp->GetSquirrelVM = pfGetSquirrelVM;
    pExp->uStructSize = sizeof(SquirrelExports);

    // Register export table
    functions->ExportFunctions(information->pluginId, (const void**)&pExp, sizeof(SquirrelExports));

    pCore = CCore::GetInstance();

    // Map server callbacks
    callbacks->OnServerInitialise = OnInitServer;
    callbacks->OnServerShutdown = OnShutdownServer;
    callbacks->OnServerFrame = OnFrame;
    callbacks->OnIncomingConnection = OnLoginAttempt;
    callbacks->OnClientScriptData = OnClientScriptData;
    callbacks->OnPlayerConnect = OnPlayerConnect;
    callbacks->OnPlayerDisconnect = OnPlayerDisconnect;
    callbacks->OnPlayerRequestClass = OnPlayerRequestClass;
    callbacks->OnPlayerRequestSpawn = OnPlayerRequestSpawn;
    callbacks->OnPlayerSpawn = OnPlayerSpawn;
    callbacks->OnPlayerDeath = OnPlayerDeath;
    callbacks->OnPlayerUpdate = OnPlayerUpdate;
    callbacks->OnPlayerAwayChange = OnPlayerAwayChange;
    callbacks->OnPlayerSpectate = OnPlayerSpectate;
    callbacks->OnPlayerCrashReport = OnPlayerCrashDump;
    callbacks->OnPlayerBeginTyping = OnPlayerBeginTyping;
    callbacks->OnPlayerEndTyping = OnPlayerEndTyping;
    callbacks->OnPlayerRequestEnterVehicle = OnPlayerRequestEnter;
    callbacks->OnPlayerEnterVehicle = OnPlayerEnterVehicle;
    callbacks->OnPlayerExitVehicle = OnPlayerExitVehicle;
    callbacks->OnPickupPickAttempt = OnPickupClaimPicked;
    callbacks->OnPickupPicked = OnPickupPickedUp;
    callbacks->OnPickupRespawn = OnPickupRespawn;
    callbacks->OnVehicleUpdate = OnVehicleUpdate;
    callbacks->OnVehicleExplode = OnVehicleExplode;
    callbacks->OnVehicleRespawn = OnVehicleRespawn;
    callbacks->OnPlayerMessage = OnPublicMessage;
    callbacks->OnPlayerCommand = OnCommandMessage;
    callbacks->OnPlayerPrivateMessage = OnPrivateMessage;
    callbacks->OnPluginCommand = OnInternalCommand;
    callbacks->OnObjectShot = OnObjectShot;
    callbacks->OnObjectTouched = OnObjectBump;
    callbacks->OnEntityPoolChange = OnEntityPoolChange;
    callbacks->OnPlayerKeyBindDown = OnKeyBindDown;
    callbacks->OnPlayerKeyBindUp = OnKeyBindUp;
    callbacks->OnPlayerNameChange = OnPlayerNameChange;
    callbacks->OnPlayerStateChange = OnPlayerStateChange;
    callbacks->OnPlayerActionChange = OnPlayerActionChange;
    callbacks->OnPlayerOnFireChange = OnPlayerOnFireChange;
    callbacks->OnPlayerCrouchChange = OnPlayerCrouchChange;
    callbacks->OnPlayerGameKeysChange = OnPlayerGameKeysChange;
    callbacks->OnCheckpointEntered = OnCheckpointEntered;
    callbacks->OnCheckpointExited = OnCheckpointExited;
    callbacks->OnPlayerModuleList = OnPlayerModuleList;

    #ifdef _WIN32
        system("chcp 65001 > nul");
    #endif

    return 1;
}