
#include <cmath>
#include <iostream>

#include "Constants.h"
#include "BeamModel.h"
#include "OutflowProcess.h"

//Null constructor
BeamModel::BeamModel()
{

    g0=0;

}

//Constructor
BeamModel::BeamModel(const Parameters parameters, Backfill backfill, Creep creep)
{

    //Proportion of internal volume available for expansion
    availableInternalVolume = 1.0 - parameters.waterInsidePipe - parameters.solidInsidePipe;

    //Factors for modelling of S4 baffle leak
    baffleLeakageArea=parameters.diameter / Constants::kilo;  //m
    baffleLeakageArea = baffleLeakageArea * baffleLeakageArea * 0.01 * Constants::pi * (parameters.sdr - 2.0) / parameters.sdr;

    //Parameters for equivalent beam model
    dynamicShearModulus = parameters.dynamicModulus / 2.0 / (1.0 + parameters.poisson);
    sdrMinus1 = parameters.sdr - 1.0;
    sdrMinus2 = sdrMinus1 - 1.0;

    zetaClosure = 2.0;				//	Point (in outflow lengths) at which crack re-closes, with first guess.
    nodeResolution = 0.5 / parameters.elementsInL;	// used as a reference tolerance
    nodeAtClosure = short(zetaClosure * parameters.elementsInL);
    zetaBackfilled = 0.2;		// first guess

}

void BeamModel::speedandreset(const Parameters parameters, const Backfill backfill, Creep creep)
{
    n = 0;
    //  Set initial outflow length
    outflowLength = parameters.lambda;
    lambdaPow4 =  pow(outflowLength, 4);

    //For FS configuration, prior decompression
    p1bar=parameters.p0bar;
    Decomp decomp;
    // Proportion of initial pressure which remains at crack tip
    decomp.p1p0r=1.0;

    if(parameters.fullScale)
                        {

                        availableInternalVolume=1.0;

                        decomp.p1p0(parameters.p0bar, Constants::gamma, parameters.aDotc0);

                        p1bar=parameters.p0bar * decomp.p1p0r; //New p1, could replace with p1 from decomp object, need to trace

                        }

    p1p0r=decomp.p1p0r; //Stopgap measure

    //	v00 becomes reference length v0 on multiplying by lambda^4
    v00 = 0.4 / Constants::c1 * sdrMinus1 * sdrMinus2 / parameters.sdr * p1bar / parameters.dynamicModulus * parameters.diameter / Constants::mega;	//	(m)

    //	Dimensionless virtual crack opening at crack tip (representing residual strain)
    v0 = v00 * lambdaPow4;			//	(m)
    vStarRes = creep.residualCrackClosure / v0 / Constants::kilo;

    // Parameters for equivalent beam model (speed dependent)
    aDotOverCL = parameters.aDotc0 * Constants::vSonic / sqrt(parameters.dynamicModulus * Constants::giga / parameters.density);
    aDotCLfactor = 1.0 + aDotOverCL * aDotOverCL;

    aDotCLfactor_backfilled = 1.0 + aDotOverCL * aDotOverCL * backfill.densityratio;

    factor = Constants::pi * Constants::c1 * 625.0 * parameters.dynamicModulus / p1bar * availableInternalVolume * sdrMinus2 / sdrMinus1 / sdrMinus1 * parameters.aDotc0;	// Note GPa / bar / 16 = 625

}

void BeamModel::stiffness()
{

    //	Dimensionless foundation stiffness [0] with backfill and [1] without
    m[1] = 8.0 / 3.0 / Constants::pi / Constants::c1 / sdrMinus1 / sdrMinus1 * lambdaPow4;
    m[0] = m[1] / aDotCLfactor_backfilled;
    m[1] /= aDotCLfactor;

    cout << endl;
    cout << "lambdaPow4:  " << lambdaPow4 << endl;
    cout << "aDotCLfactor_backfilled: " << aDotCLfactor_backfilled << endl;
    cout << "aDotCLfactor: " << aDotCLfactor << endl;

}

void BeamModel::cspeed(const Parameters parameters, const Backfill backfill)
{

    //	Dimensionless crack speed [0] with backfill and [1] without
    alpha[1] = outflowLength * sqrt(Constants::c3 * aDotOverCL * aDotOverCL + Constants::c4 / 2.0 / (1.0 + parameters.poisson) / sdrMinus1 / sdrMinus1 / aDotCLfactor);
    alpha[0] = outflowLength * sqrt(Constants::c3 * aDotOverCL * aDotOverCL * backfill.densityratio + Constants::c4 * 0.5 / (1.0 + parameters.poisson) / sdrMinus1 / sdrMinus1 / aDotCLfactor_backfilled);

    cout << endl;
    cout << "outflowLength: " << outflowLength << endl;
    cout << "aDotOverCL: " << aDotOverCL << endl;
}

