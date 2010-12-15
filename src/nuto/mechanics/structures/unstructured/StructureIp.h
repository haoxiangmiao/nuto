// $Id$

#ifndef STRUCTUREIP_H
#define STRUCTUREIP_H

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/export.hpp>
#endif //Serialize

#include <boost/ptr_container/ptr_map.hpp>

#include <set>
#include <array>

#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/constitutive/mechanics/ConstitutiveEngineeringStressStrain.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStrain2D.h"
#include "nuto/mechanics/elements/ElementBase.h"
#include "nuto/mechanics/elements/ElementDataEnum.h"
#include "nuto/mechanics/elements/IpDataEnum.h"
#include "nuto/mechanics/nodes/NodeBase.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"

namespace NuTo
{
//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... standard class for irregular (unstructured) structures, which are used as fine scale models at an integration point
class StructureIp : public Structure
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    //! @param mDimension  Structural dimension (1,2 or 3)
    StructureIp(int mDimension);

#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif //SWIG

    //! @brief ... save the object to a file
    //! @param filename ... filename
    //! @param aType ... type of file, either BINARY, XML or TEXT
    void Save (const std::string &filename, std::string rType )const;

    //! @brief ... restore the object from a file
    //! @param filename ... filename
    //! @param aType ... type of file, either BINARY, XML or TEXT
    void Restore (const std::string &filename, std::string rType );
#endif // ENABLE_SERIALIZATION

    //! @brief ... Return the name of the class, this is important for the serialize routines, since this is stored in the file
    //!            in case of restoring from a file with the wrong object type, the file id is printed
    //! @return    class name
    std::string GetTypeId()const
    {
        return std::string("StructureIp");
    }

    //************ Info routine         ***************
    //**  defined in structures/StructureIp.cpp *********
    //*************************************************
    //! @brief ... Info routine that prints general information about the object (detail according to verbose level)
    void Info()const;

    //! @brief ... calculate the displacement based on the homogeneous strain
    //! @param rCoordinates ... coordinates of the point
    //! @param rCoarseDisplacements ... return array of displacements
    void GetDisplacementsEpsilonHom2D(double rCoordinates[2], double rDisplacements[2])const;

    //! @brief ... calculate the displacement based on the crack opening
    //! @param rCoordinates ... coordinates of the point
    //! @param rCoarseDisplacements ... return array of displacements
    void GetDisplacementsCrack2D(double rCoordinates[2], double rDisplacements[2])const;

    //! @brief derivative of displacement with respect to homogeneous strain
    //! @param rdX_dEpsilonHom[3] return value, derivative of x-displacement with respect to homogeneous strain (exx, eyy, gxy)
    //! @param rdY_dEpsilonHom[3] return value, derivative of x-displacement with respect to homogeneous strain (exx, eyy, gxy)
    void GetdDisplacementdEpsilonHom(double rCoordinates[2], double rdX_dEpsilonHom[3], double rdY_dEpsilonHom[3])const;

    //! @brief derivative of displacement with respect to discontinuity (crack opening)
    //! @param rdX_dCrackOpening[2] return value, derivative of x-displacement with respect to crack opening (ux, uy)
    //! @param rdY_dCrackOpening[2] return value, derivative of x-displacement with respect to crack opening (ux, uy)
    void GetdDisplacementdCrackOpening(double rCoordinates[2], double rdX_dCrackOpening[2], double rdY_dCrackOpening[2])const;

    //! @brief second derivative of displacement with respect to alpha and discontinuity (crack opening)
    //! @param rdX_dCrackOpening[2] return value, derivative of x-displacement with respect to alpha and crack opening (ux, uy)
    //! @param rdY_dCrackOpening[2] return value, derivative of y-displacement with respect to alpha and crack opening (ux, uy)
    void Getd2Displacementd2CrackOpening(double rCoordinates[2], double rdX_dAlphaCrackOpening[2], double rdY_dAlphaCrackOpening[2])const;

    //! @brief derivative of displacement with respect to discontinuity (crack opening)
    //! @param rdX_dAlpha[2] return value, derivative of x-displacement with respect to crack orientation (alpha)
    //! @param rdy_dAlpha[2] return value, derivative of x-displacement with respect to crack orientation (alpha)
    void GetdDisplacementdCrackOrientation(double rCoordinates[2], double rdX_dAlpha[1], double rdy_dAlpha[1])const;

