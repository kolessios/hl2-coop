//==== Woots 2016. http://creativecommons.org/licenses/by/2.5/mx/ ===========//

#include "cbase.h"

#ifdef CLIENT_DLL
    #include "c_coop_player.h"
    #include "prediction.h"

    #define CCoopPlayer C_CoopPlayer
#else
    #include "coop_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//================================================================================
// Crea el sistema de animación
//================================================================================
void CCoopPlayer::CreateAnimationSystem()
{
    // Información predeterminada
    MultiPlayerMovementData_t data;
    data.Init();

    data.m_flBodyYawRate  = 120.0f;
    data.m_flRunSpeed     = 210.0f;
    data.m_flWalkSpeed    = 1.0f;
    data.m_iLegsAnimType  = LEGANIM_8WAY;

    data.m_nAimPitchPoseName    = "aim_pitch";
    data.m_nAimYawPoseName      = "aim_yaw";

    m_pAnimState = CreatePlayerAnimationSystem( this, data );
}

//====================================================================
// Traduce una actividad a otra
//====================================================================
Activity CCoopPlayer::TranslateActivity( Activity actBase )
{
	// Tenemos un arma
	if ( GetActiveWeapon() ) 
	{
		Activity weapActivity = GetActiveWeapon()->ActivityOverride( actBase, false );

		if ( weapActivity != actBase )
			return weapActivity;
	}

    //ACT_SYNERGY_GESTURE_GIVE_ITEM

    // Animaciones sin arma
	switch( actBase )
	{
		// Normal
		case ACT_HL2MP_IDLE:
			return ACT_SYNERGY_IDLE;	
		break;

		// Corriendo
		case ACT_HL2MP_RUN:
			return ACT_SYNERGY_RUN;
		break;

		// Agachado normal
		case ACT_HL2MP_IDLE_CROUCH:
			return ACT_SYNERGY_IDLE_CROUCH;
		break;

		// Agachado saltando
		case ACT_HL2MP_WALK_CROUCH:
			return ACT_SYNERGY_RUN_CROUCH;
		break;

		// Saltando
		case ACT_HL2MP_JUMP:
			return ACT_HL2MP_JUMP_SMG1;
		break;
	}

	// TODO
	return actBase;
}