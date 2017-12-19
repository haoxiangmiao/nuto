#include "StructureMesh.h"

#include <mpi.h>

#include <boost/mpi.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

#include "json.hpp"
#include "base/Exception.h"

#include "mechanics/feti/StructureFeti.h"
#include "mechanics/structures/unstructured/Structure.h"

#include <Epetra_MpiComm.h>
#include <Epetra_CrsMatrix.h>
#include <Epetra_Map.h>
#include <Epetra_Export.h>
#include <Epetra_LinearProblem.h>

#include <AztecOO.h>
#include <Amesos.h>
#include <Amesos_BaseSolver.h>
#include <Amesos_Mumps.h>
#include <Amesos_ConfigDefs.h>
#include <Teuchos_ParameterList.hpp>
#include <BelosLinearProblem.hpp>
#include <BelosBlockGmresSolMgr.hpp>
#include <BelosPseudoBlockGmresSolMgr.hpp>
#include <BelosEpetraAdapter.hpp>
#include <Ifpack.h>
#include <Ifpack_AdditiveSchwarz.h>


//void StructureMesh::importMyMeshJson(std::string rFileName, const int interpolationTypeId)
void StructureMesh::importMyMeshJson(std::string rFileName)
{
    nlohmann::json root;

    std::ifstream file(rFileName.c_str(), std::ios::in);

    file >> root;

    //+++++++++++++++ Read Nodes +++++++++++++++
    // only supports nodes.size() == 1
    for (auto const& nodes : root["Nodes"])
    {
        mMyNodes.resize(nodes["Coordinates"].size());
        for (unsigned i = 0; i < mMyNodes.size(); ++i)
        {
            mMyNodes[i].nodeInfo.mCoordinates[0] = nodes["Coordinates"][i][0];
            mMyNodes[i].nodeInfo.mCoordinates[1] = nodes["Coordinates"][i][1];
            mMyNodes[i].nodeInfo.mCoordinates[2] = nodes["Coordinates"][i][2];
            mMyNodes[i].nodeInfo.mId = nodes["Indices"][i];

            mMyNodes[i].masterDomain = -1;
        }
    }


    //+++++++++++++++ Create Nodes +++++++++++++++
    for (const auto& node : mMyNodes)
        NodeCreate(node.nodeInfo.mId, node.nodeInfo.mCoordinates.head(GetDimension()));


    int elementType = 0;
    std::string elementTypeName = "";
    //+++++++++++++++ Read Elements +++++++++++++++
    // only supports elements.size() == 1
    for (auto const& elements : root["Elements"])
    {
        mElements.resize(elements["NodalConnectivity"].size());
        elementType = elements["Type"];
        elementTypeName = elements["TypeName"];


        for (unsigned i = 0; i < mElements.size(); ++i)
        {
            if (elementType == 1)
            {
                mSubdomainBoundaryNodeIds.insert(elements["NodalConnectivity"][i][0].get<int>());
                mSubdomainBoundaryNodeIds.insert(elements["NodalConnectivity"][i][1].get<int>());
            }
            else if (elementType == 2) // 3 node tri element
            {
                mElements[i].mNodeIds.resize(3);

                mElements[i].mNodeIds[0] = elements["NodalConnectivity"][i][0];
                mElements[i].mNodeIds[1] = elements["NodalConnectivity"][i][1];
                mElements[i].mNodeIds[2] = elements["NodalConnectivity"][i][2];
                mElements[i].mId = elements["Indices"][i];
            }
            else if (elementType == 3) // 4 node quad element
            {

                mElements[i].mNodeIds.resize(4);

                mElements[i].mNodeIds[0] = elements["NodalConnectivity"][i][0];
                mElements[i].mNodeIds[1] = elements["NodalConnectivity"][i][1];
                mElements[i].mNodeIds[2] = elements["NodalConnectivity"][i][2];
                mElements[i].mNodeIds[3] = elements["NodalConnectivity"][i][3];
                mElements[i].mId = elements["Indices"][i];
            }
            else if (elementType == 5) // 8 node hexahedron
            {
                const int numNodes = 8;
                mElements[i].mNodeIds.resize(numNodes);
                for (int iNode = 0; iNode < numNodes; ++iNode)
                    mElements[i].mNodeIds[iNode] = elements["NodalConnectivity"][i][iNode];

                mElements[i].mId = elements["Indices"][i];
            }
            else
            {
                throw NuTo::Exception(__PRETTY_FUNCTION__, "Import of element type not implemented. Element type id = " +
                                                             std::to_string(elementType));
            }
        }
    }


    std::vector<std::string> dofTypes;
    std::string currDofType = "";
    std::vector<std::string> interpolationOrders;
    std::string currInterpolationOrder = "";
    int b = 0;
    //+++++++++++++++ Read DOF Nodes +++++++++++++++
    for (auto const& dofNodes : root["DofNodes"])
    {
        currDofType = dofNodes["DofType"];
        currInterpolationOrder = dofNodes["InterpolationOrder"];
        dofTypes.push_back(currDofType);
        interpolationOrders.push_back(currInterpolationOrder);


        //+++++++++++++++ Read Interface +++++++++++++++
        for (auto const& interface : dofNodes["Interfaces"])
        {
            for (int j = 0; j < interface["NodeIDs"].size(); ++j)
            {
                for (int i = 0; i < mMyNodes.size(); ++i)
                {
                    if (mMyNodes[i].nodeInfo.mId == interface["NodeIDs"][j])
                    {
                        mMyNodes[i].masterDomain = interface["Master"];
                        break;
                    }
                }
            }
        }

        int interpolationTypeId = InterpolationTypeCreate(elementTypeName);
        InterpolationTypeAdd(interpolationTypeId, NuTo::Node::eDof::COORDINATES, NuTo::Interpolation::eTypeOrder::EQUIDISTANT1);
        InterpolationTypeAdd(interpolationTypeId, currDofType, currInterpolationOrder);

        for (const auto& element : mElements)
            ElementCreate(element.mId, interpolationTypeId, element.mNodeIds);


        ++b;
        if (b == 1)
            break;
    }




    file.close();


//    //+++++++++++++++ Create Interpolations +++++++++++++++
//    const int interpolationTypeId = InterpolationTypeCreate(elementTypeName);
//    InterpolationTypeAdd(interpolationTypeId, NuTo::Node::eDof::COORDINATES, NuTo::Interpolation::eTypeOrder::EQUIDISTANT1);
//    for (int i = 0 ; i < dofTypes.size(); ++i)
//    {
//        InterpolationTypeAdd(interpolationTypeId, dofTypes[i], interpolationOrders[i]);
//    }



//    //+++++++++++++++ Create Elements +++++++++++++++
//    for (const auto& element : mElements)
//        ElementCreate(element.mId, interpolationTypeId, element.mNodeIds);

    //+++++++++++++++ Prescribe InterpolationType +++++++++++++++
    ElementTotalConvertToInterpolationType();

    NodeBuildGlobalDofs();
}


