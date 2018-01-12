﻿#include <iostream>
#include "nuto/mechanics/structures/StructureBase.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"
#include "nuto/mechanics/structures/StructureOutputBlockMatrix.h"
#include "nuto/mechanics/structures/StructureOutputBlockVector.h"

#include <eigen3/Eigen/Core>

#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"
#include "nuto/math/SparseMatrixCSRSymmetric.h"
#include "nuto/math/SparseDirectSolverMUMPS.h"
#include "nuto/math/SparseDirectSolverPardiso.h"

#include "nuto/mechanics/integrationtypes/IntegrationType1D2NLobatto3Ip.h"
#include "nuto/mechanics/integrationtypes/IntegrationType1D2NLobatto4Ip.h"
#include "nuto/mechanics/integrationtypes/IntegrationType1D2NLobatto5Ip.h"
#include "nuto/mechanics/integrationtypes/IntegrationType2D4NLobatto9Ip.h"
#include "nuto/mechanics/integrationtypes/IntegrationType2D4NLobatto16Ip.h"
#include "nuto/mechanics/integrationtypes/IntegrationType2D4NLobatto25Ip.h"
#include "nuto/mechanics/integrationtypes/IntegrationType1D2NGaussNIp.h"

#include "nuto/mechanics/constitutive/ConstitutiveEnum.h"
#include "nuto/mechanics/interpolationtypes/InterpolationTypeEnum.h"
#include "nuto/mechanics/integrationtypes/IntegrationTypeEnum.h"
#include "nuto/mechanics/elements/ElementDataEnum.h"
#include "nuto/mechanics/elements/IpDataEnum.h"

#include "nuto/mechanics/IGA/NURBSCurve.h"
#include "nuto/mechanics/IGA/NURBSSurface.h"

#include "nuto/mechanics/nodes/NodeBase.h"
#include "nuto/mechanics/nodes/NodeEnum.h"

#include "nuto/mechanics/groups/GroupEnum.h"

#include <boost/filesystem.hpp>

#include "nuto/mechanics/elements/ElementShapeFunctions.h"

#include "nuto/mechanics/timeIntegration/RungeKutta4.h"

#include "nuto/mechanics/dofSubMatrixStorage/BlockScalar.h"
#include "nuto/mechanics/dofSubMatrixStorage/BlockScalar.h"
#include "nuto/mechanics/dofSubMatrixStorage/BlockScalar.h"

#include "nuto/mechanics/timeIntegration/NewmarkDirect.h"

#include "nuto/mechanics/groups/GroupBase.h"
#include "nuto/mechanics/elements/ElementBase.h"
#include "nuto/mechanics/elements/ElementDataBase.h"

#include "nuto/mechanics/elements/IpDataEnum.h"
#include "nuto/mechanics/MechanicsException.h"

#include "nuto/mechanics/timeIntegration/NewmarkDirect.h"

#include "nuto/mechanics/elements/ContinuumContactElement.h"

#define _USE_MATH_DEFINES
#include <cmath>

#ifdef ENABLE_VISUALIZE
#include "nuto/visualize/VisualizeEnum.h"
#endif

#define PRINTRESULT true

