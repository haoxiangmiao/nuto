#pragma once

#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/mechanics/constitutive/staticData/Leaf.h"

#include "eigen3/Eigen/Eigenvalues"
#include <functional>
namespace NuTo
{

template <int TDim> class EngineeringStrain;
template <int TRows, int TCols> class ConstitutiveMatrix;
template <int TRows> class ConstitutiveVector;
class ConstitutiveScalar;
class ConstitutiveIOBase;
class Logger;

namespace Constitutive
{
    enum class ePhaseFieldEnergyDecomposition;

}// namespace Constitutive




//! \class  PhaseField
//! \author Philip Huschke
//! \date   June 14, 2016
//! \brief  A phase-field model for brittle fracture

//! Recommended literature:
//!
//! Miehe et al. \n
//! "Thermodynamically consistent phase-field models of fracture: Variational principles and multi-field FE implementations"
//!
//! Ambati et al. \n
//! "A review on phase-field models of brittle fracture and a new fast hybrid formulation"
class PhaseField : public ConstitutiveBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
public:
    //! \brief      Constructor
    //! \param[in]  rYoungsModulus Young's Modulus
    //! \param[in]  rPoissonsRatio Poisson's Ratio
    //! \param[in]  rLengthScaleParameter Parameter that corresponds to the band-width of the diffusive crack
    //! \param[in]  rFractureEnergy Fracture energy/critical energy release rate
    //! \param[in]  rArtificialViscosity Parameter to improve robustness of the model (non-physical)
    //! \param[in]  rEnergyDecomposition Decomposition of the elastic energy density \f$\psi_0 = \psi_0^+ + \psi_0^-\f$
    PhaseField(             const double rYoungsModulus,
                            const double rPoissonsRatio,
                            const double rLengthScaleParameter,
                            const double rFractureEnergy,
                            const double rArtificialViscosity,
                            const Constitutive::ePhaseFieldEnergyDecomposition rEnergyDecomposition);

    //! \brief     Determines the constitutive inputs needed to evaluate the constitutive outputs
    //! \param[in] rConstitutiveOutput Desired constitutive outputs
    //! \param[in] rInterpolationType Interpolation type to determine additional inputs
    //! \return    Constitutive inputs needed for the evaluation
    ConstitutiveInputMap GetConstitutiveInputs(const ConstitutiveOutputMap& rConstitutiveOutput, const InterpolationType& rInterpolationType) const override;

    //! \brief Evaluate the constitutive law in 1D
    //! \param[in] rConstitutiveInput Input to the constitutive law (strain, temp gradient etc.)
    //! \param[out] rConstitutiveOutput Output to the constitutive law (stress, stiffness, heat flux etc.)
    //! \param staticData Pointer to history data
    NuTo::eError Evaluate1D(
            const ConstitutiveInputMap& rConstitutiveInput,
            const ConstitutiveOutputMap& rConstitutiveOutput,
            Constitutive::StaticData::Component* staticData) override;


    //! \brief Evaluate the constitutive law in 2D
    //! \param[in] rConstitutiveInput Input to the constitutive law (strain, temp gradient etc.)
    //! \param[out] rConstitutiveOutput Output to the constitutive law (stress, stiffness, heat flux etc.)
    //! \param staticData Pointer to history data
    NuTo::eError Evaluate2D(
            const ConstitutiveInputMap& rConstitutiveInput,
            const ConstitutiveOutputMap& rConstitutiveOutput,
            Constitutive::StaticData::Component* staticData) override;


    //! \brief Evaluate the constitutive law in 3D
    //! \param[in] rConstitutiveInput Input to the constitutive law (strain, temp gradient etc.)
    //! \param[out] rConstitutiveOutput Output to the constitutive law (stress, stiffness, heat flux etc.)
    //! \param staticData Pointer to history data
    NuTo::eError Evaluate3D(
            const ConstitutiveInputMap& rConstitutiveInput,
            const ConstitutiveOutputMap& rConstitutiveOutput,
            Constitutive::StaticData::Component* staticData) override;

    double Evaluate2DAnisotropicSpectralDecomposition(const double oldEnergyDensity,
            const ConstitutiveInputMap& rConstitutiveInput, const ConstitutiveOutputMap& rConstitutiveOutput);

