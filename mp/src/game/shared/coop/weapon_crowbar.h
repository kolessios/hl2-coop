//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_CROWBAR_H
#define WEAPON_CROWBAR_H

#include "weapon_coop_basebludgeon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef CLIENT_DLL
#define CWeaponCrowbar C_WeaponCrowbar
#endif

#define	CROWBAR_RANGE	75.0f
#define	CROWBAR_REFIRE	0.4f

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

class CWeaponCrowbar : public CWeaponCoopBaseBludgeon
{
public:
	DECLARE_CLASS( CWeaponCrowbar, CWeaponCoopBaseBludgeon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponCrowbar();

	float		GetRange( void )		{	return	CROWBAR_RANGE;	}
	float		GetFireRate( void )		{	return	CROWBAR_REFIRE;	}

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );
	void		SecondaryAttack( void )	{	return;	}

	// Animation event
    #ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
    virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
    #endif

private:
    CWeaponCrowbar( CWeaponCrowbar & );
};

#endif // WEAPON_CROWBAR_H