int buildStructure2D(NuTo::Interpolation::eTypeOrder rElementTypeIdent,
                      int rNumNodesPerElementInOneDir,
                      NuTo::FullVector<double, Eigen::Dynamic>& nodeCoordinatesFirstElement,
                      int NumElementsX,
                      int NumElementsY,
                      double Height,
                      double Length,
                      double startx,
                      double starty,
                      int startNode,
                      NuTo::Structure* myStructure,
                      int rNodeGroup,
                      int rElementGroup,
                      const std::set<NuTo::Node::eDof> &setOfDOFS)
{
    /** parameters **/
    double elementSize = nodeCoordinatesFirstElement(rNumNodesPerElementInOneDir-1) - nodeCoordinatesFirstElement(0);
    // reference element Length is 2.
    double  factorX =  Length/(NumElementsX*elementSize);
    double  factorY =  Height/(NumElementsY*elementSize);

    NuTo::FullVector<double,Eigen::Dynamic> nodeStart(2);
    nodeStart(0) = startx;
    nodeStart(1) = starty;

    /** Nodes **/
    NuTo::FullVector<double,Eigen::Dynamic> nodeCoordinates(2);
    int node = startNode;
    double elementBeginX = 0.;
    double elementBeginY = 0.;

    nodeCoordinates(0) = factorX*nodeCoordinatesFirstElement(0);
    nodeCoordinates(1) = factorY*nodeCoordinatesFirstElement(0);
    nodeCoordinates += nodeStart;

    myStructure->NodeCreateDOFs(node, setOfDOFS, nodeCoordinates);
    std::cout << "Node: " << node << ", Coordinates: " << nodeCoordinates.Trans() << std::endl;
//    myStructure->NodeCreate(node, nodeCoordinates);
    myStructure->GroupAddNode(rNodeGroup, node);
    node++;

    for(int y = 0; y < NumElementsY; y++)
    {
        for (int i = ((y==0)?0:1); i < nodeCoordinatesFirstElement.size(); i++)
        {
            if (node != startNode + 1)
            {
                nodeCoordinates(0) = factorX*nodeCoordinatesFirstElement(0);
                nodeCoordinates(1) = factorY*(nodeCoordinatesFirstElement(i) + elementBeginY);
                nodeCoordinates += nodeStart;

                myStructure->NodeCreateDOFs(node, setOfDOFS, nodeCoordinates);
                std::cout << "Node: " << node << ", Coordinates: " << nodeCoordinates.Trans() << std::endl;
                myStructure->GroupAddNode(rNodeGroup, node);
//                myStructure->NodeCreate(node, nodeCoordinates);
                node++;
            }

            elementBeginX = 0.;
            for(int x = 0; x < NumElementsX; x++)
            {
                for (int j = 1; j < nodeCoordinatesFirstElement.size(); j++)
                {
                    nodeCoordinates(0) = factorX*(nodeCoordinatesFirstElement(j) + elementBeginX);
                    nodeCoordinates(1) = factorY*(nodeCoordinatesFirstElement(i) + elementBeginY);
                    nodeCoordinates += nodeStart;

                    myStructure->NodeCreateDOFs(node, setOfDOFS, nodeCoordinates);
                    std::cout << "Node: " << node << ", Coordinates: " << nodeCoordinates.Trans() << std::endl;
                    myStructure->GroupAddNode(rNodeGroup, node);
//                    myStructure->NodeCreate(node, nodeCoordinates);
                    node++;
                }
                elementBeginX += elementSize;
            }
        }
        elementBeginY += elementSize;
    }

    int interpolationType = myStructure->InterpolationTypeCreate("QUAD2D");
    myStructure->InterpolationTypeAdd(interpolationType, NuTo::Node::eDof::DISPLACEMENTS, rElementTypeIdent);
    myStructure->InterpolationTypeAdd(interpolationType, NuTo::Node::eDof::COORDINATES, rElementTypeIdent);

    /** Elements **/
    NuTo::FullVector<int,Eigen::Dynamic> elementIncidence(rNumNodesPerElementInOneDir*rNumNodesPerElementInOneDir);
    int numNodesInRow = NumElementsX*(rNumNodesPerElementInOneDir-1) + 1;
    for(int j = 0; j < NumElementsY; j++)
    {
        for (int i = 0; i < NumElementsX; i++)
        {
            // one element
            for (int k = 0; k < rNumNodesPerElementInOneDir; k++)
                for(int l = 0; l < rNumNodesPerElementInOneDir; l++)
                    elementIncidence(k + l*rNumNodesPerElementInOneDir) = startNode + i*(rNumNodesPerElementInOneDir - 1) + k + l*numNodesInRow + j*(rNumNodesPerElementInOneDir-1)*numNodesInRow;

            if (rElementTypeIdent == NuTo::Interpolation::eTypeOrder::EQUIDISTANT1)
            {
                int temp = elementIncidence(2);
                elementIncidence(2) = elementIncidence(3);
                elementIncidence(3) = temp;

            }

            int myElement = myStructure->ElementCreate(interpolationType, elementIncidence);

//            NuTo::ElementBase* el = myStructure->ElementGetElementPtr(myElement);
//            Eigen::VectorXd nodalCurrent;
//            nodalCurrent.setZero(0);
//            nodalCurrent = el->ExtractNodeValues(0, NuTo::Node::eDof::DISPLACEMENTS);
//            nodalCurrent = el->ExtractNodeValues(0, NuTo::Node::eDof::COORDINATES);

//            std::cout << "Element: " << myElement << ", Nodes: " << elementIncidence.Trans() << std::endl;
            myStructure->GroupAddElement(rElementGroup, myElement);
        }
    }
    myStructure->ElementConvertToInterpolationType(rElementGroup, 1.e-6,10);
    return node;
}



