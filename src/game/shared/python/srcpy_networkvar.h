//====== Copyright � Sandern Corporation, All rights reserved. ===========//
//
// Purpose: 
//
//=============================================================================//

#ifndef SRCPY_NETWORKVAR_H
#define SRCPY_NETWORKVAR_H

#ifdef _WIN32
#pragma once
#endif

#include "srcpy_boostpython.h"

#if 0
#include <set>
#endif // 0

namespace bp = boost::python;

extern ConVar g_debug_pynetworkvar;

#ifndef CLIENT_DLL

// Python Send proxies
class CPythonSendProxyBase
{
public:
	virtual bool ShouldSend( CBaseEntity *pEnt, int iClient ) { return true; }
};

class CPythonSendProxyOwnerOnly : public CPythonSendProxyBase
{
public:
	virtual bool ShouldSend( CBaseEntity *pEnt, int iClient );
};

class CPythonSendProxyAlliesOnly : public CPythonSendProxyBase
{
public:
	virtual bool ShouldSend( CBaseEntity *pEnt, int iClient );
};

// Python network classes
#define PYNETVAR_MAX_NAME 260
class CPythonNetworkVarBase
{
public:
	CPythonNetworkVarBase( bp::object ent, const char *name, bool changedcallback=false, bp::object sendproxy=bp::object() );
	~CPythonNetworkVarBase();

	void Remove( CBaseEntity *pEnt );

	void NetworkStateChanged( void );
	virtual void NetworkVarsUpdateClient( CBaseEntity *pEnt, int iClient ) {}

	// For optimization purposes. Returns if the data is in the unmodified state at construction.
	bool IsInInitialState() { return m_bInitialState; }

public:
	// This bit vector contains the players who don't have the most up to date data
	CBitVec<ABSOLUTE_PLAYER_LIMIT> m_PlayerUpdateBits;

protected:
	char m_Name[PYNETVAR_MAX_NAME];
	bool m_bChangedCallback;
	bool m_bInitialState;

	bp::object m_wrefEnt;

	CPythonSendProxyBase *m_pPySendProxy;
	bp::object m_pySendProxyRef;
};

class CPythonNetworkVar : public CPythonNetworkVarBase
{
public:
	CPythonNetworkVar( bp::object self, const char *name, bp::object data = bp::object(), 
		bool initstatechanged=false, bool changedcallback=false, bp::object sendproxy=bp::object() );

	void NetworkVarsUpdateClient( CBaseEntity *pEnt, int iClient );

	void Set( bp::object data );
	bp::object Get( void );

private:
	bp::object m_dataInternal;
};

class CPythonNetworkArray : public CPythonNetworkVarBase
{
public:
	CPythonNetworkArray( bp::object self, const char *name, bp::list data = bp::list(), 
		bool initstatechanged=false, bool changedcallback=false, bp::object sendproxy=bp::object() );

	void NetworkVarsUpdateClient( CBaseEntity *pEnt, int iClient );

	void SetItem( bp::object key, bp::object data );
	bp::object GetItem( bp::object key );
	void DelItem( bp::object key );

	void Set( bp::list data );

private:
	bp::list m_dataInternal;
};

class CPythonNetworkDict : public CPythonNetworkVarBase
{
public:
	CPythonNetworkDict( bp::object self, const char *name, bp::dict data = bp::dict(), 
		bool initstatechanged=false, bool changedcallback=false, bp::object sendproxy=bp::object() );

	void NetworkVarsUpdateClient( CBaseEntity *pEnt, int iClient );

	void SetItem( bp::object key, bp::object data );
	bp::object GetItem(  bp::object key );

	void Set( bp::dict data );

private:
	bp::dict m_dataInternal;

#if 0
	std::set<bp::object> m_changedKeys;
#endif // 0
};

// Reset all network variables transmit bits for the specified client (0 based)
void PyNetworkVarsResetClientTransmitBits( int iClient );
// Updates network variables for specified client index (0 based)
void PyNetworkVarsUpdateClient( CBaseEntity *pEnt, int iClient );

#else
	void HookPyNetworkVar();
#endif // CLIENT_DLL

#endif // SRCPY_NETWORKVAR_H