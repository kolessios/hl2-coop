//==== Woots 2016. http://creativecommons.org/licenses/by/2.5/mx/ ===========//

#include "cbase.h"
#include "coop_player.h"

#include "playeranimstate_proxy.h"
#include "hl2_gamerules.h"

#include "soundent.h"
#include "predicted_viewmodel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//================================================================================
// Comandos
//================================================================================

ConVar coop_respawn_time("coop_respawn_time", "3", FCVAR_REPLICATED | FCVAR_NOTIFY);

//================================================================================
// Información y Red
//================================================================================

LINK_ENTITY_TO_CLASS( player, CCoopPlayer );
PRECACHE_REGISTER( player );

// Local Players
BEGIN_SEND_TABLE_NOBASE( CCoopPlayer, DT_CoopLocalPlayerExclusive )
    // send a hi-res origin to the local player for use in prediction
    //SendPropVector( SENDINFO(m_vecOrigin), -1,  SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
END_SEND_TABLE()

// Other Players
BEGIN_SEND_TABLE_NOBASE( CCoopPlayer, DT_CoopNonLocalPlayerExclusive )
    // send a low-res origin to other players
    //SendPropVector( SENDINFO(m_vecOrigin), -1,  SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
END_SEND_TABLE()

IMPLEMENT_SERVERCLASS_ST( CCoopPlayer, DT_CoopPlayer )
    // Excluimos el envio de información del sistema de animaciones.
    // Este será procesado en el cliente por el [AnimationSystem] 
    //SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
    //SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),
    SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
    SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),    
    SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),    
    //SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
    SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),    
    //SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

    // Data that only gets sent to the local player.
    SendPropDataTable( "localdata", 0, &REFERENCE_SEND_TABLE(DT_CoopLocalPlayerExclusive), SendProxy_SendLocalDataTable ),

    // Data that gets sent to all other players
    SendPropDataTable( "nonlocaldata", 0, &REFERENCE_SEND_TABLE(DT_CoopNonLocalPlayerExclusive), SendProxy_SendNonLocalDataTable ),

    SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11, SPROP_CHANGES_OFTEN ),
    SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11, SPROP_CHANGES_OFTEN ),
    SendPropInt( SENDINFO(m_iSpawnInterpCounter), 4 ),
END_SEND_TABLE()

BEGIN_DATADESC( CCoopPlayer )
END_DATADESC()

//================================================================================
// Constructor
//================================================================================
CCoopPlayer::CCoopPlayer()
{
    // Las animaciones son procesadas en el cliente
    UseClientSideAnimation();

    // Default
    m_iSpawnInterpCounter = 0;

    // Creamos el sistema de animaciones
    CreateAnimationSystem();

    // Iniciamos la variable
    m_angEyeAngles.Init();
}

//================================================================================
//================================================================================
CCoopPlayer::~CCoopPlayer()
{
    // Liberamos el sistema de animaciones
    if ( AnimationSystem() ) 
        delete m_pAnimState;
}

//================================================================================
// Spawn
//================================================================================
void CCoopPlayer::Spawn() 
{
    Precache();

    // Base
    BaseClass::Spawn();

    // Modelo del Jugador
    SetModel( GetPlayerModel() );

    //CreateViewModel(0);

    // Propiedes fisicas
    RemoveSolidFlags( FSOLID_NOT_SOLID );

    // Nos quitamos cualquier efecto de color
    SetRenderColor( 255, 255, 255 );

    // Reiniciamos el sistema de animaciones
    DoAnimationEvent( PLAYERANIMEVENT_SPAWN );

    // Estamos en el suelo
    AddFlag( FL_ONGROUND );

    m_nRenderFX             = kRenderNormal;
    m_Local.m_iHideHUD      = 0;
    m_iSpawnInterpCounter   = (m_iSpawnInterpCounter + 1) % 8;
    m_Local.m_bDucked       = false;

    SetPlayerUnderwater( false );
}

//================================================================================
//================================================================================
CBaseEntity *CCoopPlayer::Respawn() 
{
    Spawn();
    return NULL;
}

//================================================================================
// Guarda objetos necesarios en caché
//================================================================================
void CCoopPlayer::Precache() 
{
    BaseClass::Precache();

    // Este modelo siempre es necesario
    PrecacheModel( "models/player.mdl" );

    // Modelo del Jugador
    PrecacheModel( GetPlayerModel() );

    // Luces y Glow
    PrecacheMaterial( "sprites/light_glow01" );
    PrecacheMaterial( "sprites/glow01" );
    PrecacheMaterial( "sprites/spotlight01_proxyfade" );
    PrecacheMaterial( "sprites/glow_test02.vmt" );
    PrecacheMaterial( "sprites/light_glow03" );

    // Efectos
    PrecacheMaterial( "effects/flashlight_border" );
    PrecacheMaterial( "effects/flashlight001" );
    PrecacheMaterial( "effects/muzzleflash_light" );
}

