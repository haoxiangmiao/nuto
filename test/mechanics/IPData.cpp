//
// Created by Thomas Titscher on 10/22/16.
//

#define BOOST_TEST_MODULE IPDataTest
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "mechanics/elements/IPData.h"
#include "mechanics/integrationtypes/IntegrationType1D2NGauss1Ip.h"
#include "mechanics/integrationtypes/IntegrationType1D2NGauss2Ip.h"
#include "mechanics/constitutive/staticData/IPConstitutiveLaw.h"
#include "mechanics/constitutive/inputoutput/ConstitutiveIOMap.h"
#include "mechanics/constitutive/laws/GradientDamageEngineeringStress.h"
#include <type_traits>

// necessary to build with clang when boost has been compiled by gcc
std::string boost::unit_test::ut_detail::normalize_test_case_name(const_string name)
{
    return (name[0] == '&' ? std::string(name.begin()+1, name.size()-1) : std::string(name.begin(), name.size() ));
}


const NuTo::GradientDamageEngineeringStress* AsLaw(const NuTo::Constitutive::IPConstitutiveLawBase& rLaw)
{
    const auto* law = dynamic_cast<const NuTo::GradientDamageEngineeringStress*>(&rLaw.GetConstitutiveLaw());
    BOOST_CHECK(law != nullptr);
    return law;
}


BOOST_AUTO_TEST_CASE(IPData_Setup_Test)
{
    NuTo::GradientDamageEngineeringStress law;
    NuTo::IntegrationType1D2NGauss1Ip integrationType1;
    NuTo::IntegrationType1D2NGauss2Ip integrationType2;

    NuTo::IPData data(integrationType1);
    BOOST_CHECK_THROW(data.GetIPConstitutiveLaw(0), NuTo::MechanicsException);
    BOOST_CHECK(not data.HasConstitutiveLawAssigned(0));

    data.SetConstitutiveLaw(law);
    BOOST_CHECK_EQUAL(&law, AsLaw(data.GetIPConstitutiveLaw(0)));
    BOOST_CHECK(data.HasConstitutiveLawAssigned(0));

    data.SetIntegrationType(integrationType2);
    BOOST_CHECK_EQUAL(&law, AsLaw(data.GetIPConstitutiveLaw(0)));
    BOOST_CHECK_EQUAL(&law, AsLaw(data.GetIPConstitutiveLaw(1)));

    data.SetIntegrationType(integrationType1);
    BOOST_CHECK_EQUAL(&law, AsLaw(data.GetIPConstitutiveLaw(0)));
}

BOOST_AUTO_TEST_CASE(IPData_Copy_Move)
{
    BOOST_CHECK(std::is_copy_constructible<NuTo::IPData>::value);
    BOOST_CHECK(std::is_move_constructible<NuTo::IPData>::value);

    BOOST_CHECK(std::is_copy_assignable<NuTo::IPData>::value);
    BOOST_CHECK(std::is_move_assignable<NuTo::IPData>::value);
}

BOOST_AUTO_TEST_CASE(IPData_Copy_Move_Values)
{
    NuTo::GradientDamageEngineeringStress law;
    NuTo::IntegrationType1D2NGauss1Ip integrationType;

    constexpr double kappa = 42.6174;

    NuTo::IPData data(integrationType);
    data.SetConstitutiveLaw(law);
    data.GetIPConstitutiveLaw(0).GetData<NuTo::GradientDamageEngineeringStress>().SetData(kappa);

    NuTo::IPData data2(data);   // copy construction
    BOOST_CHECK_EQUAL(&integrationType, &data2.GetIntegrationType());
    BOOST_CHECK_EQUAL(&law, AsLaw(data2.GetIPConstitutiveLaw(0)));
    BOOST_CHECK_EQUAL(kappa, data2.GetIPConstitutiveLaw(0).GetData<NuTo::GradientDamageEngineeringStress>().GetData());

    NuTo::IPData data3(integrationType);
    data3 = data;               // copy assignment
    BOOST_CHECK_EQUAL(&integrationType, &data3.GetIntegrationType());
    BOOST_CHECK_EQUAL(&law, AsLaw(data3.GetIPConstitutiveLaw(0)));
    BOOST_CHECK_EQUAL(kappa, data3.GetIPConstitutiveLaw(0).GetData<NuTo::GradientDamageEngineeringStress>().GetData());

    NuTo::IPData data4(std::move(data));  // move construction
    BOOST_CHECK_EQUAL(&integrationType, &data4.GetIntegrationType());
    BOOST_CHECK_EQUAL(&law, AsLaw(data4.GetIPConstitutiveLaw(0)));
    BOOST_CHECK_EQUAL(kappa, data4.GetIPConstitutiveLaw(0).GetData<NuTo::GradientDamageEngineeringStress>().GetData());

    NuTo::IPData data5(integrationType);
    data5 = std::move(data2);    // move assignment
    BOOST_CHECK_EQUAL(&integrationType, &data5.GetIntegrationType());
    BOOST_CHECK_EQUAL(&law, AsLaw(data5.GetIPConstitutiveLaw(0)));
    BOOST_CHECK_EQUAL(kappa, data5.GetIPConstitutiveLaw(0).GetData<NuTo::GradientDamageEngineeringStress>().GetData());

}
