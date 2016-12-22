// $Id$

#include <boost/assign/ptr_map_inserter.hpp>

#include "base/Timer.h"

#include "math/FullMatrix.h"
#include "mechanics/elements/ElementOutputFullMatrixDouble.h"
#include "mechanics/elements/ElementOutputFullVectorDouble.h"
#include "mechanics/elements/ElementOutputBlockVectorDouble.h"
#include "mechanics/elements/ElementOutputBlockVectorInt.h"
#include "mechanics/elements/ElementOutputVectorInt.h"

#include "mechanics/structures/StructureBase.h"
#include "mechanics/structures/StructureOutputBlockVector.h"
#include "mechanics/elements/ElementBase.h"
#include "mechanics/elements/ElementEnum.h"
#include "mechanics/nodes/NodeBase.h"
#include "mechanics/nodes/NodeEnum.h"
#include "mechanics/groups/Group.h"
#include "mechanics/groups/GroupEnum.h"

//! @brief sets the displacements of a node
//! @param rIdent node identifier
//! @param rDisplacements matrix (one column) with the displacements
void NuTo::StructureBase::NodeSetDisplacements(int rNode, const FullVector<double,Eigen::Dynamic>& rDisplacements)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());
    NodeBase* nodePtr=NodeGetNodePtr(rNode);
    this->mUpdateTmpStaticDataRequired=true;

    if (rDisplacements.GetNumColumns()!=1)
        throw MechanicsException("[NuTo::StructureBase::NodeSetDisplacements] Displacement matrix has to have a single column.");



    try
    {
        if (rDisplacements.GetNumRows() <= 0 or rDisplacements.GetNumRows() > 3)
            throw MechanicsException("[NuTo::StructureBase::NodeSetDisplacements] The number of displacement components is either 1, 2 or 3.");

        nodePtr->Set(Node::eDof::DISPLACEMENTS, rDisplacements);

    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeSetDisplacements] Error setting displacements.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeSetDisplacements] Error setting displacements of node (unspecified exception).");
    }
}


NuTo::StructureOutputBlockVector NuTo::StructureBase::NodeExtractDofValues() const
{
    return NodeExtractDofValues(0);
}


void NuTo::StructureBase::NodeMergeDofValues(int rTimeDerivative, const NuTo::StructureOutputBlockVector &rDofValues)
{
    NodeMergeDofValues(rTimeDerivative, rDofValues.J, rDofValues.K);
}

//! @brief sets the displacements of a node
//! @param rIdent node identifier
//! @param rTimeDerivative time derivative (0 disp, 1 vel, 2 acc)
//! @param rDisplacements matrix (one column) with the displacements
void NuTo::StructureBase::NodeSetDisplacements(int rNode, int rTimeDerivative, const FullVector<double,Eigen::Dynamic>& rDisplacements)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());
    NodeBase* nodePtr=NodeGetNodePtr(rNode);
    this->mUpdateTmpStaticDataRequired=true;

    if (rDisplacements.GetNumColumns()!=1)
        throw MechanicsException("[NuTo::StructureBase::NodeSetDisplacements] Displacement matrix has to have a single column.");
    if (nodePtr->GetNumTimeDerivatives(Node::eDof::DISPLACEMENTS)<rTimeDerivative)
        throw MechanicsException("[NuTo::StructureBase::NodeSetDisplacements] number of time derivatives stored at node is less than the required value.");
    try
    {
        if (rDisplacements.GetNumRows() <= 0 or rDisplacements.GetNumRows() > 3)
            throw MechanicsException("[NuTo::StructureBase::NodeSetDisplacements] The number of displacement components is either 1, 2 or 3.");

        nodePtr->Set(Node::eDof::DISPLACEMENTS, rTimeDerivative, rDisplacements);
    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeSetDisplacements] Error setting displacements.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeSetDisplacements] Error setting displacements of node (unspecified exception).");
    }
}

