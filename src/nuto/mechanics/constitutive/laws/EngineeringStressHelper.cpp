#include "nuto/mechanics/constitutive/laws/EngineeringStressHelper.h"
#include "nuto/mechanics/constitutive/inputoutput/EngineeringStrain.h"

template <int TDim>
NuTo::EngineeringStrain<TDim> NuTo::EngineeringStressHelper::CalculateElasticEngineeringStrain(
        const EngineeringStrain<TDim>& rEngineeringStrain,
        const InterpolationType& rInterpolationType,
        const ConstitutiveInputMap& rConstitutiveInput,
        double rThermalExpansionCoefficient)
{
    //! @todo used to be for thermal strains -> remove
    NuTo::EngineeringStrain<TDim> elasticEngineeringStrain = rEngineeringStrain;
    return elasticEngineeringStrain;
}


std::tuple<double, double, double> NuTo::EngineeringStressHelper::CalculateCoefficients2DPlaneStress(double mE, double mNu)
{
    double factor = mE / (1.0 - (mNu * mNu));
    return std::make_tuple(
            factor,                         // C11
            factor * mNu,                   // C12
            factor * 0.5 * (1.0 - mNu));    // C33
}


std::tuple<double, double, double> NuTo::EngineeringStressHelper::CalculateCoefficients3D(double mE, double mNu)
{
    double factor = mE / ((1.0 + mNu) * (1.0 - 2.0 * mNu));
    return std::make_tuple(
            factor * (1.0 - mNu),           // C11
            factor * mNu,                   // C12
            mE / (2. * (1.0 + mNu)));       // C33
}


namespace NuTo
{

template NuTo::EngineeringStrain<1> NuTo::EngineeringStressHelper::CalculateElasticEngineeringStrain(
        const EngineeringStrain<1>& rEngineeringStrain,
        const InterpolationType& rInterpolationType,
        const ConstitutiveInputMap& rConstitutiveInput,
        double rThermalExpansionCoefficient);
template NuTo::EngineeringStrain<2> NuTo::EngineeringStressHelper::CalculateElasticEngineeringStrain(
        const EngineeringStrain<2>& rEngineeringStrain,
        const InterpolationType& rInterpolationType,
        const ConstitutiveInputMap& rConstitutiveInput,
        double rThermalExpansionCoefficient);
template NuTo::EngineeringStrain<3> NuTo::EngineeringStressHelper::CalculateElasticEngineeringStrain(
        const EngineeringStrain<3>& rEngineeringStrain,
        const InterpolationType& rInterpolationType,
        const ConstitutiveInputMap& rConstitutiveInput,
        double rThermalExpansionCoefficient);

}  // namespace NuTo