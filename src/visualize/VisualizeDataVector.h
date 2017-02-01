// $Id$

#pragma once

#include <iostream>

#include "visualize/VisualizeDataBase.h"

namespace NuTo
{

//! @brief ... vector (size 3) data for visualization
//! @author Stefan Eckardt, ISM
//! @date November 2009
class VisualizeDataVector: public NuTo::VisualizeDataBase
{
public:
    // constructor
    VisualizeDataVector();

    //! @brief ... get data type
    //! @return ... visualize data type
    NuTo::eVisualizeDataType GetDataType() const override;

    //! @brief ... get number of data
    //! @return ... number of data
    unsigned int GetNumData() const override;

    //! @brief ... get data
    //! @return ... pointer to data array
    const double* GetData() const override;

    //! @brief ... set data
    //! @param rData ... data array
    void SetData(const double* rData) override;

private:
    double mData[3];

protected:
    //! @brief ... create output stream
    //! @param os ... output stream
    //! @return ... output stream
    std::ostream& Output(std::ostream& os) const override;
};
}

