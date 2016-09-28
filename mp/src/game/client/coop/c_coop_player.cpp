//==== Woots 2016. http://creativecommons.org/licenses/by/2.5/mx/ ===========//

#include "cbase.h"
#include "c_coop_player.h"

#include "prediction.h"
#include "view.h"

#include "in_buttons.h"
#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	HL2_WALK_SPEED hl2_walkspeed.GetFloat()
#define	HL2_NORM_SPEED hl2_normspeed.GetFloat()
#define	HL2_SPRINT_SPEED hl2_sprintspeed.GetFloat()

//================================================================================
// Información y Red
//================================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( CoopPlayer, DT_CoopPlayer )

// Local Player
BEGIN_RECV_TABLE_NOBASE( C_CoopPlayer, DT_CoopLocalPlayerExclusive )
    //RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
END_RECV_TABLE()

// Other Players
BEGIN_RECV_TABLE_NOBASE( C_CoopPlayer, DT_CoopNonLocalPlayerExclusive )
    //RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
END_RECV_TABLE()

BEGIN_NETWORK_TABLE( C_CoopPlayer, DT_CoopPlayer )
    RecvPropDataTable( "localdata", 0, 0, &REFERENCE_RECV_TABLE(DT_CoopLocalPlayerExclusive) ),
    RecvPropDataTable( "nonlocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_CoopNonLocalPlayerExclusive) ),

    RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
    RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),
    RecvPropInt( RECVINFO(m_iSpawnInterpCounter) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_CoopPlayer )
    DEFINE_PRED_FIELD( m_nSequence, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ), 
    DEFINE_PRED_FIELD( m_flPlaybackRate, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
    DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
      
    //DEFINE_PRED_FIELD( m_nNewSequenceParity, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
    //DEFINE_PRED_FIELD( m_nResetEventsParity, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()

//================================================================================
// Constructor
//================================================================================
C_CoopPlayer::C_CoopPlayer() : m_iv_angEyeAngles( "C_CoopPlayer::m_iv_angEyeAngles" )
{
    //
    SetPredictionEligible( true );

    // Viewangles (Lo mismo que pl.v_angle)
    m_angEyeAngles.Init();
    AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

    // Parpadeo
    m_nBlinkTimer.Invalidate();
    
    // Default
    m_iIDEntIndex = 0;
    m_iSpawnInterpCounterCache = 0;
}

//================================================================================
//================================================================================
C_CoopPlayer::~C_CoopPlayer()
{
    // Liberamos el sistema de animación
    if ( AnimationSystem() )
        AnimationSystem()->Release();
}

//================================================================================
//================================================================================
void C_CoopPlayer::PreThink() 
{
    RemoveFlag( FL_ATCONTROLS );

    QAngle vTempAngles = GetLocalAngles();

    if ( GetLocalPlayer() == this )
        vTempAngles[PITCH] = EyeAngles()[PITCH];
    else
        vTempAngles[PITCH] = m_angEyeAngles[PITCH];

    if ( vTempAngles[YAW] < 0.0f )
        vTempAngles[YAW] += 360.0f;

    SetLocalAngles( vTempAngles );

    // PreThink
    BaseClass::PreThink();

    HandleSpeedChanges();

    if ( m_HL2Local.m_flSuitPower <= 0.0f )
	{
		if ( IsSprinting() )
		{
			StopSprinting();
		}
	}
}

//================================================================================
//================================================================================
void C_CoopPlayer::PostThink() 
{
    BaseClass::PostThink();

    // Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();
}

//================================================================================
//================================================================================
void C_CoopPlayer::Simulate() 
{
    QAngle vTempAngles = GetLocalAngles();
	vTempAngles[PITCH] = m_angEyeAngles[PITCH];
    SetLocalAngles( vTempAngles );

    SetLocalAnglesDim( X_INDEX, 0 );

    // Linterna
    // Si el Jugador local me esta mirando, el se encargara de mi linterna
    if ( IsLocalPlayer() )
    {
        if ( IsAlive() )
        {
            UpdateIDTarget();
        }
    }
    else
    {
    }

    // Mientras sigas vivo
    if ( IsAlive() )
    {
        UpdateLookAt();

        //NDebugOverlay::ScreenText( 0.10f, 0.13f, UTIL_VarArgs("Client Velocity: %.2f %.2f %.2f", GetLocalVelocity().x, GetLocalVelocity().y, GetLocalVelocity().z), 255, 255, 255, 255, 0.0001f );
    }

    // BaseClass
    BaseClass::Simulate();
}

//================================================================================
//================================================================================
void C_CoopPlayer::UpdateIDTarget() 
{
    Assert( !IsLocalPlayer() );

    // Clear old target and find a new one
	m_iIDEntIndex = 0;

    // don't show IDs in chase spec mode
	if ( GetObserverMode() == OBS_MODE_CHASE || 
		 GetObserverMode() == OBS_MODE_DEATHCAM )
		 return;

    trace_t tr;
	Vector vecStart, vecEnd;

	VectorMA( MainViewOrigin(), 1500, MainViewForward(), vecEnd );
	VectorMA( MainViewOrigin(), 10,   MainViewForward(), vecStart );
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		C_BaseEntity *pEntity = tr.m_pEnt;

		if ( pEntity && (pEntity != this) )
		{
			m_iIDEntIndex = pEntity->entindex();
		}
	}
}

