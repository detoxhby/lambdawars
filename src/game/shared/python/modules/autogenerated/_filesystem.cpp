// This file has been generated by Py++.

#include "cbase.h"



#include "filesystem.h"

#include "srcpy_filesystem.h"

#include "srcpy.h"

#include "tier0/memdbgon.h"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(_filesystem){
    bp::docstring_options doc_options( true, true, false );

    { //::PyFS_FileExists
    
        typedef bool ( *FileExists_function_type )( char const *,char const * );
        
        bp::def( 
            "FileExists"
            , FileExists_function_type( &::PyFS_FileExists )
            , ( bp::arg("pFileName"), bp::arg("pPathID")=bp::object() ) );
    
    }

    { //::PyFS_FullPathToRelativePath
    
        typedef ::boost::python::api::object ( *FullPathToRelativePath_function_type )( char const *,char const * );
        
        bp::def( 
            "FullPathToRelativePath"
            , FullPathToRelativePath_function_type( &::PyFS_FullPathToRelativePath )
            , ( bp::arg("path"), bp::arg("pathid")=bp::object() ) );
    
    }

    { //::PyFS_IsDirectory
    
        typedef bool ( *IsDirectory_function_type )( char const *,char const * );
        
        bp::def( 
            "IsDirectory"
            , IsDirectory_function_type( &::PyFS_IsDirectory )
            , ( bp::arg("pFileName"), bp::arg("pathID")=bp::object() ) );
    
    }

    { //::PyFS_ListDir
    
        typedef ::boost::python::list ( *ListDir_function_type )( char const *,char const *,char const * );
        
        bp::def( 
            "ListDir"
            , ListDir_function_type( &::PyFS_ListDir )
            , ( bp::arg("path"), bp::arg("pathid")=bp::object(), bp::arg("wildcard")="*" ) );
    
    }

    { //::PyFS_ReadFile
    
        typedef ::boost::python::api::object ( *ReadFile_function_type )( char const *,char const *,bool,int,int );
        
        bp::def( 
            "ReadFile"
            , ReadFile_function_type( &::PyFS_ReadFile )
            , ( bp::arg("filepath"), bp::arg("pathid"), bp::arg("optimalalloc")=(bool)(false), bp::arg("maxtyes")=(int)(0), bp::arg("startingbyte")=(int)(0) ) );
    
    }

    { //::PyFS_RelativePathToFullPath
    
        typedef ::boost::python::api::object ( *RelativePathToFullPath_function_type )( char const *,char const * );
        
        bp::def( 
            "RelativePathToFullPath"
            , RelativePathToFullPath_function_type( &::PyFS_RelativePathToFullPath )
            , ( bp::arg("path"), bp::arg("pathid")=bp::object() ) );
    
    }
}
