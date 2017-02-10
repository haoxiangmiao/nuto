#include "mechanics/MechanicsEnums.h"
#include "mechanics/mesh/MeshGenerator.h"
#include <assert.h>

#include "mechanics/structures/unstructured/Structure.h"

using namespace NuTo::Interpolation;

std::pair<int, int> CreateInterpolationTypeAndGroup(NuTo::Structure& rS, eShapeType rElementShape)
{
    int interpolationType = rS.InterpolationTypeCreate(rElementShape);
    rS.InterpolationTypeAdd(interpolationType, NuTo::Node::eDof::COORDINATES, eTypeOrder::EQUIDISTANT1);
    int elementGroup = rS.GroupCreate(NuTo::eGroupId::Elements);
    return std::make_pair(elementGroup, interpolationType);
}

std::vector<std::vector<int>> GetElementNodeIds3D(const std::vector<int>& rCornerNodes, eShapeType rElementShape)
{
    switch (rElementShape)
    {
    case eShapeType::BRICK3D:
    {
        return {rCornerNodes};
    }
    case eShapeType::TETRAHEDRON3D:
    {
        std::vector<int> nodesTet0({rCornerNodes[0], rCornerNodes[1], rCornerNodes[3], rCornerNodes[7]});
        std::vector<int> nodesTet1({rCornerNodes[0], rCornerNodes[1], rCornerNodes[7], rCornerNodes[4]});
        std::vector<int> nodesTet2({rCornerNodes[5], rCornerNodes[4], rCornerNodes[7], rCornerNodes[1]});
        std::vector<int> nodesTet3({rCornerNodes[6], rCornerNodes[5], rCornerNodes[7], rCornerNodes[1]});
        std::vector<int> nodesTet4({rCornerNodes[2], rCornerNodes[7], rCornerNodes[1], rCornerNodes[6]});
        std::vector<int> nodesTet5({rCornerNodes[2], rCornerNodes[3], rCornerNodes[1], rCornerNodes[7]});
        return {nodesTet0, nodesTet1, nodesTet2, nodesTet3, nodesTet4, nodesTet5};
    }
    case eShapeType::PRISM3D:
    {
        std::vector<int> nodes0({rCornerNodes[0], rCornerNodes[1], rCornerNodes[2],
                                 rCornerNodes[4], rCornerNodes[5], rCornerNodes[6]});
        std::vector<int> nodes1({rCornerNodes[0], rCornerNodes[2], rCornerNodes[3],
                                 rCornerNodes[4], rCornerNodes[6], rCornerNodes[7]});
        return {nodes0, nodes1};
    }
    default:
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, ShapeTypeToString(rElementShape) + " not supported as 3D element");
    }
}

std::vector<std::vector<int>> GetElementNodeIds2D(const std::vector<int>& rCornerNodes, eShapeType rElementShape)
{
    switch (rElementShape)
    {
    case eShapeType::QUAD2D:
    {
        return {rCornerNodes};
    }
    case eShapeType::TRIANGLE2D:
    {
        std::vector<int> e1{rCornerNodes[0], rCornerNodes[1], rCornerNodes[2]};
        std::vector<int> e2{rCornerNodes[0], rCornerNodes[2], rCornerNodes[3]};
        return {e1, e2};
    }
    default:
        throw NuTo::MechanicsException(__PRETTY_FUNCTION__, ShapeTypeToString(rElementShape) + " not supported as 3D element");
    }
}

std::vector<int> CreateNodes(NuTo::Structure& rS, std::vector<int> rNumNodes, std::vector<double> rDelta)
{
    std::vector<int> nodeIds;
    switch (rS.GetDimension())
    {
    case 1:
    {
        nodeIds.reserve(rNumNodes[0]);
        for (int iX = 0; iX < rNumNodes[0]; ++iX)
            nodeIds.push_back(rS.NodeCreate(Eigen::Matrix<double, 1, 1>::Constant(iX * rDelta[0])));
        break;
    }
    case 2:
    {
        nodeIds.reserve(rNumNodes[0] * rNumNodes[1]);
        for (int iY = 0; iY < rNumNodes[1]; ++iY)
            for (int iX = 0; iX < rNumNodes[0]; ++iX)
                nodeIds.push_back(rS.NodeCreate(Eigen::Vector2d(iX * rDelta[0], iY * rDelta[1])));
        break;
    }
    case 3:
    {
        nodeIds.reserve(rNumNodes[0] * rNumNodes[1] * rNumNodes[2]);
        for (int iZ = 0; iZ < rNumNodes[2]; ++iZ)
            for (int iY = 0; iY < rNumNodes[1]; ++iY)
                for (int iX = 0; iX < rNumNodes[0]; ++iX)
                    nodeIds.push_back(rS.NodeCreate(Eigen::Vector3d(iX * rDelta[0], iY * rDelta[1], iZ * rDelta[2])));
        break;
    }
    default: throw;
    }
    return nodeIds;
}

