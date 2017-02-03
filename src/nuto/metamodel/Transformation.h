// $Id$

/*******************************************************************************
Bauhaus-Universitaet Weimar
Author: Joerg F. Unger,  Septermber 2009
*******************************************************************************/

#pragma once

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#endif  // ENABLE_SERIALIZATION

#include <eigen3/Eigen/Core>

namespace NuTo
{

template <class T, int rows, int cols> class FullMatrix;
//! @author Joerg F. Unger, ISM
//! @date September 2009
//! @brief abstract base class for the Transformations
class Transformation
{
#ifdef ENABLE_SERIALIZATION
	friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
	virtual ~Transformation(){};
#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION
    //! @brief build the transformation using the given Points
	virtual void Build(const FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCoordinates)=0;

    //! @brief transform the given points 
    virtual void TransformForward(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCoordinates)const=0;

    //! @brief transform the given points 
    virtual void TransformBackward(FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCoordinates)const=0;

protected:

};

} // namespace nuto
#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
BOOST_CLASS_EXPORT_KEY(NuTo::Transformation)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::Transformation)
#endif // SWIG
#endif // ENABLE_SERIALIZATION
