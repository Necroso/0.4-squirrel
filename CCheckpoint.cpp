#include "CCore.h"
#include "CCheckpoint.h"
#include "CPlayer.h"
#include "main.h"

// Core instance
extern CCore* pCore;

void CCheckpoint::SetWorld(int world)
{
	if (this->nCheckpointId >= 0 && functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId))
	{
		functions->SetCheckPointWorld(this->nCheckpointId, world);
	}
}

void CCheckpoint::SetColor(ARGB color)
{
	if (this->nCheckpointId >= 0 && functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId))
	{
		functions->SetCheckPointColour(this->nCheckpointId, color.r, color.g, color.b, color.a);
	}
}

void CCheckpoint::SetRadius(float radius)
{
	if (this->nCheckpointId >= 0 && functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId))
	{
		functions->SetCheckPointRadius(this->nCheckpointId, radius);
	}
}

void CCheckpoint::SetPos(Vector pos)
{
	if (this->nCheckpointId >= 0 && functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId))
	{
		functions->SetCheckPointPosition(this->nCheckpointId, pos.x, pos.y, pos.z);
	}
}

int CCheckpoint::GetWorld()
{
	if (this->nCheckpointId >= 0 && functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId))
	{
		return functions->GetCheckPointWorld(this->nCheckpointId);
	}
	return -1;
}

ARGB CCheckpoint::GetColor()
{
	int r = 0, g = 0, b = 0, a = 0;
	if (this->nCheckpointId >= 0 && functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId))
	{
		functions->GetCheckPointColour(this->nCheckpointId, &r, &g, &b, &a);
	}
	return ARGB(a, r, g, b);
}

float CCheckpoint::GetRadius()
{
	if (this->nCheckpointId >= 0 && functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId))
	{
		return functions->GetCheckPointRadius(this->nCheckpointId);
	}
	return 0.0f;
}

EntityVector CCheckpoint::GetPos()
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	if (this->nCheckpointId >= 0 && functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId))
	{
		functions->GetCheckPointPosition(this->nCheckpointId, &x, &y, &z);
	}
	return EntityVector(this->nCheckpointId, ENTITY_CHECKPOINT, -1, x, y, z);
}

void CCheckpoint::Delete()
{
	if (this->nCheckpointId >= 0 && functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId))
	{
		// PS! This instance will be dereferenced/deleted due to the plugin's callback flow
		functions->DeleteCheckPoint(this->nCheckpointId);
	}
}

int CCheckpoint::GetID()
{
	return this->nCheckpointId;
}

bool CCheckpoint::StreamedToPlayer(CPlayer* player)
{
	if (player == nullptr || this->nCheckpointId < 0)
	{
		return false;
	}

	if (functions->CheckEntityExists(vcmpEntityPoolCheckPoint, this->nCheckpointId) && functions->IsPlayerConnected(player->nPlayerId))
	{
		return (functions->IsCheckPointStreamedForPlayer(this->nCheckpointId, player->nPlayerId) == 1);
	}
	return false;
}

CPlayer* CCheckpoint::GetOwner(void)
{
	if (pCore == nullptr)
	{
		return nullptr;
	}
	return pCore->RetrievePlayer(this->nOwnerId);
}

void RegisterCheckpoint()
{
	Class<CCheckpoint> c(v, _SC("CCheckpoint_INTERNAL"));

	// Read-write properties
	c
		.Prop(_SC("World"), &CCheckpoint::GetWorld, &CCheckpoint::SetWorld)
		.Prop(_SC("Color"), &CCheckpoint::GetColor, &CCheckpoint::SetColor)
		.Prop(_SC("Radius"), &CCheckpoint::GetRadius, &CCheckpoint::SetRadius)
		.Prop(_SC("Pos"), &CCheckpoint::GetPos, &CCheckpoint::SetPos);

	// Read-only properties
	c
		.Prop(_SC("ID"), &CCheckpoint::GetID)
		.Prop(_SC("Owner"), &CCheckpoint::GetOwner);

	// Functions
	c
		.Func(_SC("Remove"), &CCheckpoint::Delete)
		.Func(_SC("StreamedToPlayer"), &CCheckpoint::StreamedToPlayer);

	RootTable(v).Bind(_SC("CCheckpoint"), c);
}