void SetDBCPatchTestRigidIGA(NuTo::Structure &myStructure, int groupNodesSlave, int groupNodesIGAlayer, int groupNodesMaster, int &countDBC)
{
    Eigen::Vector2d direction(0,0);
    double xMin(0.), xMax(0.);
    double yMin(0.), yMax(0.);
    auto LambdaNodes = [&](NuTo::NodeBase* rNodePtr) -> bool
    {
        double Tol = 1.e-6;
        if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
        {
            double x = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[0];
            double y = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[1];
            bool xR = false;
            bool yR = false;

            if (x >= xMin - Tol && x <= xMax + Tol) xR = true;
            if (y >= yMin - Tol && y <= yMax + Tol) yR = true;

            if (xR == true && yR == true) return true;
        }
        return false;
    };

    NuTo::FullMatrix<int, Eigen::Dynamic> members;
    // ===> master bottom <=== //
    xMin = 0.; xMax = std::numeric_limits<double>::infinity();
    yMin = yMax = 0.;
    int groupNodesMasterLower = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
    myStructure.GroupAddNodeFunction(groupNodesMasterLower, groupNodesMaster, LambdaNodes);

    members = myStructure.GroupGetMemberIds(groupNodesMasterLower);

    direction << 0.,1.;
    countDBC = myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodesMasterLower, direction, 0.0);
    direction << 1.,0.;
    countDBC = myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodesMasterLower, direction, 0.0);

    // ===> initial values <=== //
    NuTo::FullVector<double,Eigen::Dynamic>  dispVec(2);
    dispVec(0) =   0.;
    dispVec(1) = -0.00001;
    myStructure.NodeGroupSetDisplacements(groupNodesSlave, 0, dispVec);
    if(groupNodesIGAlayer >= 0)
        myStructure.NodeGroupSetDisplacements(groupNodesIGAlayer, 0, dispVec);

    // ===> slave left <=== //
    xMin = 1.; xMax = 1.;
    yMin = 1.; yMax = 1.;
    int groupNodesLeft = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
    myStructure.GroupAddNodeFunction(groupNodesLeft, groupNodesSlave, LambdaNodes);

    members = myStructure.GroupGetMemberIds(groupNodesLeft);

    direction << 1, 0;
    countDBC = myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodesLeft, direction, 0.0);

    // ===> PATCH TEST BOUNDARY <=== //
    double Stress = 10.;
    myStructure.SetNumLoadCases(1);
    // ===> slave top <=== //
    xMin = 1.; xMax = 2.;
    yMin = 1.; yMax = 1.;
    int groupNodesSlaveUpper = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
    myStructure.GroupAddNodeFunction(groupNodesSlaveUpper, LambdaNodes);

    members = myStructure.GroupGetMemberIds(groupNodesSlaveUpper);

    int groupElementsSlaveUpper = myStructure.GroupCreate(NuTo::eGroupId::Elements);
    myStructure.GroupAddElementsFromNodes(groupElementsSlaveUpper, groupNodesSlaveUpper, false);

    members = myStructure.GroupGetMemberIds(groupElementsSlaveUpper);

    myStructure.LoadSurfacePressureCreate2D(0, groupElementsSlaveUpper, groupNodesSlaveUpper, Stress);

    // ===> slave bottom <=== //
//    xMin = 1.; xMax = 2.;
//    yMin = 0.; yMax = 0.;
//    int groupNodesSlaveBottom = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
//    myStructure.GroupAddNodeFunction(groupNodesSlaveBottom, groupNodesSlave, LambdaNodes);
//    direction << 0, 1;
//    countDBC = myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodesSlaveBottom, direction, 0.0);


//    NuTo::FullVector<int,Eigen::Dynamic> rMembersSlaveBottom;
//    myStructure.NodeGroupGetMembers(groupNodesSlaveBottom, rMembersSlaveBottom);
//    direction << 1.,0.;
//    countDBC = myStructure.ConstraintLinearSetDisplacementNode(rMembersSlaveBottom(0), direction, 0.0);

    countDBC++;
}

