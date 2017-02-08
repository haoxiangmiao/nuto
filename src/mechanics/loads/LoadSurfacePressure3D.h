// $Id: LoadSurface3D.h 178 2009-12-11 20:53:12Z eckardt4 $
#pragma once

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif  // ENABLE_SERIALIZATION

#include "mechanics/loads/LoadSurfaceBase3D.h"

namespace NuTo
{
class NodeBase;
class StructureBase;
//! @author Jörg F. Unger, ISM
//! @date August 2013
//! @brief ... class for surface loads in 3D with a const direction and amplitude of the load
class LoadSurfacePressure3D : public LoadSurfaceBase3D
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    LoadSurfacePressure3D(int rLoadCase, StructureBase* rStructure, int rElementGroupId, int rNodeGroupId,
    		double rPressure);

    //! @brief calculates the surface load as a function of the coordinates and the normal (for pressure)
    //! @param rCoordinates ... global coordinates
    //! @param rNormal ... normal to the surface (pointing outwards)
    //! @param rLoadVector ... load vector
    void CalculateSurfaceLoad(Eigen::Vector3d& rCoordinates, Eigen::Vector3d& rNormal,
                              Eigen::Vector3d& rLoadVector)const override;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(LoadSurfaceBase3D)
           & BOOST_SERIALIZATION_NVP(mPressure);
    }
#endif // ENABLE_SERIALIZATION

protected:
    double mPressure;
};
}//namespace NuTo