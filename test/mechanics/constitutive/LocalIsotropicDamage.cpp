#include "BoostUnitTest.h"
#include "math/EigenCompanion.h"
#include "mechanics/constitutive/LocalIsotropicDamage.h"
#include "mechanics/constitutive/ModifiedMises.h"
#include "mechanics/constitutive/LinearElastic.h"
#include "mechanics/constitutive/damageLaws/DamageLawExponential.h"

#include "math/NewtonRaphson.h"


BOOST_AUTO_TEST_CASE(OneDimensional)
{
    using namespace NuTo;

    // Material parameters
    const double E = 20000;
    const double nu = 0.2;
    NuTo::Laws::LinearElastic<1> elasticLaw(E, nu);

    const double ft = 4;
    const double fc = 10 * ft;
    const double gf = 0.01;
    const double kappa0 = ft / E;
    const double beta = ft / gf;
    const double alpha = 1;

    // Define the law using policy based design principles, hopefully applied correctly
    Laws::LinearElastic<1> linearElastic(E, nu);
    Constitutive::DamageLawExponential dmg(kappa0, beta, alpha);
    ModifiedMises<1> strainNorm(nu, fc / ft);
    Laws::EvolutionImplicit<1, NuTo::ModifiedMises<1>> history(strainNorm, /*numCells=*/1, /*numIps=*/1);
    auto localDamageLaw = Laws::CreateLocalIsotropicDamage<1>(dmg, history, linearElastic);

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
            return E * strain;

        return ft * std::exp(ft / gf * (kappa0 - strain));
    };

    double stress = ft * 0.99;
    auto R = [&](double strain) { return localDamageLaw.Stress(EigenCompanion::ToEigen(strain), 0, 0, 0)[0] - stress; };
    auto DR = [&](double strain) { return localDamageLaw.Tangent(EigenCompanion::ToEigen(strain), 0, 0, 0)[0] - 1; };
    auto Norm = [&](double stress) { return std::abs(stress); };
    auto Info = [](int i, double x, double R) { BOOST_TEST_MESSAGE("" << i << ": x = " << x << " R = " << R); };
    auto problem = NuTo::NewtonRaphson::DefineProblem(R, DR, Norm, 1.e-10, Info);

    double strain = kappa0;
    localDamageLaw.Update(EigenCompanion::ToEigen(strain), 0, 0, 0);
    for (; stress > 0; stress -= ft * 0.1)
    {
        strain = NuTo::NewtonRaphson::Solve(problem, strain, NuTo::NewtonRaphson::DoubleSolver());
        localDamageLaw.Update(EigenCompanion::ToEigen(strain), 0, 0, 0);
        BOOST_CHECK_CLOSE(stress, analyticStress(strain), 1.e-10);
    }
}