//! @brief sets the rotations of a node
//! @param rIdent node identifier
//! @param rRotations matrix (one column) with the rotations
void NuTo::StructureBase::NodeSetRotations(int rNode, const FullVector<double,Eigen::Dynamic>& rRotations)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());
    NodeBase* nodePtr=NodeGetNodePtr(rNode);
    this->mUpdateTmpStaticDataRequired=true;

    if (rRotations.GetNumColumns()!=1)
        throw MechanicsException("[NuTo::StructureBase::NodeSetRotations] rotation matrix has to have a single column.");
    try
    {
        if (rRotations.GetNumRows() != 1 and rRotations.GetNumRows() != 3)
            throw MechanicsException("[NuTo::StructureBase::NodeSetRotations] The number of rotation components is either 1, 3.");

        nodePtr->Set(Node::eDof::ROTATIONS, rRotations);
    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeSetRotations] Error setting rotations.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeSetRotations] Error setting rotations of node (unspecified exception).");
    }
}

//! @brief sets the displacements of a group of nodes
//! @param rIdent node group identifier
//! @param rDisplacements matrix (one column) with the displacements
void NuTo::StructureBase::NodeGroupSetDisplacements(int rGroupIdent, const FullVector<double,Eigen::Dynamic>& rDisplacements)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());
    this->mUpdateTmpStaticDataRequired=true;
    if (rDisplacements.GetNumColumns()!=1)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] Displacement matrix has to have a single column.");

    boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rGroupIdent);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::eGroupId::Nodes)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] Group is not a node group.");
    Group<NodeBase> *nodeGroup = dynamic_cast<Group<NodeBase>*>(itGroup->second);
    assert(nodeGroup!=0);

    for (Group<NodeBase>::iterator itNode=nodeGroup->begin(); itNode!=nodeGroup->end();itNode++)
    {
        try
        {
            if (rDisplacements.GetNumRows() <= 0 or rDisplacements.GetNumRows() > 3)
                throw MechanicsException("[NuTo::StructureBase::NodeSetDisplacements] The number of displacement components is either 1, 2 or 3.");

            itNode->second->Set(Node::eDof::DISPLACEMENTS, rDisplacements);
        }
        catch(NuTo::MechanicsException & b)
        {
            b.AddMessage("[NuTo::StructureBase::NodeGroupSetDisplacements] Error setting displacements.");
            throw;
        }
        catch(...)
        {
            throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] Error setting displacements of node (unspecified exception).");
        }
    }
}

//! @brief sets the displacements of a group of nodes
//! @param rIdent node group identifier
//! @param rTimeDerivative time derivative (0 disp, 1 vel, 2 acc)
//! @param rDisplacements matrix (one column) with the displacements
void NuTo::StructureBase::NodeGroupSetDisplacements(int rGroupIdent, int rTimeDerivative, const FullVector<double,Eigen::Dynamic>& rDisplacements)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());
    this->mUpdateTmpStaticDataRequired=true;
    if (rDisplacements.GetNumColumns()!=1)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] Displacement matrix has to have a single column.");

    boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rGroupIdent);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::eGroupId::Nodes)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] Group is not a node group.");
    Group<NodeBase> *nodeGroup = dynamic_cast<Group<NodeBase>*>(itGroup->second);
    assert(nodeGroup!=0);

    for (Group<NodeBase>::iterator itNode=nodeGroup->begin(); itNode!=nodeGroup->end();itNode++)
    {
        try
        {
            if (itNode->second->GetNumTimeDerivatives(Node::eDof::DISPLACEMENTS)<rTimeDerivative)
                throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] does not have a sufficient number of time derivatives.");

            if (rDisplacements.GetNumRows() <= 0 or rDisplacements.GetNumRows() > 3)
                throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] The number of displacement components is either 1, 2 or 3.");

            itNode->second->Set(Node::eDof::DISPLACEMENTS, rTimeDerivative, rDisplacements);
        }
        catch(NuTo::MechanicsException & b)
        {
            b.AddMessage("[NuTo::StructureBase::NodeGroupSetDisplacements] Error setting displacements.");
            throw;
        }
        catch(...)
        {
            throw MechanicsException("[NuTo::StructureBase::NodeGroupSetDisplacements] Error setting displacements of node (unspecified exception).");
        }
    }
}

void NuTo::StructureBase::NodeSetTemperature(int rNode, double rTemperature)
{
    NodeBase* nodePtr = NodeGetNodePtr(rNode);
    this->mUpdateTmpStaticDataRequired = true;
    nodePtr->Set(Node::eDof::TEMPERATURE, rTemperature);
}

