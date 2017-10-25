#pragma once
#include <set>
#include <eigen3/Eigen/Sparse>
#include "mechanics/dofs/DofContainer.h"
#include "mechanics/constraintsPde/Equation.h"

namespace NuTo
{
namespace ConstraintPde
{
//! @brief stores constraint equations, separated by their dof type
class Constraints
{
    using Equations = std::vector<Equation>;

public:
    //! @brief adds an equation
    //! @param dof dof type
    //! @param equation constraint equation
    void Add(DofType dof, Equation equation);

    //! @brief adds multiple equations
    //! @param dof dof type
    //! @param equations constraint equations
    void Add(DofType dof, std::vector<Equation> equations);

    //! @brief builds the time dependent constraint rhs vector for a specific dof type
    //! @param dof dof type
    //! @param time global time
    //! @return constraint rhs vector
    Eigen::VectorXd GetRhs(DofType dof, double time) const;

    //! @brief builds a sparse matrix containing the constraint terms for a specific dof type
    //! @param dof dof type
    //! @param nIndependentDofs number of independent dofs for the dof type dof, required for a proper resize
    //!        of the sparse matrix
    //! @return sparse matrix containing the constraint terms where the last block with size (numDependent x
    //! numDependent is removed)
    Eigen::SparseMatrix<double> BuildConstraintMatrix(DofType dof, int numIndependentDofs) const;

    //! @brief calculates the number of constraint equations for a specific dof type
    //! @param dof dof type
    //! @return number of constraint equations
    int GetNumEquations(DofType dof) const;

    //!@brief gets the specified equation
    //! @param dof ... doftype of the equation
    //! @param equationNumber ... number of the equation
    //! @return selected equation
    const Equation& GetEquation(DofType dof, int equationNumber) const;

    //! @brief getter for mConstraintsChanged
    //! @return true if constraints changed
    bool HaveChanged() const;

    //! @brief setter for mConstraintsChanged
    void SetHaveChanged(bool value);

private:
    //! @brief dof-wise storage of constraint equations
    DofContainer<Equations> mEquations;


    //! @brief stores the terms of all existing equations in ordered containers
    //! @remark A naive check of all existing terms in mEquations is O(N^2) since we have to traverse them whenever an
    //! equation is added. Adding >10000 equations this way is not feasible. This class performs those checks in O(N log
    //! N) and everything is fine again.
    class TermChecker
    {
    public:
        //! @brief Check if the `e` collides with existing terms.
        //! @remark throws if
        //          - the dependent term of `e` is in any existing equation
        //          - any term of `e` is in the depenend terms of any existing equation
        void CheckEquation(Equation e);

    private:
        struct TermCompare
        {
            bool operator()(const Term& lhs, const Term& rhs) const;
        };

        std::set<Term, TermCompare> mDependentTerms;
        std::set<Term, TermCompare> mOtherTerms;
    } mTermChecker;

    //! @brief flag that indiciates whether or not new constraints were added
    //! @remark the global dof numbering needs a rebuild, if this flag is true.
    //! And it has to be set to false after rebuilding.
    //! Problems with this approach: The behavior of this class is not influenced
    //! by this state variable, currently only NuTo::Assembler is. Thus, it should
    //! be solved in a different approach. Maybe:
    //!  - Assembler keeps track of NumConstraints
    //!  - Assembler keeps track of std::hash(mEquations)
    //!  - Not allowed to add constraints after building the global dof numbering
    //!       = mConstraints is a const member at mAssembler
    bool mConstraintsChanged = false;
};


} /* Constaint */
} /* NuTo */