std::vector<std::vector<int>> StructureMesh::map2Vector(std::map<int, std::vector<int>> rMap)
{
//        int maxKey = rMap.end()->first;
    int maxKey = -1;

    for (auto element : rMap)
    {
        if (maxKey < element.first)
            maxKey = element.first;
    }


    std::vector<std::vector<int>> resultVector(maxKey+1);

    for (auto element : rMap)
    {
        resultVector[element.first] = element.second;
    }

    return resultVector;
}

std::vector<std::vector<int>> StructureMesh::map2Vector(std::map<int, Eigen::VectorXi> rMap)
{
    int maxKey = -1;

    for (auto element : rMap)
    {
        if (maxKey < element.first)
            maxKey = element.first;
    }

    std::vector<std::vector<int>> resultVector(maxKey+1);

    for (auto element : rMap)
    {
        for (int i = 0; i < element.second.size(); ++i)
        {
            resultVector[element.first].push_back(element.second[i]);
        }
    }

    return resultVector;
}


std::vector<int> StructureMesh::map2Vector(std::map<int, int> rMap)
{
    int maxKey = -1;

    for (auto element : rMap)
    {
        if (maxKey < element.first)
            maxKey = element.first;
    }

    std::vector<int> resultVector(maxKey+1);

    for (auto element : rMap)
    {
        resultVector[element.first] = element.second;
    }

    return resultVector;
}

std::vector<int> StructureMesh::map2ValueVector(std::map<int, int> rMap)
{
    int n = rMap.size();

    std::vector<int> resultVector(n);
    int i = 0;
    for (auto element : rMap)
    {
        resultVector[i] = element.second;
        ++i;
    }

    return resultVector;
}


