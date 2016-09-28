//==== Woots 2016. http://creativecommons.org/licenses/by/2.5/mx/ ===========//

#include "cbase.h"
#include "playeranimsystem.h"

#ifndef CLIENT_DLL
#include "coop_player.h"
#else
#include "c_coop_player.h"
#endif

#include "datacache/imdlcache.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//================================================================================
// Comandos
//================================================================================

//================================================================================
// Crea una nueva instancia [CPlayerAnimationSystem] para procesar
// las animaciones
//================================================================================
CPlayerAnimationSystem *CreatePlayerAnimationSystem( CCoopPlayer *pPlayer, MultiPlayerMovementData_t &data )
{
    MDLCACHE_CRITICAL_SECTION();
    return new CPlayerAnimationSystem( pPlayer, data );
}

//================================================================================
// Constructor
//================================================================================
CPlayerAnimationSystem::CPlayerAnimationSystem( CCoopPlayer *pPlayer, MultiPlayerMovementData_t &data ) : BaseClass( pPlayer, data )
{
    m_pInPlayer = pPlayer;
}

//================================================================================
// Traduce una actividad a otra
//================================================================================
Activity CPlayerAnimationSystem::TranslateActivity( Activity actBase )
{
    // Traducimos directamente al Jugador
    return GetCoopPlayer()->TranslateActivity( actBase );
}

//================================================================================
//================================================================================
bool CPlayerAnimationSystem::ShouldComputeDirection()
{
    // No esta vivo
    if ( !GetCoopPlayer()->IsAlive() ) 
        return false;

    return true;
}

//================================================================================
//================================================================================
bool CPlayerAnimationSystem::ShouldComputeYaw()
{
    // No esta vivo
    if ( !GetCoopPlayer()->IsAlive() ) 
        return false;

    if ( m_PoseParameterData.m_iAimYaw < 0 )
        return false;

    return true;
}

//================================================================================
//================================================================================
void CPlayerAnimationSystem::Update()
{
    // Profile the animation update.
    VPROF("CPlayerAnimationSystem::Update");

    // El Jugador no es válido
    if ( !GetCoopPlayer() ) 
    {
        Assert( !"GetInPlayer() == NULL" );
        return;
    }

    CStudioHdr *pStudioHdr = GetCoopPlayer()->GetModelPtr();

    // Ahora mismo no es un buen momento...
    if ( !ShouldUpdateAnimState() ) 
    {
        ClearAnimationState();
        return;
    }

    QAngle eyeAngles = GetCoopPlayer()->EyeAngles();

    m_flEyeYaw      = AngleNormalize( eyeAngles[YAW] );
    m_flEyePitch    = AngleNormalize( eyeAngles[PITCH] );

    // Calculamos la animación actual
    ComputeSequences( pStudioHdr );

    // No hemos podido configurar los parametros de movimiento
    if ( !SetupPoseParameters(pStudioHdr) )
        return;

    if ( ShouldComputeDirection() )
    {
        // Calculamos la dirección hacia donde deben ir las piernas
        ComputePoseParam_MoveYaw( pStudioHdr );

        // Calculamos la dirección hacia donde debe moverse el torso (Arriba/Abajo)
        ComputePoseParam_AimPitch( pStudioHdr );
    }

    // Calculamos la dirección hacia donde debe moverse el torso (Rotación)
    ComputePoseParam_AimYaw( pStudioHdr );

#ifdef CLIENT_DLL 
    /*
    float flSeqSpeed = GetCoopPlayer()->GetSequenceGroundSpeed( GetCoopPlayer()->GetSequence() );

    Vector vecVelocity;
    GetOuterAbsVelocity( vecVelocity );

    float flSpeed            = vecVelocity.Length2DSqr();
    bool bMoving_OnGround    = flSpeed > 0.01f && GetCoopPlayer()->GetGroundEntity();

    flSpeed = bMoving_OnGround ? clamp( (vecVelocity.Length2DSqr() / (flSeqSpeed*flSeqSpeed)), 0.2f, 2.0f ) : 1.0f;
    GetCoopPlayer()->SetPlaybackRate( flSpeed );
    */

    GetCoopPlayer()->SetPlaybackRate( 1.0f );
#endif

    ShowDebugInfo();
}

