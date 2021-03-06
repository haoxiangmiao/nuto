#include "BoostUnitTest.h"

#include "nuto/math/EigenCompanion.h"
#include "nuto/mechanics/constitutive/LocalIsotropicDamage.h"
#include "nuto/math/NewtonRaphson.h"

using namespace NuTo;

Material::Softening concrete = Material::DefaultConcrete();
double kappa0 = concrete.ft / concrete.E;

BOOST_AUTO_TEST_CASE(OneDimensional)
{
    concrete.fMin = 0;
    Laws::LocalIsotropicDamage<1> localDamageLaw(concrete);

    // Test the law by following the decreasing part of the load displacement curve after the peak load
    //
    /* Something like:
     *
     * stress
     *    ^
     *    |
     * ft |...
     *    |   /\
     *    |  /  `.
     *    | /     `•.
     *    |/          ` •  . _
     *    0----|---------------> strain
     *      kappa0
     */

    auto analyticStress = [&](double strain) {
        if (strain < kappa0)
            return concrete.E * strain;

        return concrete.ft * std::exp(concrete.ft / concrete.gf * (kappa0 - strain));
    };

    double stress = concrete.ft * 0.99;
    auto R = [&](double strain) { return localDamageLaw.Stress(EigenCompanion::ToEigen(strain), 0, {})[0] - stress; };
    auto DR = [&](double strain) { return localDamageLaw.Tangent(EigenCompanion::ToEigen(strain), 0, {})[0] - 1; };
    auto Norm = [&](double stress) { return std::abs(stress); };
    auto Info = [](int i, double x, double R) { BOOST_TEST_MESSAGE("" << i << ": x = " << x << " R = " << R); };
    auto problem = NuTo::NewtonRaphson::DefineProblem(R, DR, Norm, 1.e-12, Info);

    double strain = kappa0;
    localDamageLaw.Update(EigenCompanion::ToEigen(strain), 0, {});
    for (; stress > 0; stress -= concrete.ft * 0.1)
    {
        strain = NuTo::NewtonRaphson::Solve(problem, strain, NuTo::NewtonRaphson::DoubleSolver());
        localDamageLaw.Update(EigenCompanion::ToEigen(strain), 0, {});
        BOOST_CHECK_CLOSE(stress, analyticStress(strain), 1.e-10);
    }
}

void CheckTangent(std::initializer_list<double> values, double kappa)
{
    EngineeringStrain<3> strain = NuTo::EigenCompanion::ToEigen(values);
    Laws::LocalIsotropicDamage<3> law(concrete);
    law.mEvolution.mKappas(0, 0) = kappa;

    Eigen::Matrix<double, 6, 6> tangent = law.Tangent(strain, 0, {});
    Eigen::Matrix<double, 6, 6> tangent_cdf = law.Tangent(strain, 0, {}) * 0.;

    const double delta = 1.e-8;

    for (int i = 0; i < 6; ++i)
    {
        strain[i] -= delta / 2.;
        auto s0 = law.Stress(strain, 0, {});
        strain[i] += delta;
        auto s1 = law.Stress(strain, 0, {});
        strain[i] -= delta / 2.;
        tangent_cdf.col(i) = (s1 - s0) / delta;
    }

    BOOST_CHECK_LT((tangent_cdf - tangent).cwiseAbs().maxCoeff(), 1.e-5);
}

BOOST_AUTO_TEST_CASE(Tangents)
{
    double kappa = kappa0 / 3.;
    CheckTangent({0., 0., 0., 0., 0., 0.}, kappa);
    CheckTangent({1.e-5, 0., 0., 0., 0., 0.}, kappa);
    CheckTangent({-1.e-5, 0., 0., 0., 0., 0.}, kappa);
    CheckTangent({1.e-5, 1.e-5, 0., 0., 0., 0.}, kappa);
    CheckTangent({2.e-5, 1.e-5, 0., 0., 0., 0.}, kappa);
    CheckTangent({2.e-5, -1.e-5, 0., 0., 0., 0.}, kappa);
    CheckTangent({0, 0, 2.e-5, 0., 0., 0.}, kappa);
    CheckTangent({1.e-5, 1.e-5, 2.e-5, 0., 0., 0.}, kappa);
    CheckTangent({1.e-5, -2.e-5, 2.e-5, 0., 0., 0.}, kappa);

    // some test in damaged loading
    kappa = 2 * kappa0;
    double eps = 1.e-6; // small load increment = damaged loading
    CheckTangent({kappa + eps, 0., 0., 0., 0., 0.}, kappa);
    CheckTangent({kappa, eps, 0., 0., 0., 0.}, kappa);
    CheckTangent({kappa, 0., eps, 0., 0., 0.}, kappa);

    CheckTangent({kappa + eps, +eps, 0., 0., 0., 0.}, kappa);
    CheckTangent({kappa, eps, eps, 0., 0., 0.}, kappa);
    CheckTangent({kappa + eps, 0., eps, 0., 0., 0.}, kappa);


    // decrement = elastic unloading
    CheckTangent({kappa - eps, 0., 0., 0., 0., 0.}, kappa);
    CheckTangent({kappa, -eps, 0., 0., 0., 0.}, kappa);
    CheckTangent({kappa, 0., -eps, 0., 0., 0.}, kappa);

    CheckTangent({kappa - eps, -eps, 0., 0., 0., 0.}, kappa);
    CheckTangent({kappa, -eps, -eps, 0., 0., 0.}, kappa);
    CheckTangent({kappa - eps, 0., -eps, 0., 0., 0.}, kappa);
}

BOOST_AUTO_TEST_CASE(EvolutionEdgeCase)
{
    Laws::LocalIsotropicDamage<3> law(concrete);
    EngineeringStrain<3> strain = EigenCompanion::ToEigen({1, 2, 3, 4, 5, 6}) * 1.e-5;
    double kappa = law.mEvolution.mStrainNorm.Value(strain);
    law.mEvolution.mKappas(0, 0) = kappa;

    BOOST_CHECK_GT(law.mEvolution.DkappaDstrain(strain, 0, {})(0, 0), 0.);
}