void StructureMesh::visualizeSerializedParticularSolution(std::vector<double> rSolution, std::vector<int> rSolutionIDs, std::vector<std::vector<int>> rNode2Dof, std::string rFileName, int rNumProc)
{
    serializeData serialData = serializeSolutionParticular(rSolution, rSolutionIDs, rNode2Dof, rNumProc);

    std::ofstream file(rFileName);
    std::vector<int> serialNodeIDs = serialData.completeNodeIDs;
    std::vector<std::vector<double>> serialNodeCoords = serialData.completeNodeCoords;
    std::vector<std::vector<int>> serialElementNodeIDs = serialData.completeElementNodeIDs;
    std::vector<int> serialDisplacementIDs = serialData.completeDisplacementIDs;
    std::vector<double> serialDisplacements = serialData.completeDisplacements;
    std::vector<std::vector<int>> serialNode2Dofs = serialData.completeNode2Dofs;

//    for (int i = 0; i < serialDisplacementIDs.size(); ++i)
//    {
//        std::cout << serialDisplacementIDs.size() << " -> " << serialDisplacementIDs[i] << ": " << serialDisplacements[i] << std::endl;
//    }

    std::vector<std::vector<double>> sortedSerialNodeCoords = sortNodeCoords(serialNodeIDs, serialNodeCoords);
    std::vector<double> sortedSerialDisplacements = sortDisplacements(serialDisplacementIDs, serialDisplacements);

    if (!file.is_open())
    {
        std::cout << "ERROR: File could not be opened.";
        return;
    }

    file << R"(<VTKFile type="UnstructuredGrid" version="0.1" byte_order="LittleEndian">)";
    file << "\n\t<UnstructuredGrid>";
    file << "\n\t\t<Piece NumberOfPoints=\"" << serialNodeCoords.size() << "\" NumberOfCells=\"" << serialElementNodeIDs.size() << "\">";
    file << "\n\t\t\t<PointData>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Float32" Name="Displacements" NumberOfComponents="3" Format="ascii">)";

    int counter = 0;
    for (int i = 0; i < serialNodeCoords.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        for (int j = 0; j < 3; ++j)
        {
            if (j < mDimension)
            {
//                    file << serialDisplacements[counter] << " ";
//                    file << sortedSerialDisplacements[counter] << " ";
//                file << serialDisplacements[serialNode2Dofs[i][j]] << " ";
                file << serialDisplacements[serialNode2Dofs[serialNodeIDs[i]][j]] << " ";
                ++counter;
            }
            else
            {
                file << "0 ";
            }
        }
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t</PointData>";
//        file << "\n\t\t\t<CellData>";
//        file << "\n\t\t\t</CellData>";

    file << "\n\t\t\t<Points>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Float32" NumberOfComponents="3" Format="ascii">)";

    for (int i = 0; i < serialNodeCoords.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        for (int j = 0; j < 3; ++j)
        {
            if (j < mDimension)
            {
                file << sortedSerialNodeCoords[i][j] << " ";
            }
            else
            {
                file << "0 ";
            }
        }
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t</Points>";

    file << "\n\t\t\t<Cells>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Float32" Name="connectivity" Format="ascii">)";

    for (int i = 0; i < serialElementNodeIDs.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        for (int j = 0; j < serialElementNodeIDs[i].size(); ++j)
        {
            file << serialElementNodeIDs[i][j] << " ";
        }
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Int32" Name="offsets" Format="ascii">)";

    counter = 0;
    for (int i = 0; i < serialElementNodeIDs.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        counter += serialElementNodeIDs[i].size();
        file << counter;
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Int32" Name="types" Format="ascii">)";

    for (int i = 0; i < serialElementNodeIDs.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        file << 9;
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t</Cells>";
    file << "\n\t\t</Piece>";
    file << "\n\t</UnstructuredGrid>";
    file << "\n</VTKFile>\n";

    file.close();
}


void StructureMesh::visualizeSolution(std::vector<double> rSolution, std::string rFileName)
{
    std::ofstream file(rFileName);

    if (!file.is_open())
    {

    }

    file << R"(<VTKFile type="UnstructuredGrid" version="0.1" byte_order="LittleEndian">)";
    file << "\n\t<UnstructuredGrid>";
    file << "\n\t\t<Piece NumberOfPoints=\"" << mMyNodes.size() << "\" NumberOfCells=\"" << mElements.size() << "\">";
    file << "\n\t\t\t<PointData>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Float32" Name="Displacements" NumberOfComponents="3" Format="ascii">)";

    int counter = 0;
    for (int i = 0; i < mMyNodes.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        for (int j = 0; j < 3; ++j)
        {
            if (j < mDimension)
            {
                file << rSolution[counter] << " ";
                ++counter;
            }
            else
            {
                file << "0 ";
            }
        }
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t</PointData>";
//        file << "\n\t\t\t<CellData>";
//        file << "\n\t\t\t</CellData>";

    file << "\n\t\t\t<Points>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Float32" NumberOfComponents="3" Format="ascii">)";

    for (int i = 0; i < mMyNodes.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        for (int j = 0; j < 3; ++j)
        {
            if (j < mDimension)
            {
                file << mMyNodes[i].nodeInfo.mCoordinates[j] << " ";
            }
            else
            {
                file << "0 ";
            }
        }
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t</Points>";

    file << "\n\t\t\t<Cells>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Float32" Name="connectivity" Format="ascii">)";

    for (int i = 0; i < mElements.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        for (int j = 0; j < mElements[i].mNodeIds.size(); ++j)
        {
            file << mElements[i].mNodeIds[j] << " ";
        }
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Int32" Name="offsets" Format="ascii">)";

    counter = 0;
    for (int i = 0; i < mElements.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        counter += mElements[i].mNodeIds.size();
        file << counter;
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t\t" << R"(<DataArray type="Int32" Name="types" Format="ascii">)";

    for (int i = 0; i < mElements.size(); ++i)
    {
        file << "\n\t\t\t\t\t";
        file << 9;
    }

    file << "\n\t\t\t\t</DataArray>";
    file << "\n\t\t\t</Cells>";
    file << "\n\t\t</Piece>";
    file << "\n\t</UnstructuredGrid>";
    file << "\n</VTKFile>\n";

    file.close();
}



void StructureMesh::generateNodeToDofMapping()
{
    std::set<NuTo::Node::eDof> allDofTypes = DofTypesGet();

    for (auto const& dofType : allDofTypes)
    {
        for (int i = 0; i < mMyNodes.size(); ++i)
        {
            mMyNodes[i].dofIDs[dofType] = NodeGetDofIds(mMyNodes[i].nodeInfo.mId, dofType);
        }
    }
}


void StructureMesh::generateDofClassification()
{
    std::map<NuTo::Node::eDof, int> numberActiveDofs;
    std::map<NuTo::Node::eDof, int> numberDependentDofs;
    std::map<NuTo::Node::eDof, int> numberMasterActiveDofs;
    std::map<NuTo::Node::eDof, int> numberMasterDependentDofs;


    std::set<NuTo::Node::eDof> dofTypes = DofTypesGet();
    std::vector<int> dofIDs;
    for (int i = 0; i < mMyNodes.size(); ++i)
    {
        for (auto const& dofType : dofTypes)
        {
            dofIDs = mMyNodes[i].dofIDs[dofType];

            for (int j = 0; j < dofIDs.size(); ++j)
            {
                if (IsActiveDofId(dofIDs[j], dofType))
                {
                    mMyNodes[i].activeDofIDs[dofType].push_back(dofIDs[j]);
                    numberActiveDofs[dofType] += 1;
                }
                else
                {
                    mMyNodes[i].dependentDofIDs[dofType].push_back(dofIDs[j]);
                    numberDependentDofs[dofType] += 1;
                }
            }
        }

        if (mMyNodes[i].masterDomain == -1)  //is master node?
        {
            for (auto const& dofType : dofTypes)
            {
                dofIDs = mMyNodes[i].dofIDs[dofType];

                for (int j = 0; j < dofIDs.size(); ++j)
                {
                    if (IsActiveDofId(dofIDs[j], dofType))
                    {
                        numberMasterActiveDofs[dofType] += 1;
                    }
                    else
                    {
                        numberMasterDependentDofs[dofType] += 1;
                    }
                }
            }
        }
    }

    for (auto const& dofType : dofTypes)
    {
        mMyDofData[dofType].numberActiveDofs = numberActiveDofs[dofType];
        mMyDofData[dofType].numberDofs = numberActiveDofs[dofType] + numberDependentDofs[dofType];
        mMyDofData[dofType].numberMasterDofs = numberMasterActiveDofs[dofType] + numberMasterDependentDofs[dofType];
        mMyDofData[dofType].numberMasterActiveDofs = numberMasterActiveDofs[dofType];
        mMyDofData[dofType].numberMasterDependentDofs = numberMasterDependentDofs[dofType];
    }
}


void StructureMesh::gatherNodeToDofMapping_allProcesses(int rNumProc)
{
    boost::mpi::communicator world;
    std::vector<std::vector<dofNode>> allNodes(rNumProc);
    boost::mpi::all_gather<std::vector<dofNode>>(world, mMyNodes, allNodes);

    std::set<NuTo::Node::eDof> dofTypes = DofTypesGet();

    std::vector<std::map<NuTo::Node::eDof, std::map<int, int>>> local2GlobalMapping(rNumProc);
    std::vector<std::map<NuTo::Node::eDof, std::map<int, int>>> local2GlobalActiveMapping(rNumProc);
    std::vector<std::map<NuTo::Node::eDof, std::vector<int>>> masterGlobalDofs(rNumProc);
    std::vector<std::map<NuTo::Node::eDof, std::vector<int>>> masterGlobalActiveDofs(rNumProc);
    std::vector<std::map<NuTo::Node::eDof, std::vector<int>>> masterGlobalDependentDofs(rNumProc);

    std::map<NuTo::Node::eDof, int> counter;

    //+++++++++++++++ generate mapping for master nodes +++++++++++++++
    //+++++++++++++++ [depending on process, doftype, active/dependent]

    for (int i = 0; i < rNumProc; ++i)
    {
        for (int j = 0; j < allNodes[i].size(); ++j)
        {
            if (allNodes[i][j].masterDomain == -1)
            {
                for (auto const& dofType : dofTypes)
                {
                    //+++++++++++++++ active dofs +++++++++++++++
                    std::vector<int> actDofIDs = allNodes[i][j].activeDofIDs[dofType];
                    for (int k = 0; k < actDofIDs.size(); ++k)
                    {
                        local2GlobalMapping[i][dofType][actDofIDs[k]] = counter[dofType];
                        local2GlobalActiveMapping[i][dofType][actDofIDs[k]] = counter[dofType];
                        masterGlobalDofs[i][dofType].push_back(counter[dofType]);
                        masterGlobalActiveDofs[i][dofType].push_back(counter[dofType]);
                        counter[dofType] += 1;
                    }
                }
            }

        }
    }

    for (int i = 0; i < rNumProc; ++i)
    {
        for (int j = 0; j < allNodes[i].size(); ++j)
        {
            if (allNodes[i][j].masterDomain == -1)
            {
                for (auto const& dofType : dofTypes)
                {
                    //+++++++++++++++ dependent dofs +++++++++++++++
                    std::vector<int> depDofIDs = allNodes[i][j].dependentDofIDs[dofType];
                    for (int k = 0; k < depDofIDs.size(); ++k)
                    {
                        local2GlobalMapping[i][dofType][depDofIDs[k]] = counter[dofType];
                        masterGlobalDofs[i][dofType].push_back(counter[dofType]);
                        masterGlobalDependentDofs[i][dofType].push_back(counter[dofType]);
                        counter[dofType] += 1;
                    }
                }
            }
        }
    }


    //+++++++++++++++ generate mapping for slave nodes +++++++++++++++
    //+++++++++++++++ [depending on process, doftype, active/dependent]
    for (int i = 0; i < rNumProc; ++i)
    {
        for (int j = 0; j < allNodes[i].size(); ++j)
        {
            if (allNodes[i][j].masterDomain > -1)
            {
                //+++++++++++++++ find node in master domain ++++++++++++++++
                dofNode currNode = allNodes[i][j];
                dofNode foundNode;

                for (int l = 0; l < allNodes[currNode.masterDomain].size(); ++l)
                {
                    if (allNodes[currNode.masterDomain][l].nodeInfo.mId == currNode.nodeInfo.mId)
                    {
                        foundNode = allNodes[currNode.masterDomain][l];
                        break;
                    }
                }

                for (auto const& dofType : dofTypes)
                {
                    //+++++++++++++++ active dofs +++++++++++++++
                    std::vector<int> actDofIDs = currNode.activeDofIDs[dofType];
                    for (int k = 0; k < actDofIDs.size(); ++k)
                    {
                        local2GlobalMapping[i][dofType][actDofIDs[k]] = local2GlobalMapping[currNode.masterDomain][dofType][foundNode.activeDofIDs[dofType][k]];
                        local2GlobalActiveMapping[i][dofType][actDofIDs[k]] = local2GlobalMapping[currNode.masterDomain][dofType][foundNode.activeDofIDs[dofType][k]];
                    }

                    //+++++++++++++++ dependent dofs +++++++++++++++
                    std::vector<int> depDofIDs = currNode.dependentDofIDs[dofType];
                    for (int k = 0; k < depDofIDs.size(); ++k)
                    {
                        local2GlobalMapping[i][dofType][depDofIDs[k]] = local2GlobalMapping[currNode.masterDomain][dofType][foundNode.dependentDofIDs[dofType][k]];
                    }
                }
            }

        }
    }



    mLocalToGlobalDofMapping = local2GlobalMapping;
    mLocalToGlobalActiveDofMapping = local2GlobalActiveMapping;
    mMasterGlobalDofs = masterGlobalDofs;
    mMasterGlobalActiveDofs = masterGlobalActiveDofs;
    mMasterGlobalDependentDofs = masterGlobalDependentDofs;
}


void StructureMesh::gatherNodeToMasterDofMapping(int rNumProc, int rRank)
{
    boost::mpi::communicator world;
    std::vector<std::map<NuTo::Node::eDof, dofData>> allDofsData(rNumProc);
    boost::mpi::all_gather<std::map<NuTo::Node::eDof, dofData>>(world, mMyDofData, allDofsData);

    std::set<NuTo::Node::eDof> dofTypes = DofTypesGet();

    std::map<NuTo::Node::eDof, std::map<int, int>> myLocal2GlobalMapping;
    std::map<NuTo::Node::eDof, std::map<int, int>> myLocal2GlobalActiveMapping;
    std::map<NuTo::Node::eDof, std::vector<int>> myMasterGlobalDofs;
    std::map<NuTo::Node::eDof, std::vector<int>> myMasterGlobalActiveDofs;
    std::map<NuTo::Node::eDof, std::vector<int>> myMasterGlobalDependentDofs;

    //+++++++++++++++ generate mapping for master nodes +++++++++++++++
    //+++++++++++++++ [depending on process, doftype, active/dependent]
    std::map<NuTo::Node::eDof, int> numberMasterActiveDofs;
    std::map<NuTo::Node::eDof, int> numberMasterDependentDofs;
    std::map<NuTo::Node::eDof, int> numberMasterActiveDofs_complete;

    for (auto const& dofType : dofTypes)
    {
        for (int i = 0; i < rRank; ++i)
        {
            numberMasterActiveDofs[dofType] += allDofsData[i][dofType].numberMasterActiveDofs;
            numberMasterDependentDofs[dofType] += allDofsData[i][dofType].numberMasterDependentDofs;
        }
        numberMasterActiveDofs_complete[dofType] = numberMasterActiveDofs[dofType];

        for (int i = rRank; i < rNumProc; ++i)
        {
            numberMasterActiveDofs_complete[dofType] += allDofsData[i][dofType].numberMasterActiveDofs;
        }
    }


    for (int j = 0; j < mMyNodes.size(); ++j)
    {
        if (mMyNodes[j].masterDomain == -1)  //is master node?
        {
            for (auto const& dofType : dofTypes)
            {
                //+++++++++++++++ active dofs +++++++++++++++
                std::vector<int> actDofIDs = mMyNodes[j].activeDofIDs[dofType];
                for (int k = 0; k < actDofIDs.size(); ++k)
                {
                    myLocal2GlobalMapping[dofType][actDofIDs[k]] = numberMasterActiveDofs[dofType];
                    myLocal2GlobalActiveMapping[dofType][actDofIDs[k]] = numberMasterActiveDofs[dofType];
                    myMasterGlobalDofs[dofType].push_back(numberMasterActiveDofs[dofType]);
                    myMasterGlobalActiveDofs[dofType].push_back(numberMasterActiveDofs[dofType]);
                    numberMasterActiveDofs[dofType] += 1;
                }

                //+++++++++++++++ dependent dofs +++++++++++++++
                std::vector<int> depDofIDs = mMyNodes[j].dependentDofIDs[dofType];
                for (int k = 0; k < depDofIDs.size(); ++k)
                {
                    myLocal2GlobalMapping[dofType][depDofIDs[k]] = numberMasterActiveDofs_complete[dofType] + numberMasterDependentDofs[dofType];
                    myMasterGlobalDofs[dofType].push_back(numberMasterActiveDofs_complete[dofType] + numberMasterDependentDofs[dofType]);
                    myMasterGlobalDependentDofs[dofType].push_back(numberMasterActiveDofs_complete[dofType] + numberMasterDependentDofs[dofType]);
                    numberMasterDependentDofs[dofType] += 1;
                }
            }
        }
    }

    mMyLocalToGlobalDofMapping = myLocal2GlobalMapping;
    mMyLocalToGlobalActiveDofMapping = myLocal2GlobalActiveMapping;
    mMyMasterGlobalDofs = myMasterGlobalDofs;
    mMyMasterGlobalActiveDofs = myMasterGlobalActiveDofs;
    mMyMasterGlobalDependentDofs = myMasterGlobalDependentDofs;
}


void StructureMesh::gatherNodeToSlaveDofMapping(int rNumProc, int rRank)
{
    boost::mpi::communicator world;

    std::set<NuTo::Node::eDof> dofTypes = DofTypesGet();

    //+++++++++++++++ get information from master process ++++++++++++++++
    boost::mpi::request reqs[4];
    if (rRank < rNumProc-1)
    {
        reqs[0] = world.isend(rRank+1, 0, mMyNodes);
        reqs[1] = world.isend(rRank+1, 1, mMyLocalToGlobalDofMapping);
    }

    int source = 0;
    std::vector<int> knownSources;
    std::vector<dofNode> sourceNodes;
    std::map<NuTo::Node::eDof, std::map<int, int>> sourceLocalToGlobalDofMapping;
    if (rRank > 0)
    {
        for (int j = 0; j < mMyNodes.size(); ++j)
        {
            source = mMyNodes[j].masterDomain;
            if (source > -1)
            {
                if (find(knownSources.begin(), knownSources.end(), source) == knownSources.end())
                {
                    knownSources.push_back(source);
                    reqs[2] = world.irecv(source, 0, sourceNodes);
                    reqs[3] = world.irecv(source, 1, sourceLocalToGlobalDofMapping);
                }
            }
        }
        boost::mpi::wait_all(reqs, reqs+4);
    }


    //+++++++++++++++ generate mapping for slave nodes ++++++++++++++++
    //+++++++++++++++ [depending on process, doftype, active/dependent]
    for (int j = 0; j < mMyNodes.size(); ++j)
    {
        if (mMyNodes[j].masterDomain > -1)  //is slave node?
        {
            dofNode currNode = mMyNodes[j];
            dofNode foundNode;

            for (int l = 0; l < sourceNodes.size(); ++l)
            {
                if (sourceNodes[l].nodeInfo.mId == currNode.nodeInfo.mId)
                {
                    foundNode = sourceNodes[l];
                    break;
                }
            }

            for (auto const& dofType : dofTypes)
            {
                std::vector<int> actDofIDs = currNode.activeDofIDs[dofType];
                for (int k = 0; k < actDofIDs.size(); ++k)
                {
                    mMyLocalToGlobalDofMapping[dofType][actDofIDs[k]] = sourceLocalToGlobalDofMapping[dofType][foundNode.activeDofIDs[dofType][k]];
                    mMyLocalToGlobalActiveDofMapping[dofType][actDofIDs[k]] = sourceLocalToGlobalDofMapping[dofType][foundNode.activeDofIDs[dofType][k]];
                }

                std::vector<int> depDofIds = currNode.dependentDofIDs[dofType];
                for (int k = 0; k < depDofIds.size(); ++k)
                {
                    mMyLocalToGlobalDofMapping[dofType][depDofIds[k]] = sourceLocalToGlobalDofMapping[dofType][foundNode.dependentDofIDs[dofType][k]];
                }
            }
        }
    }
}


void StructureMesh::gatherNodeToDofMapping(int rNumProc, int rRank)
{
    gatherNodeToMasterDofMapping(rNumProc, rRank);
    gatherNodeToSlaveDofMapping(rNumProc, rRank);
}


Epetra_MultiVector StructureMesh::solveSystem(Epetra_CrsMatrix rA, Epetra_MultiVector rRhs, bool iterative, bool useAztecOO)
{
    Epetra_MultiVector lhs(rRhs);
    return solveSystem(rA, lhs, rRhs, iterative, useAztecOO);
}


Epetra_MultiVector StructureMesh::solveSystem(Epetra_CrsMatrix rA, Epetra_MultiVector rLhs, Epetra_MultiVector rRhs, bool iterative, bool useAztecOO)
{
    Epetra_LinearProblem problem(&rA, &rLhs, &rRhs);

    if (iterative)
    {
        if (useAztecOO)
        {
            /*METHODS
             * AZ_cg               0 --> preconditioned conjugate gradient method
             * AZ_gmres            1 --> preconditioned gmres method
             * AZ_cgs              2 --> preconditioned cg squared method
             * AZ_tfqmr            3 --> preconditioned transpose-free qmr method
             * AZ_bicgstab         4 --> preconditioned stabilized bi-cg method
             * AZ_fixed_pt         8 --> fixed point iteration
             * AZ_cg_condnum      11
             * AZ_gmres_condnum   12
             */
            int method = AZ_gmres;

            AztecOO Solver(problem);
            Solver.SetAztecOption(AZ_solver, method);
    //        Solver.SetAztecOption(AZ_output, AZ_all);
            Solver.SetAztecOption(AZ_diagnostics, AZ_all);
            Solver.SetAztecOption(AZ_precond, AZ_dom_decomp);
    //        Solver.SetAztecOption(AZ_precond, AZ_Jacobi);
            Solver.SetAztecOption(AZ_subdomain_solve, AZ_ilut);
            Solver.SetAztecOption(AZ_overlap, 1);
            Solver.SetAztecOption(AZ_orthog, AZ_classic);
            Solver.SetAztecOption(AZ_kspace, 50);
            Solver.Iterate(1000,1e-8);
        }
        else
        {
            //++++++++++++Ifpack preconditioner+++++++++++++
            Teuchos::ParameterList paramList;
            Ifpack Factory;

            // Create the preconditioner. For the list of PrecType values check the IFPACK documentation.
            string PrecType = "ILU"; // incomplete LU
            int OverlapLevel = 1; // must be >= 0. If Comm.NumProc() == 1,
                                // it is ignored.

            RCP<Epetra_CrsMatrix> A = rcp (new Epetra_CrsMatrix(rA));
            RCP<Ifpack_Preconditioner> prec = rcp(Factory.Create(PrecType, &*A, OverlapLevel));
            TEUCHOS_TEST_FOR_EXCEPTION(prec == null, std::runtime_error,
                     "IFPACK failed to create a preconditioner of type \""
                     << PrecType << "\" with overlap level "
                     << OverlapLevel << ".");

            // Specify parameters for ILU.  ILU is local to each MPI process.
            paramList.set("fact: drop tolerance", 1e-9);
            paramList.set("fact: level-of-fill", 1);

            // how to combine overlapping results:
            // "Add", "Zero", "Insert", "InsertAdd", "Average", "AbsMax"
            paramList.set("schwarz: combine mode", "Add");
//            IFPACK_CHK_ERR(prec->SetParameters(paramList));
            prec->SetParameters(paramList);

            // Initialize the preconditioner. At this point the matrix must have
            // been FillComplete()'d, but actual values are ignored.
//            IFPACK_CHK_ERR(prec->Initialize());
            prec->Initialize();

            // Build the preconditioner, by looking at the values of the matrix.
//            IFPACK_CHK_ERR(prec->Compute());
            prec->Compute();

            // Create the Belos preconditioned operator from the Ifpack preconditioner.
            // NOTE:  This is necessary because Belos expects an operator to apply the
            //        preconditioner with Apply() NOT ApplyInverse().
            RCP<Belos::EpetraPrecOp> belosPrec = rcp (new Belos::EpetraPrecOp(prec));

            //++++++++++++end preconditioner definition+++++++++++++

            //++++++++++++perform solve+++++++++++++++++++++

            RCP<Epetra_MultiVector> LHS = rcp (new Epetra_MultiVector (rLhs));
            RCP<Epetra_MultiVector> RHS = rcp (new Epetra_MultiVector (rRhs));

            // Need a Belos::LinearProblem to define a Belos solver
            typedef Epetra_MultiVector                MV;
            typedef Epetra_Operator                   OP;
            RCP<Belos::LinearProblem<double,MV,OP> > belosProblem
            = rcp (new Belos::LinearProblem<double,MV,OP>(A, LHS, RHS));

            belosProblem->setRightPrec (belosPrec);

            bool set = belosProblem->setProblem();
            TEUCHOS_TEST_FOR_EXCEPTION( ! set,
                      std::runtime_error,
                      "*** Belos::LinearProblem failed to set up correctly! ***");

            // Create a parameter list to define the Belos solver.
            RCP<ParameterList> belosList = rcp (new ParameterList ());
            belosList->set ("Block Size", 1);              // Blocksize to be used by iterative solver
            belosList->set ("Num Blocks", 30);              //Krylov dimension
            belosList->set ("Maximum Restarts", 20);
            belosList->set ("Maximum Iterations", 1000);   // Maximum number of iterations allowed
            belosList->set ("Convergence Tolerance", 1e-8);// Relative convergence tolerance requested
            belosList->set ("Verbosity", Belos::Errors+Belos::Warnings+Belos::TimingDetails+Belos::FinalSummary );

            // Create an iterative solver manager.
            Belos::PseudoBlockGmresSolMgr<double, MV, OP> belosSolver(belosProblem, belosList);

            // Perform solve.
            //Belos::ReturnType ret = belosSolver.solve();  //for error handling
            belosSolver.solve();
            return *belosSolver.getProblem().getLHS().get();

            //++++++++++++end of solve++++++++++++++++++++

        }
    }
    else
    {
        /*METHODS
         * Klu
         * Mumps
         * Lapack
         * Scalapack
         * Umfpack
         * Superlu
         * Superludist
         * Dscpack
         * Taucs
         */
        Amesos Factory;
        std::string solverType = "Mumps";
        bool solverAvail = Factory.Query(solverType);
        if (solverAvail)
        {
            Teuchos::ParameterList params;
            params.set("PrintStatus", true);
            params.set("PrintTiming", true);
            params.set("MaxProcs", -3); //all processes in communicator will be used
            Amesos_Mumps* solver;
            solver = new Amesos_Mumps(problem);
            Teuchos::ParameterList mumpsList = params.sublist("mumps");
            int* icntl = new int[40];
            double* cntl = new double[5];
            icntl[0] = 0;
            icntl[1] = 0;
            icntl[2] = 0;
            icntl[3] = 0;
            icntl[5] = 7;
            icntl[6] = 7;
            icntl[7] = 0;
            icntl[28] = 2;
            icntl[29] = 0;
            mumpsList.set("ICNTL", icntl);
            solver->SetParameters(params);
            solver->Solve();

            delete[] icntl;
            delete[] cntl;
            delete solver;
        }
        else
        {
            Epetra_MultiVector zeroVec(*(problem.GetLHS()));
            zeroVec.PutScalar(0.);
            problem.SetLHS(&zeroVec);
        }
    }

    Epetra_MultiVector* lhs = problem.GetLHS();
    return *lhs;
}


std::map<int, Eigen::VectorXi> StructureMesh::getNodeActiveDOFs(NuTo::Node::eDof rDofType, int rRank)
{
    std::map<int, Eigen::VectorXi> activeDofs;

    for (dofNode currNode : mMyNodes)
    {
        Eigen::VectorXi currDofs(currNode.activeDofIDs[rDofType].size());
        for (int i = 0; i < currNode.activeDofIDs[rDofType].size(); ++i)
        {
            currDofs(i) = mLocalToGlobalActiveDofMapping[rRank][rDofType][currNode.activeDofIDs[rDofType][i]];
        }
        activeDofs[currNode.nodeInfo.mId] = currDofs;
    }

    return activeDofs;
}

std::map<int, Eigen::VectorXi> StructureMesh::getNodeDOFs(NuTo::Node::eDof rDofType, int rRank)
{
    std::map<int, Eigen::VectorXi> allDofs;

    for (dofNode currNode : mMyNodes)
    {
        Eigen::VectorXi currDofs(currNode.dofIDs[rDofType].size());
        for (int i = 0; i < currNode.dofIDs[rDofType].size(); ++i)
        {
            currDofs(i) = mLocalToGlobalDofMapping[rRank][rDofType][currNode.dofIDs[rDofType][i]];
        }
        allDofs[currNode.nodeInfo.mId] = currDofs;
    }

    return allDofs;
}

std::map<int, Eigen::VectorXi> StructureMesh::getMyNodeDOFs(NuTo::Node::eDof rDofType)
{
    std::map<int, Eigen::VectorXi> allDofs;

    for (dofNode currNode : mMyNodes)
    {
        Eigen::VectorXi currDofs(currNode.dofIDs[rDofType].size());
        for (int i = 0; i < currNode.dofIDs[rDofType].size(); ++i)
        {
            currDofs(i) = mMyLocalToGlobalDofMapping[rDofType][currNode.dofIDs[rDofType][i]];
        }
        allDofs[currNode.nodeInfo.mId] = currDofs;
    }

    return allDofs;
}

std::vector<int> StructureMesh::getAllNodeIDs()
{
    std::vector<int> ids(mMyNodes.size());
    for (int i = 0; i < mMyNodes.size(); ++i)
    {
        ids[i] = mMyNodes[i].nodeInfo.mId;
    }
    return ids;
}


std::vector<std::vector<double>> StructureMesh::sortNodeCoords(std::vector<int> rNodeIDs, std::vector<std::vector<double>> rNodeCoords)
{
    int n = rNodeIDs.size();

    bool swapped = false;
    int tmpID = 0;
    std::vector<double> tmpCoords(rNodeCoords[0].size());
    do
    {
        swapped = false;
        for (int i = 0; i < n-1; ++i)
        {
            if (rNodeIDs[i] > rNodeIDs[i+1])
            {
                tmpID = rNodeIDs[i];
                rNodeIDs[i] = rNodeIDs[i+1];
                rNodeIDs[i+1] = tmpID;

                tmpCoords = rNodeCoords[i];
                rNodeCoords[i] = rNodeCoords[i+1];
                rNodeCoords[i+1] = tmpCoords;

                swapped = true;
            }
        }
        --n;
    } while(swapped);


    return rNodeCoords;
}


std::vector<double> StructureMesh::sortDisplacements(std::vector<int> rDisplacementIDs, std::vector<double> rDisplacements)
{
    int n = rDisplacementIDs.size();
    bool swapped = false;
    int tmpID = 0;
    double tmpDispl = 0.0;
    do
    {
        swapped = false;
        for (int i = 0; i < n-1; ++i)
        {
            if (rDisplacementIDs[i] > rDisplacementIDs[i+1])
            {
                tmpID = rDisplacementIDs[i];
                rDisplacementIDs[i] = rDisplacementIDs[i+1];
                rDisplacementIDs[i+1] = tmpID;

                tmpDispl = rDisplacements[i];
                rDisplacements[i] = rDisplacements[i+1];
                rDisplacements[i+1] = tmpDispl;

                swapped = true;
            }
        }
        --n;
    } while(swapped);

    return rDisplacements;
}


StructureMesh::serializeData StructureMesh::serializeSolutionParticular(std::vector<double> rSolution, std::vector<int> rSolutionIDs, std::vector<std::vector<int>> rNode2Dofs, int rNumProc)
{
    boost::mpi::communicator world;

    std::vector<int> nodeIDs(mMyNodes.size());
    std::vector<std::vector<double>> nodeCoords(mMyNodes.size());
    std::vector<std::vector<int>> elementNodeIDs(mElements.size());

    for (int i = 0; i < mMyNodes.size(); ++i)
    {
        nodeIDs[i] = mMyNodes[i].nodeInfo.mId;

        for (int j = 0; j < 3; ++j)
        {
            nodeCoords[i].push_back(mMyNodes[i].nodeInfo.mCoordinates[j]);
        }
    }

    for (int i = 0; i < mElements.size(); ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            elementNodeIDs[i].push_back(mElements[i].mNodeIds[j]);
        }
    }

    serializeData myData;
    myData.completeNodeIDs = nodeIDs;
    myData.completeNodeCoords = nodeCoords;
    myData.completeElementNodeIDs = elementNodeIDs;
    myData.completeDisplacementIDs = rSolutionIDs;
    myData.completeDisplacements = rSolution;
    myData.completeNode2Dofs = rNode2Dofs;

    std::vector<serializeData> allData(rNumProc);
    boost::mpi::all_gather(world, myData, allData);

    serializeData resultData;

    for (int i = 0; i < rNumProc; ++i)
    {
        int currentIndex = 0;
        for (int j = 0; j < allData[i].completeNodeIDs.size(); ++j)
        {
            currentIndex = allData[i].completeNodeIDs[j];
            bool indexFound = false;
            for (int k = 0; k < i; ++k)
            {
                for (int l = 0; l < allData[k].completeNodeIDs.size(); ++l)
                {
                    if (currentIndex == allData[k].completeNodeIDs[l])
                    {
                        indexFound = true;
                        break;
                    }
                }
                if (indexFound)
                    break;
            }

            if (!indexFound)
            {
                resultData.completeNodeIDs.push_back(allData[i].completeNodeIDs[j]);
                std::vector<double> currentCoords(allData[i].completeNodeCoords[j].size());
                for (int m = 0; m < allData[i].completeNodeCoords[j].size(); ++m)
                {
                    currentCoords[m] = allData[i].completeNodeCoords[j][m];
                }
                resultData.completeNodeCoords.push_back(currentCoords);
            }
        }

        for (int j = 0; j < allData[i].completeElementNodeIDs.size(); ++j)
        {
            std::vector<int> currentIDs(allData[i].completeElementNodeIDs[j].size());
            for (int k = 0; k < allData[i].completeElementNodeIDs[j].size(); ++k)
            {
                currentIDs[k] = allData[i].completeElementNodeIDs[j][k];
            }
            resultData.completeElementNodeIDs.push_back(currentIDs);
        }

        for (int j = 0; j < allData[i].completeDisplacementIDs.size(); ++j)
        {
            resultData.completeDisplacementIDs.push_back(allData[i].completeDisplacementIDs[j]);
        }
    }

    resultData.completeNode2Dofs.resize(resultData.completeNodeCoords.size());
    resultData.completeDisplacements.resize(resultData.completeDisplacementIDs.size());
    for (int i = 0; i < rNumProc; ++i)
    {
        for (int j = 0; j < allData[i].completeNodeIDs.size(); ++j)
        {
            std::vector<int> currentDofs(3);
            for (int k = 0; k < allData[i].completeNode2Dofs[allData[i].completeNodeIDs[j]].size(); ++k)
            {
                currentDofs[k] = allData[i].completeNode2Dofs[allData[i].completeNodeIDs[j]][k];
            }
            resultData.completeNode2Dofs[allData[i].completeNodeIDs[j]] = currentDofs;
        }

        for (int j = 0; j < allData[i].completeDisplacements.size(); ++j)
        {
            resultData.completeDisplacements[allData[i].completeDisplacementIDs[j]] = allData[i].completeDisplacements[j];
        }
    }

    return resultData;
}
