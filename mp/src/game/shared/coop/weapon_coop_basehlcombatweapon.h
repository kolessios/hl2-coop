//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef WEAPON_COOP_BASEHLCOMBATWEAPON_H
#define WEAPON_COOP_BASEHLCOMBATWEAPON_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_coop_base.h"

#ifdef CLIENT_DLL
    #define CWeaponCoopBaseHLCombat C_WeaponCoopBaseHLCombat
#endif

//================================================================================
// La base para un arma cooperativo
//================================================================================
class CWeaponCoopBaseHLCombat : public CWeaponCoopBase
{
public:
    DECLARE_CLASS( CWeaponCoopBaseHLCombat, CWeaponCoopBase );
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();
    DECLARE_DATADESC();
    
    CWeaponCoopBaseHLCombat() { }

    virtual bool WeaponShouldBeLowered( void );

	virtual bool Ready( void );
	virtual bool Lower( void );
	virtual bool Deploy( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void WeaponIdle( void );

    virtual void ItemHolsterFrame( void );

	virtual void AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual	float CalcViewmodelBob( void );

	virtual Vector GetBulletSpread( WeaponProficiency_t proficiency );
	virtual float GetSpreadBias( WeaponProficiency_t proficiency );

	virtual const WeaponProficiencyInfo_t *GetProficiencyValues();
	static const WeaponProficiencyInfo_t *GetDefaultProficiencyValues();

protected:
    bool			m_bLowered;			// Whether the viewmodel is raised or lowered
	float			m_flRaiseTime;		// If lowered, the time we should raise the viewmodel
	float			m_flHolsterTime;	// When the weapon was holstered

private:
    CWeaponCoopBaseHLCombat( const CWeaponCoopBaseHLCombat & );
};

#endif // WEAPON_COOP_BASEHLCOMBATWEAPON_H