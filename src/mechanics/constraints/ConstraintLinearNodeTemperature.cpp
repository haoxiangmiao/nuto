#include "ConstraintLinearNodeTemperature.h"
#include "mechanics/MechanicsException.h"
#include "mechanics/nodes/NodeBase.h"
#include "mechanics/nodes/NodeEnum.h"
#include "math/SparseMatrix.h"
#include "math/FullVector.h"

NuTo::ConstraintLinearNodeTemperature::ConstraintLinearNodeTemperature(const NodeBase* rNode, double rValue)
    :   ConstraintLinear(),
		ConstraintNode(rNode),
        mRHS{rValue}
{}

void  NuTo::ConstraintLinearNodeTemperature::AddToConstraintMatrix(
        int& curConstraintEquation, NuTo::SparseMatrix<double>& rConstraintMatrix) const
{
    // add constraint to constrain matrix
    if (mNode->GetNum(Node::eDof::TEMPERATURE)!=1)
    {
        throw MechanicsException(__PRETTY_FUNCTION__,
                "Node does not have a temperature component or has more than one temperature component.");
    }
    rConstraintMatrix.AddValue(curConstraintEquation,mNode->GetDof(Node::eDof::TEMPERATURE), 1);

    // increase constraint equation number
    curConstraintEquation++;
}

int NuTo::ConstraintLinearNodeTemperature::GetNumLinearConstraints() const
{
    return 1;
}

double NuTo::ConstraintLinearNodeTemperature::GetRHS() const
{
    return mRHS;
}

void NuTo::ConstraintLinearNodeTemperature::SetRHS(double rRHS)
{
    mRHS = rRHS;
}

void NuTo::ConstraintLinearNodeTemperature::GetRHS(int& rCurConstraintEquation,
        Eigen::VectorXd& rRHS) const
{
    // add constraint to constrain matrix
    if (mNode->GetNum(Node::eDof::TEMPERATURE) != 1)
    {
        throw MechanicsException(__PRETTY_FUNCTION__,
            "Node does not have a temperature component or has more than one temperature component.");
    }
    // set right hand side value
    rRHS(rCurConstraintEquation) = mRHS;

    // increase constraint equation number
    rCurConstraintEquation++;
}

void NuTo::ConstraintLinearNodeTemperature::Info(unsigned short rVerboseLevel) const
{
    throw MechanicsException(__PRETTY_FUNCTION__, " to be implemented.");
}

NuTo::Node::eDof NuTo::ConstraintLinearNodeTemperature::GetDofType() const
{
    return Node::eDof::TEMPERATURE;
}
