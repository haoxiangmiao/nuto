#include "nuto/math/FullMatrix.h"
#include "nuto/mechanics/structures/unstructured/Structure.h"
#include "nuto/mechanics/MechanicsException.h"

#include "nuto/math/SparseDirectSolverMUMPS.h"
#include "nuto/math/SparseMatrixCSRVector2General.h"

#define MAXNUMNEWTONITERATIONS 20
//#define MAXNORMRHS 100
#define PRINTRESULT true

// there is still an error at the very end of the calculation
// uncomment at the end of void NuTo::NonlocalDamagePlasticity::YieldSurfaceRankine2DRoundedDerivatives
// the check for the derivatives of the yield surface to see the problem

int main()
{
try
{
    //
    double lX(100);
    double lY(100);

    //create structure
    NuTo::Structure myStructure(2);
    myStructure.SetShowTime(true);

    NuTo::FullMatrix<int> createdGroupIds;
    myStructure.ImportFromGmsh("/home/unger3/develop/nuto/examples/c++/ImportGmsh.msh","displacements", "ConstitutiveLawIpNonlocal", "StaticDataNonlocal",createdGroupIds);
    myStructure.Info();

    //create constitutive law nonlocal damage
    int myMatDamage = myStructure.ConstitutiveLawCreate("NonlocalDamagePlasticity");
    double YoungsModulusDamage(20000);
    myStructure.ConstitutiveLawSetYoungsModulus(myMatDamage,YoungsModulusDamage);
    myStructure.ConstitutiveLawSetPoissonsRatio(myMatDamage,0.2);
    myStructure.ConstitutiveLawSetNonlocalRadius(myMatDamage,1.5);
    double fct(2);
    myStructure.ConstitutiveLawSetTensileStrength(myMatDamage,fct);
    myStructure.ConstitutiveLawSetCompressiveStrength(myMatDamage,fct*10);
    myStructure.ConstitutiveLawSetBiaxialCompressiveStrength(myMatDamage,fct*12.5);
    myStructure.ConstitutiveLawSetFractureEnergy(myMatDamage,.1);

    //create constitutive law linear elastic (finally not used, since the elements are deleted)
    int myMatLinear = myStructure.ConstitutiveLawCreate("LinearElastic");
    double YoungsModulusLE(1);
    myStructure.ConstitutiveLawSetYoungsModulus(myMatLinear,YoungsModulusLE);
    myStructure.ConstitutiveLawSetPoissonsRatio(myMatLinear,0.2);

    //create section
    double thickness(1);
    int mySectionParticle = myStructure.SectionCreate("Plane_Strain");
    myStructure.SectionSetThickness(mySectionParticle,thickness);

    int mySectionMatrix = myStructure.SectionCreate("Plane_Strain");
    myStructure.SectionSetThickness(mySectionMatrix,thickness);

    //assign constitutive law
    myStructure.ElementGroupSetSection(101,mySectionParticle);
    myStructure.ElementGroupSetSection(102,mySectionMatrix);
    myStructure.ElementGroupSetConstitutiveLaw(101,myMatLinear);
    myStructure.ElementGroupSetConstitutiveLaw(102,myMatDamage);
    bool deleteNodes=true;
    myStructure.ElementGroupDelete(101,deleteNodes);

    myStructure.ElementTotalSetSection(mySectionMatrix);
	myStructure.ElementTotalSetConstitutiveLaw(myMatDamage);

    //Build nonlocal elements
	myStructure.BuildNonlocalData(myMatDamage);

	//Create groups to apply the boundary conditions
	//left boundary
	int GrpNodes_LeftBoundary = myStructure.GroupCreate("Nodes");
	int direction = 0; //either 0,1,2
	double min(0.);
	double max(0.);
	myStructure.GroupAddNodeCoordinateRange(GrpNodes_LeftBoundary,direction,min,max);

	//lower left node
	int GrpNodes_LowerBoundary = myStructure.GroupCreate("Nodes");
	direction=1;
	min=0;
	max=0;
	myStructure.GroupAddNodeCoordinateRange(GrpNodes_LowerBoundary,direction,min,max);
	int GrpNodes_LowerLeftNode = myStructure.GroupIntersection(GrpNodes_LowerBoundary,GrpNodes_LeftBoundary);

	//right boundary
	int GrpNodes_RightBoundary = myStructure.GroupCreate("Nodes");
	direction = 0; //either 0,1,2
	min=lX;
	max=lX;
	myStructure.GroupAddNodeCoordinateRange(GrpNodes_RightBoundary,direction,min,max);


	//fix left support
	NuTo::FullMatrix<double> DirectionX(2,1);
	DirectionX.SetValue(0,0,1.0);
	DirectionX.SetValue(1,0,0.0);

	NuTo::FullMatrix<double>DirectionY(2,1);
	DirectionY.SetValue(0,0,0.0);
	DirectionY.SetValue(1,0,1.0);

	myStructure.ConstraintSetDisplacementNodeGroup(GrpNodes_LeftBoundary,DirectionX, 0);
	myStructure.ConstraintSetDisplacementNodeGroup(GrpNodes_LowerLeftNode,DirectionY, 0);

	// update the RHS of the constrain equation with myStructure.ConstraintSetRHS
	int ConstraintRHS = myStructure.ConstraintSetDisplacementNodeGroup(GrpNodes_RightBoundary,DirectionX, 0);

#ifdef ENABLE_VISUALIZE
    myStructure.AddVisualizationComponentSection();
    myStructure.AddVisualizationComponentConstitutive();
    myStructure.AddVisualizationComponentDisplacements();
    myStructure.AddVisualizationComponentEngineeringStrain();
    myStructure.AddVisualizationComponentEngineeringStress();
    myStructure.AddVisualizationComponentDamage();
    myStructure.AddVisualizationComponentEngineeringPlasticStrain();
	myStructure.ElementTotalUpdateTmpStaticData();
    myStructure.ExportVtkDataFile("ImportGmsh.vtk");
#endif

    // init some result data
    NuTo::FullMatrix<double> PlotData(1,6);
    double externalEnergy(0.);

    // start analysis
    double maxDisp(30*fct/YoungsModulusDamage*lX);
    double deltaDisp(0.02*fct/YoungsModulusDamage*lX);
    double curDisp(0.02*fct/YoungsModulusDamage*lX);

    //update displacement of boundary (disp controlled)
	myStructure.ConstraintSetRHS(ConstraintRHS,curDisp);
	//update conre mat
	myStructure.NodeBuildGlobalDofs();

	//update tmpstatic data with zero displacements
	myStructure.ElementTotalUpdateTmpStaticData();

	//init some auxiliary variables
	NuTo::SparseMatrixCSRVector2General<double> stiffnessMatrixCSRVector2;
	NuTo::FullMatrix<double> dispForceVector;
	NuTo::FullMatrix<double> intForceVector;
	NuTo::FullMatrix<double> extForceVector;
	NuTo::FullMatrix<double> rhsVector;

	//allocate solver
	NuTo::SparseDirectSolverMUMPS mySolver;
	mySolver.SetShowTime(true);

    //calculate stiffness
	myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);

	// build global external load vector and RHS vector
	myStructure.BuildGlobalExternalLoadVector(extForceVector);
	rhsVector = extForceVector + dispForceVector;

	//calculate absolute tolerance for matrix entries to be not considered as zero
	double maxValue, minValue, ToleranceZeroStiffness;
	stiffnessMatrixCSRVector2.Max(maxValue);
	stiffnessMatrixCSRVector2.Min(minValue);
	std::cout << "min and max " << minValue << " , " << maxValue << std::endl;

	ToleranceZeroStiffness = (1e-14) * (fabs(maxValue)>fabs(minValue) ?  fabs(maxValue) : fabs(minValue));
	myStructure.SetToleranceStiffnessEntries(ToleranceZeroStiffness);
	int numRemoved = stiffnessMatrixCSRVector2.RemoveZeroEntries(ToleranceZeroStiffness,0);
	int numEntries = stiffnessMatrixCSRVector2.GetNumEntries();
	std::cout << "stiffnessMatrix: num zero removed " << numRemoved << ", numEntries " << numEntries << std::endl;

	//update displacements of all nodes according to the new conre mat
	{
	    NuTo::FullMatrix<double> displacementsActiveDOFsCheck;
	    NuTo::FullMatrix<double> displacementsDependentDOFsCheck;
	    myStructure.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
	    myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
	    myStructure.ElementTotalUpdateTmpStaticData();
	}

	//repeat until max displacement is reached
	while (curDisp<maxDisp)
    {

        double normResidual(1);
        double maxResidual(1);
        int numNewtonIterations(0);
		double normRHS(1.);
		double alpha(1.);
		int convergenceStatus(0);
		//0 - not converged, continue Newton iteration
		//1 - converged
		//2 - stop iteration, decrease load step
		while(convergenceStatus==0)
		{
			numNewtonIterations++;

			if (numNewtonIterations>MAXNUMNEWTONITERATIONS)
			{
				if (PRINTRESULT)
		    	{
				    std::cout << "numNewtonIterations (" << numNewtonIterations << ") > MAXNUMNEWTONITERATIONS (" << MAXNUMNEWTONITERATIONS << ")" << std::endl;
		    	}
				convergenceStatus = 2; //decrease load step
				break;
			}

			normRHS = rhsVector.Norm();

			/*if (normRHS>MAXNORMRHS)
			{
				if (PRINTRESULT)
		    	{
				    std::cout << "normRHs (" << normRHS << ") > MAXNORMRHS (" << MAXNORMRHS << ")" << std::endl;
		    	}
				convergenceStatus = 2; //decrease load step
				break;
			}*/

			// solve
			NuTo::FullMatrix<double> deltaDisplacementsActiveDOFs;
			NuTo::FullMatrix<double> oldDisplacementsActiveDOFs;
			NuTo::FullMatrix<double> displacementsActiveDOFs;
			NuTo::FullMatrix<double> displacementsDependentDOFs;
			NuTo::SparseMatrixCSRGeneral<double> stiffnessMatrixCSR(stiffnessMatrixCSRVector2);
			stiffnessMatrixCSR.SetOneBasedIndexing();
			mySolver.Solve(stiffnessMatrixCSR, rhsVector, deltaDisplacementsActiveDOFs);

			// write displacements to node
			myStructure.NodeExtractDofValues(oldDisplacementsActiveDOFs, displacementsDependentDOFs);

			//perform a linesearch
			alpha = 1.;
			do
			{
				//add new displacement state
				displacementsActiveDOFs = oldDisplacementsActiveDOFs + deltaDisplacementsActiveDOFs*alpha;
				myStructure.NodeMergeActiveDofValues(displacementsActiveDOFs);
				myStructure.ElementTotalUpdateTmpStaticData();

				// calculate residual
				myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector);
				rhsVector = extForceVector - intForceVector;
				normResidual = rhsVector.Norm();
				std::cout << "alpha " << alpha << ", normResidual " << normResidual << ", normResidualInit "<< normRHS << ", normRHS*(1-0.5*alpha) " << normRHS*(1-0.5*alpha) << std::endl;
				alpha*=0.5;
			}
			while(alpha>1e-3 && normResidual>normRHS*(1-0.5*alpha));

		    maxResidual = rhsVector.Max();

			std::cout << std::endl << "Newton iteration " << numNewtonIterations << ", final alpha " << 2*alpha << ", normResidual " << normResidual<< ", maxResidual " << maxResidual<<std::endl;
			//char cDummy[100]="";
			//std::cin.getline(cDummy, 100);

			//check convergence
			if (normResidual<1e-5 || maxResidual<1e-5)
			{
				if (PRINTRESULT)
		    	{
					std::cout << "Convergence after " << numNewtonIterations << " Newton iterations, cur_disp " << curDisp << ", delta disp "<< deltaDisp << std::endl<< std::endl;
		    	}
				convergenceStatus=1;
				break;
			}

			//convergence status == 0 (continue Newton iteration)
			//build new stiffness matrix
			myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);
			int numRemoved = stiffnessMatrixCSRVector2.RemoveZeroEntries(ToleranceZeroStiffness,0);
			int numEntries = stiffnessMatrixCSRVector2.GetNumEntries();
			std::cout << "stiffnessMatrix: num zero removed " << numRemoved << ", numEntries " << numEntries << std::endl;
		}

		if (convergenceStatus==1)
		{
			myStructure.ElementTotalUpdateStaticData();

			//store result/plot data
			NuTo::FullMatrix<double> SupportingForce;
			myStructure.NodeGroupInternalForce(GrpNodes_RightBoundary,SupportingForce);
			NuTo::FullMatrix<double> SinglePlotData(1,6);
			SinglePlotData(0,0) = curDisp;
			SinglePlotData(0,1) = SupportingForce(0,0)/(thickness*lY);
			SinglePlotData(0,2) = SupportingForce(0,0);
		    externalEnergy+=deltaDisp*SupportingForce(0,0);

			PlotData.AppendRows(SinglePlotData);
		    PlotData.WriteToFile("ImportGmshLoadDisp.txt"," ","#load displacement curve, disp, stress, force, sxx in center element, syy in center element","  ");

			// visualize results
#ifdef ENABLE_VISUALIZE
			myStructure.ExportVtkDataFile("ImportGmsh.vtk");
#endif
			if (numNewtonIterations<MAXNUMNEWTONITERATIONS/3)
				deltaDisp*=1.5;

			//increase displacement
			curDisp+=deltaDisp;
			if (curDisp>maxDisp)
				curDisp=maxDisp;

			//old stiffness matrix is used in first step of next load increment in order to prevent spurious problems at the boundary
			std::cout << "press enter to next load increment" << std::endl;
			char cDummy[100]="";
			std::cin.getline(cDummy, 100);;
		}
		else
		{
            assert(convergenceStatus==2);
			//calculate stiffness of previous loadstep (used as initial stiffness in the next load step)
			//this is done within the loop in order to ensure, that for the first step the stiffness matrix of the previous step is used
			//otherwise, the additional boundary displacements will result in an artifical localization in elements at the boundary
			curDisp-=deltaDisp;
			myStructure.ConstraintSetRHS(ConstraintRHS,curDisp);

			// build global dof numbering
			myStructure.NodeBuildGlobalDofs();

			//set previous converged displacements
			NuTo::FullMatrix<double> displacementsActiveDOFsCheck;
			NuTo::FullMatrix<double> displacementsDependentDOFsCheck;
			myStructure.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
			myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
			myStructure.ElementTotalUpdateTmpStaticData();

			// calculate previous residual (should be almost zero)
			myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector);

			//decrease load step
			deltaDisp*=0.5;
			curDisp+=deltaDisp;

			//check for minimum delta (this mostly indicates an error in the software
			if (deltaDisp<1e-7)
				throw NuTo::MechanicsException("Example ImportGmsh : No convergence, delta disp < 1e-7");

			std::cout << "press enter to reduce load increment" << std::endl;
			char cDummy[100]="";
			std::cin.getline(cDummy, 100);;
		}
		//update new displacement of RHS
		myStructure.ConstraintSetRHS(ConstraintRHS,curDisp);

		// build global dof numbering
		myStructure.NodeBuildGlobalDofs();

		//update stiffness in order to calculate new dispForceVector
		myStructure.BuildGlobalCoefficientMatrix0(stiffnessMatrixCSRVector2, dispForceVector);
		int numRemoved = stiffnessMatrixCSRVector2.RemoveZeroEntries(ToleranceZeroStiffness,0);
		int numEntries = stiffnessMatrixCSRVector2.GetNumEntries();
		std::cout << "stiffnessMatrix: num zero removed " << numRemoved << ", numEntries " << numEntries << std::endl;

		//update rhs vector for next Newton iteration
		rhsVector = dispForceVector + extForceVector - intForceVector;

        //update displacements of all nodes according to the new conre mat
		NuTo::FullMatrix<double> displacementsActiveDOFsCheck;
		NuTo::FullMatrix<double> displacementsDependentDOFsCheck;
		myStructure.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
		myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
		myStructure.ElementTotalUpdateTmpStaticData();
    }
	if (PRINTRESULT)
        std::cout<< "numerical fracture energy "<< externalEnergy/(thickness*lY) << std::endl;
}
catch (NuTo::Exception& e)
{
    std::cout << e.ErrorMessage() << std::endl;
}
return 0;
}

