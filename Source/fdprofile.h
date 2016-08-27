//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.

//  For the underlying model, see:
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

//  Class representing the dynamic Beam on Elastic Foundation differential
//  equation [D^4 + alpha^2 D^2 + m] * v_star = f(zeta),
//  and determining its displacement solution v_star(zeta).


#ifndef FDprofileH
#define FDprofileH

#include <vector>
#include "SymDoubleMatrix.h"

class FDprofile
{
	public:
        int cod_plot_points;
		vector<double> zeta;
        vector<double> vptra;
        FDprofile();            // Null constructor
        FDprofile(const double alpha[2],
                    const double m[2],
                    double zeta_at_max_dzetadz, //  backfill ejection point
                    double residualpressure,
                    short elementsPerUnitLength,
                    short nodeAtClosure);
                                // Constructs an FDprofile from specified BCs
        FDprofile(const FDprofile& original);
                                // Copy constructor
        FDprofile& operator=(const FDprofile&);
                                // Assignment by '='
        ~FDprofile();           // Destructor

        void resetSize(short newSize);
                                // Resets array size, discarding data in it

        void ShowCODProfile();
        double VStarMax();
        double VStar_2();       // vStar at outflow point
        double DVStarDZeta_2(); // First derivative at outflow point
        double D2VStarDZeta2_2();
                                // Second derivative at outflow point
        double IntegralVStarDZeta_12();
                                // Integral (vStar dZeta)
                                // from crack tip point '1' to outflow point '2'

        double ClosureMoment(); //  Find closure-point:crack-tip ratio of
                                //  of dvStar2_dzeta2,
        short NodeAtMinimum();  //  Identify last positive COD node
        short IsUnphysical();   //  Node has negative crack opening
        short NodeAtClosure();	//  Identify last node in FD array
        void GetBackfillEjectPoint(double& zeta_at_max_dzetadz,
                                   double& vStarDashBackfillEject);

private:
    int arraySize;              //  Total number of active (nonzero COD) nodes
    double* v_ptr;              //  Vector of matrix equation RHS elmnts
                                //  then, after solution, vStar elmnts
    short elementsPerUnitLength;	//  in outflow length (0 < zeta < 1)
    void resetBackfill(const double alpha[2],
                        const double m[2],
                        int noBackfill,
                        double& weight0,
                        double& weight1,
                        double& weight2);
    void copy(const FDprofile& original);	// Copy

};

#endif
