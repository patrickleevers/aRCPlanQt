//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#include <iostream>
#include <cmath>
#include <cstdlib>
using namespace std;

#include "Constants.h"
#include "OutflowProcess.h"
#include "File.h"

const short OutflowProcess::maxTimeSteps = 100;


OutflowProcess::OutflowProcess()
//  Null constructor
{
}	


OutflowProcess::OutflowProcess(const double p1Gauge)
{
    extern File file;
//  A vessel of volume V, which initially contained gas at pressure p1Gauge,
//  discharges adiabatically through an aperture of throat area = exit area = A.
//  Pressure is computed as a function of time t/tChar
//  where tChar = V / (A c0) and c0 is the sonic velocity.
//  The pressure decay curve is then characterised by a single time, either:
//  1. By equating the integral of gauge pressure wrt time
//          to the integral of linear decay from the same initial value; or
//  2. Doubling the time taken for gauge pressure to fall
//          by half its initial value.

    //  Convert to absolute pressure:
    double p1 = p1Gauge + Constants::pAtm;
    p_now = p1;
    pHalfp1 = p1 - p1Gauge / 2.0;
    //  Pressure below which outflow to atmosphere is unchoked:
    pUnchoke = pow((Constants::gamma + 1.0) / 2.0, Constants::gamma
                   / (Constants::gamma - 1.0)) * Constants::pAtm;
    unchoked = 0;
    firstHalf = 1;  //  While gauge pressure still > half initial value…

    //  For any choked adiabatic discharge phase, calculate pressure at
    //  uniform time intervals deltaTStar and integrate by Simpson quadrature.
    double pLast = 0.0;
    double simpson_integral = p1;  // 0th Simpson quadrature point weight=1

    short i = 0;
    short iIsOdd = 0;

    file.collect(this,0);

    tStarHalfp1 = 0.0;
    tStarUnchoke = 0.0;
    tStarOutflow = 0.0;
    double deltaTStar = 5.0 / OutflowProcess::maxTimeSteps;
    double correction = 0.0;

    while (p_now > pUnchoke)
    {
        i++;
        iIsOdd = i - 2 * (i / 2);
        tStar = double(i) * deltaTStar;
        pLast = p_now;
        p_now = p1 * pp1WhileChoked(tStar);
//      Did gauge pressure just fall below half?
        if (firstHalf && (p_now <= pHalfp1))
//      Then linearly interpolate the time it did so.
        {
            firstHalf = 0;
            tStarHalfp1 = tStar
                    - (pHalfp1 - p_now) / (pLast - p_now) * deltaTStar;
            //  Will be recalced if it turns out to be in unchoked region…
        }
        if (iIsOdd) //  Simpson quadrature weights are 1, 4, 2, 4, 2 … 4, 1
            simpson_integral += p_now * 4.0;
        else
            simpson_integral += p_now * 2.0;
    }
//  Flow is now, or always was, unchoked.
    unchoked = 1;
    if (i > 0)
//  …then there was a choked region and we already placed a Simpson node
//  in this unchoked region. To correct:
    {   // Node is inside unchoked region by this fraction of a trapezoid width
        correction = (p_now - pUnchoke) / (p_now - pLast);
        tStarUnchoke = tStar - deltaTStar * correction;
        if (iIsOdd) //  then the last point lay between a new strip pair, so
        //  cancel its contribution
            simpson_integral = simpson_integral - p_now * 4.0
        //  and add a trapezoid instead, accounting for later division by 3
                                + 1.5 * (pUnchoke + pLast) * (1 - correction);
        else //  then the last point completed a strip pair.
        //  Correct its weighting to complete, then trim off excess trapezoid.
            simpson_integral = simpson_integral - p_now
                                    - 1.5 * (pUnchoke + p_now) * correction;
        //  Reset initial conditions to unchoking values
        p1 = pUnchoke;
    };
    integral_pressure_dt = simpson_integral * deltaTStar / 3.0;
    file.collect(this,0);

//	Calculate times for, and integrate by Simpson, 10 strips at
//  pressure intervals from the initial value down to atmospheric
    simpson_integral = p1;
    double deltaP = (p1 - Constants::pAtm) / 10.0;
    xUnchoked_i = x(p1 / Constants::pAtm);  //  Upper (initial) value of x
    for (short j = 1; j < 11; j++)
    {
        short jIsOdd = j - 2 * (j / 2);
        p_now = p1 - double(j) * deltaP;
        tStar = tStarWhileUnchoked(xUnchoked_i,
                                   p_now/Constants::pAtm,
                                   Constants::pAtm/p1);
        if (jIsOdd)
            simpson_integral += tStar * 4.0;
        else
            simpson_integral += tStar * 2.0;
    }
//  Final (11th point) value at which outflow completed
    tStar =  tStarWhileUnchoked(xUnchoked_i, 1.0, Constants::pAtm/p1);
    simpson_integral += tStar;
    integral_pressure_dt += simpson_integral * deltaP / 3.0
            - tStarUnchoke * Constants::pAtm;

//  Hence outflow time by method 1, assuming uniform linear pressure/time decay
    tStarOutflow = 2.0 * integral_pressure_dt / p1Gauge;

//  If pressure had not halved while choked, then calculate (or recalculate)
//  time from unchoking to half-pressure and add to unchoking time:
    if (pHalfp1 < pUnchoke)
    {
        tStarHalfp1 = tStarUnchoke
            + tStarWhileUnchoked(xUnchoked_i,
                                    pHalfp1 / Constants::pAtm,
                                    Constants::pAtm/pUnchoke);
        firstHalf = 0;
    }
//  and in either case double it to get outflow time
    tStarOutflow2 = 2.0 * tStarHalfp1;

//  We choose method 2!
    tStarOutflow = tStarOutflow2;

    file.collect(this,0);
} // end constructor.

