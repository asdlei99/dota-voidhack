
#pragma once


#include "cbase.h"

#include "entityhelper.h"


#ifdef GetProp
#undef GetProp
#endif


#define CEntProp( propType, propName ) \
	class CEntProp_##propName  \
	{ \
	public: \
		propType Get() \
		{ \
			RecvPropInfo_t propInfo; \
			C_BaseEntity *pEnt = GET_OUTER( ThisClass, propName )->m_pEntity; \
			if ( EntityHelper().GetRecvPropInfo( pEnt, #propName, &propInfo ) ) \
			{ \
				return *(propType *)( (uint8 *)pEnt + propInfo.actualOffset ); \
			} \
			return (propType)0; \
		} \
	} propName;

#define CEntPropEnt( propName ) \
	class CEntPropEnt_##propName \
	{ \
	public: \
		C_BaseEntity *Get() \
		{ \
			C_BaseEntity *pOuterEnt = GET_OUTER( ThisClass, propName )->m_pEntity; \
			C_BaseEntity *pEnt = NULL; \
			if ( EntityHelper().GetEntPropEnt( pOuterEnt, EntProp_RecvProp, #propName, &pEnt ) ) \
			{ \
				return pEnt; \
			} \
			return NULL; \
		} \
	} propName;

#define CEntPropArray( propType, propName ) \
	class CEntPropArray_##propName \
	{ \
	public: \
		propType Get( int element ) \
		{ \
			RecvPropInfo_t propInfo; \
			C_BaseEntity *pEnt = GET_OUTER( ThisClass, propName )->m_pEntity; \
			if ( EntityHelper().GetRecvPropInfo( pEnt, #propName, &propInfo ) ) \
			{ \
				RecvTable *pTable = propInfo.prop->GetDataTable(); \
				if ( element < 0 || element >= pTable->GetNumProps() ) \
					return (propType)0; \
				\
				RecvProp *pProp = pTable->GetProp( element ); \
				propInfo.actualOffset += pProp->GetOffset(); \
				return *(propType *)( (uint8 *)pEnt + propInfo.actualOffset ); \
			} \
			return (propType)0; \
		} \
	} propName;