void NuTo::StructureBase::NodeSetTemperature(int rNode, int rTimeDerivative, double rTemperature)
{
    NodeBase* nodePtr = NodeGetNodePtr(rNode);
    this->mUpdateTmpStaticDataRequired = true;
    if (nodePtr->GetNumTimeDerivatives(Node::eDof::TEMPERATURE)<rTimeDerivative)
        throw MechanicsException(__PRETTY_FUNCTION__,
                "Number of time derivatives stored at node is less than the required value.");
    nodePtr->Set(Node::eDof::TEMPERATURE, rTimeDerivative, rTemperature);
}

void NuTo::StructureBase::NodeGroupGetMembers(int rGroupId, NuTo::FullVector<int,Eigen::Dynamic>& rMembers)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rGroupId);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::NodeGroupGetMembers] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::eGroupId::Nodes)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupGetMembers] Group is not an node group.");
    Group<NodeBase> *nodeGroup = itGroup->second->AsGroupNode();
    assert(nodeGroup!=0);

    rMembers.Resize(nodeGroup->GetNumMembers());
    int countNode(0);
    for (Group<NodeBase>::const_iterator itNode=nodeGroup->begin(); itNode!=nodeGroup->end();itNode++,countNode++)
    {
        rMembers[countNode] = itNode->first;
    }
}


//! @brief gets the displacements of a node
//! @param rIdent node identifier
//! @param rDisplacements matrix (one column) with the displacements
void NuTo::StructureBase::NodeGetDisplacements(int rNode, FullVector<double,Eigen::Dynamic>& rDisplacements)const
{
    this->NodeGetDisplacements(rNode,0,rDisplacements);
}


//! @brief gets the displacements of a node
//! @param rIdent node identifier
//! @param rDisplacements matrix (one column) with the displacements
void NuTo::StructureBase::NodeGetDisplacements(int rNode, int rTimeDerivative, FullVector<double,Eigen::Dynamic>& rDisplacements)const
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    const NodeBase* nodePtr = NodeGetNodePtr(rNode);

    try
    {
        if (nodePtr->GetNum(Node::eDof::DISPLACEMENTS) == 0)
            throw MechanicsException("[NuTo::StructureBase::NodeGetDisplacements] Node has no displacements.");

        rDisplacements = nodePtr->Get(Node::eDof::DISPLACEMENTS, rTimeDerivative);
    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeGetDisplacements] Error getting displacements.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeGetDisplacements] Error getting displacements of node (unspecified exception).");
    }
}


//! @brief gets the displacement dofs of a node
//! @param rIdent node identifier
//! @param rDisplacements matrix (one column) with the displacements
void NuTo::StructureBase::NodeGetDisplacementDofs(int rNode, FullVector<int,Eigen::Dynamic>& rDisplacementDofs)const
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    const NodeBase* nodePtr = NodeGetNodePtr(rNode);

    try
    {
        int numDofs = nodePtr->GetNum(Node::eDof::DISPLACEMENTS);
        if (numDofs == 0)
            throw MechanicsException(__PRETTY_FUNCTION__, "Node has no displacements.");
        rDisplacementDofs.resize(numDofs);
        for (int i = 0; i < numDofs; ++i)
            rDisplacementDofs[i] = nodePtr->GetDof(Node::eDof::DISPLACEMENTS, i);

    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage(__PRETTY_FUNCTION__, "Error getting displacement dofs.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Error getting displacement dofs of node (unspecified exception).");
    }
}

std::vector<int> NuTo::StructureBase::NodeGetDofIds(const int rNodeId, NuTo::Node::eDof rDof)const
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    const NodeBase* nodePtr = NodeGetNodePtr(rNodeId);

    try
    {
        int numDofIds = nodePtr->GetNum(rDof);

        if (numDofIds == 0)
            throw MechanicsException(__PRETTY_FUNCTION__, "Node does not have the requested dof.");

        std::vector<int> dofIds(numDofIds);
        for (int i = 0; i < numDofIds; ++i)
            dofIds[i] = nodePtr->GetDof(rDof, i);

        return dofIds;

    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage(__PRETTY_FUNCTION__, "Error getting the requested dof identifiers.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "Error getting the requested dof identifiers.");
    }
}

