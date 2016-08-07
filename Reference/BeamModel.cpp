#include "BeamModel.h"
short BeamModel::maxIterations = 100;


BeamModel::BeamModel()
{//	Null constructor
		g0 = 0.0;
} // end


BeamModel::BeamModel(const Material material, const Pipe pipe, const TestSetup testSetup, const Control control, const double aDotc0)
{//	Constructor
	g0 = (pipe.sdr - 2) * testSetup.p0bar;
	g0 = Constants::pi / 800.0/ Constants::kilo * g0 * g0 / material.dynamicModulus * pipe.diameter * (1.0 - 1.0 / pipe.sdr);		// kJ/m2
	cout << "                     Irwin-Corten crack driving force = " << g0 << "kJ/m^2 \n";
		
	
	
	densityBackfillRatio = 1.0 + testSetup.backfillDensity / material.density * pipe.sdr / 2.0 * ((1.0 + 2.0 * testSetup.backfillDepth * double(testSetup.isBackfilled) / pipe.diameter) - 1.0);

//	For FS configuration, prior decompression:
	p1bar = testSetup.p0bar;
	decompression = 1.0;
	double availableInternalVolume = 1.0 - testSetup.waterInsidePipe - testSetup.solidInsidePipe;	// Proportion of internal volume available for expansion
	if (testSetup.fullScale)
	{
		availableInternalVolume = 1.0;
		decompression = p1p0(testSetup.p0bar, Constants::gamma, aDotc0);
		p1bar = p1bar * decompression;
	}
//	Calculate natural diameter of pipe due to residual strain contraction in time scale of fracture
	double creepModulusRatio = material.creepModulus/material.dynamicModulus;
	diameterRes0 = pipe.diameter / ((1.0 - creepModulusRatio) + creepModulusRatio / pipe.diameterCreepRatio);
	residualCrackClosure = (pipe.diameter - diameterRes0) * Constants::pi;	// (mm)
	
//  Parameters for equivalent beam model
	dynamicShearModulus = material.dynamicModulus / 2.0 / (1.0 + material.poisson);
	aDotOverCL = aDotc0 * Constants::vSonic / Constants::kilo / sqrt(material.dynamicModulus * Constants::kilo / material.density);
	aDotCLfactor = 1.0 + aDotOverCL * aDotOverCL;
	aDotCLfactor_backfilled = 1.0 + aDotOverCL * aDotOverCL * densityBackfillRatio;
	dStarMinus1 = pipe.sdr - 1.0;
	dStarMinus2 = dStarMinus1 - 1.0;
	
	factor = Constants::pi * Constants::c1 * 625.0 * material.dynamicModulus / p1bar * availableInternalVolume * dStarMinus2 / pow(dStarMinus1, 2) * aDotc0;	// Note GPa / bar / 16 = 625
	
//  v00 becomes reference length v0 on multiplying by lambda^4	
	v00 = 0.4 / Constants::c1 / aDotCLfactor * dStarMinus1 * dStarMinus2 / pipe.sdr * p1bar / material.dynamicModulus * pipe.diameter / Constants::kilo / Constants::kilo;	//	(m)
	
	
	double zetaClosure = 2.0;				//	Point (in outflow lengths) at which crack re-closes, with first guess.
	nodeResolution = 0.5 / control.elementsInL;	// used as a reference tolerance
	fdArraySize = short(zetaClosure * control.elementsInL);
	zetaBackfilled = 0.2;		// first guess
	
//  Set initial outflow length
	double outflowLength = control.lambda;
	lambdaPow4 =  pow(outflowLength, 4);
	
//	Dimensionless virtual crack opening at crack tip (representing residual strain)
	double v0 = v00 * lambdaPow4;			//	(m)
	vStarRes = residualCrackClosure / v0 / Constants::kilo;
} 


double BeamModel::p1p0(double p0, double gamma_, double aDotC0)
//computes decompression ratio at crack tip using guillotine model
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
} // end p1p0.


BeamModel::BeamModel(const BeamModel& original)
{	// Copy constructor
//	arraySize = original.arraySize;
//	elementsPerUnitLength = original.elementsPerUnitLength;
//	v_ptr = new double[arraySize];
//	copy(original);
}//	end copy constructor


