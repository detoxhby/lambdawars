// This file has been generated by Py++.

#include "cbase.h"
// This file has been generated by Py++.

#include "cbase.h"
#include "npcevent.h"
#include "srcpy_entities.h"
#include "bone_setup.h"
#include "basegrenade_shared.h"
#include "takedamageinfo.h"
#include "c_ai_basenpc.h"
#include "c_basetoggle.h"
#include "c_triggers.h"
#include "soundinfo.h"
#include "saverestore.h"
#include "vcollide_parse.h"
#include "iclientvehicle.h"
#include "steam/steamclientpublic.h"
#include "view_shared.h"
#include "c_playerresource.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "c_smoke_trail.h"
#include "beam_shared.h"
#include "c_hl2wars_player.h"
#include "unit_base_shared.h"
#include "wars_func_unit.h"
#include "hl2wars_player_shared.h"
#include "wars_mapboundary.h"
#include "srcpy_util.h"
#include "c_wars_weapon.h"
#include "srcpy_converters_ents.h"
#include "srcpy.h"
#include "tier0/memdbgon.h"
#include "IClientEntity_pypp.hpp"

namespace bp = boost::python;

struct IClientEntity_wrapper : IClientEntity, bp::wrapper< IClientEntity > {

    IClientEntity_wrapper()
    : IClientEntity()
      , bp::wrapper< IClientEntity >(){
        // null constructor
        
    }

    virtual ::CBaseHandle const & GetRefEHandle(  ) const {
        throw std::logic_error("warning W1049: This method could not be overriden in Python - method returns reference to local variable!");
    }

    virtual void SetRefEHandle( ::CBaseHandle const & handle ){
        bp::override func_SetRefEHandle = this->get_override( "SetRefEHandle" );
        try {
            func_SetRefEHandle( boost::ref(handle) );
        } catch(bp::error_already_set &) {
            throw boost::python::error_already_set();
        }
    }

};

void register_IClientEntity_class(){

    bp::class_< IClientEntity_wrapper, bp::bases< IClientUnknown >, boost::noncopyable >( "IClientEntity", bp::no_init )    
        .def( 
            "GetRefEHandle"
            , bp::pure_virtual( (::CBaseHandle const & ( ::IHandleEntity::* )(  ) const)(&::IHandleEntity::GetRefEHandle) )
            , bp::return_value_policy< bp::copy_const_reference >() )    
        .def( 
            "SetRefEHandle"
            , bp::pure_virtual( (void ( ::IHandleEntity::* )( ::CBaseHandle const & ) )(&::IHandleEntity::SetRefEHandle) )
            , ( bp::arg("handle") ) );

}