    double Evaluate2DIsotropic(const double oldEnergyDensity,
            const ConstitutiveInputMap& rConstitutiveInput, const ConstitutiveOutputMap& rConstitutiveOutput);

    //! @brief calculates the error of the extrapolation
    //! @param rElement ... element
    //! @param rIp ... integration point
    //! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
    //! @return ... error of the extrapolation
    double CalculateStaticDataExtrapolationError(ElementBase& rElement, int rIp, const ConstitutiveInputMap& rConstitutiveInput) const;


    //! @brief Create new static data object for an integration point
    //! @return Pointer to static data object
    Constitutive::StaticData::Leaf<double>* AllocateStaticData1D(const ElementBase* rElement) const override;

    //! @brief Create new static data object for an integration point
    //! @return Pointer to static data object
    Constitutive::StaticData::Leaf<double>* AllocateStaticData2D(const ElementBase* rElement) const override;

    //! @brief Create new static data object for an integration point
    //! @return Pointer to static data object
    Constitutive::StaticData::Leaf<double>* AllocateStaticData3D(const ElementBase* rElement) const override;

    //! @brief Determines which submatrices of a multi-doftype problem can be solved by the constitutive law
    //! @param rDofRow row dof
    //! @param rDofCol column dof
    //! @param rTimeDerivative time derivative
    virtual bool CheckDofCombinationComputable(Node::eDof rDofRow,
                                                Node::eDof rDofCol,
                                                int rTimeDerivative) const override;

    //! @brief Gets a parameter of the constitutive law which is selected by an enum
    //! @param rIdentifier ... Enum to identify the requested parameter
    //! @return ... value of the requested variable
    virtual double GetParameterDouble(Constitutive::eConstitutiveParameter rIdentifier) const override;

    //! @brief Sets a parameter of the constitutive law which is selected by an enum
    //! @param rIdentifier ... Enum to identify the requested parameter
    //! @param rValue ... new value for requested variable
    virtual void SetParameterDouble(Constitutive::eConstitutiveParameter rIdentifier, double rValue) override;

    //! @brief Gets the type of constitutive relationship
    //! @return Type of constitutive relationship
    //! @sa eConstitutiveType
    Constitutive::eConstitutiveType GetType() const override;

    //! @brief Check parameters of the constitutive relationship
    void CheckParameters()const override;

    //! @brief Check compatibility between element type and type of constitutive relationship
    //! @param rElementType ... element type
    //! @return ... <B>true</B> if the element is compatible with the constitutive relationship, <B>false</B> otherwise.
    bool CheckElementCompatibility(Element::eElementType rElementType) const override;

    //! @brief Print information about the object
    //! @param rVerboseLevel ... verbosity of the information
    void Info(unsigned short rVerboseLevel, Logger& rLogger) const override;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

    //! @brief Returns true, if a material model has tmp static data (which has to be updated before stress or stiffness are calculated)
    //! @return ... see brief explanation
    bool HaveTmpStaticData() const override {return false;}



protected:
    //! @brief Young's modulus \f$ E \f$
    const double mYoungsModulus;

    //! @brief Poisson's ratio \f$ \nu \f$
    const double mPoissonsRatio;

    //! @brief Length scale parameter \f$ l \f$
    const double mLengthScaleParameter;

    //! @brief Fracture energy \f$ G_f \f$
    const double mFractureEnergy;

    //! @brief Artificial viscosity to improve numerical robustness \f$ \eta \f$
    const double mArtificialViscosity;

    //! @brief First Lame parameter \f$ \lambda \f$
    const double mLameLambda;

    //! @brief Second Lame parameter \f$ \mu \f$
    const double mLameMu;

    //! @brief Type of degradation function
    const Constitutive::ePhaseFieldEnergyDecomposition mEnergyDecomposition;

private:

    // template this function over dimension and reduce the number of input arguments once its working
    double CalculateComponentsSpectralDecompositionDStressDStrain(int rI, int rJ, int rK, int rL, const Eigen::SelfAdjointEigenSolver<Eigen::Matrix<double,2,2>>& rEigenSolver, std::function<double (double,double)> rRampFunction, std::function<bool (double)> rStepFunction);

    void CalculateSpectralDecompositionDStressDStrain(ConstitutiveIOBase& tangent, const double factor, const Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d>& eigenSolver);


};
}// namespace NuTo

#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::PhaseField)
#endif // ENABLE_SERIALIZATION
