//==== Woots 2016. http://creativecommons.org/licenses/by/2.5/mx/ ===========//

#include "cbase.h"
#include "playeranimstate_proxy.h"
#include "playeranimsystem.h"

#ifdef CLIENT_DLL
    #include "c_coop_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//================================================================================
// Información y Red
//================================================================================

#ifndef CLIENT_DLL
static CPlayerAnimationSystemProxy g_PlayerAnimStateProxy( "PlayerAnimationSystemProxy" );
#endif

#ifndef CLIENT_DLL
IMPLEMENT_SERVERCLASS_ST( CPlayerAnimationSystemProxy, DT_PlayerAnimationSystemProxy )
#else
#undef CPlayerAnimationSystemProxy
IMPLEMENT_CLIENTCLASS_EVENT_DT( C_PlayerAnimationSystemProxy, DT_PlayerAnimationSystemProxy, CPlayerAnimationSystemProxy )
#endif
#ifndef CLIENT_DLL
    SendPropEHandle( SENDINFO( m_nPlayer ) ),
    SendPropInt( SENDINFO( m_iEvent ) ),
    SendPropInt( SENDINFO( m_nData ), 32 ),
#else
    RecvPropEHandle( RECVINFO( m_nPlayer ) ),
    RecvPropInt( RECVINFO( m_iEvent ) ),
    RecvPropInt( RECVINFO( m_nData ) )
#endif
END_NETWORK_TABLE()

#ifndef CLIENT_DLL

//================================================================================
// Envia un evento de animación atravez del proxy hacia el Cliente
//================================================================================
void SendPlayerAnimation( CBasePlayer *pPlayer, PlayerAnimEvent_t pEvent, int nData )
{
    // Se lo enviamos solo a los Jugadores que se encuentren visibles
    // en la vista del Jugador a animar
    CPVSFilter filter( pPlayer->EyePosition() );
    DevMsg(2, "[Server] SendPlayerAnimation %i, %i \n", (int)pEvent, nData);

    // Establecemos la información y enviamos
    g_PlayerAnimStateProxy.Init( pPlayer, pEvent, nData );
    g_PlayerAnimStateProxy.Create( filter );
}

//================================================================================
//================================================================================
void CPlayerAnimationSystemProxy::Init( CBasePlayer *pPlayer, PlayerAnimEvent_t pEvent, int nData )
{
    m_nPlayer   = pPlayer;
    m_iEvent    = pEvent;
    m_nData     = nData;
}

#else

#define CPlayerAnimationSystemProxy C_PlayerAnimationSystemProxy

CPlayerAnimationSystemProxy::CPlayerAnimationSystemProxy()
{
    m_nPlayer   = NULL;
    m_iEvent    = 0;
    m_nData     = 0;
}

//================================================================================
//================================================================================
void CPlayerAnimationSystemProxy::PostDataUpdate( DataUpdateType_t nData )
{
    int event = (int)m_iEvent.Get();

    // FIXME: Ahora mismo hay un BUG que agrega 192 al valor originalmente enviado
    //if ( event > PLAYERANIMEVENT_COUNT )
        //event = event - 224;

    C_CoopPlayer *pPlayer = dynamic_cast<C_CoopPlayer *>( m_nPlayer.Get() );
    DevMsg(2, "[Client] PostDataUpdate %i, %i \n", event, m_nData);

    if ( pPlayer && !pPlayer->IsDormant() ) {
        pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)event, m_nData );
    }
}

#endif