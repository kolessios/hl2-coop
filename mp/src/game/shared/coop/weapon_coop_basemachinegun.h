//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef WEAPON_COOP_BASEMACHINEGUN_H
#define WEAPON_COOP_BASEMACHINEGUN_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_coop_base.h"

#ifdef CLIENT_DLL
    #define CWeaponCoopMachineGun C_WeaponCoopMachineGun
#endif

//================================================================================
// La base para un arma cooperativo
//================================================================================
class CWeaponCoopMachineGun : public CWeaponCoopBase
{
public:
    DECLARE_CLASS( CWeaponCoopMachineGun, CWeaponCoopBase );
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();
    DECLARE_DATADESC();

    CWeaponCoopMachineGun() { }

    // Default calls through to m_hOwner, but plasma weapons can override and shoot projectiles here.
	virtual void ItemPostFrame( void );
	//virtual void FireBullets( const FireBulletsInfo_t &info );
	virtual bool Deploy( void );

    virtual const Vector &GetBulletSpread( void );
    int WeaponSoundRealtime( WeaponSound_t shoot_type );

    static void DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime );

protected:
    int	m_nShotsFired;	// Number of consecutive shots fired
    float	m_flNextSoundTime;	// real-time clock of when to make next sound

private:
    CWeaponCoopMachineGun( const CWeaponCoopMachineGun & );
};

#endif // WEAPON_COOP_BASEMACHINEGUN_H