//! @brief gets the rotations of a node
//! @param rIdent node identifier
//! @param rRotation matrix (one column) with the rotations
void NuTo::StructureBase::NodeGetRotations(int rNode, FullVector<double,Eigen::Dynamic>& rRotations)const
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    const NodeBase* nodePtr = NodeGetNodePtr(rNode);

    try
    {
        if (nodePtr->GetNum(Node::eDof::ROTATIONS) != 1 and nodePtr->GetNum(Node::eDof::ROTATIONS) != 3)
            throw MechanicsException("[NuTo::StructureBase::NodeGetRotations] Node has neither 1(2D) or 3(3D) rotations.");

        rRotations = nodePtr->Get(Node::eDof::ROTATIONS);
    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeGetRotations] Error getting rotations.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeGetRotations] Error getting rotations of node (unspecified exception).");
    }
}
//! @brief gets the displacements of a group of nodes
//! @param rNodeGroup node group identifier
//! @param rDisplacements matrix (rows/nodes columns/rDisplacements)
void NuTo::StructureBase::NodeGroupGetDisplacements(int rGroupIdent, FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic>& rDisplacements)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rGroupIdent);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::NodeGroupGetDisplacements] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::eGroupId::Nodes)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupGetDisplacements] Group is not a node group.");
    Group<NodeBase> *nodeGroup = itGroup->second->AsGroupNode();
    assert(nodeGroup!=0);

    //all nodes have to have the same dimension
    if(nodeGroup->GetNumMembers()<1)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupGetDisplacements] Group has no members.");

    int numDisp= nodeGroup->begin()->second->GetNum(Node::eDof::DISPLACEMENTS);
    //resize the matrix
    rDisplacements.Resize(nodeGroup->GetNumMembers(),numDisp);

    int theNode(0);
    for (Group<NodeBase>::iterator itNode=nodeGroup->begin(); itNode!=nodeGroup->end();itNode++, theNode++)
    {
        try
        {
            if (numDisp != 1 and numDisp != 2 and numDisp != 3)
                throw MechanicsException("[NuTo::StructureBase::NodeGroupGetDisplacements] The number of displacement components is either 1, 2 or 3.");

            rDisplacements.SetRow(theNode, itNode->second->Get(Node::eDof::DISPLACEMENTS).transpose());

        }
        catch(NuTo::MechanicsException & b)
        {
            b.AddMessage("[NuTo::StructureBase::NodeGroupGetDisplacements] Error getting displacements.");
            throw;
        }
        catch(...)
        {
            throw MechanicsException("[NuTo::StructureBase::NodeGroupGetDisplacements] Error getting displacements of node (unspecified exception).");
        }
    }
}

double NuTo::StructureBase::NodeGetTemperature(int rNode) const
{
    return this->NodeGetTemperature(rNode, 0);
}

double NuTo::StructureBase::NodeGetTemperature(int rNode, int rTimeDerivative) const
{
    const NodeBase* nodePtr = NodeGetNodePtr(rNode);
    if (nodePtr->GetNum(Node::eDof::TEMPERATURE) == 0)
        throw MechanicsException(__PRETTY_FUNCTION__, "Node doesn't have a temperature.");
    return nodePtr->Get(Node::eDof::TEMPERATURE, rTimeDerivative)[0];
}

//! @brief gets the coordinates of a node
//! @param rNode node identifier
//! @param rCoordinates matrix (one column) with the coordinates
void NuTo::StructureBase::NodeGetCoordinates(int rNode, NuTo::FullVector<double,Eigen::Dynamic>& rCoordinates)const
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    const NodeBase* nodePtr = NodeGetNodePtr(rNode);

    try
    {
        if (nodePtr->GetNum(Node::eDof::COORDINATES) == 0)
            throw MechanicsException("[NuTo::StructureBase::NodeGetCoordinates] Node has no coordinates.");

        rCoordinates = nodePtr->Get(Node::eDof::COORDINATES);

    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeGetCoordinates] Error getting coordinates.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeGetCoordinates] Error getting coordinates of node (unspecified exception).");
    }
}