void BeamModel::converteffopen(const Parameters parameters)
{
    wStar2dash *= v0 / 2.0 / outflowLength / parameters.radius;
    wStar2dash2 *= v0 / pow(2.0 * outflowLength * parameters.radius, 2);
}

void BeamModel::iteration(const Parameters parameters, Interface interface, Backfill backfill, Creep creep)
{
    maxIterations=100;
    notConverged = 1;
    iterations = 0;
    noCrackOpening = 0;

    if (parameters.outflowModelOn and interface.infoLevel > 1)

        interface.line("Starting outflowLength refinement with outflow length = ", outflowLength);

    do
    {	// Refine given outflow length using the discharge analysis:
        lambdaLast = outflowLength;
        zetaBackfilledLast = zetaBackfilled;

        //	Dimensionless foundation stiffness
        stiffness();

        //	Dimensionless crack speed
        cspeed(parameters, backfill);

        cout << endl << "Iteration number: " << n++ << endl;

        //	Compute the v*(zeta) profile (either numerically or analytically) from alpha, m, and vStarRes and zetaBackfilled:

        //	Determine (either by analytical or FD method) the opening profile v*(zeta) for a given outflow length control.lambda and the properties of it which are needed for analysis.
        if (not parameters.analyticalSolutionMode)
        {	// ...then use FINITE DIFFERENCE solution method:
            //		short waitForMe;

            // Make first guess for closure length -- two nodes less than input value -- and construct FD solution:
            nodeAtClosure -= 2;
            double wStarMax = 0.0;
            FDprofile fdSolution(alpha, m, zetaBackfilled, vStarRes, parameters.elementsInL, nodeAtClosure);

            interface.line("first guess profile: ");

            fdSolution.fprofile();
            interface.iprofile(fdSolution.zeta, fdSolution.vptra, fdSolution.l);

            short nodeAtClosure_previous = nodeAtClosure;				// Store position of last node in this FD array
            double errorLast = fdSolution.closureMoment();				// ...and resulting d2v/dz2 at closure point, divided by that at crack tip
        if (interface.infoLevel > 1)
        {

            interface.oneline("Starting closure length refinement with closure node = ",nodeAtClosure_previous);
            interface.oneline(" closure moment = ", errorLast);

        }

    // Make second guess for closure length (two nodes MORE than input value):
        nodeAtClosure += 4;

        if (interface.infoLevel > 1)
            interface.line("Second-guess closure node = ", nodeAtClosure);

        // Prepare to refine closure length by iteration
        double dontNeedThis;
        double error;
        short notConverged = 1;
        iterations = 0;
        short maximumNonContact = 0;
        do
        {	// ...until bending moment at closure point is negligible compared to that at crack tip:

            fdSolution = FDprofile(alpha, m, zetaBackfilled, vStarRes, parameters.elementsInL, nodeAtClosure);
            error = fdSolution.closureMoment();
            short noSurfaceContact = 1;								// FIXME:  necessary?
            short minPoint = fdSolution.nodeAtMinimum();			// this is set to -1 if NO minimum is found within domain
            if (interface.infoLevel > 1)
            {

                interface.oneline("At closure length iteration ",iterations);
                interface.oneline(" with nodeAtClosure = ",nodeAtClosure);
                interface.oneline(" closure moment = ", error);
                interface.oneline(" min at point ",minPoint);

            }
            if (minPoint > 0)
            {
                if (interface.infoLevel > 1)

                    interface.line("BUT there's a minimum (crack surface overlap) to left of closure point ", minPoint);

                // So back up to find maximum closure length with NO overlap:
                double tempError;
                nodeAtClosure = minPoint - 2;
                if (nodeAtClosure < (parameters.elementsInL + 1))
                    nodeAtClosure = parameters.elementsInL + 1;	// Is this really necessary?  Closure might really occur inside decompression length!
                short newMin;
                do
                {
                    fdSolution = FDprofile(alpha, m, zetaBackfilled, vStarRes, parameters.elementsInL, nodeAtClosure);
                    tempError = fdSolution.closureMoment();
                    newMin = fdSolution.nodeAtMinimum();
                    if (interface.infoLevel > 1)
                    {

                        interface.oneline("nodeAtClosure = ",nodeAtClosure);
                        interface.oneline(" error = ", tempError);
                        interface.oneline(" min point = ", newMin);

                    }
                    nodeAtClosure++;
                }
                while (newMin < 0);
                nodeAtClosure = nodeAtClosure - 1;
                if (interface.infoLevel > 1)

                    interface.line("Least worst non-contacting solution nodeAtClosure = ", nodeAtClosure);

                maximumNonContact = true;
                fdSolution = FDprofile(alpha, m, -1.0, vStarRes, parameters.elementsInL, nodeAtClosure);

                fdSolution.fprofile();
                // interface.iprofile(fdSolution.zeta, fdSolution.vptra, fdSolution.l);

                error = fdSolution.closureMoment();
                integral_wStar2 = fdSolution.integral_wStar2();
                fdSolution.findBackfillEjectPoint(zetaBackfillEject, dontNeedThis);

            }
            else
            {
                nodeAtClosure++;
                if (interface.infoLevel > 1)

                    interface.line("node interpolated = ", nodeAtClosure);

            }

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
                    if (interface.infoLevel > 1)

                        interface.line("Next try for iteration will be nodeAtClosure = ", nodeAtClosure);

                    errorLast = error;
                    iterations++;

                }
            } // done that refinement iteration
            while (notConverged & (iterations < maxIterations));

            if (interface.infoLevel > 1)
            {

                interface.oneline("At nodeAtClosure = ", nodeAtClosure);
                interface.oneline(" converged in ", iterations);
                interface.oneline(" iterations with error = ", error);

            }

        } // done FD solution

        else
        { //Formally location of analytical solution

        }

        if (interface.infoLevel > 1)
        {
        interface.line("Computed profile properties");
        interface.line("Ejection point = ", zetaBackfillEject);
        interface.line("Opening at outflow point = ", wStarMax);
        interface.line("1st-deriv at outflow = ", wStar2dash);
        interface.line("2nd-deriv at outflow = ", wStar2dash2);
        interface.line("Integral to outflow = ", integral_wStar2);

        }

        if (integral_wStar2 > 0.0)
        {
            if (parameters.outflowModelOn)
            {
                double throatArea = integral_wStar2;
                OutflowProcess outflow(p1bar);
                outflowLength = pow(factor * outflow.get_tStarOutflow() / throatArea, 0.2);

            }
        //	tStarOutflow being the number of characteristic times for discharge
            if (interface.infoLevel > 1)
            {
                interface.line("alpha = ", alpha[1]);
                interface.line("m = ", m[1]);
                interface.line("integral_wStar2 = ", integral_wStar2);
                interface.line("New outflowLength = ", outflowLength);

            }
            lambdaPow4 =  pow(outflowLength, 4);
            v0 = v00 * lambdaPow4;
            vStarRes = creep.residualCrackClosure / v0 / Constants::kilo;

            if ((fabs(1.0 - lambdaLast / outflowLength) < nodeResolution) and (fabs(1.0 - zetaBackfilledLast / zetaBackfilled) < nodeResolution))
                notConverged = false;
            iterations++;
            short waitForMe;
            if (interface.infoLevel > 2)
            {
                waitForMe=interface.input("enter digit: ");
            }
        }
        else
        {
            noCrackOpening = 1;
        }

    } // end outflow length refinement
    while (notConverged & (iterations < maxIterations) and not noCrackOpening);
    cout << endl << "iterations1:" << iterations << endl;
}

