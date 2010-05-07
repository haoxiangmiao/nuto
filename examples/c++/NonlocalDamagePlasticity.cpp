#include "nuto/math/FullMatrix.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"
int main()
{
    try
    {
	//create structure
	NuTo::Structure myStructure(2);

	//create nodes
        NuTo::FullMatrix<double> Coordinates(2,1);
	Coordinates(0,0) = 0.0;
	Coordinates(1,0) = 0.0;
	int node1 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0,0) = 1.0;
	Coordinates(1,0) = 0.0;
	int node2 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0,0) = 2.0;
	Coordinates(1,0) = 0.0;
	int node3 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0,0) = 0.0;
	Coordinates(1,0) = 1.0;
	int node4 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0,0) = 1.0;
	Coordinates(1,0) = 1.0;
	int node5 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0,0) = 2.0;
	Coordinates(1,0) = 1.0;
	int node6 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0,0) = 0.0;
	Coordinates(1,0) = 2.0;
	int node7 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0,0) = 1.0;
	Coordinates(1,0) = 2.0;
	int node8 = myStructure.NodeCreate("displacements",Coordinates);

	Coordinates(0,0) = 2.0;
	Coordinates(1,0) = 2.0;
	int node9 = myStructure.NodeCreate("displacements",Coordinates);

	//create elements
        NuTo::FullMatrix<int> Incidence(4,1);
	Incidence(0,0) = node1;
	Incidence(1,0) = node2;
	Incidence(2,0) = node5;
	Incidence(3,0) = node4;
        int myElement1 = myStructure.ElementCreate("PLANE2D4N",Incidence,"ConstitutiveLawIpNonlocal","StaticDataNonlocal");
        myStructure.ElementSetIntegrationType(myElement1,"2D4NGauss1Ip","StaticDataNonlocal");
		
	Incidence(0,0) = node2;
	Incidence(1,0) = node3;
	Incidence(2,0) = node6;
	Incidence(3,0) = node5;
        int myElement2 = myStructure.ElementCreate("PLANE2D4N",Incidence,"ConstitutiveLawIpNonlocal","StaticDataNonlocal");
		
	Incidence(0,0) = node4;
	Incidence(1,0) = node5;
	Incidence(2,0) = node8;
	Incidence(3,0) = node7;
        int myElement3 = myStructure.ElementCreate("PLANE2D4N",Incidence,"ConstitutiveLawIpNonlocal","StaticDataNonlocal");
		
	Incidence(0,0) = node5;
	Incidence(1,0) = node6;
	Incidence(2,0) = node9;
	Incidence(3,0) = node8;
        int myElement4 = myStructure.ElementCreate("PLANE2D4N",Incidence,"ConstitutiveLawIpNonlocal","StaticDataNonlocal");	
	
	//create constitutive law
	int myMatLin = myStructure.ConstitutiveLawCreate("NonlocalDamagePlasticity");
	myStructure.ConstitutiveLawSetYoungsModulus(myMatLin,10);
	myStructure.ConstitutiveLawSetPoissonsRatio(myMatLin,0.25);
	myStructure.ConstitutiveLawSetNonlocalRadius(myMatLin,1);
	myStructure.ConstitutiveLawSetTensileStrength(myMatLin,2);
	myStructure.ConstitutiveLawSetCompressiveStrength(myMatLin,20);
	myStructure.ConstitutiveLawSetBiaxialCompressiveStrength(myMatLin,25);
	myStructure.ConstitutiveLawSetFractureEnergy(myMatLin,0.2);

	//create section
	myStructure.SectionCreate("mySection","Plane_Strain");
	myStructure.SectionSetThickness("mySection",1);

	//assign constitutive law 
	myStructure.ElementTotalSetConstitutiveLaw(myMatLin);
	myStructure.ElementTotalSetSection("mySection");

	//Build nonlocal elements
	myStructure.BuildNonlocalData(myMatLin);

	// visualize results
	myStructure.AddVisualizationComponentNonlocalWeights(myElement1,0);
	//myStructure.AddVisualizationComponentNonlocalWeights(myElement1,1);
	//myStructure.AddVisualizationComponentNonlocalWeights(myElement1,2);
	//myStructure.AddVisualizationComponentNonlocalWeights(myElement1,3);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement2,0);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement2,1);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement2,2);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement2,3);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement3,0);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement3,1);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement3,2);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement3,3);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement4,0);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement4,1);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement4,2);
	myStructure.AddVisualizationComponentNonlocalWeights(myElement4,3);
	myStructure.ExportVtkDataFile("PlaneNonlocalWeights.vtk");
        
	//calculate the stiffness matrix
	NuTo::FullMatrix<double> Ke;
        NuTo::FullMatrix<int> rowIndex;
    	NuTo::FullMatrix<int> colIndex;
    	myStructure.ElementStiffness(myElement1,Ke,rowIndex,colIndex);
    }
    catch (NuTo::Exception& e)
    {
        std::cout << e.ErrorMessage() << std::endl;
    }
    return 0;
}