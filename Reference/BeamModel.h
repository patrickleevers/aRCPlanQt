#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <string>
using namespace std;

#include "Constants.h"
#include "Material.h"
#include "Pipe.h"
#include "TestSetup.h"
#include "Control.h"
#include "FDprofile.h"
#include "OutflowProcess.h"

#ifndef BeamModelH
#define BeamModelH

class BeamModel 
{// 
	private:
		Material material;
		Pipe pipe;
		TestSetup testSetup;
		Control control;
		double aDotc0;
		double g0;
		double m[2];
		double alpha[2];
		double outflowLength;
		short nodeAtClosure;
		double v0;
		double vStarRes;
		double dynamicShearModulus;
		double aDotOverCL;
		double aDotCLfactor;
		double aDotCLfactor_backfilled;
		double p1p0(double p0, double gamma_, double aDotC0);
		double p1bar;
		
		double dStarMinus1;
		double dStarMinus2;
		double zetaBackfilled;
		double zetaBackfillEject;
		double lambdaPow4;
		double factor;
		double nodeResolution;
		short fdArraySize;
		double densityBackfillRatio;
		double wStar2, wStarMax, wStar2dash, wStar2dash2, integral_wStar2;
		double residualCrackClosure;
		double diameterRes0;
		double v00;
		double decompression;
		static short maxIterations;
		
public:		
		//Null constructors
	    BeamModel();			
	    BeamModel(const Material material, const Pipe pipe, const TestSetup testSetup, const Control control, const double aDotc0);
								// Constructs a dynamic beam-on-elastic-foundation model of the specific pipe RCP case
		BeamModel(const BeamModel& original);			// Copy constructor
		void copy(const BeamModel& original);
		void resetSize(short newSize);
		BeamModel& operator=(const BeamModel& rhs);
		void fdSolutionTo();
		~BeamModel();			// Destructor
};

#endif