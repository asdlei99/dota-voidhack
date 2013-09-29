
#include "vh.h"

#include "zeusmanager.h"
#include "entityhelper.h"

#include "tier1/interface.h"

#include "cbase.h"

#include "vstdlib/cvar.h"
#include "toolframework/ienginetool.h"
#include "cdll_int.h"

#include "dotaplayer.h"
#include "dotahero.h"
#include "dotaresource.h"

#include <sourcehook/sourcehook.h>


CVH g_VH;

CVH &VH()
{
	return g_VH;
}


SourceHook::Impl::CSourceHookImpl g_SourceHook;

SourceHook::ISourceHook *g_SHPtr = &g_SourceHook;
int g_PLID = 0;

SH_DECL_HOOK1_void( IBaseClientDLL, FrameStageNotify, SH_NOATTRIB, 0, ClientFrameStage_t );


CVH::CVH() :
	m_pEngineClient( NULL ), m_pClientDLL( NULL ),
	m_pEngineTool( NULL ), m_pClientTools( NULL ),

	m_fnClientFactory( NULL ),
	m_fnEngineFactory( NULL ),
	m_fnCvarFactory( NULL )
{
}


CON_COMMAND( vh_test, "Test convar" )
{
	Msg( "In game: %d\n", VH().EngineClient()->IsInGame() );
	Msg( "Ent index: %d\n", VH().ClientTools()->GetEntIndex( VH().ClientTools()->GetLocalPlayer() ) );

	for ( int x = 1 ; x <= MAX_PLAYERS ; ++x )
	{
		C_DOTAPlayer player = C_DOTAPlayer::GetPlayerByIndex( x );

		if ( !player.IsValid() )
			continue;

		C_DOTAHero hero = player.m_hAssignedHero;

		if ( !hero.IsValid() )
			continue;

		Msg(
			"Player %d (%d) is playing as %d at level %d, with %d/%d/%d kda\n",
			x, ( int )hero.m_iPlayerID, hero.GetHeroID(), hero.GetLevel(), hero.GetNumKills(), hero.GetNumDeaths(), hero.GetNumAssists()
		);
	}

}

void CVH::Init()
{
	m_fnEngineFactory = Sys_GetFactory( "engine" );
	m_fnClientFactory = Sys_GetFactory( "client" );
	m_fnCvarFactory = VStdLib_GetICVarFactory();

	CreateInterfaceFn factories[] =
	{
		m_fnEngineFactory,
		m_fnClientFactory,
		m_fnCvarFactory,
	};

	// connect tier1
	// passing the vstdlib factory sets up icvar for us
	ConnectTier1Libraries( factories, 3 );

	m_pEngineClient = reinterpret_cast<IVEngineClient *>( m_fnEngineFactory( VENGINE_CLIENT_INTERFACE_VERSION, NULL ) );
	m_pClientDLL = reinterpret_cast<IBaseClientDLL *>( m_fnClientFactory( CLIENT_DLL_INTERFACE_VERSION, NULL ) );
	m_pEngineTool = reinterpret_cast<IEngineTool *>( m_fnEngineFactory( VENGINETOOL_INTERFACE_VERSION, NULL ) );
	m_pClientTools = reinterpret_cast<IClientTools *>( m_fnClientFactory( VCLIENTTOOLS_INTERFACE_VERSION, NULL ) );

	SH_ADD_HOOK( IBaseClientDLL, FrameStageNotify, m_pClientDLL, SH_MEMBER( this, &CVH::FrameStageNotify ), false );

	ConVar_Register();

	// init our things
	EntityHelper().Init();
	ZeusManager().Init();
}

void CVH::Shutdown()
{
	// un-init our things
	ZeusManager().Shutdown();
	EntityHelper().Shutdown();

	ConVar_Unregister();

	SH_REMOVE_HOOK( IBaseClientDLL, FrameStageNotify, m_pClientDLL, SH_MEMBER( this, &CVH::FrameStageNotify ), false );

	m_pClientTools = NULL;
	m_pEngineTool = NULL;
	m_pClientDLL = NULL;
	m_pEngineClient = NULL;

	DisconnectTier1Libraries();

	m_fnCvarFactory = NULL;
	m_fnClientFactory = NULL;
	m_fnEngineFactory = NULL;
}


void CVH::AddFrameHook( FrameFunction func )
{
	m_FrameHooks.AddToTail( func );
}

void CVH::RemoveFrameHook( FrameFunction func )
{
	FOR_EACH_VEC( m_FrameHooks, i )
	{
		FrameFunction &frameFunc = m_FrameHooks[ i ];

		if ( frameFunc == func )
		{
			m_FrameHooks.Remove( i );
			break;
		}
	}
}

void CVH::FrameStageNotify( ClientFrameStage_t curStage )
{
	if ( curStage == FRAME_RENDER_START )
	{
		// entity simulation happens here, so we'll think alongside
		Think();
	}
}

void CVH::Think()
{
	FOR_EACH_VEC( m_FrameHooks, i )
	{
		// think for every installed frame hook
		m_FrameHooks[ i ]();
	}
}