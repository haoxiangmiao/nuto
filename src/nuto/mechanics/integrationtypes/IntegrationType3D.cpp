// $Id$ 
// IntegrationType3D.cpp
// created Apr 30, 2010 by Joerg F. Unger

#include "nuto/mechanics/integrationtypes/IntegrationType3D.h"

//! @brief ... check compatibility between element type and type of constitutive relationship
//! @param rElementType ... element type
//! @return ... <B>true</B> if the element is compatible with the constitutive relationship, <B>false</B> otherwise.
bool NuTo::IntegrationType3D::CheckElementCompatibility(NuTo::Element::eElementType rElementType) const
{
    switch (rElementType)
    {
    case NuTo::Element::ELEMENT3D:
        return true;
    case NuTo::Element::TETRAHEDRON4N:
        return true;
    case NuTo::Element::TETRAHEDRON10N:
        return true;
    case NuTo::Element::BRICK8N:
        return true;
    case NuTo::Element::BRICK20N:
        return true;
    default:
        return false;
    }
}

#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::IntegrationType3D)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::IntegrationType3D)
#endif