    //! @brief second derivative of displacement with respect to discontinuity (crack opening)
    //! @param rdX_dAlpha[2] return value, second derivative of x-displacement with respect to crack orientation (alpha)
    //! @param rdy_dAlpha[2] return value, second derivative of y-displacement with respect to crack orientation (alpha)
    void Getd2Displacementd2CrackOrientation(double rCoordinates[2], double rd2X_d2Alpha[1], double rd2Y_d2Alpha[1])const;

    //! @brief ... the boundary nodes were transformed from pure displacement type nodes to multiscale nodes
    //! the displacements are decomposed into a local displacement field and a global homogeneous/crack displacement
    void TransformBoundaryNodes(int rBoundaryNodesId);

#ifndef SWIG
    //! @brief ... the boundary nodes were transformed from pure displacement type nodes to multiscale nodes
    //! the displacements are decomposed into a local displacement field and a global homogeneous/crack displacement
    void TransformBoundaryNodes(Group<NodeBase>* rBoundaryNodes);
#endif

    //! @brief numbers non standard DOFs' e.g. in StructureIp, for standard structures this routine is empty
    void NumberAdditionalGlobalDofs();

    //! @brief ...merge additional dof values
    void NodeMergeAdditionalGlobalDofValues(const NuTo::FullMatrix<double>& rActiveDofValues, const NuTo::FullMatrix<double>& rDependentDofValues);

    //! @brief extract dof values additional dof values
    //! @param rActiveDofValues ... vector of global active dof values (ordering according to global dofs, size is number of active dofs)
    //! @param rDependentDofValues ... vector of global dependent dof values (ordering according to (global dofs) - (number of active dofs), size is (total number of dofs) - (number of active dofs))
    void NodeExtractAdditionalGlobalDofValues(NuTo::FullMatrix<double>& rActiveDofValues, NuTo::FullMatrix<double>& rDependentDofValues) const;

    //! @brief ... based on the global dofs build submatrices of the global coefficent matrix0
    //! @param rMatrixJJ ... submatrix jj (number of active dof x number of active dof)
    //! @param rMatrixJK ... submatrix jk (number of active dof x number of dependent dof)
    void BuildGlobalCoefficientSubMatrices0General(NuTo::SparseMatrix<double>& rMatrixJJ, NuTo::SparseMatrix<double>& rMatrixJK) const;

    //! @brief ... based on the global dofs build submatrices of the global coefficent matrix0
    //! @param rMatrixJJ ... submatrix jj (number of active dof x number of active dof)
    //! @param rMatrixJK ... submatrix jk (number of active dof x number of dependent dof)
    //! @param rMatrixKJ ... submatrix kj (number of dependent dof x number of active dof)
    //! @param rMatrixKK ... submatrix kk (number of dependent dof x number of dependent dof)
    void BuildGlobalCoefficientSubMatrices0General(NuTo::SparseMatrix<double>& rMatrixJJ, NuTo::SparseMatrix<double>& rMatrixJK, NuTo::SparseMatrix<double>& rMatrixKJ, NuTo::SparseMatrix<double>& rMatrixKK) const;

    //! @brief ... based on the global dofs build submatrices of the global coefficent matrix0
    //! @param rMatrixJJ ... submatrix jj (number of active dof x number of active dof)
    //! @param rMatrixJK ... submatrix jk (number of active dof x number of dependent dof)
    void BuildGlobalCoefficientSubMatrices0Symmetric(NuTo::SparseMatrix<double>& rMatrixJJ, NuTo::SparseMatrix<double>& rMatrixJK) const;

    //! @brief ... based on the global dofs build submatrices of the global coefficent matrix0
    //! @param rMatrixJJ ... submatrix jj (number of active dof x number of active dof)
    //! @param rMatrixJK ... submatrix jk (number of active dof x number of dependent dof)
    //! @param rMatrixKK ... submatrix kk (number of dependent dof x number of dependent dof)
    void BuildGlobalCoefficientSubMatrices0Symmetric(NuTo::SparseMatrix<double>& rMatrixJJ, NuTo::SparseMatrix<double>& rMatrixJK, NuTo::SparseMatrix<double>& rMatrixKK) const;

