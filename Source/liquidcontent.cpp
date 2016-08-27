//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#include "cmath"
#include "liquidcontent.h"

//  Null constructor
LiquidContent::LiquidContent()
{
    effective_density = 0.0;
    bulk_modulus = 0.0;
}

LiquidContent::LiquidContent(const Parameters parameters)
{   //  NB This code accounts for dependence of effective density on physical
    //  crack opening w. If not required, set w = 0:
    double w = 0.0;
    density = 1000.0;
    bulk_modulus = 3.0;                          //  GPa
    effective_density = 0.0;
    if (parameters.liquid_inside_pipe > 0.0)
    {
        double gkl = 0.0;   //  2 * KE of liquid / unit length / dwdt^2
        double phi_g0 = Constants::pi * (1.0 - parameters.liquid_inside_pipe
                                        / (1.0 - parameters.solid_inside_pipe));

        //  Pipe geometry
        double h = parameters.diameter / parameters.sdr
                                / Constants::kilo;      //  thickness [m]
        double rm = 0.5 * h * (parameters.sdr - 1.0);   //  mean radius [m]
        double ri = 0.5 * h * (parameters.sdr - 2.0);   //  internal radius [m]
        //  All solid content assumed to be concentrated within radius r_core:
        double r_core = sqrt(parameters.solid_inside_pipe)
                            * parameters.diameter / 2000.0
                            * (1.0 - 2.0 / parameters.sdr); //  [m]

        //  Ratio "dphi_g_dw" of gas sector opening rate to crack opening rate
        double dphi_g_dw = parameters.liquid_inside_pipe / 2.0 / Constants::pi
                            / (1.0 - parameters.solid_inside_pipe);

        //  2 * kinetic energy per unit length / (dw/dt)^2 =
        //  …due to radial velocity:
        gkl = (Constants::pi - phi_g0)
                    / pow(2.0 * Constants::pi * (ri - r_core), 2)
                    * (
                        + 0.25 * pow(ri, 4)
                        - pow(r_core, 4) / 12.0
                        - 2.0 / 3.0 * r_core * pow(ri, 3)
                        + 0.5 * pow(ri * r_core, 2)
                    );
        //  …plus due to circumferential velocity:
        gkl += density
                    * pow(dphi_g_dw / (Constants::pi - phi_g0), 2)
                    *   (
                            + (pow(Constants::pi, 3) - pow(phi_g0, 3)) / 3.0
                            + Constants::pi * phi_g0 * (Constants::pi - phi_g0)
                        )
                    * 0.25 * (pow(ri, 4) - pow(r_core, 4));

        //  Divided by 2 * KE per unit length / (dw/dt)^2 for pipe wall:
        effective_density = density * gkl / Constants::c2 / rm / h;
    }
}   //  end constructor

