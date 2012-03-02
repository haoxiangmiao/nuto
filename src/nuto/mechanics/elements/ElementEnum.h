// $Id$
#ifndef ELEMENTENUM_H_
#define ELEMENTENUM_H_

namespace NuTo
{
namespace Element
{
enum eElementType
{
    BRICK8N=0,           //!< three-dimensional brick element with 8 nodes
    LATTICE2D,           //!< two-dimensional lattice element
    LATTICE3D,           //!< two-dimensional lattice element
    PLANE2D3N,           //!< two-dimensional plane element with 3 nodes
    PLANE2D4N,           //!< two-dimensional plane element with 4 nodes
    PLANE2D6N,           //!< two-dimensional plane element with 6 nodes
    TETRAHEDRON4N,       //!< three-dimensional tetrahedron element with 4 nodes
    TETRAHEDRON10N,      //!< three-dimensional tetrahedron element with 10 nodes
    TRUSS1D2N,           //!< one-dimensional truss element with two nodes
    TRUSS1D3N,           //!< one-dimensional truss element with three nodes
    VOXEL8N              //!< three-dimensional cube element of a grid structure with 8 coincident nodes
};

enum eUpdateType
{
    STATICDATA=0,		          //!< @ToDo[eUpdateType]: Description
    TMPSTATICDATA,		          //!< @ToDo[eUpdateType]: Description
    CRACK,				          //!< update for crack informations
    SWITCHMULTISCALE2NONLINEAR    //!< move the fine scale model in a multiscale approach to the nonlinear part
};
}
}
#endif /* ELEMENTENUM_H_ */
