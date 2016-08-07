//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef _DECOMPRESSION_H
#define _DECOMPRESSION_H

class Decomp
//  Models the pressure distribution in a uniform pipe, containing gas with
//  specific heat ratio gamma_ and at pressure p0, after it is instantaneously
//  guillotined to allow free outflow to ambient pressure.
{

private:

    double p0;          //  Initial pressure
    double p1;          //  Pressure at plane 1
    double aDotc0;      //  Axial velocity of plane 1 / sonic velocity
    double argument;    //
    double gamma_;      //  Ratio of specific heats of gas (usually 1.4)

public:

    double p1p0r;

//  Null constructor
	Decomp();

//  Returns p1 as a proportion of p0
    void p1p0(double p0,
              double gamma_,
              double aDotC0);

};

#endif
