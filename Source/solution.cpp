//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

//  Class which collects solution values, as well as crack profiles,
//  into a single object for easy storage and access later.

#include <iostream>
#include <iomanip>

#include "Solution.h"

//Null constructor
Solution::Solution()
{
    soln = 0;
    adotc0.push_back(0.0);
    p0bar.push_back(0.0);
    tempdegc.push_back(0.0);
    z.push_back(0.0);
    k=0;

    decompression.push_back(0.0);
    alpha.push_back(0.0);
    m.push_back(0.0);
    lambda.push_back(0.0);
    deltadstar.push_back(0.0);
    gs1.push_back(0.0);
    gue.push_back(0.0);
    gsb.push_back(0.0);
    gkb.push_back(0.0);
    gg0.push_back(0.0);
    g_total.push_back(0.0);
    g0.push_back(0.0);
    w_integral_12.push_back(1);
    lambda_is_converged.push_back(1);
    closure_is_converged.push_back(1);
    iterations.push_back(0);
}

//  Clears all values within the solution class and resizes their arrays to 1
void Solution::clear()
{
    soln=0;
    k=0;
    adotc0.clear();
    adotc0.resize(1);
    p0bar.clear();
    p0bar.resize(1);
    tempdegc.clear();
    tempdegc.resize(1);
    z.clear();
    z.resize(1);
    w.clear();
    w.resize(1);

    decompression.clear();
    decompression.resize(1);
    alpha.clear();
    alpha.resize(1);
    m.clear();
    m.resize(1);
    lambda.clear();
    lambda.resize(1);
    deltadstar.clear();
    deltadstar.resize(1);
    gs1.clear();
    gs1.resize(1);
    gue.clear();
    gue.resize(1);
    gsb.clear();
    gsb.resize(1);
    gkb.clear();
    gkb.resize(1);
    gg0.clear();
    gg0.resize(1);
    g_total.clear();
    g_total.resize(1);
    g0.clear();
    g0.resize(1);
    w_integral_12.clear();
    w_integral_12.resize(1);
    lambda_is_converged.clear();
    lambda_is_converged.resize(1);
    closure_is_converged.clear();
    closure_is_converged.resize(1);
    iterations.clear();
    iterations.resize(1);
}

//  Sets up displacement values for crack profiles
void Solution::displacement(Parameters &parameters)
{
//  n = (parameters.elements_in_l * (parameters.lambda + 2)) + 1;
    n = parameters.elements_in_l * 3;
    vector<double> row;

    //  Create column (used as row) vector
    for (i = 0; i < n; i++ )
        row.push_back(0.0);

    //  Stores vector in vector to provide a "Matrix"
    for (i = 0; i < parameters.range_number; i++)
        w.push_back(row);

    //  Generate axial coordinate from crack tip
    for (i = 1; i < n; i++)
        z.push_back(double(i) / double(parameters.elements_in_l));
}   //  end displacement()

void Solution::collectProfile(const vector<double> vptras, const int points)
//  For each solution k, collect the crack profile and write it into the matrix
//  already created
{
    k++;
    for (i = 1; i <= points+1; i++)
        w[k][i] = vptras[i];
    for (i = points; i <= n+1; i++)
        w[k][i] = 0.0;
}   //  end collectProfile()

//  Collects the values from the arguments provided, storing them into
//  a single solution object
void Solution::collect(const double adotc0s,
                       const double p0bars,
                       const double tempdegcs,
                       const double decompressions,
                       const double alphas,
                       const double ms,
                       const double lambdas,
                       const double deltadstars,
                       const double gs1s,
                       const double gues,
                       const double gsbs,
                       const double gkbs,
                       const double g0s,
                       const double gg0s,
                       const double g_totals,
                       const double w_integral_12s,
                       const short closure_is_convergeds,
                       const short lambda_is_convergeds,
                       const short iterationss)

{
	soln++;
    adotc0.push_back(adotc0s);
    p0bar.push_back(p0bars);
    tempdegc.push_back(tempdegcs);

	decompression.push_back(decompressions);
	alpha.push_back(alphas);
	m.push_back(ms);
    lambda.push_back(lambdas);
    deltadstar.push_back(deltadstars);
    gs1.push_back(gs1s);
    gue.push_back(gues);
    gsb.push_back(gsbs);
    gkb.push_back(gkbs);
	g0.push_back(g0s);

    //  Flips values so that they make sense under the headings of the tables
    w_integral_12.push_back(!w_integral_12s);
    closure_is_converged.push_back(!closure_is_convergeds);
    lambda_is_converged.push_back(!lambda_is_convergeds);
    iterations.push_back(iterationss);

    //  Removes spurious errors, need to find source of these
    if (gg0s < 1000)
    {gg0.push_back(gg0s);}
    else
    {gg0.push_back(0.0);}

    if (g_totals < 1000)
    {g_total.push_back(g_totals);}
    else
    {g_total.push_back(0.0);}
}
