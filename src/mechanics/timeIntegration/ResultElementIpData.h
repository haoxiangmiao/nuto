#pragma once


#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#endif // ENABLE_SERIALIZATION

#include "mechanics/timeIntegration/ResultBase.h"


namespace NuTo
{

class StructureBase;

namespace IpData
{
    enum class eIpStaticDataType;
}// namespace IpData

//! @author Philip Huschke
//! @date October 2015
//! @brief Outputs integration point values
class ResultElementIpData : public ResultBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
public:
    //! @brief constructor
    //! @param rFileName:   file name
    //! @param rElementId:  element id
    //! @param rIpDataType: data type at the integration points, e.g. stress, strain, damage...
    ResultElementIpData(const std::string& rFileName, int rElementId, NuTo::IpData::eIpStaticDataType rIpDataType);

    //! @brief calculates the relevant integration point data and adds them to the internal routine
    void CalculateAndAddValues(const StructureBase& rStructure, int rTimeStepPlot);

    //! @brief calculates the relevant integration point data
    void CalculateValues(const StructureBase& rStructure, Eigen::Matrix<double, 1, Eigen::Dynamic>& rValues) const;

    //! @brief number of data points per time step, e.g. number of stress components for an integration point
    int GetNumData(const StructureBase& rStructure) const override;

    NuTo::eTimeIntegrationResultType GetResultType() const override;

    //! @brief returns the class name
    std::string GetTypeId() const
    {
    	return std::string("ResultElementIpValue");
    }

    //! @brief this is used to cast an object of ResultBase to an object of ResultElementIpData
    ResultElementIpData* AsResultElementIpData() override
    {
        return this;
    }

    //! @brief ... Info routine that prints general information about the object
    void Info()const override;

private:
    int mElementId;
    NuTo::IpData::eIpStaticDataType mIpDataType;
};
}// namespace NuTo

#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
#include <boost/serialization/assume_abstract.hpp>
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::ResultElementIpValue)
#endif      // SWIG
#endif      // ENABLE_SERIALIZATION