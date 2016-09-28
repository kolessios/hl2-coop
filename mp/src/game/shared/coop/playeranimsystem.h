//==== Woots 2016. http://creativecommons.org/licenses/by/2.5/mx/ ===========//

#ifndef IN_PLAYERANIMSTATE_H
#define IN_PLAYERANIMSTATE_H

#pragma once

#include "multiplayer_animstate.h"

#ifdef CLIENT_DLL
    class C_CoopPlayer;

    #define CCoopPlayer C_CoopPlayer
    #define CPlayerAnimationSystem C_PlayerAnimationSystem
#else
    class CCoopPlayer;
#endif

//================================================================================
// Sistema de procesamiento de animaciones
//================================================================================
class CPlayerAnimationSystem : public CMultiPlayerAnimState
{
    DECLARE_CLASS( CPlayerAnimationSystem, CMultiPlayerAnimState );

public:
    // Devolución
    CCoopPlayer *GetCoopPlayer() { return m_pInPlayer; }

    // Principales
    CPlayerAnimationSystem( CCoopPlayer *, MultiPlayerMovementData_t & );

    virtual bool ShouldComputeDirection();
    virtual bool ShouldComputeYaw();

    virtual Activity TranslateActivity( Activity );

    virtual void Update();
    virtual bool SetupPoseParameters( CStudioHdr * );

    virtual void ComputePoseParam_AimYaw( CStudioHdr * );

    virtual Activity CalcMainActivity();
    virtual void HandleStatus( Activity &nActivity, Activity nNormal, Activity nInjured, Activity nCalm );

    virtual bool HandleSwimming( Activity & );
    virtual bool HandleMoving( Activity & );
    virtual bool HandleDucking( Activity & );

    virtual void DoAnimationEvent( PlayerAnimEvent_t, int = 0 );

protected:
    CCoopPlayer *m_pInPlayer;
};

CPlayerAnimationSystem *CreatePlayerAnimationSystem( CCoopPlayer *, MultiPlayerMovementData_t & );

#endif // IN_PLAYERANIMSTATE_H