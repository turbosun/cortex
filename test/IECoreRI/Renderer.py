##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import unittest
import os
from IECore import *
import IECoreRI

class SimpleProcedural( Renderer.Procedural ) :

	def __init__( self, scale ) :
	
		Renderer.Procedural.__init__( self, "name", "description" )
		self.__scale = scale
		self.__t = StringData( "hello" )
		self.__c = CompoundData()
		self.__c["a"] = IntData( 4 )

	def doBound( self, args ) :
	
		return Box3f( V3f( -self.__scale ), V3f( self.__scale ) )
		
	def doRender( self, renderer, args ) :
	
		renderer.transformBegin()
		
		m = M44f()
		m.scale( V3f( self.__scale ) )
		renderer.concatTransform( m )
		
		renderer.transformEnd()
		

class RendererTest( unittest.TestCase ) :

	def loadShader( self, shader ) :
	
		return IECoreRI.SLOReader( os.path.join( os.environ["SHADER_PATH"], shader + ".sdl" ) ).read()

	def testTypeName( self ) :
	
		r = IECoreRI.Renderer()
		self.assertEqual( r.typeName(), "IECoreRI::Renderer" )

	def testNoContext( self ) :
	
		r = IECoreRI.Renderer()

	def test( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/test.rib" )
		
		r.setOption( "ri:searchpath:shader", StringData( os.environ["SHADER_PATH"] ) )
		r.setOption( "ri:render:bucketorder", StringData( "zigzag" ) )
		r.setOption( "user:magicNumber", IntData( 42 ) )
		
		r.worldBegin()
		
		r.transformBegin()
		r.attributeBegin()
		
		self.loadShader( "plastic" ).render( r )
		
		Reader.create( "test/IECoreRI/data/sphere.cob" ).read().render( r )
		
		r.attributeEnd()
		r.transformEnd()
		
		r.worldEnd()
	
	def testAttributes( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testAttributes.rib" )
		
		r.worldBegin()
		
		r.setAttribute( "ri:shadingRate", FloatData( 2 ) )
		r.setAttribute( "ri:matte", BoolData( 0 ) )
		r.setAttribute( "user:whatever", StringData( "whatever" ) )
		r.setAttribute( "ri:color", Color3fData( Color3f( 0, 1, 1 ) ) )
		r.setAttribute( "ri:opacity", Color3fData( Color3f( 0.5 ) ) )
		r.setAttribute( "ri:sides", IntData( 1 ) )
		r.setAttribute( "ri:geometricApproximation:motionFactor", FloatData( 1 ) )
		r.setAttribute( "ri:geometricApproximation:focusFactor", FloatData( 1 ) )
		r.setAttribute( "ri:cull:hidden", IntData( 0 ) )
		
		r.worldEnd()
		
	def testProcedural( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testProcedural.rib" )
		r.worldBegin()
		
		r.procedural( SimpleProcedural( 10.5 ) )
		
		r.worldEnd()
		
	def testGetOption( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testGetOption.rib" )
		
		r.worldBegin()
		
		s = r.getOption( "shutter" )
		self.assertEqual( s, V2fData( V2f( 0 ) ) )
		
		r.worldEnd()
		
	def testDisplay( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testDisplay.rib" )
		
		r.display( "test.tif", "tiff", "rgba", { "quantize" : FloatVectorData( [ 0, 1, 0, 1 ] ) } )
		
		r.worldBegin()
		r.worldEnd()
		
	def testCamera( self ) :
	
		s = M44f()
		s.scale( V3f( 10 ) )
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/testCamera.rib" )
		
		# we can't use concatTransform to position the camera until
		# we get support for RxTransformPoints working in rib generation
		# mode from 3delight - instead we're using the nasty transform
		# parameter in the list below.
		#r.concatTransform( s )
		r.camera( "main", {
			"resolution" : V2iData( V2i( 1024 ) ),
			"screenWindow" : Box2fData( Box2f( V2f( -1 ), V2f( 1 ) ) ),
			"cropWindow" : Box2fData( Box2f( V2f( 0.1, 0.1 ), V2f( 0.9, 0.9 ) ) ),
			"clippingPlanes" : V2fData( V2f( 1, 1000 ) ),
			"projection" : StringData( "perspective" ),
			"projection:fov" : FloatData( 45 ),
			"hider" : StringData( "hidden" ),
			"hider:jitter" : IntData( 1 ),
			"shutter" : V2fData( V2f( 0, 0.1 ) ),
			"transform" : M44fData( s )
		} )
		
		r.worldBegin()		
		r.worldEnd()
		
	def testSubDivs( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/subdiv.rib" )
		
		r.display( "test", "idisplay", "rgba", {} )
		
		r.worldBegin()
		
		t = M44f()
		t.translate( V3f( 0, 0, 10 ) )
		r.concatTransform( t )
		m = ObjectReader( "test/IECoreRI/data/openSubDivCube.cob" ).read()
		m.render( r )
		
		r.worldEnd()
	
	def testCommands( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/commands.rib" )
		
		r.worldBegin()
		
		r.command( "ri:readArchive", { "name" : StringData( "nameOfArchive" ) } )
		
		r.worldEnd()
		
	def testMotion( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/motion.rib" )
		
		r.worldBegin()
		
		m = MatrixMotionTransform()
		m[0] = M44f.createTranslated( V3f( 0, 1, 0 ) )
		m[1] = M44f.createTranslated( V3f( 0, 10, 0 ) )
		
		m.render( r )
		
		r.worldEnd()	
	
	def testStringPrimVars( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/stringPrimVars.rib" )	
		
		r.worldBegin()
		
		m = ObjectReader( "test/IECoreRI/data/stringPrimVars.cob" ).read()
		m.render( r )
		
		r.worldEnd()
		
	def testGetTransform( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/transform.rib" )
		
		r.worldBegin()
		
		self.assertEqual( r.getTransform(), M44f() )
		self.assertEqual( r.getTransform( "world" ), M44f() )
		self.assertEqual( r.getTransform( "object" ), M44f() )
		
		r.transformBegin()
		
		t = M44f.createTranslated( V3f( 1, 2, 3 ) ) * M44f.createScaled( V3f( 2, 1, 0 ) ) * M44f.createRotated( V3f( 20, 0, 90 ) )
		r.concatTransform( t )
		self.assert_( r.getTransform( "object" ).equalWithAbsError( t, 0.000001 ) )
		self.assert_( r.getTransform().equalWithAbsError( t, 0.000001 ) )
		
		r.coordinateSystem( "coordSys" )
		self.assert_( r.getTransform( "coordSys" ).equalWithAbsError( t, 0.000001 ) )
		
		r.transformEnd()
		
		self.assertEqual( r.getTransform(), M44f() )
		self.assertEqual( r.getTransform( "world" ), M44f() )
		self.assertEqual( r.getTransform( "object" ), M44f() )
		self.assert_( r.getTransform( "coordSys" ).equalWithAbsError( t, 0.000001 ) )
		
		r.worldEnd()
		
	def testIgnoreOtherAttributesAndOptions( self ) :
	
		r = IECoreRI.Renderer( "test/IECoreRI/output/transform.rib" )
		
		r.setOption( "someOthereRenderer:someOtherOption", IntData( 10 ) )
		
		r.worldBegin()
		
		# this should be silently ignored
		r.setAttribute( "someOtherRenderer:someOtherAttribute", IntData( 10 ) )
		
		r.worldEnd()

if __name__ == "__main__":
    unittest.main()   