//================================================================================
//================================================================================
void C_CoopPlayer::UpdateLookAt()
{
    // Parpadeamos!
    if ( m_nBlinkTimer.IsElapsed() )
    {
        m_blinktoggle = !m_blinktoggle;
        m_nBlinkTimer.Start( RandomFloat(1.5, 4.0f) );
    }

    Vector vecForward;
    AngleVectors( EyeAngles(), &vecForward );

    // Miramos hacia enfrente
    m_viewtarget = EyePosition() + 50.0f * vecForward;
}

//================================================================================
//================================================================================
void C_CoopPlayer::ItemPreFrame() 
{
    if ( GetFlags() & FL_FROZEN )
		 return;

	// Disallow shooting while zooming
	if ( m_nButtons & IN_ZOOM )
	{
		//FIXME: Held weapons like the grenade get sad when this happens
		m_nButtons &= ~(IN_ATTACK|IN_ATTACK2);
	}

	BaseClass::ItemPreFrame();
}

//================================================================================
//================================================================================
void C_CoopPlayer::ItemPostFrame() 
{
    if ( GetFlags() & FL_FROZEN )
		 return;

	BaseClass::ItemPostFrame();
}

//================================================================================
//================================================================================
const QAngle &C_CoopPlayer::EyeAngles()
{
    if ( IsLocalPlayer() )
        return BaseClass::EyeAngles();
    else
        return m_angEyeAngles;
}

//================================================================================
//================================================================================
const QAngle& C_CoopPlayer::GetRenderAngles()
{
    if ( IsRagdoll() )
    {
        return vec3_angle;
    }
    else
    {
        if ( AnimationSystem() )
            return AnimationSystem()->GetRenderAngles();
        else
            return BaseClass::GetRenderAngles();
    }
}

//================================================================================
//================================================================================
bool C_CoopPlayer::CanSprint() 
{
    return ( (!m_Local.m_bDucked && !m_Local.m_bDucking) && (GetWaterLevel() != 3) );
}

//================================================================================
//================================================================================
void C_CoopPlayer::StartSprinting() 
{
    ConVarRef hl2_sprintspeed("hl2_sprintspeed");

    if( m_HL2Local.m_flSuitPower < 10 )
	{
		// Don't sprint unless there's a reasonable
		// amount of suit power.
		CPASAttenuationFilter filter( this );
		filter.UsePredictionRules();
		EmitSound( filter, entindex(), "HL2Player.SprintNoPower" );
		return;
	}

	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();
	EmitSound( filter, entindex(), "HL2Player.SprintStart" );

	SetMaxSpeed( HL2_SPRINT_SPEED );
	m_fIsSprinting = true;
}

//================================================================================
//================================================================================
void C_CoopPlayer::StopSprinting() 
{
    ConVarRef hl2_normspeed("hl2_normspeed");

    SetMaxSpeed( HL2_NORM_SPEED );
	m_fIsSprinting = false;
}