/*
            this is a test routine for the element stiffness matrices (each single element is tested separately)
			if (numNewtonIterations==-1)
			{
			    double delta(1e-9);
			    NuTo::FullMatrix<double> difference;
				int maxElement = myStructure.ElementTotalCoefficientMatrix_0_Check(delta,difference);
				double maxError = difference.Abs().Max();
				std::cout << "maximum error " << maxError << " in element " << maxElement << std::endl;

				if (maxError>5e-1)
				{
					myStructure.SetVerboseLevel(10);
					NuTo::FullMatrix<double> analyticStiffness;
					NuTo::FullMatrix<double> finiteDifferencesStiffness;
					NuTo::FullMatrix<int> dofsRow, dofsCol;
					myStructure.ElementCoefficientMatrix_0(maxElement,analyticStiffness,dofsRow,dofsCol);
					std::cout << "analytic solution " << std::endl << analyticStiffness << std::endl << std::endl;
					finiteDifferencesStiffness.Resize(analyticStiffness.GetNumRows(), analyticStiffness.GetNumColumns());
					myStructure.ElementCoefficientMatrix_0_Resforce(maxElement,delta,finiteDifferencesStiffness);
					std::cout << "finiteDifferencesStiffness " << std::endl << finiteDifferencesStiffness << std::endl << std::endl;

					std::cout << "relative difference " << std::endl << difference << std::endl << std::endl;

					//todo check principal stresses and yield functions for all integration points

					std::cout << std::endl << "check for different delta" << std::endl;

					std::cout << "delta " << delta*100 << std::endl;
					myStructure.ElementCoefficientMatrix_0_Check(maxElement,delta*100,difference);
					std::cout << "maximum error " << difference.Abs().Max() << std::endl;
					if (maxError>difference.Abs().Max())
						maxError = difference.Abs().Max();

					std::cout << "delta " << delta*10 << std::endl;
					myStructure.ElementCoefficientMatrix_0_Check(maxElement,delta*10,difference);
					std::cout << "maximum error " << difference.Abs().Max() << std::endl;
					if (maxError>difference.Abs().Max())
						maxError = difference.Abs().Max();

					std::cout << "delta " << delta*1 << std::endl;
					myStructure.ElementCoefficientMatrix_0_Check(maxElement,delta*1,difference);
					std::cout << "maximum error " << difference.Abs().Max() << std::endl;
					if (maxError>difference.Abs().Max())
						maxError = difference.Abs().Max();

					std::cout << "delta " << delta*.1 << std::endl;
					myStructure.ElementCoefficientMatrix_0_Check(maxElement,delta*.1,difference);
					std::cout << "maximum error " << difference.Abs().Max() << std::endl;
					if (maxError>difference.Abs().Max())
						maxError = difference.Abs().Max();

					std::cout << "delta " << delta*0.01 << std::endl;
					myStructure.ElementCoefficientMatrix_0_Check(maxElement,delta*0.01,difference);
					std::cout << "maximum error " << difference.Abs().Max() << std::endl;
					if (maxError>difference.Abs().Max())
						maxError = difference.Abs().Max();

                    if (maxError>1e-2)
                    {
						std::cout << "maximum error in stiffness is bigger than tolerance" << std::endl;
						char cDummy[100]="";
						std::cin.getline(cDummy, 100);
                    }
				}
			}

			if (numNewtonIterations==-2)
			{
				// for the first step, the stiffness matrix from the previous step is used (no boundary conditions modified) that's why check only the second step
				bool oldShowTime(myStructure.GetShowTime());
				myStructure.SetShowTime(false);
				//check stiffness matrix
				NuTo::FullMatrix<double> KeFull(stiffnessMatrixCSRVector2);
				double delta=1e-10;
				NuTo::FullMatrix<double> displacementsActiveDOFsCheck;
				NuTo::FullMatrix<double> displacementsDependentDOFsCheck;
				myStructure.NodeExtractDofValues(displacementsActiveDOFsCheck, displacementsDependentDOFsCheck);
				NuTo::FullMatrix<double> intForceVector1;
				NuTo::FullMatrix<double> intForceVector2;
				myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector1);
				NuTo::FullMatrix<double> KeApprox(stiffnessMatrixCSRVector2.GetNumRows(),stiffnessMatrixCSRVector2.GetNumColumns());
				for (int theDof=0;  theDof<displacementsActiveDOFsCheck.GetNumRows(); theDof++)
				{
					displacementsActiveDOFsCheck(theDof,0) +=delta;
					myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
					myStructure.ElementTotalUpdateTmpStaticData();
					myStructure.BuildGlobalGradientInternalPotentialVector(intForceVector2);
					KeApprox.SetColumn(theDof,(intForceVector2-intForceVector1)*(1./delta));
					displacementsActiveDOFsCheck(theDof,0) -=delta;
					myStructure.NodeMergeActiveDofValues(displacementsActiveDOFsCheck);
					myStructure.ElementTotalUpdateTmpStaticData();
				}
				NuTo::FullMatrix<double> errorMatrix((KeApprox-KeFull).Abs());
				for (int theRow=0; theRow<errorMatrix.GetNumRows(); theRow++)
				{
					for (int theColumn=0; theColumn<errorMatrix.GetNumColumns(); theColumn++)
					{
						if (fabs(KeApprox(theRow,theColumn))>1.)
					    	errorMatrix(theRow,theColumn)/=fabs(KeApprox(theRow,theColumn));
					}
				}

				int row(-1),col(-1);
				double maxError = errorMatrix.Max(row,col);
//				if (errorMatrix.Max(row,col)>1e-1)
				{
					//std::cout << "orig " << KeFull.GetBlock(0,0,9,9) << std::endl << std::endl;
					//std::cout << "approx " << KeApprox.GetBlock(0,0,9,9) << std::endl << std::endl;
					//std::cout << "errorMatrix " << errorMatrix.GetBlock(0,0,9,9) << std::endl << std::endl;
					//std::cout << "orig " << KeFull << std::endl << std::endl;
					//std::cout << "approx " << KeApprox << std::endl << std::endl;
					//std::cout << "delta " << errorMatrix << std::endl << std::endl;
					//std::cout << "delta scaled " << errorMatrix << std::endl << std::endl;
					std::cout << "error " << maxError << " at " << row << "," << col << std::endl << std::endl;
					std::cout << "exact and approx at this entry " << KeFull(row,col) << " " << KeApprox(row,col) << std::endl << std::endl;
					std::cout << "curDisp " << curDisp << std::endl;
					//throw NuTo::MechanicsException("ImportGmsh - stiffness matrix is not correct.");
					//char cDummy[100]="";
					//std::cin.getline(cDummy, 100);
				}
				myStructure.SetShowTime(oldShowTime);
			}
*/