//================================================================================
//================================================================================
bool CPlayerAnimationSystem::SetupPoseParameters( CStudioHdr *pStudioHdr )
{
    // Ya hemos inicializado esto.
    if ( m_bPoseParameterInit )
        return true;

    // Save off the pose parameter indices.
    if ( !pStudioHdr )
        return false;

    bool bResult = BaseClass::SetupPoseParameters( pStudioHdr );

    // Movimiento del torso (arriba/abajo)
    //m_PoseParameterData.m_iAimPitch = GetBasePlayer()->LookupPoseParameter(pStudioHdr, "aim_pitch");

    // Movimiento del torso (rotación)
    //m_PoseParameterData.m_iAimYaw = GetBasePlayer()->LookupPoseParameter(pStudioHdr, "aim_yaw");

    return bResult;
}

//================================================================================
//================================================================================
void CPlayerAnimationSystem::ComputePoseParam_AimYaw( CStudioHdr *pStudioHdr )
{
    // Get the movement velocity.
    Vector vecVelocity;
    GetOuterAbsVelocity( vecVelocity );

    // Verificamos si nos estamos movimiendo o se debe forzar el Yaw
    bool bMoving = ( vecVelocity.Length() > 1.0f || m_bForceAimYaw ) ? true : false;

    // Si nos estamos moviendo la dirección de los pies debe coincidir a donde mira el jugador
    if ( bMoving )
    {
        m_flGoalFeetYaw = m_flEyeYaw;
    }
    else
    {
        // Iniciamos por primera vez las variables.
        if ( m_PoseParameterData.m_flLastAimTurnTime <= 0.0f )
        {
            m_flGoalFeetYaw        = m_flEyeYaw;
            m_flCurrentFeetYaw    = m_flEyeYaw;
            m_PoseParameterData.m_flLastAimTurnTime = gpGlobals->curtime;
        }

        // Verificamos si la dirección hacia donde mira el Jugador ya se ha alejado de la dirección de los pies
        else
        {
            float flYawDelta = AngleNormalize(  m_flGoalFeetYaw - m_flEyeYaw );

            // Así es, actualizamos el destino de la dirección
            if ( fabs(flYawDelta) > 70.0f )
            {
                float flSide = ( flYawDelta > 0.0f ) ? -1.0f : 1.0f;
                m_flGoalFeetYaw += ( 45.0f * flSide );
            }
        }
    }

    // Normalizamos el destino
    m_flGoalFeetYaw = AngleNormalize( m_flGoalFeetYaw );

    // Actualización
    if ( m_flGoalFeetYaw != m_flCurrentFeetYaw )
    {
        // Estamos forzando el Yaw, lo hacemos al instante
        if ( m_bForceAimYaw )
        {
            m_flCurrentFeetYaw = m_flGoalFeetYaw;
        }

        // Si no, volteamos el modelo poco a poco
        else
        {
            // Velocidad para voltearnos            
            float flYawRate = m_MovementData.m_flBodyYawRate;
                
            // Si nos estamos moviendo, lo aumentamos
            if ( bMoving )
                flYawRate += 220.0f;

            // Calculamos las diferencias
            ConvergeYawAngles( m_flGoalFeetYaw, flYawRate, gpGlobals->frametime, m_flCurrentFeetYaw );
            m_flLastAimTurnTime = gpGlobals->curtime;
        }
    }

    // Establecemos la posición
    if ( ShouldComputeYaw() )
        m_angRender[YAW] = m_flCurrentFeetYaw;


    // Find the aim(torso) yaw base on the eye and feet yaws.
    float flAimYaw = m_flEyeYaw - m_flCurrentFeetYaw;
    flAimYaw = AngleNormalize( flAimYaw );

    // Set the aim yaw and save.
    GetBasePlayer()->SetPoseParameter( pStudioHdr, m_PoseParameterData.m_iAimYaw, flAimYaw );
    m_DebugAnimData.m_flAimYaw = flAimYaw;

    // Turn off a force aim yaw - either we have already updated or we don't need to.
    m_bForceAimYaw = false;

#ifndef CLIENT_DLL
    if ( ShouldComputeYaw() )
    {
        QAngle angle = GetBasePlayer()->GetAbsAngles();
        angle[YAW] = m_flCurrentFeetYaw;

        GetBasePlayer()->SetAbsAngles( angle );
    }
#endif
}

