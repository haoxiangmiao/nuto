#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "nuto/math/CustomBoostSerializationExtensions.h"
#endif  // ENABLE_SERIALIZATION

#include "nuto/mechanics/interpolationtypes/InterpolationBaseIGA.h"
#include "nuto/mechanics/integrationtypes/IntegrationTypeBase.h"

NuTo::InterpolationBaseIGA::InterpolationBaseIGA(Node::eDof rDofType, Interpolation::eTypeOrder rTypeOrder, int rDimension) :
    InterpolationBase::InterpolationBase(rDofType, rTypeOrder, rDimension)
{}


void NuTo::InterpolationBaseIGA::Initialize()
{
    mNumNodes = CalculateNumNodes();
    mNumDofs = mNumNodes*GetNumDofsPerNode();
}

Eigen::MatrixXd  NuTo::InterpolationBaseIGA::ConstructMatrixN(Eigen::VectorXd rShapeFunctions) const
{
    int numNodes = GetNumNodes();
    int dimBlock = GetNumDofsPerNode();

    Eigen::MatrixXd matrixN(dimBlock, numNodes * dimBlock);
    for (int iNode = 0, iBlock = 0; iNode < numNodes; ++iNode, iBlock += dimBlock)
    {
        matrixN.block(0, iBlock, dimBlock, dimBlock) = Eigen::MatrixXd::Identity(dimBlock, dimBlock) * rShapeFunctions(iNode);
    }

    return matrixN;
}

#ifdef ENABLE_SERIALIZATION
NuTo::InterpolationBaseIGA::InterpolationBaseIGA():
{
}

template void NuTo::InterpolationBaseIGA::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::InterpolationBaseIGA::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::InterpolationBaseIGA::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::InterpolationBaseIGA::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::InterpolationBaseIGA::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::InterpolationBaseIGA::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::InterpolationBaseIGA::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize InterpolationBaseIGA" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(InterpolationBase);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize InterpolationBaseIGA" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::InterpolationBaseIGA)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::InterpolationBaseIGA)
#endif  // ENABLE_SERIALIZATION
