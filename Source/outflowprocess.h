//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef OutflowProcess_H
#define OutflowProcess_H

class OutflowProcess
//  Models the process of outflow of a gas from a vessel at initial pressure p_i
//  through a throat of area A_t, to ambient pressure p_amb.
//  The key result is the variation of pressure with time.
{
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
        double pUnchoke;        //  Pressure at which throat unchokes
        double pHalfp1;
        double xUnchoked_i;     //  value at beginning of unchoked discharge
        double tStar;
        double tStarHalfp1;
        double tStarOutflow;
        double tStarOutflow2;
        double tStarUnchoke;
        double integral_pressure_dt;
        short firstHalf;
        short unchoked;
        short firstHalf_choked;

        OutflowProcess();       // null constructor
        OutflowProcess(const double pGauge);
		OutflowProcess& operator=(const OutflowProcess& rhs);
		void pressurePlot();
		double get_tStarOutflow();
};

#endif
