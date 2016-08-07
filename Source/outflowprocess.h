//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef OutflowProcess_H
#define OutflowProcess_H

class OutflowProcess
{// pressure/time function for outflow from a vessel at defined initial pressure, through a
	private:

        static const short maxTimeSteps;        // Time at which throat unchokes
		double pp1WhileChoked(double tStar);
        double tStarWhileUnchoked(double xUnch,
                                    double p_abs_over_p_amb,
                                    double p_amb_over_p_i);
        double x(double p_abs_over_p_amb);
		double xFunction(double x);

	public:

        double p_now;
        double pUnchoke;    //  Pressure at which throat unchokes
        double pHalfp1;
        double xUnchoked_i; //  value at beginning of unchoked discharge
        double tStar;
        double tStarHalfp1;
        double tStarOutflow;
        double tStarOutflow2;
        double tStarUnchoke;
        double integral_pressure_dt;
        short firstHalf;
        short unchoked;
        short firstHalf_choked;

        OutflowProcess();				// null constructor
        OutflowProcess(const double pGauge);
                    // construct a Control definition from the library or a file
		OutflowProcess& operator=(const OutflowProcess& rhs);
		void pressurePlot();
		double get_tStarOutflow();
};

#endif
