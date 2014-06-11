// $Id: ConstitutiveStaticDataDamageViscoPlasticity3D.h 87 2009-11-06 10:35:39Z unger3 $

#ifndef CONSTITUTIVESTATICDATADAMAGEVISCOPLASTICITY3D_H
#define CONSTITUTIVESTATICDATADAMAGEVISCOPLASTICITY3D_H

#include "nuto/mechanics/constitutive/mechanics/ConstitutiveStaticDataPrevEngineeringStressStrain3D.h"

//! @brief ... base class, storing the static data (history variables) of a constitutive relationship
//! @author Jörg F. Unger, ISM
//! @date December 2009
namespace NuTo
{
class IpDataStaticDataBase;

class ConstitutiveStaticDataDamageViscoPlasticity3D : public ConstitutiveStaticDataPrevEngineeringStressStrain3D
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
    friend class DamageViscoPlasticityEngineeringStress;
    friend class DamageViscoPlasticityHardeningEngineeringStress;
public:
	//! @brief constructor
    ConstitutiveStaticDataDamageViscoPlasticity3D();

    //! @brief copy constructor
    ConstitutiveStaticDataDamageViscoPlasticity3D(ConstitutiveStaticDataDamageViscoPlasticity3D const& rOther)
    {
        (*this) = rOther;
    }

    //! @brief clones (copies) the data
    ConstitutiveStaticDataDamageViscoPlasticity3D* Clone()const
    {
    	return new ConstitutiveStaticDataDamageViscoPlasticity3D(*this);
    }

    //! @brief assignment operator
    ConstitutiveStaticDataDamageViscoPlasticity3D& operator= (ConstitutiveStaticDataDamageViscoPlasticity3D const& rOther);

    //! @brief check, if the static data is compatible with a given element and a given constitutive model
    virtual bool CheckConstitutiveCompatibility(NuTo::Constitutive::eConstitutiveType rConstitutiveType, NuTo::Element::eElementType rElementType)const;

    //!@ brief reinterpret as damage viscoplasticity static data
    ConstitutiveStaticDataDamageViscoPlasticity3D* AsDamageViscoPlasticity3D();

    //!@ brief reinterpret as damage viscoplasticity static data
    const ConstitutiveStaticDataDamageViscoPlasticity3D* AsDamageViscoPlasticity3D()const;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

protected:
    //! @brief distributed accumulated inelastic strain
    double mKappaInelastic;

    //! @brief local damage variable associated with plastic strain (compressive damage)
    double mOmegaCompr;

    //! @brief plastic strain
    EngineeringStrain3D mEpsilonP;

    //! @brief viscoplastic strain
    EngineeringStrain3D mEpsilonVp;

    //! @brief plasticity state variable
    double mVP;

    //! @brief hardening state variable
    double mPrevHardening;
};

}
#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::ConstitutiveStaticDataDamageViscoPlasticity3D)
#endif // ENABLE_SERIALIZATION

#endif // CONSTITUTIVESTATICDATADAMAGEVISCOPLASTICITY3D_H
