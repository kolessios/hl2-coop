//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef WEAPON_COOP_BASEBLUDGEON_H
#define WEAPON_COOP_BASEBLUDGEON_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_coop_basehlcombatweapon.h"

#ifdef CLIENT_DLL
    #define CWeaponCoopBaseBludgeon C_WeaponCoopBaseBludgeon
#endif

//================================================================================
// La base para un arma cooperativo
//================================================================================
class CWeaponCoopBaseBludgeon : public CWeaponCoopBaseHLCombat
{
public:
    DECLARE_CLASS( CWeaponCoopBaseBludgeon, CWeaponCoopBaseHLCombat );
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();
    DECLARE_DATADESC();

    CWeaponCoopBaseBludgeon();

    virtual	void Spawn( void );
	virtual	void Precache( void );

    //Attack functions
	virtual	void PrimaryAttack( void );
	virtual	void SecondaryAttack( void );

    virtual void ItemPostFrame( void );
    virtual	void ImpactEffect( trace_t &trace );

    bool ImpactWater( const Vector &start, const Vector &end );
	void Swing( int bIsSecondary );
	void Hit( trace_t &traceHit, Activity nHitActivity );
	Activity ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );

    //Functions to select animation sequences 
	virtual Activity GetPrimaryAttackActivity( void )	{	return	ACT_VM_HITCENTER;	}
	virtual Activity GetSecondaryAttackActivity( void )	{	return	ACT_VM_HITCENTER2;	}

    virtual	float	GetFireRate( void )								{	return	0.2f;	}
	virtual float	GetRange( void )								{	return	32.0f;	}
	virtual	float	GetDamageForActivity( Activity hitActivity )	{	return	1.0f;	}

protected:
    int	m_nShotsFired;	// Number of consecutive shots fired
    float	m_flNextSoundTime;	// real-time clock of when to make next sound

private:
    CWeaponCoopBaseBludgeon( const CWeaponCoopBaseBludgeon & );
};

#endif // WEAPON_COOP_BASEMACHINEGUN_H