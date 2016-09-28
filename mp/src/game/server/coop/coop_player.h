//==== Woots 2016. http://creativecommons.org/licenses/by/2.5/mx/ ===========//

#ifndef COOP_PLAYER_H
#define COOP_PLAYER_H

#ifdef _WIN32
#pragma once
#endif

#include "hl2_player.h"
#include "playeranimsystem.h"

//================================================================================
// Jugador para el modo cooperativo
//================================================================================
class CCoopPlayer : public CHL2_Player
{
public:
    DECLARE_CLASS( CCoopPlayer, CHL2_Player );
    DECLARE_SERVERCLASS();
    DECLARE_DATADESC();

    CCoopPlayer();
    ~CCoopPlayer();

    // Utilidades
    virtual bool IsCrouching()  { return (GetFlags() & FL_DUCKING ) ? true : false; }
    virtual bool IsCrouching() const  { return (GetFlags() & FL_DUCKING ) ? true : false; }
    virtual bool IsInGround() { return (GetFlags() & FL_ONGROUND) ? true : false; }
    virtual bool InGodMode() { return (GetFlags() & FL_GODMODE) ? true : false; }
    virtual bool InBuddhaMode() { return (m_debugOverlays & OVERLAY_BUDDHA_MODE) ? true : false; }

    // Principales
    virtual void Spawn();
    virtual CBaseEntity *Respawn();
    virtual void Precache();

    virtual void PreThink();
    virtual void PostThink();

    virtual const char *GetPlayerModel();

    virtual void CreateViewModel( int index );

    // Salud
    virtual void Event_Killed( const CTakeDamageInfo &info );
    virtual void PlayerDeathThink();
    virtual void PlayerDeathPostThink();

    // Animaciones
    virtual void CreateAnimationSystem();
    virtual Activity TranslateActivity( Activity actBase );
    virtual void FixAngles();

    virtual CPlayerAnimationSystem *AnimationSystem() { return m_pAnimState; }
    virtual void HandleAnimEvent( animevent_t *event );
    virtual void SetAnimation( PLAYER_ANIM );
    virtual void DoAnimationEvent( PlayerAnimEvent_t, int = 0 );

    // Utilidades
    virtual bool FEyesVisible( CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL );
    virtual bool FEyesVisible( const Vector &vecTarget, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL );

    virtual const char* GetConVar( const char * );
    virtual void ExecuteCommand( const char * );

    virtual void Kick();
    virtual bool Possess( CCoopPlayer *pOther );
    virtual void Spectate( CCoopPlayer * = NULL );

    virtual bool ClientCommand( const CCommand &args );

protected:
    CPlayerAnimationSystem *m_pAnimState;

    CNetworkQAngle( m_angEyeAngles );
    CNetworkVar( int, m_iSpawnInterpCounter );
};
#endif // COOP_PLAYER_H