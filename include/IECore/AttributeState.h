//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_ATTRIBUTESTATE_H
#define IE_CORE_ATTRIBUTESTATE_H

#include "IECore/StateRenderable.h"

namespace IECore
{

/// A class to manipulate the attribute state of a Renderer.
/// \todo Make order of rendering match order of addition - there are cases where
/// 3delight cares very much what order attributes are specified in (subsurface attributes in particular).
/// \ingroup renderingGroup
class AttributeState : public StateRenderable
{
	public:

		/// Attributes is referenced directly rather than copied.
		AttributeState( CompoundDataPtr attributes );
		AttributeState( const CompoundDataMap &attributes = CompoundDataMap() );
		virtual ~AttributeState();

		IE_CORE_DECLAREOBJECT( AttributeState, StateRenderable );

		CompoundDataMap &attributes();
		const CompoundDataMap &attributes() const;
		/// This is mostly of use for the binding - the attributes()
		/// function gives more direct access to the contents of the CompoundData
		/// (it calls readable() or writable() for you).
		CompoundDataPtr attributesData();

		/// Calls renderer->setAttribute() for each name, data pair in attributes()
		virtual void render( Renderer *renderer ) const;

	private:

		CompoundDataPtr m_attributes;

		static const unsigned int m_ioVersion;
};

IE_CORE_DECLAREPTR( AttributeState );

} // namespace IECore


#endif // IE_CORE_ATTRIBUTESTATE_H
