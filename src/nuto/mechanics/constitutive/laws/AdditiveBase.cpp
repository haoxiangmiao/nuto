#include "nuto/mechanics/constitutive/laws/AdditiveBase.h"
#include "nuto/mechanics/nodes/NodeEnum.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveIOMap.h"
#include "nuto/mechanics/constitutive/inputoutput/ConstitutiveIOBase.h"



NuTo::AdditiveBase::AdditiveBase(const int& rNumTimeDerivatives)
    : mNumTimeDerivatives(rNumTimeDerivatives)
{
    // Even though you get the number of time derivatives during construction, it might change later. So we set the size to 3 (disp, vel, acc) to avoid problems.
    mComputableDofCombinations.resize(3);
}


void NuTo::AdditiveBase::AddConstitutiveLaw(NuTo::ConstitutiveBase& rConstitutiveLaw, Constitutive::eInput)
{
    if(rConstitutiveLaw.HaveTmpStaticData())
        throw MechanicsException(__PRETTY_FUNCTION__,
            "Constitutive law has tmp static data! The HaveTmpStaticData function is only called on construction of "
            "the AdditiveInputExplicit law, but at this time, no constitutive law is attached. Therefore it does not "
            "know if it will have tmpstatic data or not and returns false by default. Find a way to update this "
            "information at the necessary code sections if a law with tmpstatic data is attached.");

    if(mStaticDataAllocated)
        throw MechanicsException(__PRETTY_FUNCTION__,
                "All constitutive laws have to be attached before static data is allocated!");

    mSublaws.push_back(&rConstitutiveLaw);
    AddCalculableDofCombinations(rConstitutiveLaw);
}


void NuTo::AdditiveBase::AddCalculableDofCombinations(NuTo::ConstitutiveBase& rConstitutiveLaw)
{
    std::set<Node::eDof> allDofs = Node::GetDofSet();
    for (unsigned int i = 0;  i < mComputableDofCombinations.size(); ++i)
        for (auto itRow : allDofs)
            for (auto itCol : allDofs)
            {
                if (rConstitutiveLaw.CheckDofCombinationComputable(itRow,itCol,i))
                        mComputableDofCombinations[i].emplace(itRow,itCol);
            }
}


bool NuTo::AdditiveBase::CheckElementCompatibility(Element::eElementType rElementType) const
{
    for (auto& sublaw : mSublaws)
    {
        if(!sublaw->CheckElementCompatibility(rElementType))
            return false;
    }
    return true;
}


void NuTo::AdditiveBase::CheckParameters() const
{
    for (auto& sublaw : mSublaws)
    {
        sublaw->CheckParameters();
    }
}


bool NuTo::AdditiveBase::HaveTmpStaticData() const
{
    for (auto& sublaw : mSublaws)
    {
        if(sublaw->HaveTmpStaticData())
            return true;
    }
    return false;
}

bool NuTo::AdditiveBase::CheckDofCombinationComputable(NuTo::Node::eDof rDofRow, NuTo::Node::eDof rDofCol,
        int rTimeDerivative) const
{
    return mComputableDofCombinations[rTimeDerivative].find(std::pair<Node::eDof, Node::eDof>(rDofRow, rDofCol))
        != mComputableDofCombinations[rTimeDerivative].end();
}


NuTo::ConstitutiveInputMap NuTo::AdditiveBase::GetConstitutiveInputs(
        const NuTo::ConstitutiveOutputMap &rConstitutiveOutput, const NuTo::InterpolationType &rInterpolationType) const
{
    ConstitutiveInputMap constitutiveInputMap;

    for (auto& sublaw : mSublaws)
    {
        ConstitutiveInputMap singleLawInputMap = sublaw->GetConstitutiveInputs(rConstitutiveOutput, rInterpolationType);
        constitutiveInputMap.Merge(singleLawInputMap);
    }
    return constitutiveInputMap;
}


NuTo::ConstitutiveBase& NuTo::AdditiveBase::GetSublaw(int rIndex)
{
    try
    {
        return (*mSublaws[rIndex]);
    }
    catch (...)
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Error accessing sublaw");
    }

}
