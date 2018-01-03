#pragma once

#include <vector>
#include <Eigen/Core>
#include "base/Exception.h"

namespace NuTo
{

class IntegrationPointBase;

namespace ElementOld
{
enum class eElementType;
} // namespace Element


//! @author Jörg F. Unger, ISM
//! @date November 2009
//! @brief ... standard abstract class for all integration types
class IntegrationTypeBase
{

public:
    IntegrationTypeBase() = default;
    IntegrationTypeBase(const IntegrationTypeBase&) = default;
    IntegrationTypeBase(IntegrationTypeBase&&) = default;

    IntegrationTypeBase& operator=(const IntegrationTypeBase&) = default;
    IntegrationTypeBase& operator=(IntegrationTypeBase&&) = default;

    //! @brief ... destructor
    virtual ~IntegrationTypeBase() = default;

    virtual int GetDimension() const = 0;

    //! @brief returns the local coordinates of an integration point
    //! @param rIpNum integration point (counting from zero)
    //! @return rCoordinates (result)
    virtual Eigen::VectorXd GetLocalIntegrationPointCoordinates(int rIpNum) const = 0;


    virtual Eigen::MatrixXd GetNaturalIntegrationPointCoordinates() const;

    //! @brief returns the total number of integration points for this integration type
    //! @return number of integration points
    virtual int GetNumIntegrationPoints() const = 0;

    //! @brief returns the weight of an integration point
    //! @param rIpNum integration point (counting from zero)
    //! @return weight of integration points
    virtual double GetIntegrationPointWeight(int rIpNum) const = 0;

    //! @brief info about the integration type
    //! @param rVerboseLevel determines how detailed the information is
    void Info(int rVerboseLevel) const;
};
} // namespace NuTo