//================================================================================
// Calcula la mejor animación para este momento
//================================================================================
Activity CPlayerAnimationSystem::CalcMainActivity()
{
    // Sin hacer nada, calmados...
    Activity nActivity = ACT_HL2MP_IDLE;

    /*
    || 
        HandleSwimming(nActivity) || 
        HandleDying(nActivity)
    */

    if ( 
        HandleJumping(nActivity) || 
        HandleDucking(nActivity) 
        )
    { }
    else
    {
        HandleMoving( nActivity );
    }

    return nActivity;
}

//================================================================================
//================================================================================
void CPlayerAnimationSystem::HandleStatus( Activity &nActivity, Activity nNormal, Activity nInjured, Activity nCalm )
{
    nActivity = nNormal;
}

//================================================================================
//================================================================================
bool CPlayerAnimationSystem::HandleSwimming( Activity &nActivity )
{
    // El nivel de agua no es suficiente
    if ( GetCoopPlayer()->GetWaterLevel() < WL_Waist ) {
        m_bInSwim = false;
        return false;
    }

    // Nos estamos moviendo mientras nadamos
    //if ( GetOuterXYSpeed() > m_MovementData.m_flWalkSpeed )
        nActivity = ACT_HL2MP_IDLE;

    // Estamos quietos
    //else
      //  nActivity = ACT_MP_SWIM_IDLE;

    m_bInSwim = true;
    return true;
}

//================================================================================
//================================================================================
bool CPlayerAnimationSystem::HandleMoving( Activity &nActivity )
{
    float flSpeed = GetOuterXYSpeed();

    // Estamos corriendo
    if ( flSpeed > m_MovementData.m_flRunSpeed )
        nActivity = ACT_HL2MP_RUN;

    // Estamos caminando
    else if ( flSpeed > m_MovementData.m_flWalkSpeed )
        nActivity = ACT_HL2MP_RUN;//ACT_MP_WALK;

    else
        return false;

    return true;
}

//================================================================================
//================================================================================
bool CPlayerAnimationSystem::HandleDucking( Activity &nActivity )
{
    // No estamos agachados
    if ( !GetCoopPlayer()->IsCrouching() )
        return false;

    float flSpeed = GetOuterXYSpeed();

    if ( flSpeed > m_MovementData.m_flWalkSpeed )
        nActivity = ACT_HL2MP_WALK_CROUCH;
    else
        nActivity = ACT_HL2MP_IDLE_CROUCH;

    return true;
}

//================================================================================
// Procesa un evento y determina la animación
//================================================================================
void CPlayerAnimationSystem::DoAnimationEvent( PlayerAnimEvent_t pEvent, int nData )
{
    switch ( pEvent )
    {
        // Disparo primario
        case PLAYERANIMEVENT_ATTACK_PRIMARY:
            RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_HL2MP_GESTURE_RANGE_ATTACK );
        break;

        // Disparo secundario
        case PLAYERANIMEVENT_ATTACK_SECONDARY:
            RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_HL2MP_GESTURE_RANGE_ATTACK );
        break;

        // Mostrar el arma
        case PLAYERANIMEVENT_DEPLOY:
            //RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_MP_DEPLOY );      
            Assert( false );
        break;

        case PLAYERANIMEVENT_RELOAD:
        {
            // Weapon reload.
            RestartGesture( GESTURE_SLOT_ATTACK_AND_RELOAD, ACT_HL2MP_GESTURE_RELOAD );

            // Set the modified reload playback rate
            float flPlaybackRate = 1.0f;
            #if defined(TF_CLIENT_DLL) || defined(TF_DLL)
                CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( GetBasePlayer(), flPlaybackRate, mult_reload_time );
                CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( GetBasePlayer(), flPlaybackRate, mult_reload_time_hidden );
                CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( GetBasePlayer(), flPlaybackRate, fast_reload );
            #endif
            m_aGestureSlots[ GESTURE_SLOT_ATTACK_AND_RELOAD ].m_pAnimLayer->m_flPlaybackRate = flPlaybackRate;

            break;
        }

        default:
            BaseClass::DoAnimationEvent( pEvent, nData );
        break;
    }
}