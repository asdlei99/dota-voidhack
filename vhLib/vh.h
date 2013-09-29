
#pragma once


#include "interface.h"

#include "utlvector.h"

#include "cbase.h"
#include "toolframework/ienginetool.h"

#include <sourcehook/sourcehook_impl.h>


class IVEngineClient;
class IBaseClientDLL;
class IEngineTool;
class IClientTools;
enum ClientFrameStage_t;


typedef void (*FrameFunction)();



class CVH
{

public:
	CVH();


public:
	void Init();
	void Shutdown();

	void AddFrameHook( FrameFunction func );
	void RemoveFrameHook( FrameFunction func );

	IVEngineClient *EngineClient() { return m_pEngineClient; }
	IBaseClientDLL *ClientDLL() { return m_pClientDLL; }
	IEngineTool *EngineTool() { return m_pEngineTool; }
	IClientTools *ClientTools() { return m_pClientTools; }


private:
	void FrameStageNotify( ClientFrameStage_t curStage );
	void Think();


private:
	CreateInterfaceFn m_fnEngineFactory;
	CreateInterfaceFn m_fnClientFactory;
	CreateInterfaceFn m_fnCvarFactory;

	IVEngineClient *m_pEngineClient;
	IBaseClientDLL *m_pClientDLL;
	IEngineTool *m_pEngineTool;
	IClientTools *m_pClientTools;

	CUtlVector<FrameFunction> m_FrameHooks;
};

CVH &VH();

extern SourceHook::ISourceHook *g_SHPtr;
extern int g_PLID;