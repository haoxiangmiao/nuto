/*
 * BoundaryElement2D.h
 *
 *  Created on: 9 Jun 2015
 *      Author: ttitsche
 */

#ifndef BOUNDARYELEMENT2D_H_
#define BOUNDARYELEMENT2D_H_

#include "nuto/mechanics/elements/BoundaryElementBase.h"

namespace NuTo
{

class StructureBase;
class ConstitutiveTangentLocal3x1;
class Element2D;
class BoundaryElement2D: public BoundaryElementBase
{
public:
    BoundaryElement2D(const ElementBase* rBaseElement, int rSurfaceEdge);

    virtual ~BoundaryElement2D()
    {

    }

    //! @brief returns the number of nodes in this element
    //! @sa See GetBoundaryNodeIndex for further information.
    //! @return number of nodes
    int GetNumNodes() const override;

    //! @brief calculates output data for the element
    //! @param eOutput ... coefficient matrix 0 1 or 2  (mass, damping and stiffness) and internal force (which includes inertia terms)
    //!                    @param updateStaticData (with DummyOutput), IPData, globalrow/column dofs etc.
    virtual Error::eError Evaluate(boost::ptr_multimap<NuTo::Element::eOutput, NuTo::ElementOutputBase>& rElementOutput) override;

    //! @brief returns the enum (type of the element)
    //! @return enum
    NuTo::Element::eElementType GetEnumType() const override
    {
        return Element::BOUNDARYELEMENT2D;
    }

    //! @brief returns the local dimension of the element
    //! this is required to check, if an element can be used in a 1d, 2D or 3D Structure
    //! @return local dimension
    int GetLocalDimension() const override
    {
        return 2;
    }

    //! @brief Allocates static data for an integration point of an element
    //! @param rConstitutiveLaw constitutive law, which is called to allocate the static data object
    ConstitutiveStaticDataBase* AllocateStaticData(const ConstitutiveBase* rConstitutiveLaw) const override;

    //! @brief cast the base pointer to an BoundaryElement1D, otherwise throws an exception
    const BoundaryElement2D* AsBoundaryElement2D() const override;

    //! @brief cast the base pointer to an BoundaryElement1D, otherwise throws an exception
    BoundaryElement2D* AsBoundaryElement2D() override;

    //! @brief returns true, if the boundary conditions are fulfilled, post-processing
    bool IsBoundaryConditionFulfilled() const;

#ifdef ENABLE_VISUALIZE
    void Visualize(VisualizeUnstructuredGrid& rVisualize, const boost::ptr_list<NuTo::VisualizeComponentBase>& rWhat) override;
#endif // ENABLE_VISUALIZE

protected:
    //! @brief ... just for serialization
    BoundaryElement2D()
    {
    }

    //! @brief calculates the base element's node index on the boundary
    //! @param rBoundaryNodeNumber node index of the boundary
    //! @sa See PointIsOnBoundary.
    int GetBoundaryNodeIndex(int rBoundaryNodeIndex) const override;


private:
    //! @brief returns a vector of boundary node indices
    //! @remark this method could be moved to the interpolation type where the node indices of all boundaries could be stored. For better performance.
    //! I think that this method (and the GetNode and GetNumNodes) is mostly used for debugging/info routines and performance doesn't matter here.
    const Eigen::VectorXi GetBoundaryNodeIndices() const;

    //! @brief returns true, if P is in line with A and B
    //! @remark The boundary consists of two corner vectors A and B. A point P is on the connecting line between them, if det(B-A  A-P) = 0.
    //! This is used to determine these P that are on the boundary of A and B.
    //! @param rA rB rP points A B P
    bool PointIsOnBoundary(const Eigen::VectorXd rA, const Eigen::VectorXd rB, const Eigen::VectorXd rP) const;

};

} /* namespace NuTo */

#endif /* BOUNDARYELEMENT1D_H_ */
