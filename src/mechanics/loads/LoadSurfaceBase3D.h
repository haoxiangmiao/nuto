// $Id: LoadSurface3D.h 178 2009-12-11 20:53:12Z eckardt4 $
#ifndef LoadSurfaceBase3D_H
#define LoadSurfaceBase3D_H

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif  // ENABLE_SERIALIZATION

#include "mechanics/MechanicsException.h"
#include "mechanics/loads/LoadBase.h"
#include <vector>

namespace NuTo
{
class IntegrationTypeBase;
class NodeBase;
template<int TDim> class ContinuumElement;
class StructureBase;
//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... abstract class for all surface loads in 3D
class LoadSurfaceBase3D : public LoadBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    LoadSurfaceBase3D(int rLoadCase, StructureBase* rStructure, int rElementGroupId, int rNodeGroupId);

    //! @brief adds the load to global sub-vectors
    //! @param rLoadCase number of the current load case
    //! @param rActiceDofsLoadVector ... global load vector which correspond to the active dofs
    //! @param rDependentDofsLoadVector ... global load vector which correspond to the dependent dofs
    void AddLoadToGlobalSubVectors(int rLoadCase, Eigen::VectorXd& rActiceDofsLoadVector, Eigen::VectorXd& rDependentDofsLoadVector)const override;

    //! @brief calculates the surface load as a function of the coordinates and the normal (for pressure)
    //! @param rCoordinates ... global coordinates
    //! @param rNormal ... normal to the surface (pointing outwards)
    //! @param rLoadVector ... load vector
    virtual void CalculateSurfaceLoad(Eigen::Vector3d& rCoordinates, Eigen::Vector3d& rNormal,
                                      Eigen::Vector3d& rLoadVector)const=0;

#ifdef ENABLE_SERIALIZATION
    //! @brief deserializes (loads) the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
#ifdef DEBUG_SERIALIZATION
        std::cout << "start load LoadSurface3D\n";
#endif
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(LoadBase);

        std::vector<std::pair<std::uintptr_t, int> >  mVolumeElementsAddress;
        ar & boost::serialization::make_nvp("mVolumeElements", mVolumeElementsAddress);
        for(auto it : mVolumeElementsAddress)
        {
            mVolumeElements.push_back(reinterpret_cast<std::pair<const ContinuumElement<3>*, int> >(it));
        }

        ar & BOOST_SERIALIZATION_NVP(mIntegrationTypeTriangleGauss1)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeTriangleGauss2)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadGauss1)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadGauss2)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadLobatto2)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadLobatto3)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadLobatto4);
#ifdef DEBUG_SERIALIZATION
        std::cout << "finish load LoadSurface3D\n";
#endif
    }

    //! @brief serializes (saves) the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
#ifdef DEBUG_SERIALIZATION
        std::cout << "start save LoadSurface3D\n";
#endif
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(LoadBase);

        std::vector<std::pair<std::uintptr_t, int> >  mVolumeElementsAddress;
        for(auto it : mVolumeElements)
        {
            mVolumeElementsAddress.push_back(reinterpret_cast<std::pair<std::uintptr_t, int> >(it));
        }
        ar & boost::serialization::make_nvp("mVolumeElements", mVolumeElementsAddress);

        ar & BOOST_SERIALIZATION_NVP(mIntegrationTypeTriangleGauss1)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeTriangleGauss2)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadGauss1)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadGauss2)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadLobatto2)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadLobatto3)
           & BOOST_SERIALIZATION_NVP(mIntegrationTypeQuadLobatto4);

#ifdef DEBUG_SERIALIZATION
        std::cout << "finish save LoadSurface3D\n";
#endif
    }
#endif // ENABLE_SERIALIZATION

protected:
    std::vector<std::pair<const ContinuumElement<3>*, int> > mVolumeElements;
    IntegrationTypeBase* mIntegrationTypeTriangleGauss1;
    IntegrationTypeBase* mIntegrationTypeTriangleGauss2;

    IntegrationTypeBase* mIntegrationTypeQuadGauss1;
    IntegrationTypeBase* mIntegrationTypeQuadGauss2;
    IntegrationTypeBase* mIntegrationTypeQuadLobatto2;
    IntegrationTypeBase* mIntegrationTypeQuadLobatto3;
    IntegrationTypeBase* mIntegrationTypeQuadLobatto4;


};
}//namespace NuTo

#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::LoadSurfaceBase3D)
#endif

#endif //LoadSurfaceBase3D_H