    //! @brief ... based on the global dofs build sub-vectors of the global internal potential gradient
    //! @param rActiveDofGradientVector ... global internal potential gradient which corresponds to the active dofs
    //! @param rDependentDofGradientVector ... global internal potential gradient which corresponds to the dependent dofs
    void BuildGlobalGradientInternalPotentialSubVectors(NuTo::FullMatrix<double>& rActiveDofGradientVector, NuTo::FullMatrix<double>& rDependentDofGradientVector) const;

#ifndef SWIG
    //! @brief calculate the distance of a point to the crack
    //! @param rCoordinates[2] coordinates of the point
    //! @return distance to crack
    double CalculateDistanceToCrack2D(double rCoordinates[2])const;

    //! @brief calculate the derivative of the distance of a point to the crack with respect to the crack orientation alpha
    //! @param rCoordinates[2] coordinates of the point
    //! @return distance to crack
    double CalculatedDistanceToCrack2DdAlpha(double rCoordinates[2])const;

    //! @brief calculate the second derivative of the distance of a point to the crack
    //! @param rCoordinates[2] coordinates of the point
    //! @return second derivative of distance to crack
    double Calculated2DistanceToCrack2Dd2Alpha(double rCoordinates[2])const;

    //! @brief calculate the derivative of the displacements at the nodes with respect to homogeneous strain, crack opening and crack orientation
    //! @param rMappingDofMultiscaleNode return value, for each dof, the corresponding entry in the rDOF vector, for nonmultiscale dofs, there is a -1
    //! @param rDOF return value, for each dof, the corresponding derivatives (ehomxx, ehomyy, gammahomxy, ux, uy, alpha)
    //! @param rDOF2 return value, for each dof, the corresponding second derivatives (alpha^2, alpha ux, alpha uy)
    void CalculatedDispdGlobalDofs(std::vector<int>& rMappingDofMultiscaleNode, std::vector<std::array<double,3> >& rDOF, std::vector<std::array<double,3> >& rDOF2)const;

    //! @briefset the total strain
    void SetTotalEngineeringStrain(EngineeringStrain2D& rTotalEngineeringStrain);

    //! @brief add constraint equation for alpha in case of norm of crackopening less than a prescribed value
    void SetConstraintAlpha(EngineeringStrain2D& rTotalEngineeringStrain);

    //! @brief return the total strain
    NuTo::EngineeringStrain2D GetTotalStrain()const;

    //! @brief renumbers the global dofs in the structure after
    void ReNumberAdditionalGlobalDofs(std::vector<int>& rMappingInitialToNewOrdering);

#endif

    //! @brief save the state as a binary
    void Save(std::stringstream& previousState)const;

    //! @brief restore the state from a binary
    void Restore(std::stringstream& previousState);

    double GetDimensionX()const
    {
        return mlX;
    }

    double GetDimensionY()const
    {
        return mlY;
    }

    int GetDofCrackAngle()const
    {
        return mDOFCrackAngle;
    }
protected:
    //! @brief ... standard constructor just for the serialization routine
    StructureIp()
    {}

    //! @brief Calculate the derivate of the homogeneous strain with respect to changes of the crack orientation and crack opening
    //! this is due to the constraint equation relating total strain, homogeneous strain and cracking strain
    //! @parameter rbHomAlpha dHom wrt alpha
    //! @paramter rbHomU[0-2] for wrt ux [3-5] for wrt uy
    //! @parameter bHessian depsilondalpha2[0-2], depsilondalphadux[3-5], depsilondalphadux[6-8]
    void GetdEpsilonHomdCrack(double rbHomAlpha[3], double rbHomU[6], double rbHessian[9])const;

    //calculate from the existing crack opening and orientation the cracking strain
    void CalculateHomogeneousEngineeringStrain();

    double mCrackAngle;
    int mDOFCrackAngle;
    double mCrackOpening[2];
    int mDOFCrackOpening[2];
    EngineeringStrain2D mEpsilonTot;
    EngineeringStrain2D mEpsilonHom;
    int mConstraintFineScaleX;
    int mConstraintFineScaleY;
    int mConstraintAlpha;
    double mlX,mlY;
    double mCrackTransitionZone;
    bool mBoundaryNodesTransformed;
    std::string mIPName;

};
}
#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
BOOST_CLASS_EXPORT_KEY(NuTo::StructureIp)
#endif // SWIG
#endif // ENABLE_SERIALIZATION
#endif // STRUCTUREIP_H