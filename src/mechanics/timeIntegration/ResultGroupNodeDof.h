// $Id: $
#pragma once

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#endif // ENABLE_SERIALIZATION

#include "mechanics/timeIntegration/ResultBase.h"

namespace NuTo
{
//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... standard abstract class for all results
template <class T>
class Group;

class NodeBase;

class ResultGroupNodeDof : public ResultBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
public:

    //! @brief constructor
    ResultGroupNodeDof(const std::string& rIdent, int rNodeGroupId);

    NuTo::ResultGroupNodeDof* AsResultGroupNodeDof()override
    {
    	return this;
    }

    virtual Eigen::VectorXd CalculateValues(const StructureBase& rStructure,
    		   const Eigen::VectorXd& rResidual_j, const Eigen::VectorXd& rResidual_k) const = 0;

    void CalculateAndAddValues(const StructureBase& rStructure, int rTimeStepPlot,
    		const Eigen::VectorXd& rResidual_j,
    		const Eigen::VectorXd& rResidual_k);

    const NuTo::Group<NodeBase>* GetGroupNodePtr(const StructureBase& rStructure)const;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif  // ENABLE_SERIALIZATION

    //! @brief ... Info routine that prints general information about the object (detail according to verbose level)
    void Info() const override;

protected:
    int mGroupNodeId;
};
}

//namespace NuTo
#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
#include <boost/serialization/assume_abstract.hpp>
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::ResultGroupNodeDof)
#endif // SWIG
#endif  // ENABLE_SERIALIZATION