//! @brief gets the coordinates of a group of nodes
//! @param rNodeGroup node group identifier
//! @param rCoordinates matrix (rows/nodes columns/rCoordinates)
void NuTo::StructureBase::NodeGroupGetCoordinates(int rGroupIdent, FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic>& rCoordinates)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rGroupIdent);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::NodeGroupGetCoordinates] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::eGroupId::Nodes)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupGetCoordinates] Group is not a node group.");
    Group<NodeBase> *nodeGroup = itGroup->second->AsGroupNode();
    assert(nodeGroup!=0);

    //all nodes have to have the same dimension
    if(nodeGroup->GetNumMembers()<1)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupGetCoordinates] Group has no members.");

    int numCoords= nodeGroup->begin()->second->GetNum(Node::eDof::COORDINATES);
    //resize the matrix
    rCoordinates.Resize(nodeGroup->GetNumMembers(),numCoords);
    int theNode(0);
    for (Group<NodeBase>::iterator itNode=nodeGroup->begin(); itNode!=nodeGroup->end();itNode++, theNode++)
    {
        try
        {
            if (numCoords != 1 and numCoords != 2 and numCoords != 3)
                throw MechanicsException("[NuTo::StructureBase::NodeGroupGetCoordinates] The number of coordinates components is either 1, 2 or 3.");

            rCoordinates.SetRow(theNode, itNode->second->Get(Node::eDof::COORDINATES).transpose());

        }
        catch(NuTo::MechanicsException & b)
        {
            b.AddMessage("[NuTo::StructureBase::NodeGroupGetCoordinates] Error getting coordinates.");
            throw;
        }
        catch(...)
        {
            throw MechanicsException("[NuTo::StructureBase::NodeGroupGetCoordinates] Error getting coordinates of node (unspecified exception).");
        }
    }
}

//! @brief gets the global nonlocal eq plastic strain variables of a node
//! @param rNode node identifier
//! @return global (nodal) nonlocal eq plastic strain
void NuTo::StructureBase::NodeGetNonlocalEqPlasticStrain(int rNode, NuTo::FullVector<double,Eigen::Dynamic>& rNonlocalEqPlasticStrain)const
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());
    const NodeBase* nodePtr = NodeGetNodePtr(rNode);

    try
    {
        if (nodePtr->GetNum(Node::eDof::NONLOCALEQPLASTICSTRAIN) != 2)
        {
            throw MechanicsException("[NuTo::StructureBase::NodeGetNonlocalEqPlasticStrain] Node does not have nonlocal equivalent plastic strains.");
        }
        rNonlocalEqPlasticStrain = nodePtr->Get(Node::eDof::NONLOCALEQPLASTICSTRAIN);
    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeGetNonlocalEqPlasticStrain] Error getting global damage.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeGetNonlocalEqPlasticStrain] Error getting NodeGetNonlocalEqPlasticStrain of node (unspecified exception).");
    }
}

//! @brief gets the global nonlocal total strain variables of a node
//! @param rNode node identifier
//! @return global (nodal) nonlocal total strain
void NuTo::StructureBase::NodeGetNonlocalTotalStrain(int rNode, NuTo::FullVector<double,Eigen::Dynamic>& rNonlocalTotalStrain)const
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    const NodeBase* nodePtr = NodeGetNodePtr(rNode);

    try
    {
        int num = nodePtr->GetNum(Node::eDof::NONLOCALTOTALSTRAIN);
        if (num != 1 and num != 3 and num != 6)
            throw MechanicsException("[NuTo::StructureBase::NodeGetNonlocalTotalStrain] Number of nonlocal total strain components is either 1, 3 or 6 .");

        rNonlocalTotalStrain = nodePtr->Get(Node::eDof::NONLOCALTOTALSTRAIN);

    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeGetNonlocalTotalStrain] Error getting nonlocal total strain.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeGetNonlocalTotalStrain] Error getting nonlocal total strain of node (unspecified exception).");
    }
}