//================================================================================
//================================================================================
void C_CoopPlayer::HandleSpeedChanges() 
{
    int buttonsChanged = m_afButtonPressed | m_afButtonReleased;

	if( buttonsChanged & IN_SPEED )
	{
		// The state of the sprint/run button has changed.
		if ( IsSuitEquipped() )
		{
			if ( !(m_afButtonPressed & IN_SPEED)  && IsSprinting() )
			{
				StopSprinting();
			}
			else if ( (m_afButtonPressed & IN_SPEED) && !IsSprinting() )
			{
				if ( CanSprint() )
				{
					StartSprinting();
				}
				else
				{
					// Reset key, so it will be activated post whatever is suppressing it.
					m_nButtons &= ~IN_SPEED;
				}
			}
		}
	}
	else if( buttonsChanged & IN_WALK )
	{
		if ( IsSuitEquipped() )
		{
			// The state of the WALK button has changed. 
			if( IsWalking() && !(m_afButtonPressed & IN_WALK) )
			{
				StopWalking();
			}
			else if( !IsWalking() && !IsSprinting() && (m_afButtonPressed & IN_WALK) && !(m_nButtons & IN_DUCK) )
			{
				StartWalking();
			}
		}
	}

	if ( IsSuitEquipped() && m_fIsWalking && !(m_nButtons & IN_WALK)  ) 
		StopWalking();
}

//================================================================================
//================================================================================
void C_CoopPlayer::StartWalking() 
{
    ConVarRef hl2_walkspeed("hl2_walkspeed");

    SetMaxSpeed( HL2_WALK_SPEED );
	m_fIsWalking = true;
}

//================================================================================
//================================================================================
void C_CoopPlayer::StopWalking( void )
{
    ConVarRef hl2_normspeed("hl2_normspeed");

	SetMaxSpeed( HL2_NORM_SPEED );
	m_fIsWalking = false;
}

//================================================================================
//================================================================================
void C_CoopPlayer::DoAnimationEvent( PlayerAnimEvent_t nEvent, int nData )
{
    if ( IsLocalPlayer() )
    {
        // Evitamos reproducir animaciones que llegaran después
        if ( prediction->InPrediction() /*&& !prediction->IsFirstTimePredicted()*/ )
            return;
    }

    MDLCACHE_CRITICAL_SECTION();

    // Procesamos la animación en el cliente
    if ( AnimationSystem() )
        AnimationSystem()->DoAnimationEvent( nEvent, nData );
}

//================================================================================
//================================================================================
void C_CoopPlayer::UpdateClientSideAnimation()
{
    //
    if ( AnimationSystem() )
        AnimationSystem()->Update();

    BaseClass::UpdateClientSideAnimation();
}

//================================================================================
//================================================================================
void C_CoopPlayer::PostDataUpdate( DataUpdateType_t updateType )
{
    // C_BaseEntity assumes we're networking the entity's angles, so pretend that it
    // networked the same value we already have.
    //SetNetworkAngles( GetLocalAngles() );

    if ( updateType == DATA_UPDATE_CREATED )
    {
        // Creamos el sistema de animaciones
        CreateAnimationSystem();
    }

    if ( m_iSpawnInterpCounter != m_iSpawnInterpCounterCache )
	{
		MoveToLastReceivedPosition( true );
		ResetLatched();
		m_iSpawnInterpCounterCache = m_iSpawnInterpCounter;
	}
    
    // Base!
    BaseClass::PostDataUpdate( updateType );
}

//================================================================================
//================================================================================
void C_CoopPlayer::OnDataChanged( DataUpdateType_t type )
{
    // Base!
    BaseClass::OnDataChanged( type );

    // Make sure we're thinking
    if ( type == DATA_UPDATE_CREATED )
        SetNextClientThink( CLIENT_THINK_ALWAYS );

    //
    UpdateVisibility();
}

//================================================================================
//================================================================================
CStudioHdr *C_CoopPlayer::OnNewModel()
{
    CStudioHdr *pHDR = BaseClass::OnNewModel();
    InitializePoseParams();

    // Reset the players animation states, gestures
    if ( AnimationSystem() )
        AnimationSystem()->OnNewModel();

    return pHDR;
}

//================================================================================
//================================================================================
void C_CoopPlayer::InitializePoseParams()
{
    if ( !IsAlive() )
        return;

    CStudioHdr *pHDR = GetModelPtr();

    if ( !pHDR )
        return;

    for ( int i = 0; i < pHDR->GetNumPoseParameters() ; i++ )
        SetPoseParameter( pHDR, i, 0.0 );
}


//================================================================================
//================================================================================
ShadowType_t C_CoopPlayer::ShadowCastType()
{
    if ( !IsVisible() )
         return SHADOWS_NONE;

    return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
}

//================================================================================
//================================================================================
bool C_CoopPlayer::ShouldReceiveProjectedTextures( int flags )
{
    if ( !IsVisible() )
         return false;

    return true;
}