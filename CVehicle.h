#include "main.h"
#pragma once

class CPlayer; // Forward declaration

class CVehicle
{
public:
    void Init(int lnVehicleId, bool lIsOurs = true) { this->isOurs = lIsOurs; this->nVehicleId = lnVehicleId; }

public:
    // --- Setters ---
    void Set3DArrowForPlayer(CPlayer* target, bool enable);
    void SetAlarm(bool isAlarmOn);
    void SetAngle(float angle);
    void SetBonnetOpen(bool b);
    void SetBootOpen(bool b);
    void SetColour1(int colour1);
    void SetColour2(int colour2);
    void SetDamage(unsigned int damage);
    void SetEngineEnabled(bool b);
    void SetEulerRotation(Vector angle);
    void SetGhost(bool isGhost);
    void SetHealth(float health);
    void SetIdleRespawnTimer(unsigned int time);
    void SetImmunity(int immunity);
    void SetLightFlags(unsigned int flags);
    void SetLights(bool lightsOn);
    void SetLocked(bool isLocked);
    void SetPosition(Vector pos);
    void SetRadio(int nRadioId);
    void SetRadioLocked(bool isRadioLocked);
    void SetRelativeSpeed(Vector speed);
    void SetRelativeTurnSpeed(Vector speed);
    void SetRotation(Quaternion rotation);
    void SetSingleUse(bool isSingleUse);
    void SetSiren(bool isSirenOn);
    void SetSpawnAngle(Quaternion angle);
    void SetSpawnAngleEuler(Vector angle);
    void SetSpawnPos(Vector pos);
    void SetSpeed(Vector speed);
    void SetTaxiLight(bool hasTaxiLight);
    void SetTurnSpeed(Vector speed);
    void SetWorld(int world);

public:
    // --- Getters ---
    bool Get3DArrowForPlayer(CPlayer* target);
    bool GetAlarm();
    bool GetBonnetOpen();
    bool GetBootOpen();
    int GetColour1();
    int GetColour2();
    unsigned int GetDamage();
    CPlayer* GetDriver();
    bool GetEngineEnabled();
    EntityVector GetEulerRotation();
    bool GetGhost();
    float GetHealth();
    int GetID();
    unsigned int GetIdleRespawnTimer();
    int GetImmunity();
    unsigned int GetLightFlags();
    bool GetLights();
    bool GetLocked();
    int GetModel();
    CPlayer* GetOccupant(int slot);
    EntityVector GetPosition();
    int GetRadio();
    bool GetRadioLockStatus();
    EntityVector GetRelativeSpeed();
    EntityVector GetRelativeTurnSpeed();
    EntityQuaternion GetRotation();
    bool GetSiren();
    EntityQuaternion GetSpawnAngle();
    EntityVector GetSpawnAngleEuler();
    EntityVector GetSpawnPos();
    EntityVector GetSpeed();
    int GetSyncSource();
    int GetSyncType();
    bool GetTaxiLight();
    EntityVector GetTurnSpeed();
    Vector GetTurretRotation();
    bool GetSingleUse(void);
    bool GetWrecked();
    int GetWorld();

public:
    // --- Functions ---
    void AddVehicleRelSpeed(Vector speed);
    void AddVehicleRelTurnSpeed(Vector speed);
    void AddVehicleSpeed(Vector speed);
    void AddVehicleTurnSpeed(Vector speed);
    void Delete();
    void Fix();
    double GetHandlingData(int rule);
    int GetPartStatus(int part);
    bool GetStreamedForPlayer(CPlayer* player);
    int GetTyreStatus(int tyre);
    bool IsHandlingSet(int rule);
    void Kill();
    void Remove();
    void ResetAllHandling();
    void ResetHandlingData(int rule);
    void Respawn();
    int SetHandlingData(int rule, float value);
    void SetFlatTyres(bool isFlat);
    void SetPartStatus(int part, int status);
    void SetTyreStatus(int part, int status);

public:
    bool isOurs;
    int nVehicleId;
};

void RegisterVehicle();