void BeamModel::opening(Parameters parameters, Interface interface, Solution solution, Creep creep, Plot plot, Results results)
{

    //	So we now have the correct numerical or analytical crack opening profile vStar(zeta), and can output it if needed

        if (interface.infoLevel > 1)
        {

            interface.oneline("Final outflowLength convergence in ", iterations);
            interface.oneline(" iterations for outflowLength = ", outflowLength);

        }
        if (interface.printOpeningProfile)
        {

            if (not parameters.analyticalSolutionMode)
            {	// then recalculate and print the numerical solution

                FDprofile final(alpha, m, zetaBackfilled, vStarRes, parameters.elementsInL, nodeAtClosure);
                FDprofile* ptr=&final;
                final.fprofile();

                final.findBackfillEjectPoint(zetaBackfillEject, vStarDashBackfillEject);
                final.outflowPointValues(wStar2, wStar2dash, wStar2dash2, integral_wStar2);

                zeta=final.zeta;
                crackdisplacement=final.vptra;
                l=final.l;
            }
            else
            {

            //Originally Analytical Method

            }
        }

        wStar2 *= v0;

        // Convert derivatives from vStar(zeta) to v(z)
        converteffopen(parameters);

        //	Flaring of pipe wall at decompression point:
        deltaDStar = wStar2 / Constants::pi / parameters.diameter * Constants::kilo + creep.diameterRes0 / parameters.diameter - 1.0;

        if (iterations == maxIterations) interface.line("UNCONVERGED"); cout << endl;

}

