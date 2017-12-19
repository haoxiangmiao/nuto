#pragma once

#include "mechanics/constitutive/inputoutput/ConstitutiveIOBase.h"
#include "mechanics/constitutive/inputoutput/ConstitutivePlaneStateEnum.h"

namespace NuTo
{

//! @brief Input to tell the law whether the material is under plane stress or plane strain.
class ConstitutivePlaneState : public ConstitutiveIOBase
{
public:
    ConstitutivePlaneState(ePlaneState initialState)
        : ConstitutiveIOBase()
        , mPlaneState(initialState)
    {
    }

    virtual std::unique_ptr<ConstitutiveIOBase> clone() override
    {
        return std::make_unique<ConstitutivePlaneState>(*this);
    }

    ePlaneState GetPlaneState() const
    {
        return mPlaneState;
    }

    void SetPlaneState(ePlaneState newState)
    {
        mPlaneState = newState;
    }

private:
    ePlaneState mPlaneState;
};

} // namespace NuTo
