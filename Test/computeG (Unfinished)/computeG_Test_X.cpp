#include <iostream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <string>
using namespace std;

#include "Material.h"
#include "Pipe.h"
#include "TestSetup.h"
#include "Control.h"
#include "FDprofile.h"
#include "AnalyticalProfile.h"
#include "Constants.h"
#include "SymDoubleMatrix.h"
#include "OutflowProcess.h"
#include "ConfigFile.h"
#include "GaDotSolutionFD.h"

//Initialise global variables
double aDotc0;
double g0;
double densityBackfillRatio;
double gG0;

short maxIterations = 100;

ofstream resultsFile, profileFile;

//Prototype function to be tested
void computeG(const Material material, const Pipe pipe, const TestSetup testSetup, const Control control);

//Test function
int main ()
{
	//Read in congfigfile values for:
	ConfigFile config("caseInputData.txt");
	
	Material material(6, config);
	Pipe pipe(6, config);
	TestSetup testSetup(6, config);
	Control control(6, config);
	
	material.outputData();
	pipe.outputData();
	testSetup.outputData();
	control.outputData();
	
	computeG(material, pipe, testSetup, control);	
}

//  Computes decompression ratio at crack tip using Maxey guillotine model
double p1p0(double p0, double gamma_, double aDotC0)

{
	double p1;
	if (aDotC0 < 0)
		return (0.0);
	if (aDotC0 > 1)
		return (1.0);
	double argument = 1 - (gamma_ - 1) / (gamma_ + 1) * (1 - aDotC0);
	p1 = (p0 + Constants::pAtm) * pow(argument, 2 * gamma_ / (gamma_ - 1)) - Constants::pAtm;
	if (p1 < 0.0)
		p1 = 0.0;
	return (p1 / p0);
} 

