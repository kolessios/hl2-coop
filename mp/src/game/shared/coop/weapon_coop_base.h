//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef WEAPON_COOP_BASE_H
#define WEAPON_COOP_BASE_H

#ifdef _WIN32
#pragma once
#endif

#include "basecombatweapon_shared.h"

#ifdef CLIENT_DLL
    #define CWeaponCoopBase C_WeaponCoopBase
    void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip );
#endif

//================================================================================
// La base para un arma cooperativo
//================================================================================
class CWeaponCoopBase : public CBaseCombatWeapon
{
public:
    DECLARE_CLASS( CWeaponCoopBase, CBaseCombatWeapon );
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();
    DECLARE_DATADESC();

    CWeaponCoopBase();

    // Predicción
    virtual bool IsPredicted() const { return true; }

    #ifdef CLIENT_DLL
    virtual void OnDataChanged( DataUpdateType_t type );
    virtual bool ShouldPredict();
    #endif

    //
    virtual void Spawn();
    virtual void WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f );

    virtual void PrimaryAttack();
};

#endif // WEAPON_COOP_BASE_H