void AddIGALayer2(NuTo::Structure *myStructure,
                  const std::function<bool(NuTo::NodeBase *)> &rFunction,
                  int rNodesGroupId,
                  int rDegree,
                  int &groupElementsIGAlayer,
                  int &groupNodesIGAlayer,
                  Eigen::Matrix<std::pair<int, int>, Eigen::Dynamic, Eigen::Dynamic> &rElements,
                  int &countDBC)
{
    // Nodes on the part to interpolate
    int groupFENodes = myStructure->GroupCreate(NuTo::eGroupId::Nodes);
    myStructure->GroupAddNodeFunction(groupFENodes, rNodesGroupId, rFunction);
    NuTo::FullVector<int,Eigen::Dynamic> idsFENodes;
    myStructure->NodeGroupGetMembers(groupFENodes, idsFENodes);

    // Matrix containing the ids and coordinates of the FE nodes => 'coordinatesAndIDs'
    NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic> coordinates;
    myStructure->NodeGroupGetCoordinates(groupFENodes, coordinates);

    NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic> coordinatesAndIDs;
    coordinatesAndIDs.resize(coordinates.rows(), coordinates.cols() + 1);
    coordinatesAndIDs.block(0, 0, coordinates.rows(), coordinates.cols()) = coordinates;

    for(int i = 0; i < coordinates.rows(); i++)
        coordinatesAndIDs(i, coordinates.cols()) = idsFENodes(i);

    coordinatesAndIDs.SortRow(0);

    Eigen::MatrixXd controlPointsCoordinates(coordinatesAndIDs.rows(), 2);
    for(int i = 0; i < coordinatesAndIDs.rows(); i++)
    {
        controlPointsCoordinates(i,0) = coordinatesAndIDs(i,0);
        controlPointsCoordinates(i,1) = coordinatesAndIDs(i,1);
    }

    int numKnots = controlPointsCoordinates.rows() + rDegree + 1;

    Eigen::VectorXd weights(controlPointsCoordinates.rows());
    weights.fill(1.);

    Eigen::VectorXd knots(numKnots);
    int numElements  = controlPointsCoordinates.rows() - rDegree;

    for (int i = 0; i <= rDegree; i++) knots(i) = 0.;
    for (int i = rDegree + 1; i <= rDegree + numElements - 1 ; i++) knots(i) = knots(i-1) + 1./numElements;
    for (int i = rDegree + numElements ; i < numKnots; i++) knots(i) = 1;

    NuTo::NURBSCurve curve(knots, controlPointsCoordinates, weights, rDegree);

//    for(int i = 0; i < 1; i++) curve.DuplicateKnots();

    // Build the IGA layer
    std::set<NuTo::Node::eDof> setOfDOFS;
    setOfDOFS.insert(NuTo::Node::eDof::COORDINATES);
    setOfDOFS.insert(NuTo::Node::eDof::DISPLACEMENTS);

    groupNodesIGAlayer = myStructure->GroupCreate("Nodes");
    groupElementsIGAlayer  = myStructure->GroupCreate("Elements");

    Eigen::VectorXi nodeIDs;
    rElements = curve.buildIGAStructure(*myStructure, setOfDOFS, groupElementsIGAlayer, groupNodesIGAlayer, "IGA1DLAYER", nodeIDs);

//    std::cout << curve.CurvePoint(1,0) << std::endl;
//    std::cout << curve.CurvePoint(0,0) << std::endl;

    int dim = coordinatesAndIDs.cols() - 1;
    for(int i = 0; i < coordinatesAndIDs.rows(); i++)
    {
        double parameter = 0.;
        if(i == 0)
        {
            parameter = 0.;
        }
        else if(i == coordinatesAndIDs.rows()-1)
        {
            parameter = 1.;
        }
        else
        {
            parameter = 0.5;
            curve.findMinimalDistance(coordinatesAndIDs.block(i, 0, 1, 2).transpose(), parameter);
        }
        Eigen::VectorXd basis = curve.BasisFunctions(parameter);
        Eigen::VectorXi controlPointIDs =  curve.GetParameterControlPoints(parameter);

        Eigen::Vector2d test(0.,0.);
        for(int k = 0; k < controlPointIDs.rows(); k++) test += basis(k)*curve.GetControlPoint(controlPointIDs(k));
        std::cout << coordinatesAndIDs.block(i, 0, 1, 2).transpose() - test << std::endl;

        for(int dof = 0; dof < dim; dof++)
        {
            myStructure->ConstraintLinearEquationCreate (countDBC, coordinatesAndIDs(i, dim), NuTo::Node::eDof::DISPLACEMENTS, dof,  1., 0.);
            for(int controlPoint = 0; controlPoint < controlPointIDs.rows(); controlPoint++)
            {
                myStructure->ConstraintLinearEquationAddTerm(countDBC, nodeIDs(controlPointIDs(controlPoint)), NuTo::Node::eDof::DISPLACEMENTS, dof, -basis(controlPoint));
            }
            countDBC++;
        }
    }
}