void BeamModel::copy(const BeamModel& original)
{	// Copies values of array into *this object, having already checked that size is same
//	double* p = v_ptr + arraySize;
//	double* q = original.v_ptr + arraySize;
//	while (p > v_ptr)
//		*--p = *--q;
}//	end copy


void BeamModel::resetSize(short newSize)
{	// Resets size of array, discarding data in it
//	if (newSize != arraySize) {
//		delete [] v_ptr;					// Delete old elements,
//		arraySize = newSize;				// set new count,
//		v_ptr = new double[newSize];		// and allocate new elements
//	}
}//	end resetSize


BeamModel& BeamModel::operator=(const BeamModel& rhs)
{// Assign another BeamModel rhs to this one by operator '='
//	if (v_ptr != rhs.v_ptr)
//    {
//		resetSize(rhs.arraySize);
//		copy(rhs);
//    }
//	elementsPerUnitLength = rhs.elementsPerUnitLength;
	return *this;
}//	end operator=.


BeamModel::~BeamModel()
{//	Destructor
//	delete [] v_ptr;
} // end destructor.


// FDprofile 
void BeamModel::fdSolutionTo()
{	
	short notConverged = 1;
	short iterations = 0;
	short noCrackOpening = 0;
	double lambdaLast;
	double zetaBackfilledLast;

	if (control.outflowModelOn and control.infoLevel > 1)
		cout << "Starting outflowLength refinement with outflow length = " << outflowLength << endl;
	do
	{	// Refine given outflow length using the discharge analysis:
		lambdaLast = outflowLength;
		zetaBackfilledLast = zetaBackfilled;
		
//	Dimensionless foundation stiffness with [0] and without [1] backfill
	    m[1] = 8.0 / 3.0 / Constants::pi / Constants::c1 / pow(dStarMinus1, 2) * lambdaPow4;
		m[0] = m[1] / aDotCLfactor_backfilled;
		m[1] /= aDotCLfactor;
		
//	Dimensionless crack speed with [0] and without [1] backfill
		alpha[1] = outflowLength * sqrt((Constants::c3 * aDotOverCL * aDotOverCL + Constants::c4 * 0.5 / (1.0 + material.poisson) / pow(dStarMinus1, 2)) / aDotCLfactor);
		alpha[0] = outflowLength * sqrt((Constants::c3 * aDotOverCL * aDotOverCL * densityBackfillRatio + Constants::c4 * 0.5 / (1.0 + material.poisson) / pow(dStarMinus1, 2)) / aDotCLfactor_backfilled);

//	Compute the v*(zeta) profile (numerically) from alpha, m, and vStarRes and zetaBackfilled:

// Determines (by FD method) the opening profile v*(zeta) for a given outflow length control.lambda and the properties of it which are needed for analysis.
//		short waitForMe;
								
// Make first guess for closure length -- two nodes less than input value -- and construct FD solution:
		nodeAtClosure -= 2;
		double wStarMax = 0.0;
		FDprofile fdSolution(alpha, m, zetaBackfilled, vStarRes, control.elementsInL, nodeAtClosure);
		cout << "first guess profile: \n";
		fdSolution.terminalPrint();
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
		fdSolution.terminalPrint();
				
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
// fdSolution.terminalPrint();
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

			if (control.infoLevel > 1)
			cout << " Computed profile properties.  Ejection point = " << zetaBackfillEject << "; opening at outflow point = " << "; max opening = " << wStarMax << "; 1st-deriv at outflow = " << wStar2dash << "; 2nd-deriv at outflow = " << wStar2dash2 << "; integral to outflow = " << integral_wStar2 << endl;
		if (integral_wStar2 > 0.0)
		{
			if (control.outflowModelOn)
			{
				OutflowProcess outflow(p1bar);
				outflowLength = pow(factor * outflow.get_tStarOutflow() / integral_wStar2, 0.2);
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
	while (notConverged & (iterations < maxIterations) and not noCrackOpening);
	
	
//	So we now have the correct numerical crack opening profile vStar(zeta), and can output it if needed:
	double vStarDashBackfillEject;
	GSolution gSolutionFD;
	gSolutionFD.aDotc0 = aDotc0;
	gSolutionFD.decompression = decompression;

	if (not noCrackOpening)
	{
		if (control.infoLevel > 1)
			cout << "Final outflowLength convergence in " << iterations << " iterations for outflowLength = " << outflowLength << " . \n\n";
		short printOpeningProfile = 1;
		if (printOpeningProfile)
		{
			FDprofile final(alpha, m, zetaBackfilled, vStarRes, control.elementsInL, nodeAtClosure);
			final.terminalPrint();
			final.findBackfillEjectPoint(zetaBackfillEject, vStarDashBackfillEject);
			final.outflowPointValues(wStar2, wStar2dash, wStar2dash2, integral_wStar2);
		}
		
		gSolutionFD.outflowLength = outflowLength;	//	in diameters
cout << "v0 = " << v0 << endl;
		wStar2 *= v0;
//	Flaring of pipe wall at decompression point:
		gSolutionFD.deltaDStar = wStar2 / Constants::pi / pipe.diameter * Constants::kilo + diameterRes0 / pipe.diameter - 1.0;;		//	Maximum
		
//  Compute contributions to crack driving force:
		gSolutionFD.g0 = g0;
		double crackWidth = pipe.diameter / pipe.sdr - pipe.notchDepth;			//	(mm, giving kJ/m2 for G;  not necessarily equal to h)
//	GS1 due to residual strain bending energy in pipe wall ahead of crack:
		double temp = (pipe.diameter - diameterRes0);			// (mm)
		gSolutionFD.gS1 = Constants::pi / 6.0 * material.dynamicModulus * Constants::kilo / pow(dStarMinus1, 3) * temp * temp / crackWidth;
		double hOverR = 2.0 / dStarMinus1;
		double h = pipe.diameter / pipe.sdr / Constants::kilo;	// (m)
		double radius = h / hOverR;						// (m)
// Convert derivatives from vStar(zeta) to v(z)
		wStar2dash *= v0 / 2.0 / outflowLength / radius;
		wStar2dash2 *= v0 / pow(2.0 * outflowLength * radius, 2);
//  GUE due to work done on flaps (= dUE/da / crack width):
		gSolutionFD.gUE = (radius - 0.5 * h) * p1bar * Constants::bar * v0 * integral_wStar2  / crackWidth;
//  GSb due to strain energy at decompression point behind crack:
		temp = 0.5 * material.dynamicModulus * Constants::giga * 
						(pow(hOverR, 3) / 24.0 / Constants::pi * pow(wStar2, 2) + Constants::c1 * pow(radius, 3) * h * pow(wStar2dash2, 2));
		gSolutionFD.gSb = -(temp + Constants::pi / 48.0 * dynamicShearModulus * Constants::giga * radius * h * pow(hOverR * wStar2dash, 2)) / crackWidth;
//  GKb due to kinetic energy at decompression point behind crack:
		gSolutionFD.gKb = -0.5 * material.dynamicModulus * Constants::giga * radius * h * aDotOverCL * aDotOverCL * (Constants::c2 * pow(wStar2dash, 2) + Constants::c1 * pow(wStar2dash2 * radius, 2)) / crackWidth;
//  ... account for kinetic energy of detached backfill mass: 
		temp = vStarDashBackfillEject * v0 * aDotc0 * Constants::vSonic * pipe.sdr / 2.0 / dStarMinus1 / outflowLength;
		gSolutionFD.gKb -= Constants::pi * testSetup.backfillDensity  * pow(temp, 2) * log(1.0 + 2.0 * testSetup.backfillDepth * double(testSetup.isBackfilled)/ pipe.diameter) / Constants::kilo;
//  Total G:
		gSolutionFD.gTotal = 0.0;
		temp = g0 + gSolutionFD.gUE + gSolutionFD.gS1 + gSolutionFD.gSb + gSolutionFD.gKb;
		if (temp > 0.0)
			gSolutionFD.gTotal = temp;

		if (iterations == maxIterations)
			gSolutionFD.notes = " UNCONVERGED";
//		resultsFile << setw(8) << aDotc0 * Constants::vSonic << "\t" << gTotal / g0 << "\t" <<  gTotal << "\n";
	}
	else
	{
		gSolutionFD.outflowLength = 0.0;
		gSolutionFD.deltaDStar = 0.0;
		gSolutionFD.g0 = 0.0;
		gSolutionFD.gS1 = 0.0;
		gSolutionFD.gUE = 0.0;
		gSolutionFD.gSb = 0.0;
		gSolutionFD.gKb = 0.0;
		gSolutionFD.gTotal = 0.0;
		string notes = " No crack opening.\n";
	}
	
}// end