double OutflowProcess::pp1WhileChoked(double tStar)
{   //  Choked adiabatic discharge
	double gPlus12 = (Constants::gamma + 1) / 2;
	double gMinus1 =  Constants::gamma - 1;
	double temp = pow(gPlus12, -gPlus12 / gMinus1); 
    return pow((1 + gMinus1 / 2 * temp * tStar),
               -2 * Constants::gamma / gMinus1);
}   // end pp1WhileChoked()


double OutflowProcess::tStarWhileUnchoked(double xUnch,
                                          double p_abs_over_p_amb,
                                          double p_amb_over_p_i)
{   //  Time since transition to (or beginning of) unchoked adiabatic discharge
    //  to ambient pressure.
	double gMinus12 =  (Constants::gamma - 1) / 2;
//    double watchMe = pow(p_abs_over_p_amb, -gMinus12 / Constants::gamma)
//                      / sqrt(gMinus12)
//                      * (xFunction(xUnch) - xFunction(x(p_abs_over_p_amb)));
    double watchMe = xFunction(xUnch);
    double temp = xFunction(x(p_abs_over_p_amb));
    watchMe -= temp;
    watchMe *= pow(p_amb_over_p_i, -gMinus12 / Constants::gamma);
    watchMe /= sqrt(gMinus12);
    return watchMe;
}   // end tStarWhileUnchoked()


double OutflowProcess::x(double p_abs_over_p_amb)
{   //  Function of current absolute pressure, in model for
    //  unchoked adiabatic outflow to ambient absolute pressure
    return sqrt(pow(p_abs_over_p_amb, (1.0 - 1.0 / Constants::gamma)) - 1.0);
}   // end x()


double OutflowProcess::xFunction(double x)
{   //  Used in tStarWhileUnchoked()
    double sqrtx21 = sqrt(x * x + 1);
	return x * (x * x / 4 + 0.625) * sqrtx21 + 0.375 * log(x + sqrtx21);
}   // end xFunction()


double OutflowProcess::get_tStarOutflow()
{
	return tStarOutflow;
}   // end get_tStarOutflow()