void computeG(const Material material, const Pipe pipe, const TestSetup testSetup, const Control control)
{
	
//	Account for reduction in air volume inside pipe due to internal solid components and/or water
	double availableInternalVolume = 1.0 - testSetup.waterInsidePipe - testSetup.solidInsidePipe;

//	For FS configuration, compute prior decompression:
	double p1bar = testSetup.p0bar;
	double decompression = 1.0;		// Proportion of initial pressure which remains at crack tip
	if (testSetup.fullScale)
	{
		availableInternalVolume = 1.0;
		decompression = p1p0(testSetup.p0bar, Constants::gamma, aDotc0);
		p1bar = p1bar * decompression;
	}

//	Factors for modelling of S4 baffle leakage (FIXME)
	double baffleLeakageArea = pipe.diameter / Constants::kilo;		// m
	baffleLeakageArea = baffleLeakageArea * baffleLeakageArea * 0.01 * Constants::pi * (pipe.sdr - 2.0) / pipe.sdr;  // m^2
	
//	Calculate natural diameter of pipe dues to residual strain contraction in time scale of fracture
	double creepModulusRatio = material.creepModulus / material.dynamicModulus;
	double diameterRes0 = pipe.diameter / ((1.0 - creepModulusRatio) + creepModulusRatio / pipe.diameterCreepRatio);
	double residualCrackClosure = (pipe.diameter - diameterRes0) * Constants::pi;	// (mm)
	
//	Parameters for equivalent beam model
	double dynamicShearModulus = material.dynamicModulus / 2.0 / (1.0 + material.poisson);
	double aDotOverCL = aDotc0 * Constants::vSonic / sqrt(material.dynamicModulus * Constants::giga / material.density);
	double aDotCLfactor = 1.0 + aDotOverCL * aDotOverCL;
	double aDotCLfactor_backfilled = 1.0 + aDotOverCL * aDotOverCL * densityBackfillRatio;
	double sdrMinus1 = pipe.sdr - 1.0;
	double sdrMinus2 = sdrMinus1 - 1.0;
	
	double factor = Constants::pi * Constants::c1 * 625.0 * material.dynamicModulus / p1bar * availableInternalVolume * sdrMinus2 / sdrMinus1 / sdrMinus1 * aDotc0;	// Note GPa / bar / 16 = 625
	
	double v00 = 0.4 / Constants::c1 * sdrMinus1 * sdrMinus2 / pipe.sdr * p1bar / material.dynamicModulus * pipe.diameter / Constants::mega;	//	(m)
//	v00 becomes reference length v0 on multiplying by lambda^4

	double alpha[2];						//  [0] with backfill or [1] without
	double m[2];							//  [0] with backfill or [1] without

	double zetaClosure = 2.0;				//	First guess for point behind crack front at which crack re-closes (in outflow lengths)
	double nodeResolution = 0.5 / control.elementsInL;	// used as a reference tolerance
	short nodeAtClosure = short(zetaClosure * control.elementsInL);
	double lambdaLast;
	double zetaBackfilledLast;
	short notConverged = 1;
	short iterations = 0;
	short noCrackOpening = 0;
	double wStar2, wStarMax, wStar2dash, wStar2dash2, integral_wStar2;
	double zetaBackfilled = 0.2;			// first guess
	double zetaBackfillEject;
	
//  Set initial outflow length
	double outflowLength = control.lambda;
	double lambdaPow4 =  pow(outflowLength, 4);
	
//	Dimensionless virtual crack opening at crack tip (representing residual strain)
	double v0 = v00 * lambdaPow4;			//	(m)
	double vStarRes = residualCrackClosure / v0 / Constants::kilo;
	
	if (control.outflowModelOn and control.infoLevel > 1)
		cout << "Starting outflowLength refinement with outflow length = " << outflowLength << endl;
	do
	{	// Refine given outflow length using the discharge analysis:
		lambdaLast = outflowLength;
		zetaBackfilledLast = zetaBackfilled;
		
//	Dimensionless foundation stiffness [0] with backfill and [1] without
	    m[1] = 8.0 / 3.0 / Constants::pi / Constants::c1 / sdrMinus1 / sdrMinus1 * lambdaPow4;
		m[0] = m[1] / aDotCLfactor_backfilled;
		m[1] /= aDotCLfactor;
		
//	Dimensionless crack speed [0] with backfill and [1] without
		alpha[1] = outflowLength * sqrt(Constants::c3 * aDotOverCL * aDotOverCL + Constants::c4 / 2.0 / (1.0 + material.poisson) / sdrMinus1 / sdrMinus1 / aDotCLfactor);
		alpha[0] = outflowLength * sqrt(Constants::c3 * aDotOverCL * aDotOverCL * densityBackfillRatio + Constants::c4 * 0.5 / (1.0 + material.poisson) / sdrMinus1 / sdrMinus1 / aDotCLfactor_backfilled);

//	Compute the v*(zeta) profile (either numerically or analytically) from alpha, m, and vStarRes and zetaBackfilled:

//	Determine (either by analytical or FD method) the opening profile v*(zeta) for a given outflow length control.lambda and the properties of it which are needed for analysis.
	if (not control.analyticalSolutionMode)
	{	// ...then use FINITE DIFFERENCE solution method:
//		short waitForMe;								
// Make first guess for closure length -- two nodes less than input value -- and construct FD solution:
		nodeAtClosure -= 2;
		double wStarMax = 0.0;
		FDprofile fdSolution(alpha, m, zetaBackfilled, vStarRes, control.elementsInL, nodeAtClosure);
		cout << "first guess profile: \n";
		fdSolution.printProfile();
		short nodeAtClosure_previous = nodeAtClosure;				// Store position of last node in this FD array
		double errorLast = fdSolution.closureMoment();				// ...and resulting d2v/dz2 at closure point, divided by that at crack tip
		if (control.infoLevel > 1)
			cout << "    Starting closure length refinement with closure node = " << nodeAtClosure_previous << " closure moment = " <<  errorLast << endl;
// Make second guess for closure length (two nodes MORE than input value):
		nodeAtClosure += 4;
		if (control.infoLevel > 1)
			cout << "    Second-guess closure node = " << nodeAtClosure << endl;
// Prepare to refine closure length by iteration
		double dontNeedThis;
		double error;
		short notConverged = 1;
		short iterations = 0;
		short maximumNonContact = 0;
		do
		{	// ...until bending moment at closure point is negligible compared to that at crack tip:
			fdSolution = FDprofile(alpha, m, zetaBackfilled, vStarRes, control.elementsInL, nodeAtClosure);	
			error = fdSolution.closureMoment();
			short noSurfaceContact = 1;								// FIXME:  necessary?
			short minPoint = fdSolution.nodeAtMinimum();			// this is set to -1 if NO minimum is found within domain
			if (control.infoLevel > 1)
				cout << "    At closure length iteration " << iterations << " with nodeAtClosure = " << nodeAtClosure << " closure moment = " <<  error << " min at point " << minPoint << endl;
			if (minPoint > 0)
			{
				if (control.infoLevel > 1)
					cout << "      BUT there's a minimum (crack surface overlap) to left of closure point " << minPoint << endl;
// So back up to find maximum closure length with NO overlap:
				double tempError;
				nodeAtClosure = minPoint - 2;
				if (nodeAtClosure < (control.elementsInL + 1))
					nodeAtClosure = control.elementsInL + 1;	// Is this really necessary?  Closure might really occur inside decompression length!
				short newMin;
				do
				{
					fdSolution = FDprofile(alpha, m, zetaBackfilled, vStarRes, control.elementsInL, nodeAtClosure);
					tempError = fdSolution.closureMoment();
					newMin = fdSolution.nodeAtMinimum();
					if (control.infoLevel > 1)
						cout <<	"    nodeAtClosure = " << nodeAtClosure << " error = " << tempError << " min point = " << newMin << endl;
					nodeAtClosure++;
				}
				while (newMin < 0);
				nodeAtClosure = nodeAtClosure - 1;
				if (control.infoLevel > 1)
					cout << "least worst non-contacting solution nodeAtClosure = " << nodeAtClosure << endl;
				maximumNonContact = true;
				fdSolution = FDprofile(alpha, m, -1.0, vStarRes, control.elementsInL, nodeAtClosure);
		
				fdSolution.printProfile();
				
				error = fdSolution.closureMoment();
//				wStar2 = fdSolution.wStar2();
//				wStar2dash = fdSolution.wStar2dash();
//				wStar2dash2 = fdSolution.wStar2dash2();
				integral_wStar2 = fdSolution.integral_wStar2();
				fdSolution.findBackfillEjectPoint(zetaBackfillEject, dontNeedThis);
//				cout << "    Enter a digit to continue: "; cin >> waitForMe;
			}
			else
			{// If there is no internal minimum, get a new closure length by linear interpolation:
//				if (control.infoLevel > 1)
//					cout << "    nodeAtClosure_previous " << nodeAtClosure_previous <<  " errorLast "  << errorLast <<  " nodeAtClosure "  << nodeAtClosure <<  " error " << error << endl;
//				nodeAtClosure = short((errorLast * float(nodeAtClosure) - error * float(nodeAtClosure_previous)) / (errorLast - error) + 0.5);
nodeAtClosure++;
				if (control.infoLevel > 1)
					cout << "node interpolated = " << nodeAtClosure;
			}
// fdSolution.printProfile();
			if (maximumNonContact)	// not necessarily converged, but the best available
				notConverged = 0;
			else if ((fabs(error) < 0.01 and noSurfaceContact) or maximumNonContact)
				{	// converged:
					notConverged = false;
					fdSolution.outflowPointValues(wStar2, wStar2dash, wStar2dash2, integral_wStar2);	
					fdSolution.findBackfillEjectPoint(zetaBackfillEject, dontNeedThis);
					wStarMax = fdSolution.wStarMax(nodeAtClosure);
				}
				else
				{
					if (control.infoLevel > 1)
						cout << "Next try for iteration will be nodeAtClosure = " << nodeAtClosure << endl;
					errorLast = error;
					iterations++;
//					cout << "    Enter a digit to continue iteration: "; cin >> waitForMe;
				}
		} // done that refinement iteration
		while (notConverged & (iterations < maxIterations));
		if (control.infoLevel > 1)
			cout << "    ** At nodeAtClosure = " << nodeAtClosure << " converged in " << iterations << " iterations with error = " << error << endl;
// cout << "    enter digit: ";	cin >> waitForMe;
	} // done FD solution
	
	
	else
	{ // i.e. control.analyticalSolutionMode = 1 or 2, the ANALYTICAL solutions:
		AnalyticalProfile psl(alpha, m, testSetup, control);
		psl.outflowPointValues(wStar2, wStar2dash, wStar2dash2, integral_wStar2);
		zetaBackfillEject = 1.0;
		nodeAtClosure = 0;
		if (control.infoLevel > 1)
			cout << " Computed profile properties.  Opening at outflow point = " << wStar2 << "; 1st-deriv at outflow = " << wStar2dash << "; 2nd-deriv at outflow = " << wStar2dash2 << "; integral to outflow = " << integral_wStar2 << endl;
	} // done analytical solution
			if (control.infoLevel > 1)
			cout << " Computed profile properties.  Ejection point = " << zetaBackfillEject << "; opening at outflow point = " << "; max opening = " << wStarMax << "; 1st-deriv at outflow = " << wStar2dash << "; 2nd-deriv at outflow = " << wStar2dash2 << "; integral to outflow = " << integral_wStar2 << endl;
		if (integral_wStar2 > 0.0)
		{
			if (control.outflowModelOn)
			{
				double throatArea = integral_wStar2;
//	Experimental compensation for baffle leakage
//				throatArea += baffleLeakageArea / v0 * outflowLength / pipe.diameter * Constants::kilo * pipe.sdr / (pipe.sdr - 1.0);
				OutflowProcess outflow(p1bar);
				outflowLength = pow(factor * outflow.get_tStarOutflow() / throatArea, 0.2);
			}
//	tStarOutflow being the number of characteristic times for discharge
			if (control.infoLevel > 1)
				cout << " alpha = " << alpha[1] << " m = " << m[1] << " integral_wStar2 = " << integral_wStar2 << " New outflowLength = " << outflowLength << endl;
			lambdaPow4 =  pow(outflowLength, 4);
			v0 = v00 * lambdaPow4;
			vStarRes = residualCrackClosure / v0 / Constants::kilo;
			if ((fabs(1.0 - lambdaLast / outflowLength) < nodeResolution) and (fabs(1.0 - zetaBackfilledLast / zetaBackfilled) < nodeResolution))
				notConverged = false;
			iterations++;
			short waitForMe;
			if (control.infoLevel > 2)
			{
				cout << "enter digit: ";	
				cin >> waitForMe;
			}
		}
		else
			noCrackOpening = 1;
	} // end outflow length refinement
	while (notConverged & iterations < maxIterations and not noCrackOpening);
	
//	So we now have the correct numerical or analytical crack opening profile vStar(zeta), and can output it if needed:
	double vStarDashBackfillEject;
	if (not noCrackOpening)
	{
		if (control.infoLevel > 1)
			cout << "Final outflowLength convergence in " << iterations << " iterations for outflowLength = " << outflowLength << " . \n\n";
		short printOpeningProfile = 1;
		if (printOpeningProfile)
		{
			if (not control.analyticalSolutionMode)
			{	// then recalculate and print the numerical solution
				FDprofile final(alpha, m, zetaBackfilled, vStarRes, control.elementsInL, nodeAtClosure);
				final.printProfile();
				final.findBackfillEjectPoint(zetaBackfillEject, vStarDashBackfillEject);
				final.outflowPointValues(wStar2, wStar2dash, wStar2dash2, integral_wStar2);
				cout << "with v0 = " << v0 << endl;
			}
		else
			{	// print the analytical solution
				AnalyticalProfile final(alpha, m, testSetup, control);
				final.printProfile();
			}
		}
		
		wStar2 *= v0;
//	Flaring of pipe wall at decompression point:
		double deltaDStar = wStar2 / Constants::pi / pipe.diameter * Constants::kilo + diameterRes0 / pipe.diameter - 1.0;

//  Compute contributions to crack driving force:
		double crackWidth = pipe.diameter / pipe.sdr - pipe.notchDepth;			//	(mm, giving kJ/m2 for G;  not necessarily equal to h)
		
//	GS1 due to residual strain bending energy in pipe wall ahead of crack:
		double gS1 = (pipe.diameter - diameterRes0);			// (mm)
		gS1 = Constants::pi / 6.0 * material.dynamicModulus * Constants::kilo / pow(sdrMinus1, 3) * gS1 * gS1 / crackWidth;
		double hOverR = 2.0 / sdrMinus1;
		double h = pipe.diameter / pipe.sdr / Constants::kilo;	// (m)
		double radius = h / hOverR;						// (m)
		
// Convert derivatives from vStar(zeta) to v(z)
		wStar2dash *= v0 / 2.0 / outflowLength / radius;
		wStar2dash2 *= v0 / pow(2.0 * outflowLength * radius, 2);
		
//  GUE due to work done on flaps (= dUE/da / crack width):
		double gUE = (radius - 0.5 * h) * p1bar * Constants::bar * v0 * integral_wStar2  / crackWidth;
		
//  GSb due to strain energy at decompression point behind crack:
		double gSb = 0.5 * material.dynamicModulus * Constants::giga *  (pow(hOverR, 3) / 24.0 / Constants::pi * wStar2 * wStar2 + Constants::c1 * pow(radius, 3) * h * wStar2dash2 * wStar2dash2);
		gSb = (gSb + Constants::pi / 48.0 * dynamicShearModulus * Constants::giga * radius * h * pow(hOverR * wStar2dash, 2)) / crackWidth;
		
//  GKb due to kinetic energy at decompression point behind crack:
		double gKb = 0.5 * material.dynamicModulus * Constants::giga * radius * h * aDotOverCL * aDotOverCL * 
				(Constants::c2 * wStar2dash * wStar2dash + Constants::c1 * pow(wStar2dash2 * radius, 2)) / crackWidth;
		if (not control.analyticalSolutionMode)
		{//  ... account for kinetic energy of detached backfill mass: 
			factor = vStarDashBackfillEject * v0 * aDotc0 * Constants::vSonic * pipe.sdr / 2.0 / sdrMinus1 / outflowLength;
			gKb -= Constants::pi * testSetup.backfillDensity  * pow(factor, 2) * log(1.0 + 2.0 * testSetup.backfillDepth * double(testSetup.isBackfilled)/ pipe.diameter) / Constants::kilo;
		}
//  Total G:
		double gTotal = gUE + g0 + gS1 - gSb - gKb;
		if (gTotal < 0.0)
			gTotal = 0.0;
		gG0 = gTotal / g0;
		cout << setw(5) << setprecision(3) << aDotc0 << " " 
			<< setw(9) << setprecision(4) << decompression << " " 
			<< setw(9) << setprecision(4) << alpha[1] << " " 
			<< setw(9) << setprecision(4) << m[1] << " " 
			<< setw(9) << setprecision(4) << outflowLength << " "
			<< setw(9) << setprecision(4) << deltaDStar * 100 << " " 
			<< setw(9) << setprecision(4) << gS1 / g0 << " "
			<< setw(9) << setprecision(4) << gUE / g0 << " "
			<< setw(9) << setprecision(4) << gSb / g0 << " "
			<< setw(9) << setprecision(4) << gKb / g0 << " "
			<< setw(9) << setprecision(4) << gG0 << " * "
			<< setw(9) << setprecision(4) << gTotal << " kJ/m^2 * ";
			
		if (iterations == maxIterations)
			cout << " UNCONVERGED \n";
		else
			cout << endl;
		resultsFile << setw(8) << aDotc0 * Constants::vSonic << "\t" << gG0 << "\t" <<  gTotal << "\n";
	}
	else
	{
			cout << setw(5) << setprecision(3) << aDotc0 << " " 
			<< setw(9) << setprecision(4) << decompression << " " 
			<< setw(9) << setprecision(4) << alpha[1] << " " 
			<< setw(9) << setprecision(4) << m[1] << " No crack opening.\n";
	}
} // end computeG


