//==== Woots 2016. http://creativecommons.org/licenses/by/2.5/mx/ ===========//

#ifndef C_COOP_PLAYER_H
#define C_COOP_PLAYER_H

#ifdef _WIN32
#pragma once
#endif

#include "c_basehlplayer.h"
#include "playeranimsystem.h"

//================================================================================
// Jugador para el modo cooperativo
//================================================================================
class C_CoopPlayer : public C_BaseHLPlayer
{
public:
    DECLARE_CLASS( C_CoopPlayer, C_BaseHLPlayer );
    DECLARE_CLIENTCLASS();
    DECLARE_PREDICTABLE();
    DECLARE_INTERPOLATION();

    C_CoopPlayer();
    ~C_CoopPlayer();

    // Utilidades
    virtual bool IsCrouching()  { return (GetFlags() & FL_DUCKING ) ? true : false; }
    virtual bool IsCrouching() const  { return (GetFlags() & FL_DUCKING ) ? true : false; }
    virtual bool IsInGround() { return (GetFlags() & FL_ONGROUND) ? true : false; }
    virtual bool IsGod() { return (GetFlags() & FL_GODMODE) ? true : false; }

    virtual int GetButtons() { return m_nButtons; }
    virtual bool IsButtonPressing( int btn ) { return ((m_nButtons & btn)) ? true : false; }
    virtual bool IsButtonPressed( int btn ) { return ((m_afButtonPressed & btn)) ? true : false; }
    virtual bool IsButtonReleased( int btn ) { return ((m_afButtonReleased & btn)) ? true : false; }

    // Principales
    virtual void PreThink();
    virtual void PostThink();
    virtual void Simulate();

    virtual int GetIDTarget() const { return m_iIDEntIndex; }
    virtual void UpdateIDTarget();

    virtual void UpdateLookAt();

    virtual void ItemPreFrame();
    virtual void ItemPostFrame();

    // Posición y Render
    virtual const QAngle &EyeAngles();
    virtual const QAngle &GetRenderAngles();

    // Velocidad
    virtual bool CanSprint();
    virtual void StartSprinting();
    virtual void StopSprinting();
    virtual void HandleSpeedChanges();

    bool IsWalking( void ) { return m_fIsWalking; }
    virtual void StartWalking();
    virtual void StopWalking();

    //
    virtual CPlayerAnimationSystem *AnimationSystem() { return m_pAnimState; }
    virtual void CreateAnimationSystem();

    void DoAnimationEvent( PlayerAnimEvent_t, int = 0 );
    virtual void UpdateClientSideAnimation();

    virtual void PostDataUpdate( DataUpdateType_t updateType );
    virtual void OnDataChanged( DataUpdateType_t updateType );

    virtual CStudioHdr *OnNewModel();
    virtual void InitializePoseParams();

    virtual ShadowType_t ShadowCastType();
    virtual bool ShouldReceiveProjectedTextures( int flags );

    virtual Activity TranslateActivity( Activity actBase );

protected:
    QAngle m_angEyeAngles;
    CInterpolatedVar<QAngle> m_iv_angEyeAngles;

    bool m_fIsWalking;
    int m_iSpawnInterpCounter;
    int m_iSpawnInterpCounterCache;

    CPlayerAnimationSystem *m_pAnimState;
    CountdownTimer m_nBlinkTimer;

    int m_iIDEntIndex;
    Vector m_vLookAtTarget;

private:
    C_CoopPlayer( C_CoopPlayer & );
};

#endif // C_COOP_PLAYER_H