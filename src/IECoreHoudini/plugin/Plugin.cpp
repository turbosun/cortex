//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include <boost/python.hpp>

#include <UT/UT_DSOVersion.h>
#include <UT/UT_IOTable.h>
#include <UT/UT_Version.h>
#include <OP/OP_OperatorTable.h>
#include <GR/GR_RenderTable.h>

/// Used to our new Render Hook for Houdini 12.5 and later
#if UT_MAJOR_VERSION_INT > 12 || UT_MINOR_VERSION_INT >= 5

#include "DM/DM_RenderTable.h"

#endif

//#include "IECoreHoudini/OBJ_SceneCacheGeometry.h"
//#include "IECoreHoudini/OBJ_SceneCacheTransform.h"
#include "IECoreHoudini/SOP_OpHolder.h"
#include "IECoreHoudini/SOP_ParameterisedHolder.h"
#include "IECoreHoudini/SOP_ProceduralHolder.h"
#include "IECoreHoudini/SOP_ToHoudiniConverter.h"
#include "IECoreHoudini/SOP_InterpolatedCacheReader.h"
//#include "IECoreHoudini/SOP_SceneCacheSource.h"
//#include "IECoreHoudini/ROP_SceneCacheWriter.h"
#include "IECoreHoudini/GEO_CobIOTranslator.h"
#include "IECoreHoudini/GR_Cortex.h"
#include "IECoreHoudini/GU_CortexPrimitive.h"
#include "IECoreHoudini/GUI_CortexPrimitiveHook.h"
#include "IECoreHoudini/UT_ObjectPoolCache.h"

using namespace IECoreHoudini;

/// Tell Houdini that this plugin should be loaded with RTLD_GLOBAL
extern "C"
{
	DLLEXPORT void HoudiniDSOInit( UT_DSOInfo &dsoinfo )
	{
		dsoinfo.loadGlobal = true;
		
		/// Create the default ObjectPool cache
		UT_ObjectPoolCache::defaultObjectPoolCache();
	}
}

/// Declare our new SOPs
void newSopOperator(OP_OperatorTable *table)
{
	OP_Operator *opHolder = new OP_Operator(
		"ieOpHolder", "Cortex Op",
		SOP_OpHolder::create, SOP_ParameterisedHolder::parameters, 0, 4,
		SOP_ParameterisedHolder::variables, OP_FLAG_GENERATOR
	);
	opHolder->setIconName( "SOP_ieOpHolder" );
	
	OP_Operator *proceduralHolder = new OP_Operator(
		"ieProceduralHolder", "Cortex Procedural",
		SOP_ProceduralHolder::create, SOP_ParameterisedHolder::parameters, 0, 4,
    		SOP_ParameterisedHolder::variables, OP_FLAG_GENERATOR
	);
	proceduralHolder->setIconName( "SOP_ieProceduralHolder" );
	
	OP_Operator *converter = new OP_Operator(
		/*SOP_ToHoudiniConverter::typeName*/"ieCortexConverter", "Cortex Convert",
		SOP_ToHoudiniConverter::create, SOP_ToHoudiniConverter::parameters, 1,	1,
		SOP_ToHoudiniConverter::variables, OP_FLAG_GENERATOR
	);
	converter->setIconName( "SOP_ieToHoudiniConverter" );
	
	////OP_Operator *cacheReader = new OP_Operator(
	////	"ieInterpolatedCacheReader", "Interpolated Cache Reader",
	////	SOP_InterpolatedCacheReader::create, SOP_InterpolatedCacheReader::parameters, 1, 1, 0
	////);
	////cacheReader->setIconName( "SOP_ieInterpolatedCacheReader" );
	
#if defined(USESCENECACHE)
	OP_Operator *sceneCacheSource = new OP_Operator(
		SOP_SceneCacheSource::typeName, "SceneCache Source",
		SOP_SceneCacheSource::create, SOP_SceneCacheSource::buildParameters(), 0, 0,
		NULL, OP_FLAG_GENERATOR
	);

	/// \todo: get a new icon
	sceneCacheSource->setIconName( "SOP_ieToHoudiniConverter" );
#endif
	
	table->addOperator( proceduralHolder );
	table->addOperator( opHolder );
	table->addOperator( converter );
	////table->addOperator( cacheReader );
#if defined(USESCENECACHE)
	table->addOperator( sceneCacheSource );
#endif
	
	table->addOpHidden( opHolder->getName() );
	table->addOpHidden( proceduralHolder->getName() );
	table->addOpHidden( converter->getName() );
	////table->addOpHidden( cacheReader->getName() );
#if defined(USESCENECACHE)
	table->addOpHidden( sceneCacheSource->getName() );
#endif
}

