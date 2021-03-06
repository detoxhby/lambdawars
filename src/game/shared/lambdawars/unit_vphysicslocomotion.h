//====== Copyright � Sandern Corporation, All rights reserved. ===========//
//
// Purpose: 
//
//=============================================================================//

#ifndef UNIT_VPHYSICSLOCOMOTION_H
#define UNIT_VPHYSICSLOCOMOTION_H

#ifdef _WIN32
#pragma once
#endif

#include "unit_locomotion.h"

//-----------------------------------------------------------------------------
// Unit Base Locomotion class. Deals with movement.
//-----------------------------------------------------------------------------
class UnitVPhysicsLocomotion : public UnitBaseLocomotion
{
public:
#ifdef ENABLE_PYTHON
	UnitVPhysicsLocomotion( boost::python::object outer );
#endif // ENABLE_PYTHON

	virtual void FinishMove( UnitBaseMoveCommand &mv );

	virtual void	FullWalkMove();
	virtual void	VPhysicsMove();
	virtual void	VPhysicsMoveStep();
};

#endif // UNIT_VPHYSICSLOCOMOTION_H