//! @brief calculate the internal force vector for a node
//! @param rId ... node id
//! @param rGradientInternalPotential ...vector for all the dofs the corresponding internal force (return value)
void NuTo::StructureBase::NodeInternalForce(int rId, NuTo::FullVector<double,Eigen::Dynamic>& rNodeForce)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    try
    {
        const NodeBase* nodePtr = NodeGetNodePtr(rId);
        NodeInternalForce(nodePtr,rNodeForce);
    }
    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage("[NuTo::StructureBase::NodeGradientInternalPotential] Error getting gradient of internal potential.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException("[NuTo::StructureBase::NodeGradientInternalPotential] Error getting gradient of internal potential (unspecified exception).");
    }
}

//! @brief calculate the internal force vector for a node group of nodes
//! @param rGroupIdent ... group identifier
//! @param rGradientInternalPotential ...vector for all the dofs the corresponding internal force (return value)
void NuTo::StructureBase::NodeGroupInternalForce(int rGroupIdent, NuTo::FullVector<double,Eigen::Dynamic>& rNodeForce)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    boost::ptr_map<int,GroupBase>::const_iterator itGroup = mGroupMap.find(rGroupIdent);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::NodeGroupInternalForce] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=NuTo::eGroupId::Nodes)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupInternalForce] Group is not a node group.");
    const Group<NodeBase> *nodeGroup = dynamic_cast<const Group<NodeBase>*>(itGroup->second);
    assert(nodeGroup!=0);

    NuTo::FullVector<double,Eigen::Dynamic>nodeForceLocal;

    if (nodeGroup->GetNumMembers()==0)
        throw MechanicsException("[NuTo::StructureBase::NodeGroupInternalForce] Node group is empty.");
    rNodeForce.Resize(nodeGroup->begin()->second->GetNum(Node::eDof::DISPLACEMENTS));

    for (Group<NodeBase>::const_iterator itNode=nodeGroup->begin(); itNode!=nodeGroup->end();itNode++)
    {
        try
        {
            NodeInternalForce(itNode->second, nodeForceLocal);
            if (nodeForceLocal.GetNumRows()!=rNodeForce.GetNumRows())
                throw MechanicsException("[NuTo::StructureBase::NodeGroupInternalForce] The number of displacement components is not equal for all members of the group.");
            rNodeForce+=nodeForceLocal;
        }
        catch(NuTo::MechanicsException & b)
        {
            b.AddMessage("[NuTo::StructureBase::NodeGroupInternalForce] Error getting gradient of internal potential.");
            throw;
        }
        catch(...)
        {
            throw MechanicsException("[NuTo::StructureBase::NodeGroupInternalForce] Error getting gradient of internal potential (unspecified exception).");
        }
    }
}

//! @brief calculate the internal force vector for a node
//! @param rNodePtr  node for which this has to be calculated
//! @param rGradientInternalPotential ...vector for all the dofs the corresponding internal force (return value)
void NuTo::StructureBase::NodeInternalForce(const NodeBase* rNodePtr, NuTo::FullVector<double,Eigen::Dynamic>& rNodeForce)
{
    try
    {
        std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>> elementOutputMap;
        elementOutputMap[Element::eOutput::INTERNAL_GRADIENT] = std::make_shared<ElementOutputBlockVectorDouble>(mDofStatus);
        elementOutputMap[Element::eOutput::GLOBAL_ROW_DOF] = std::make_shared<ElementOutputBlockVectorInt>(mDofStatus);

        std::vector<ElementBase*> elements;
        this->NodeGetElements(rNodePtr, elements);

        rNodeForce.Resize(rNodePtr->GetNum(Node::eDof::DISPLACEMENTS));
        rNodeForce.setZero();

        for (auto element : elements)
        {
            element->Evaluate(elementOutputMap);
            const auto& internalGradient = elementOutputMap.at(Element::eOutput::INTERNAL_GRADIENT)->GetBlockFullVectorDouble()[Node::eDof::DISPLACEMENTS];
            const auto& globalRowDof = elementOutputMap.at(Element::eOutput::GLOBAL_ROW_DOF)->GetBlockFullVectorInt()[Node::eDof::DISPLACEMENTS];
            assert(internalGradient.GetNumRows() == globalRowDof.GetNumRows());

            for (int countDof=0; countDof< rNodePtr->GetNum(Node::eDof::DISPLACEMENTS); countDof++)
            {
                int theDof = rNodePtr->GetDof(Node::eDof::DISPLACEMENTS, countDof);
                for (int iDof=0; iDof < globalRowDof.GetNumRows(); iDof++)
                {
                    if (globalRowDof[iDof] == theDof)
                    {
                        rNodeForce(countDof)+=internalGradient(iDof);
                    }
                }
            }
        }


    }    catch(NuTo::MechanicsException & b)
    {
        b.AddMessage(std::string("[") + __PRETTY_FUNCTION__ + "] Error getting gradient of internal potential.");
        throw;
    }
    catch(...)
    {
        throw MechanicsException(std::string("[") + __PRETTY_FUNCTION__ + "] Error getting gradient of internal potential (unspecified exception).");
    }
}