//================================================================================
//================================================================================
void CCoopPlayer::PreThink() 
{
    RemoveFlag( FL_ATCONTROLS );

    QAngle vOldAngles  = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

    vTempAngles = EyeAngles();

    if ( vTempAngles[PITCH] > 180.0f )
		vTempAngles[PITCH] -= 360.0f;

    SetLocalAngles( vTempAngles );

    // Arreglamos
    //FixAngles();

    // BaseClass
    BaseClass::PreThink();

    SetLocalAngles( vOldAngles );
}

//================================================================================
//================================================================================
void CCoopPlayer::PostThink() 
{
    // BaseClass
    BaseClass::PostThink();

    // Actualizamos el sistema de animaciones
    if ( AnimationSystem() )
        AnimationSystem()->Update();

    m_angEyeAngles = EyeAngles();

    QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );

    //NDebugOverlay::ScreenText( 0.10f, 0.10f, UTIL_VarArgs("Server Velocity: %.2f %.2f %.2f", GetLocalVelocity().x, GetLocalVelocity().y, GetLocalVelocity().z), 255, 255, 255, 255, 0.1f );
}

//================================================================================
// Devuelve el modelo que debe tener el jugador
//================================================================================
const char *CCoopPlayer::GetPlayerModel() 
{
    // YO :)
    if ( FStrEq(GetPlayerName(), "Ivan") || UTIL_GetListenServerHost() == this )
    {
        return "models/player/rebel/hero_male.mdl";
    }

    // Valeria :3
    else
    {
        return "models/player/rebel/hero_female.mdl";
    }
}

//================================================================================
// Crea el viewmodel, el modelo en primera persona
//================================================================================
void CCoopPlayer::CreateViewModel( int index ) 
{
    // Ya ha sido creado.
    if ( GetViewModel(index) )
        return;

    CPredictedViewModel *vm = (CPredictedViewModel *)CreateEntityByName("predicted_viewmodel");

    if ( vm ) 
    {
        vm->SetAbsOrigin( GetAbsOrigin() );
        vm->SetOwner( this );
        vm->SetIndex( index );

        DispatchSpawn( vm );
        vm->FollowEntity( this, false );

        m_hViewModel.Set( index, vm );
    }
}

//================================================================================
//================================================================================
void CCoopPlayer::Event_Killed( const CTakeDamageInfo & info ) 
{
    CSound *pSound              = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex(edict()) );
    IPhysicsObject *pObject     = VPhysicsGetObject();

    // Aragghhhhhh!....
    DeathSound( info );

    // Notificamos a las reglas del Juego
    HL2GameRules()->PlayerKilled( this, info );

    // Ya no estamos usando nada
    ClearUseEntity();

    // Propiedades físicas
    if ( pObject ) {
        pObject->RecheckContactPoints();
    }

    // Reiniciamos el sonido
    if ( pSound ) {
        pSound->Reset();
    }

    // Limpiamos los sonidos de las armas
    EmitSound( "BaseCombatCharacter.StopWeaponSounds" );

    // Evitamos problemas con la salud
    m_iHealth = 0;

    // Apgamos la linterna
    if ( FlashlightIsOn() )
        RemoveEffects( EF_DIMLIGHT );

    // Ya no estamos en ningún area del NavMesh
    ClearLastKnownArea();

    // 
    SendOnKilledGameEvent( info );

    // Estoy en proceso de morir
    pl.deadflag     = true;
    m_flDeathTime   = gpGlobals->curtime;
    m_lifeState     = LIFE_DYING;

    StopZooming();
    DoAnimationEvent( PLAYERANIMEVENT_DIE );
}