std::pair<int, int> CreateElements(NuTo::Structure& rS,
                                   std::vector<int> rNodeIds,
                                   std::vector<int> rNumDivisions,
                                   NuTo::Interpolation::eShapeType rElementShape)
{
    auto info = CreateInterpolationTypeAndGroup(rS, rElementShape);
    switch (rS.GetDimension())
    {
    case 1:
    {
        for (int iX = 0; iX < rNumDivisions[0]; ++iX)
        {
            std::vector<int> cornerNodes(2);
            cornerNodes[0] = rNodeIds[iX  ];
            cornerNodes[1] = rNodeIds[iX+1];
            rS.GroupAddElement(info.first, rS.ElementCreate(info.second, cornerNodes));
        }
        break;
    }
    case 2:
    {
        for (int iY = 0; iY < rNumDivisions[1]; ++iY)
            for (int iX = 0; iX < rNumDivisions[0]; ++iX)
            {
                std::vector<int> cornerNodes(4);

                cornerNodes[0] = rNodeIds[iX   +  iY    * (rNumDivisions[0] + 1)];
                cornerNodes[1] = rNodeIds[iX+1 +  iY    * (rNumDivisions[0] + 1)];
                cornerNodes[2] = rNodeIds[iX+1 + (iY+1) * (rNumDivisions[0] + 1)];
                cornerNodes[3] = rNodeIds[iX   + (iY+1) * (rNumDivisions[0] + 1)];

                auto elementNodes = GetElementNodeIds2D(cornerNodes, rElementShape);
                for (auto& nodes : elementNodes)
                    rS.GroupAddElement(info.first, rS.ElementCreate(info.second, nodes));
            }
        break;
    }
    case 3:
    {
        for (int iZ = 0; iZ < rNumDivisions[2]; ++iZ)
            for (int iY = 0; iY < rNumDivisions[1]; ++iY)
                for (int iX = 0; iX < rNumDivisions[0]; ++iX)
                {
                    std::vector<int> cornerNodes(8);
                    int numX = rNumDivisions[0] + 1;
                    int numY = rNumDivisions[1] + 1;

                    cornerNodes[0] = rNodeIds[iX   +  iY    * numX +  iZ    * numX * numY];
                    cornerNodes[1] = rNodeIds[iX+1 +  iY    * numX +  iZ    * numX * numY];
                    cornerNodes[2] = rNodeIds[iX+1 + (iY+1) * numX +  iZ    * numX * numY];
                    cornerNodes[3] = rNodeIds[iX   + (iY+1) * numX +  iZ    * numX * numY];
                    cornerNodes[4] = rNodeIds[iX   +  iY    * numX + (iZ+1) * numX * numY];
                    cornerNodes[5] = rNodeIds[iX+1 +  iY    * numX + (iZ+1) * numX * numY];
                    cornerNodes[6] = rNodeIds[iX+1 + (iY+1) * numX + (iZ+1) * numX * numY];
                    cornerNodes[7] = rNodeIds[iX   + (iY+1) * numX + (iZ+1) * numX * numY];

                    auto elementNodes = GetElementNodeIds3D(cornerNodes, rElementShape);
                    for (auto& nodes : elementNodes)
                        rS.GroupAddElement(info.first, rS.ElementCreate(info.second, nodes));
                }
        break;
    }
    default: throw;
    }
    return info;
}



std::pair<int, int> NuTo::MeshGenerator::Grid(Structure& rS,
                                              std::vector<double> rEnd,
                                              std::vector<int> rNumDivisions,
                                              Interpolation::eShapeType rElementShape)
{
    assert(rS.GetDimension() == static_cast<int>(rEnd.size()));
    assert(rS.GetDimension() == static_cast<int>(rNumDivisions.size()));
    std::vector<int> numNodes(rEnd.size());
    std::vector<double> delta(rEnd.size());

    for (int i = 0; i < rS.GetDimension(); ++i)
    {
        numNodes[i] = rNumDivisions[i] + 1;
        delta[i] = rEnd[i] / rNumDivisions[i];
    }

    std::vector<int> nodeIds = CreateNodes(rS, numNodes, delta);
    return CreateElements(rS, nodeIds, rNumDivisions, rElementShape);
}

std::pair<int, int> NuTo::MeshGenerator::Grid(Structure& rS,
                                              std::vector<double> rEnd,
                                              std::vector<int> rNumDivisions)
{
    switch (rS.GetDimension())
    {
    case 1: return MeshGenerator::Grid(rS, rEnd, rNumDivisions, eShapeType::TRUSS1D);
    case 2: return MeshGenerator::Grid(rS, rEnd, rNumDivisions, eShapeType::QUAD2D);
    case 3: return MeshGenerator::Grid(rS, rEnd, rNumDivisions, eShapeType::BRICK3D);
    default: throw;
    }
}


std::function<Eigen::Vector3d(Eigen::Vector3d)> NuTo::MeshGenerator::GetCylinderMapping(
    double rRadius,
    double rHeight)
{
    return [&rRadius,rHeight](Eigen::Vector3d v) -> Eigen::VectorXd
                {
                    v.x() = v.x() * 2 -1;
                    v.y() = v.y() * 2 -1;
                    v.z() = v.z() * 2 -1;
                    v.x() *= 1. + (1. - std::abs(v.x())) / 2.;
                    v.y() *= 1. + (1. - std::abs(v.y())) / 2.;
                    v.z() *= 1. + (1. - std::abs(v.z())) / 2.;
                    Eigen::VectorXd CoordVec(3);
                    CoordVec << v.x() * sqrt(1 - (v.y() * v.y()) / 2.0 ) * rRadius / 2.0,
                                v.y() * sqrt(1 - (v.x() * v.x()) / 2.0 ) * rRadius / 2.0,
                                v.z() * rHeight / 2.0;
                    return CoordVec;
                };
}