void newObjectOperator( OP_OperatorTable *table )
{
#if defined(USESCENECACHE)
	OP_Operator *sceneCacheTransform = new OP_Operator(
		OBJ_SceneCacheTransform::typeName, "SceneCache Xform",
		OBJ_SceneCacheTransform::create, OBJ_SceneCacheTransform::buildParameters(), 0, 1
	);
	/// \todo: get a new icon
	sceneCacheTransform->setIconName( "SOP_ieToHoudiniConverter" );
	
	OP_Operator *sceneCacheGeometry = new OP_Operator(
		OBJ_SceneCacheGeometry::typeName, "SceneCache GEO",
		OBJ_SceneCacheGeometry::create, OBJ_SceneCacheGeometry::buildParameters(), 0, 1
	);
	/// \todo: get a new icon
	sceneCacheGeometry->setIconName( "SOP_ieProceduralHolder" );

	table->addOperator( sceneCacheTransform );
	table->addOperator( sceneCacheGeometry );
	
	table->addOpHidden( sceneCacheTransform->getName() );
	table->addOpHidden( sceneCacheGeometry->getName() );
#endif
}
#if defined(USESCENECACHE)
void newDriverOperator( OP_OperatorTable *table )
{
	OP_Operator *sceneCacheWriter = new OP_Operator(
		ROP_SceneCacheWriter::typeName, "SceneCache Writer",
		ROP_SceneCacheWriter::create, ROP_SceneCacheWriter::buildParameters(), 0, 999, 0,
		OP_FLAG_GENERATOR
	);
	/// \todo: get a new icon
	sceneCacheWriter->setIconName( "SOP_ieToHoudiniConverter" );
	
	table->addOperator( sceneCacheWriter );
	
	table->addOpHidden( sceneCacheWriter->getName() );
}
#endif

/// Declare our new Render Hooks for Houdini 12.0 and 12.1 only
#if UT_MAJOR_VERSION_INT >= 12 && UT_MINOR_VERSION_INT <= 1
void newRenderHook( GR_RenderTable *table )
{
	GR_Cortex *hook = new GR_Cortex;
	table->addHook( hook, GR_RENDER_HOOK_VERSION );
}
#endif

void newGeometryPrim( GA_PrimitiveFactory *factory )
{
	//GA_PrimitiveDefinition *primDef = factory->registerDefinition(
	//	GU_CortexPrimitive::typeName, GU_CortexPrimitive::create,
	//	GA_FAMILY_NONE, ( std::string( GU_CortexPrimitive::typeName ) + "s" ).c_str()
	//);

	GA_PrimitiveDefinition *primDef = factory->registerDefinition(
		"CortexObject", GU_CortexPrimitive::create,
		GA_FAMILY_NONE, ( std::string( "CortexObject" ) + "s" ).c_str()
	);
	
	if ( !primDef )
	{
		std::cerr << "Warning: Duplicate definition for GU_CortexPrimitive. Make sure only 1 version of the ieCoreHoudini plugin is on your path." << std::endl;
		return;
	}
	
	/// \todo: This method is silly. Should we just give up and do the whole registration in GU_CortexPrimitive?
	GU_CortexPrimitive::setTypeDef( primDef );
	
/// Declare our new Render Hook for Houdini 12.5 and later
#if UT_MAJOR_VERSION_INT > 12 || UT_MINOR_VERSION_INT >= 5

	DM_RenderTable::getTable()->registerGEOHook( new GUI_CortexPrimitiveHook, primDef->getId(), 0 );

#endif

}

/// Declare our new IO Translators
void newGeometryIO( void * )
{
	GU_Detail::registerIOTranslator( new GEO_CobIOTranslator() );
	
	UT_ExtensionList *geoextension = UTgetGeoExtensions();
	if ( !geoextension->findExtension( "cob" ) )
	{
		geoextension->addExtension( "cob" );
	}
	if ( !geoextension->findExtension( "pdc" ) )
	{
		geoextension->addExtension( "pdc" );
	}
}