//================================================================================
// Pensamiento al morir
//================================================================================
void CCoopPlayer::PlayerDeathThink()
{
    // Volvemos a pensar en 0.1s
    SetNextThink( gpGlobals->curtime + 0.1f );

    // Estamos en el suelo
    if ( IsInGround() ) 
    {
        float flForward = GetAbsVelocity().Length() - 20;

        if ( flForward <= 0 ) {
            SetAbsVelocity( vec3_origin );
        }
        else {
            Vector vecNewVelocity = GetAbsVelocity();
            VectorNormalize( vecNewVelocity );
            vecNewVelocity *= flForward;
            SetAbsVelocity( vecNewVelocity );
        }
    }

    // Tenemos armas
    if ( HasWeapons() ) {
        PackDeadPlayerItems();
    }

    // Reproduciendo animación de muerte...
    if ( m_lifeState == LIFE_DYING ) 
    {
        // No somos solidos
        AddSolidFlags( FSOLID_NOT_SOLID );

        // Aumentamos un frame a la espera de la animación
        if ( GetModelIndex() && !IsSequenceFinished() ) 
        {
            ++m_iRespawnFrames;

            // Menos de 60 frames, por ahora todo bien y seguimos esperando...
            // Pero si sobrepasamos los 60 frames significa que hubo un problema
            // con la animación (o es muy larga) y debemos forzar la muerte
            if ( m_iRespawnFrames < 60 )
                return;
            else
                Assert(!"m_iRespawnFrames > 60!");
        }

        // Creamos nuestro cadaver
        BecomeRagdollOnClient( vec3_origin );

        // Ya no estamos en el suelo
        SetMoveType( MOVETYPE_FLYGRAVITY );
        SetGroundEntity( NULL );

        // Invisibles
        AddEffects( EF_NODRAW );

        // Hemos muerto
        m_lifeState            = LIFE_DEAD;
        m_flDeathAnimTime    = m_flDeathTime = gpGlobals->curtime ;// Volvemos a establecerlo para que "CInGameRules::FlPlayerSpawnTime" funcione correctamente

        return;
    }

    CBaseEntity *pOwner = GetOwnerEntity();

    // Notificamos a nuestro dueño
    if ( pOwner )
    {
        pOwner->DeathNotice( this );
        SetOwnerEntity( NULL );
    }

    // Paramos todas las animaciones
    StopAnimation();

    //
    AddEffects( EF_NOINTERP );
    SetPlaybackRate( 0.0f );

    // Pensamiento al estar muerto
    PlayerDeathPostThink();
}

//================================================================================
// Pensamiento al estar muerto (modo espectador)
//================================================================================
void CCoopPlayer::PlayerDeathPostThink()
{
    // Entramos en modo espectador
    Spectate();

    // El Jugador no puede hacer respawn
    if ( !HL2GameRules()->FPlayerCanRespawn(this) )
        return;

    // ¡Podemos reaparecer!
    m_lifeState = LIFE_RESPAWNABLE;

    // ¡Respawneamos!
    if ( gpGlobals->curtime >= (GetDeathTime() + coop_respawn_time.GetFloat()) ) 
    {
        SetNextThink( TICK_NEVER_THINK );
        Respawn();
    }
}

//================================================================================
//================================================================================
void CCoopPlayer::FixAngles() 
{
    // Evitamos errores con el PITCH
    // en servidor
    QAngle angles = GetLocalAngles();
    angles[PITCH] = 0;
    SetLocalAngles( angles );

    // Copiamos los angulos de la cámara (Viewangles)
    // y lo transmitimos a los jugadores excepto local
    QAngle viewAngles = EyeAngles();

    // Si no tenemos un sistema de animación debemos
    // limpiar el PITCH para evitar problemas con el modelo
    if ( !AnimationSystem() )
        viewAngles[PITCH] = 0;

    // Evitamos errores con el YAW
    //if ( angles[YAW] < 0 )
        //angles[YAW] += 360.0f;

    m_angEyeAngles = viewAngles;
}

//================================================================================
//================================================================================
void CCoopPlayer::HandleAnimEvent( animevent_t *event ) 
{
    BaseClass::HandleAnimEvent( event );
}

//================================================================================
//================================================================================
void CCoopPlayer::SetAnimation( PLAYER_ANIM nAnim ) 
{
    if ( nAnim == PLAYER_WALK || nAnim == PLAYER_IDLE ) return;

    // Recarga
    if ( nAnim == PLAYER_RELOAD ) 
    {
        DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
    }
    else if ( nAnim == PLAYER_JUMP ) 
    {
        DoAnimationEvent( PLAYERANIMEVENT_JUMP );
    }
    else if ( nAnim == PLAYER_ATTACK1 ) 
    {
        DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
    }
    else 
    {
        Assert( !"Deprecated SetAnimation" );
    }
}