//! @brief ... store all element ids connected to this node in a vector
//! @param rNode (Input) 			... node id
//! @param rElementNumbers (Output) ... vector of element ids
void NuTo::StructureBase::NodeGetElements(const int rNodeId, NuTo::FullVector<int,Eigen::Dynamic>& rElementNumbers)
{
    throw MechanicsException("[NuTo::StructureBase::NodeGetElements] Not available for this structure type.");
}

//! @brief ... store all elements connected to this node in a vector
//! @param rNode (Input) 		... node pointer
//! @param rElements (Output) 	... vector of element pointers
void NuTo::StructureBase::NodeGetElements(const NuTo::NodeBase* rNodePtr, std::vector<NuTo::ElementBase*>& rElements)
{
    throw MechanicsException("[NuTo::StructureBase::NodeGetElements] Not available for this structure type.");
}


//! @brief ... returns the (first) node that has the specified coordinates within the range
//! @param ... rCoordinates
//! @param ... rRange
//! @return ... node id
int NuTo::StructureBase::NodeGetIdAtCoordinate(FullVector<double, Eigen::Dynamic> rCoordinates, double rRange)
{
    NuTo::Timer(__FUNCTION__, GetShowTime(), GetLogger());

    std::vector<std::pair<int,NodeBase*> > nodeVector;
    this->GetNodesTotal(nodeVector);

    double distance;

    int nodeId = -1;
    for (unsigned int countNode=0; countNode<nodeVector.size(); countNode++)
    {
        NodeBase* nodePtr(nodeVector[countNode].second);
        if (nodePtr->GetNum(Node::eDof::COORDINATES)<1)
            continue;

        distance = (nodePtr->Get(Node::eDof::COORDINATES)-rCoordinates).norm();

        if (distance<rRange)
        {
            if (nodeId==-1)
            {
                nodeId = nodeVector[countNode].first;
            }
            else
                throw MechanicsException("[NuTo::StructureBase::NodeGetIdAtCoordinate] there is more than one node at that coordinate position.");
        }
    }
    if (nodeId==-1)
    {
        mLogger << "[NuTo::StructureBase::NodeGetIdAtCoordinate] no node could be found, return -1 as node id\n";
    }
    return nodeId;
}


#ifdef ENABLE_VISUALIZE
//! @brief ... adds all the nodes in the vector to the data structure that is finally visualized
void NuTo::StructureBase::NodeTotalAddToVisualize(VisualizeUnstructuredGrid& rVisualize, const std::list<std::shared_ptr<NuTo::VisualizeComponent>>& rVisualizationList) const
{
    std::vector<const NodeBase*> nodeVec;
    this->GetNodesTotal(nodeVec);
    NodeVectorAddToVisualize(rVisualize,rVisualizationList,nodeVec);
}

//! @brief ... adds all the nodes in the vector to the data structure that is finally visualized
void NuTo::StructureBase::NodeVectorAddToVisualize(VisualizeUnstructuredGrid& rVisualize, const std::list<std::shared_ptr<NuTo::VisualizeComponent>>& rVisualizationList, const std::vector<const NodeBase*>& rNodes) const
{
    for (unsigned int nodeCount = 0; nodeCount < rNodes.size(); nodeCount++)
    {
        rNodes[nodeCount]->Visualize(rVisualize, rVisualizationList);
    }
}
#endif //ENABLE_VISUALIZE