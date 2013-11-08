// $Id$
#ifndef ELEMENTENUM_H_
#define ELEMENTENUM_H_

namespace NuTo
{
namespace Element
{
enum eElementType
{
    BOUNDARYGRADIENTDAMAGE1D=0, //!< boundary element for gradient models
    BRICK8N,                    //!< three-dimensional brick element with 8 nodes
    PLANE2D3N,                  //!< two-dimensional plane element with 3 nodes
    PLANE2D4N,                  //!< two-dimensional plane element with 4 nodes
    PLANE2D4NSPECTRALORDER2,    //!< two-dimensional plane element with 4 nodes for geometry interpolation and 3x3 for field interpolation
    PLANE2D6N,                  //!< two-dimensional plane element with 6 nodes
    TETRAHEDRON4N,              //!< three-dimensional tetrahedron element with 4 nodes
    TETRAHEDRON10N,             //!< three-dimensional tetrahedron element with 10 nodes
    TRUSS1D2N,                  //!< one-dimensional truss element with two nodes
    TRUSS1D3N                  //!< one-dimensional truss element with three nodes
};

enum eUpdateType
{
    STATICDATA=0,		          //!< @ToDo[eUpdateType]: Description
    TMPSTATICDATA,		          //!< @ToDo[eUpdateType]: Description
    CRACK,				          //!< update for crack informations
    SWITCHMULTISCALE2NONLINEAR    //!< move the fine scale model in a multiscale approach to the nonlinear part
};

enum eOutput
{
	INTERNAL_GRADIENT,               //!<
	HESSIAN_0_TIME_DERIVATIVE,       //!<
	HESSIAN_1_TIME_DERIVATIVE,       //!<
	HESSIAN_2_TIME_DERIVATIVE,       //!<
	LUMPED_HESSIAN_2_TIME_DERIVATIVE,//!<
	UPDATE_STATIC_DATA,
	UPDATE_TMP_STATIC_DATA,
	IP_DATA,                         //!< this is primarily for plotting, give the 3D state  so for plane stress there is a z-component in the strain
	GLOBAL_ROW_DOF,                  //!< calculates the row dofs of the local element matrices
	GLOBAL_COLUMN_DOF                 //!< calculates the column dofs of the local element matrices
};

}
}
#endif /* ELEMENTENUM_H_ */