//================================================================================
//================================================================================
void CCoopPlayer::DoAnimationEvent( PlayerAnimEvent_t pEvent, int nData ) 
{
    // No hay sistema de animación
    if ( !AnimationSystem() ) return;
    
    // Procesamos la animación en el servidor
    AnimationSystem()->DoAnimationEvent( pEvent, nData );

    // La enviamos al cliente
    SendPlayerAnimation( this, pEvent, nData );
}

//================================================================================
// Devuelve si la entidad es visible por el jugador y esta dentro del campo de visión
//================================================================================
bool CCoopPlayer::FEyesVisible( CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker )
{
    if ( pEntity == this )
        return false;

    if ( !FInViewCone(pEntity) )
        return false;

    return CBaseEntity::FVisible( pEntity, traceMask, ppBlocker );
}

//================================================================================
// Devuelve si la posición es visible por el jugador y esta dentro del campo de visión
//================================================================================
bool CCoopPlayer::FEyesVisible( const Vector &vecTarget, int traceMask, CBaseEntity **ppBlocker )
{
    if ( !vecTarget.IsValid() )
        return false;

    if ( !FInViewCone(vecTarget) )
        return false;

    return CBaseEntity::FVisible( vecTarget, traceMask, ppBlocker );
}

//================================================================================
// Devuelve el valor de un comando, sea servidor o del cliente
//================================================================================
const char *CCoopPlayer::GetConVar( const char *pCommand )
{
    ConVarRef command( pCommand );

    // Es un comando de servidor...
    if ( command.IsValid() ) 
    {
        return command.GetString();
    }

    // Obtenemos el comando desde el cliente
    return engine->GetClientConVarValue( entindex(), pCommand );
}

//================================================================================
// Ejecuta un comando desde el Cliente
//================================================================================
void CCoopPlayer::ExecuteCommand( const char *pCommand )
{
    engine->ClientCommand( edict(), pCommand );
}

//================================================================================
// Expulsa al jugador del servidor
//================================================================================
void CCoopPlayer::Kick()
{
    // Le quitamos todas las cosas
    RemoveAllItems( true );

    // Lo matamos para eliminar linternas y otras cosas
    if ( IsAlive() )
        CommitSuicide();

    engine->ServerCommand( UTIL_VarArgs("kickid %d\n", engine->GetPlayerUserId(edict())) );
}

//================================================================================
// Ejecuta una posesión demoniaca sobre este jugador
//================================================================================
bool CCoopPlayer::Possess( CCoopPlayer *pOther )
{
    if ( !pOther )
        return false;

    //
    edict_t *otherSoul    = pOther->edict();
    edict_t *mySoul       = edict();

    // Algo salio mal...
    if ( !otherSoul || !mySoul )
        return false;

    // Backup
    //edict_t oldPlayerEdict    = *playerSoul;
    //edict_t oldBotEdict        = *botSoul;

    // Cambio!!
    edict_t ootherSoul = *otherSoul;
    *otherSoul    = *mySoul;
    *mySoul        = ootherSoul;

    //
    CCoopPlayer *pMyPlayer          = dynamic_cast<CCoopPlayer *>(CBaseEntity::Instance( otherSoul ));
    CCoopPlayer *pOtherPlayer       = dynamic_cast<CCoopPlayer *>(CBaseEntity::Instance( mySoul ));

    //
    pMyPlayer->NetworkProp()->SetEdict( otherSoul );
    pOtherPlayer->NetworkProp()->SetEdict( mySoul );

    return true;
}

//================================================================================
// Comienza el modo espectador
//================================================================================
void CCoopPlayer::Spectate( CCoopPlayer *pTarget )
{
    // Ya estamos como espectador
    if ( IsObserver() )
        return;

    RemoveAllItems( true );

    // Seguimos vivos, hay que suicidarnos
    if ( IsAlive() )
        CommitSuicide();

    if ( pTarget ) 
    {
        StartObserverMode( OBS_MODE_CHASE );
        SetObserverTarget( pTarget );
    }
    else 
    {
        StartObserverMode( OBS_MODE_ROAMING );
    }
}

//================================================================================
// Procesa un comando enviado directamente desde el cliente
//================================================================================
bool CCoopPlayer::ClientCommand( const CCommand &args )
{
    // Convertirse en espectador
    if ( FStrEq(args[0], "spectate") ) 
    {
        Spectate();
        return true;
    }

    // Hacer respawn
    if ( FStrEq(args[0], "respawn") ) 
    {
        if ( IsAlive() )
            CommitSuicide();

        Respawn();
        return true;
    }

    // BaseClass
    return BaseClass::ClientCommand( args );
}