void ContactTestOneElementLayerSlave(const std::string &resultDir,
                                     NuTo::Interpolation::eTypeOrder rElementTypeIdent,
                                     int rNumNodesPerElementInOneDir,
                                     NuTo::FullVector<double, Eigen::Dynamic>& nodeCoordinatesFirstElement,
                                     int rDegree,
                                     double rPenalty,
                                     NuTo::eIntegrationType rIntegrationType,
                                     int rContactAlgo,
                                     int numElXSlave,
                                     int numElYSlave)
{
    NuTo::Structure myStructure(2);
    myStructure.SetNumTimeDerivatives(0);

#ifdef _OPENMP
    int numThreads = 4;
    myStructure.SetNumProcessors(numThreads);
#endif

    // *** slave *** //
    std::set<NuTo::Node::eDof> setOfDOFSSlave;
    setOfDOFSSlave.insert(NuTo::Node::eDof::COORDINATES);
    setOfDOFSSlave.insert(NuTo::Node::eDof::DISPLACEMENTS);

    int groupNodesSlave = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
    int groupElementsSlave = myStructure.GroupCreate(NuTo::eGroupId::Elements);
    double startX = 1.;
    double length = 1.;
    buildStructure2D(rElementTypeIdent, rNumNodesPerElementInOneDir, nodeCoordinatesFirstElement,
                     numElXSlave, numElYSlave, 1., length, startX, 0., 0, &myStructure, groupNodesSlave, groupElementsSlave, setOfDOFSSlave);

    // ===> build contact elements slave elements
    auto LambdaGetSlaveNodesLower = [](NuTo::NodeBase* rNodePtr) -> bool
    {
        double Tol = 1.e-6;
        if (rNodePtr->GetNum(NuTo::Node::eDof::COORDINATES)>0)
        {
            double y = rNodePtr->Get(NuTo::Node::eDof::COORDINATES)[1];
            if ((y >= 0. - Tol && y <= 0. + Tol))
            {
                return true;
            }
        }
        return false;
    };  // LambdaGetSlaveNodesLower

    int groupNodesSlaveLower = myStructure.GroupCreate(NuTo::eGroupId::Nodes);
    myStructure.GroupAddNodeFunction(groupNodesSlaveLower, groupNodesSlave, LambdaGetSlaveNodesLower);

    int groupElementsSlaveLower = myStructure.GroupCreate(NuTo::eGroupId::Elements);
    myStructure.GroupAddElementsFromNodes(groupElementsSlaveLower, groupNodesSlaveLower, false);

    // *** master - rigid *** //
    int numPoints = 10;
    double LengthMaster = 3.;
    NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic> points(numPoints,2);
    for(int i = 0; i < numPoints; i++)
    {
        points(i,0) = i*LengthMaster/(numPoints-1);
        points(i,1) = 0.;
    }

    NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic> AInv;
    NuTo::NURBSCurve curve(rDegree, points, AInv);
    for(int i = 0; i < 1; i++) curve.DuplicateKnots();

    std::set<NuTo::Node::eDof> setOfDOFS;
    setOfDOFS.insert(NuTo::Node::eDof::COORDINATES);
    setOfDOFS.insert(NuTo::Node::eDof::DISPLACEMENTS);

    int groupNodesIGA    = myStructure.GroupCreate("Nodes");
    int groupElementsIGA = myStructure.GroupCreate("Elements");

    Eigen::VectorXi nodeIDs;
    // master side fixed
    Eigen::Matrix<std::pair<int, int>, Eigen::Dynamic, Eigen::Dynamic> elementsMaster =
    curve.buildIGAStructure(myStructure, setOfDOFS, groupElementsIGA, groupNodesIGA, "IGA1DLAYER", nodeIDs);

    int countDBC = 0;

    // contact
    int groupElementsIGAlayer, groupNodesIGAlayer;
    Eigen::Matrix<std::pair<int, int>, Eigen::Dynamic, Eigen::Dynamic> rElements;
    AddIGALayer2(&myStructure,
                 LambdaGetSlaveNodesLower,
                 groupNodesSlave,
                 rDegree,
                 groupElementsIGAlayer, groupNodesIGAlayer,
                 rElements,
                 countDBC);

    SetDBCPatchTestRigidIGA(myStructure, groupNodesSlave, groupNodesIGAlayer, groupNodesIGA,  countDBC);

    ///////////////////
    // ===> material //
    ///////////////////

    double Thickness = 1.;
    int section = myStructure.SectionCreate("PLANE_STRESS");
    myStructure.SectionSetThickness(section, Thickness);

    double E = rPenalty;
    double nue = 0.;
    double rho = 0.;

    int constitutiveLaw = myStructure.ConstitutiveLawCreate("Linear_Elastic_Engineering_Stress");
    myStructure.ConstitutiveLawSetParameterDouble(constitutiveLaw, NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, E);
    myStructure.ConstitutiveLawSetParameterDouble(constitutiveLaw, NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO, nue);
    myStructure.ConstitutiveLawSetParameterDouble(constitutiveLaw, NuTo::Constitutive::eConstitutiveParameter::DENSITY, rho);
    myStructure.ElementTotalSetSection(section);
    myStructure.ElementTotalSetConstitutiveLaw(constitutiveLaw);

    ////////////////////////////
    // ===> CONTACT ELEMENTS  //
    ////////////////////////////

    int constitutiveLawPC = myStructure.ConstitutiveLawCreate("Contact_Constitutive_Law");
    std::function<double(double)> constitutiveContactLaw  =
            [rPenalty](double rGap) -> double
    {
        if(rGap < 0.)
            return rPenalty*rGap;
        else
            return 0.;
    };

    std::function<double(double)> constitutiveContactLawDerivative =
            [rPenalty](double rGap) -> double
    {
        if(rGap < 0.)
            return rPenalty;
        else
            return 0.;
    };

    myStructure.ConstitutiveLawSetParameterFunction(constitutiveLawPC, NuTo::Constitutive::eConstitutiveParameter::CONSTITUTIVE_LAW_FUNCTION, constitutiveContactLaw);
    myStructure.ConstitutiveLawSetParameterFunction(constitutiveLawPC, NuTo::Constitutive::eConstitutiveParameter::CONSTITUTIVE_LAW_DERIVATIVE_FUNCTION, constitutiveContactLawDerivative);

    // contact
    myStructure.NuTo::Structure::ContactElementsCreate<1,1>(groupElementsIGAlayer, groupNodesIGAlayer, elementsMaster, rIntegrationType, rContactAlgo, constitutiveLawPC);

    ///////////////////
    // ===> Solution //
    ///////////////////
    //set result directory
    if (boost::filesystem::exists(resultDir))
    {
        if (boost::filesystem::is_directory(resultDir))
        {
            boost::filesystem::remove_all(resultDir);
        }
    }

    // create result directory
    boost::filesystem::create_directory(resultDir);

    myStructure.CalculateMaximumIndependentSets();
    myStructure.NodeBuildGlobalDofs();

    // layer tying
//    int cceID = myStructure.NuTo::Structure::ContactElementsCreate<2,1>(groupElementsSlaveLower, groupNodesSlaveLower,
//                                                                        rElements,
//                                                                        rIntegrationType, rContactAlgo, constitutiveLawPC);

//    NuTo::ElementBase* elementBase = myStructure.ElementGetElementPtr(cceID);
//    NuTo::ContinuumContactElement<2,1> &contactElement = elementBase->AsContinuumContactElement21();

//    Eigen::MatrixXd D, M;
//    contactElement.ComputeGapMatrix(D, M);
    // end- layer tying

    NuTo::NewmarkDirect myIntegrationScheme(&myStructure);
    double timeStep = 1.;
    double simulationTime = 1.;

    myIntegrationScheme.SetResultDirectory(resultDir, true);

    std::vector<int> IPIdsSlave;
    std::vector<int> IPIdsMaster;

    switch(rIntegrationType)
    {
    case NuTo::eIntegrationType::IntegrationType1D2NGauss1Ip:
    {
        IPIdsMaster.clear();
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        break;
    }
    case NuTo::eIntegrationType::IntegrationType1D2NGauss2Ip:
    {
        IPIdsMaster.clear();
        IPIdsMaster.push_back(1);
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        IPIdsSlave.push_back(1);
        break;
    }
    case NuTo::eIntegrationType::IntegrationType1D2NGauss3Ip:
    {
        IPIdsMaster.clear();
        IPIdsMaster.push_back(2);
        IPIdsMaster.push_back(1);
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        IPIdsSlave.push_back(1);
        IPIdsSlave.push_back(2);
        break;
    }
    case NuTo::eIntegrationType::IntegrationType1D2NGauss4Ip:
    {
        IPIdsMaster.clear();
        IPIdsMaster.push_back(3);
        IPIdsMaster.push_back(2);
        IPIdsMaster.push_back(1);
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        IPIdsSlave.push_back(1);
        IPIdsSlave.push_back(2);
        IPIdsSlave.push_back(3);
        break;
    }
    case NuTo::eIntegrationType::IntegrationType1D2NGauss5Ip:
    {
        IPIdsMaster.clear();
        IPIdsMaster.push_back(4);
        IPIdsMaster.push_back(3);
        IPIdsMaster.push_back(2);
        IPIdsMaster.push_back(1);
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        IPIdsSlave.push_back(1);
        IPIdsSlave.push_back(2);
        IPIdsSlave.push_back(3);
        IPIdsSlave.push_back(4);
        break;
    }
    case NuTo::eIntegrationType::IntegrationType1D2NLobatto6Ip:
    {
        IPIdsMaster.clear();

        IPIdsMaster.push_back(5);
        IPIdsMaster.push_back(4);
        IPIdsMaster.push_back(3);
        IPIdsMaster.push_back(2);
        IPIdsMaster.push_back(1);
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        IPIdsSlave.push_back(1);
        IPIdsSlave.push_back(2);
        IPIdsSlave.push_back(3);
        IPIdsSlave.push_back(4);
        IPIdsSlave.push_back(5);
        break;
    }
    case NuTo::eIntegrationType::IntegrationType1D2NLobatto5Ip:
    {
        IPIdsMaster.clear();

        IPIdsMaster.push_back(4);
        IPIdsMaster.push_back(3);
        IPIdsMaster.push_back(2);
        IPIdsMaster.push_back(1);
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        IPIdsSlave.push_back(1);
        IPIdsSlave.push_back(2);
        IPIdsSlave.push_back(3);
        IPIdsSlave.push_back(4);
        break;
    }
    case NuTo::eIntegrationType::IntegrationType1D2NLobatto4Ip:
    {
        IPIdsMaster.clear();

        IPIdsMaster.push_back(3);
        IPIdsMaster.push_back(2);
        IPIdsMaster.push_back(1);
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        IPIdsSlave.push_back(1);
        IPIdsSlave.push_back(2);
        IPIdsSlave.push_back(3);
        break;
    }
    case NuTo::eIntegrationType::IntegrationType1D2NLobatto3Ip:
    {
        IPIdsMaster.clear();

        IPIdsMaster.push_back(2);
        IPIdsMaster.push_back(1);
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        IPIdsSlave.push_back(1);
        IPIdsSlave.push_back(2);
        break;
    }
    case NuTo::eIntegrationType::IntegrationType1D2NGauss12Ip:
    {
        IPIdsMaster.clear();
        IPIdsMaster.push_back(11);
        IPIdsMaster.push_back(10);
        IPIdsMaster.push_back(9);
        IPIdsMaster.push_back(8);
        IPIdsMaster.push_back(7);
        IPIdsMaster.push_back(6);
        IPIdsMaster.push_back(5);
        IPIdsMaster.push_back(4);
        IPIdsMaster.push_back(3);
        IPIdsMaster.push_back(2);
        IPIdsMaster.push_back(1);
        IPIdsMaster.push_back(0);

        IPIdsSlave.clear();
        IPIdsSlave.push_back(0);
        IPIdsSlave.push_back(1);
        IPIdsSlave.push_back(2);
        IPIdsSlave.push_back(3);
        IPIdsSlave.push_back(4);
        IPIdsSlave.push_back(5);
        IPIdsSlave.push_back(6);
        IPIdsSlave.push_back(7);
        IPIdsSlave.push_back(8);
        IPIdsSlave.push_back(9);
        IPIdsSlave.push_back(10);
        IPIdsSlave.push_back(11);
        break;
    }
    default:
        throw NuTo::MechanicsException("[NuTo::Test::Contact] No Integration Type Defined.");
    }

    // create boundary elements for visualizing the results slave
    int slaveContactElementsGroupId = myStructure.BoundaryElementsCreate(groupElementsSlaveLower, groupNodesSlaveLower, NULL);
    NuTo::FullVector<int,Eigen::Dynamic> rMembersSlaveContactBoundaryVisualize;
    myStructure.ElementGroupGetMembers(slaveContactElementsGroupId, rMembersSlaveContactBoundaryVisualize);
    for(int i = 0; i < rMembersSlaveContactBoundaryVisualize.rows(); i++)
    {
        NuTo::ElementBase* elementPtr = myStructure.ElementGetElementPtr(rMembersSlaveContactBoundaryVisualize(i));
        NuTo::IpData::eIpDataType ipDataType = elementPtr->GetIpDataType(0);
        elementPtr->SetIntegrationType(myStructure.GetPtrIntegrationType(rIntegrationType), ipDataType);
    }

//    NuTo::BlockFullVector<double> f_i =  myStructure.BuildGlobalInternalGradient().J;
//    Eigen::MatrixXd A = myStructure.BuildGlobalHessian0().JJ.ExportToFullMatrix() ;

//    std::cout << "\n\n\nf_i\n" << std::endl;
//    Eigen::VectorXd f_c = f_i.Export().tail(10);
//    std::cout << f_c << std::endl;

//    std::cout << "A\n" << std::endl;
//    int numrows = A.rows();
//    int numcols = A.cols();
//    Eigen::MatrixXd B = A.block(numrows-10, numcols-10, 10, 10);
//    Eigen::VectorXd h(10);
//    h << 0., 1.e-5, 0, 1.e-5, 0, 1.e-5, 0, 1.e-5, 0, 1.e-5;
//    std::cout <<  B*h << std::endl;



    std::cout << std::flush;

    myIntegrationScheme.AddResultElementGroupIpData("ContactStressSlave1",  slaveContactElementsGroupId, 1, IPIdsSlave, NuTo::IpData::eIpStaticDataType::ENGINEERING_STRESS);

    myIntegrationScheme.SetMinTimeStepPlot(1.);
    myIntegrationScheme.SetLastTimePlot(0.);

    myIntegrationScheme.SetToleranceForce(1.e-10);
    myIntegrationScheme.SetMaxNumIterations(50);
    myIntegrationScheme.SetTimeStep(timeStep);
    myIntegrationScheme.SetPerformLineSearch(false);
    myIntegrationScheme.Solve(simulationTime);

    //    myStructure.SolveGlobalSystemStaticElastic();

#ifdef ENABLE_VISUALIZE
    int visualizationGroup = groupElementsSlave;
    myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::DISPLACEMENTS);
    myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::ENGINEERING_STRAIN);
    myStructure.AddVisualizationComponent(visualizationGroup, NuTo::eVisualizeWhat::ENGINEERING_STRESS);
    myStructure.ElementGroupExportVtkDataFile(visualizationGroup, resultDir+"/Elements.vtu", true);

    myStructure.AddVisualizationComponent(groupElementsIGAlayer, NuTo::eVisualizeWhat::DISPLACEMENTS);
    myStructure.ElementGroupExportVtkDataFile(groupElementsIGAlayer, resultDir+"/ElementsLayerSlave.vtu", true);

    myStructure.AddVisualizationComponent(groupElementsIGA, NuTo::eVisualizeWhat::DISPLACEMENTS);
    myStructure.ElementGroupExportVtkDataFile(groupElementsIGA, resultDir+"/ElementsLayer.vtu", true);
#endif
}

int main()
{
    std::string resultDir = "";
    int degree = 0;

    // LOBATTO 2
    NuTo::FullVector<double, Eigen::Dynamic> nodeCoordinates(3);

    int contactAlgorithm = 0;
    degree = 2;

    nodeCoordinates.resize(3);
    nodeCoordinates(0) = 0;
    nodeCoordinates(1) = 1;
    nodeCoordinates(2) = 2;
    contactAlgorithm = 0;
    resultDir = "./ResultsStaticFEM_IGAL_Rigid";

    ContactTestOneElementLayerSlave(resultDir,
                                    NuTo::Interpolation::eTypeOrder::LOBATTO2,
                                    3,
                                    nodeCoordinates,
                                    degree,
                                    1.e6,
                                    NuTo::eIntegrationType::IntegrationType1D2NGauss12Ip, contactAlgorithm, 2, 1);

    return 0;
}
