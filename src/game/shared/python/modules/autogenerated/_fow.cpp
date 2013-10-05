// This file has been generated by Py++.

#include "cbase.h"
#ifdef CLIENT_DLL
#include "cbase.h"

#include "hl2wars/fowmgr.h"

#include "srcpy.h"

#include "tier0/memdbgon.h"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(_fow){
    bp::docstring_options doc_options( true, true, false );

    bp::class_< CFogOfWarMgr, boost::noncopyable >( "CFogOfWarMgr", bp::no_init )    
        .def( bp::init< >() )    
        .def( 
            "ComputeFOWPosition"
            , (void ( ::CFogOfWarMgr::* )( ::Vector const &,int &,int & ) )( &::CFogOfWarMgr::ComputeFOWPosition )
            , ( bp::arg("vPos"), bp::arg("x"), bp::arg("y") ) )    
        .def( 
            "ComputeWorldPosition"
            , (::Vector ( ::CFogOfWarMgr::* )( int,int ) )( &::CFogOfWarMgr::ComputeWorldPosition )
            , ( bp::arg("x"), bp::arg("y") ) )    
        .def( 
            "DebugPrintEntities"
            , (void ( ::CFogOfWarMgr::* )(  ) )( &::CFogOfWarMgr::DebugPrintEntities ) )    
        .def( 
            "GetGridSize"
            , (int ( ::CFogOfWarMgr::* )(  ) )( &::CFogOfWarMgr::GetGridSize ) )    
        .def( 
            "GetHeightAtPoint"
            , (float ( ::CFogOfWarMgr::* )( ::Vector const & ) )( &::CFogOfWarMgr::GetHeightAtPoint )
            , ( bp::arg("vPoint") ) )    
        .def( 
            "GetHeightAtTile"
            , (float ( ::CFogOfWarMgr::* )( int,int ) )( &::CFogOfWarMgr::GetHeightAtTile )
            , ( bp::arg("x"), bp::arg("y") ) )    
        .def( 
            "GetTileSize"
            , (int ( ::CFogOfWarMgr::* )(  ) )( &::CFogOfWarMgr::GetTileSize ) )    
        .def( 
            "ModifyHeightAtExtent"
            , (void ( ::CFogOfWarMgr::* )( ::Vector const &,::Vector const &,float ) )( &::CFogOfWarMgr::ModifyHeightAtExtent )
            , ( bp::arg("vMins"), bp::arg("vMaxs"), bp::arg("fHeight") ) )    
        .def( 
            "ModifyHeightAtPoint"
            , (void ( ::CFogOfWarMgr::* )( ::Vector const &,float ) )( &::CFogOfWarMgr::ModifyHeightAtPoint )
            , ( bp::arg("vPoint"), bp::arg("fHeight") ) )    
        .def( 
            "ModifyHeightAtTile"
            , (void ( ::CFogOfWarMgr::* )( int,int,float ) )( &::CFogOfWarMgr::ModifyHeightAtTile )
            , ( bp::arg("x"), bp::arg("y"), bp::arg("fHeight") ) )    
        .def( 
            "ResetExplored"
            , (void ( ::CFogOfWarMgr::* )(  ) )( &::CFogOfWarMgr::ResetExplored ) );

    { //::FogOfWarMgr
    
        typedef ::CFogOfWarMgr * ( *FogOfWarMgr_function_type )(  );
        
        bp::def( 
            "FogOfWarMgr"
            , FogOfWarMgr_function_type( &::FogOfWarMgr )
            , bp::return_value_policy< bp::reference_existing_object >() );
    
    }
}
#else
#include "cbase.h"

#include "hl2wars/fowmgr.h"

#include "srcpy.h"

#include "tier0/memdbgon.h"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(_fow){
    bp::docstring_options doc_options( true, true, false );

    bp::class_< CFogOfWarMgr, boost::noncopyable >( "CFogOfWarMgr", bp::no_init )    
        .def( bp::init< >() )    
        .def( 
            "ComputeFOWPosition"
            , (void ( ::CFogOfWarMgr::* )( ::Vector const &,int &,int & ) )( &::CFogOfWarMgr::ComputeFOWPosition )
            , ( bp::arg("vPos"), bp::arg("x"), bp::arg("y") ) )    
        .def( 
            "ComputeWorldPosition"
            , (::Vector ( ::CFogOfWarMgr::* )( int,int ) )( &::CFogOfWarMgr::ComputeWorldPosition )
            , ( bp::arg("x"), bp::arg("y") ) )    
        .def( 
            "DebugPrintEntities"
            , (void ( ::CFogOfWarMgr::* )(  ) )( &::CFogOfWarMgr::DebugPrintEntities ) )    
        .def( 
            "GetGridSize"
            , (int ( ::CFogOfWarMgr::* )(  ) )( &::CFogOfWarMgr::GetGridSize ) )    
        .def( 
            "GetHeightAtPoint"
            , (float ( ::CFogOfWarMgr::* )( ::Vector const & ) )( &::CFogOfWarMgr::GetHeightAtPoint )
            , ( bp::arg("vPoint") ) )    
        .def( 
            "GetHeightAtTile"
            , (float ( ::CFogOfWarMgr::* )( int,int ) )( &::CFogOfWarMgr::GetHeightAtTile )
            , ( bp::arg("x"), bp::arg("y") ) )    
        .def( 
            "GetTileSize"
            , (int ( ::CFogOfWarMgr::* )(  ) )( &::CFogOfWarMgr::GetTileSize ) )    
        .def( 
            "ModifyHeightAtExtent"
            , (void ( ::CFogOfWarMgr::* )( ::Vector const &,::Vector const &,float ) )( &::CFogOfWarMgr::ModifyHeightAtExtent )
            , ( bp::arg("vMins"), bp::arg("vMaxs"), bp::arg("fHeight") ) )    
        .def( 
            "ModifyHeightAtPoint"
            , (void ( ::CFogOfWarMgr::* )( ::Vector const &,float ) )( &::CFogOfWarMgr::ModifyHeightAtPoint )
            , ( bp::arg("vPoint"), bp::arg("fHeight") ) )    
        .def( 
            "ModifyHeightAtTile"
            , (void ( ::CFogOfWarMgr::* )( int,int,float ) )( &::CFogOfWarMgr::ModifyHeightAtTile )
            , ( bp::arg("x"), bp::arg("y"), bp::arg("fHeight") ) )    
        .def( 
            "PointInFOW"
            , (bool ( ::CFogOfWarMgr::* )( ::Vector const &,int ) )( &::CFogOfWarMgr::PointInFOW )
            , ( bp::arg("vPoint"), bp::arg("iOwner") ) );

    { //::FogOfWarMgr
    
        typedef ::CFogOfWarMgr * ( *FogOfWarMgr_function_type )(  );
        
        bp::def( 
            "FogOfWarMgr"
            , FogOfWarMgr_function_type( &::FogOfWarMgr )
            , bp::return_value_policy< bp::reference_existing_object >() );
    
    }
}
#endif

