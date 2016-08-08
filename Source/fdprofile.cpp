//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
using namespace std;

#include "FDprofile.h"


FDprofile::FDprofile()
//	Null constructor
{
    arraySize = 0;
    v_ptr = 0;
    elementsPerUnitLength = 0;
} // end


FDprofile::FDprofile(const double alpha[2],
                        const double m[2],
                        double zeta_at_max_dzetadz,
                        double residualpressure,
                        short elementsInL,
                        short nodeAtClosure)
//	Constructor
{
    elementsPerUnitLength = elementsInL;
//  Crack-tip and closure points with v* = 0 are not represented in the array
    arraySize = nodeAtClosure;

//  Create and set to zero a matrix [M] of finite difference coefficients
    SymDoubleMatrix mMx(arraySize);
    mMx = 0.0;
//  …and an RHS vector for pressure forces.
    v_ptr = new double[arraySize];

    double h = 1.0 / float(elementsPerUnitLength);
//  First disregard backfill attached to the fixed crack-tip node
    zeta_at_max_dzetadz -= 0.5 * h;

    double mLocal, alphaLocal;
//  For each row representing a non-boundary point
    for (int i = 0; i < arraySize; i++)
    {   //  How much backfill on this element, if any?
        if (zeta_at_max_dzetadz <= 0.0)
        {   //  No backfill: set alpha[1] and m[1]
            alphaLocal = alpha[1];
            mLocal = m[1];
        }
        else
        {if (zeta_at_max_dzetadz > h)    //  …there's backfill here and beyond
            {   //  Attach an entire backfill element to this point
                alphaLocal = alpha[0];
                mLocal = m[0];
                zeta_at_max_dzetadz -= h;
            }
            else
            {   //  Hang the leftover part-element of backfill on this element
                //  using simple linear interpolation for alpha and m
                alphaLocal = alpha[1]
                        + zeta_at_max_dzetadz * (alpha[0] - alpha[1]);
                mLocal = m[1]
                        + zeta_at_max_dzetadz * (m[0] - m[1]);
                zeta_at_max_dzetadz -= h;
//  This sends zeta_at_max_dzetadz negative to signal that backfill has gone,
//  so no more points need be tested.
            }
        }

//	Set elements on diagonal bands according to the FD equation.
        double h2inv = float(elementsPerUnitLength * elementsPerUnitLength);
        double alpha2 = alphaLocal * alphaLocal;
        double elOnDiagonal = 0.0;
        double el1OffDiagonal = 0.0;
        double el2OffDiagonal = 0.0;

        if (i == 0 or i == (arraySize - 1))         //  First non-boundary node
        {
//  Elements on diagonal:
            elOnDiagonal = 12.0 * h2inv * h2inv     //  4th diff term
                    - alpha2 * h2inv                //  2nd diff term
                    + mLocal;                       //  linear term
//  Elements one position either side of diagonal:
            el1OffDiagonal = -6.0 * h2inv * h2inv   //  4th diff term
                    + 0.5 * alpha2 * h2inv;         //  2nd diff term
//  Elements two positions either side of diagonal:
            el2OffDiagonal = 4.0 / 3.0 * h2inv * h2inv;
//  and all other elements are unchanged.
            if (i == 0)
            {
                mMx.setElement(0, 0, elOnDiagonal);
                mMx.setElement(0, 1, el1OffDiagonal);
                mMx.setElement(0, 2, el2OffDiagonal);
            }
            else
            {
                mMx.setElement(i, i, elOnDiagonal);
                mMx.setElement(i, i-1, el1OffDiagonal);
                mMx.setElement(i, i-2, el2OffDiagonal);
            }
        }
        else    //  Every internal point
        {
            elOnDiagonal = 6.0 * h2inv * h2inv          //  4th diff term
                    - 2.5 * alpha2 * h2inv              //  2nd diff term
                    + mLocal;                           //  linear term
            el1OffDiagonal = -4.0 * h2inv * h2inv       //  4th diff term
                    + 4.0/3.0 * alpha2 * h2inv;         //  2nd diff term
            el2OffDiagonal = h2inv * h2inv
                    - alpha2 * h2inv / 12.0;
            mMx.setElement(i, i-2, el2OffDiagonal);
//  The i=0 case was caught above…
            if (i > 1) //  but for i=1 this term is not needed
                mMx.setElement(i, i-1, el1OffDiagonal);
            mMx.setElement(i, i,   elOnDiagonal);
            if (i < (arraySize - 2)) // but for i=(arraySize-2) not needed
                mMx.setElement(i, i+1, el1OffDiagonal);
            mMx.setElement(i, i+2, el2OffDiagonal);
//  and finally the i=(arraySize-1) case was caught above.
        }

//  Preset RHS to 'residual strain pressure'…
        v_ptr[i] = residualpressure;
//  …then add internal pressure in outflow region,
//  decaying linearly within 0 < zeta < 1, and scale total:
        if (i < elementsPerUnitLength)
            v_ptr[i] = v_ptr[i] + 1.0 - float(i + 1)
                            / float(elementsPerUnitLength);
    }
//  [M] finished. Decompose it using LU method, to prepare for solution:
    short error = 0.0;
    mMx.decompose(error);
//	Solve, putting solution in v_ptr
    mMx.backSubstitute(v_ptr);
    error *= 2.0;
}   //  end constructor


FDprofile::FDprofile(const FDprofile& original)
// Copy constructor
{
    arraySize = original.arraySize;
    elementsPerUnitLength = original.elementsPerUnitLength;
    v_ptr = new double[arraySize];
    copy(original);
}   //  end copy constructor


void FDprofile::copy(const FDprofile& original)
//  Copy values of array into *this, having already checked that size is same
{
    double* p = v_ptr + arraySize;
    double* q = original.v_ptr + arraySize;
    while (p > v_ptr)
        *--p = *--q;
}   //  end copy()


void FDprofile::resetSize(short newSize)
// Resets size of an array, discarding data in it
{
    if (newSize != arraySize) {
        delete [] v_ptr;					// Delete old elements,
        arraySize = newSize;				// set new count,
        v_ptr = new double[newSize];		// and allocate new elements
    }
}   //  end resetSize()


FDprofile& FDprofile::operator=(const FDprofile& rhs)
// Assign another FDprofile rhs to this one by operator '='
{
    if (v_ptr != rhs.v_ptr)
    {
        resetSize(rhs.arraySize);
        copy(rhs);
    }
    elementsPerUnitLength = rhs.elementsPerUnitLength;
    return *this;
}   //  end operator=()


FDprofile::~FDprofile()
//	Destructor
{
    delete [] v_ptr;
}   //  end destructor()


double FDprofile::VStarMax()
//  Find greatest v* anywhere within open-crack region,
//  using quadratic interpolation
{
    double y0 = v_ptr[0];
    double y1 = v_ptr[1];
    double y2;
    short i = 2;
    while ((v_ptr[i] > y1) && i < elementsPerUnitLength)
    {
        y0 = y1;
        y1 = y2;
        y2 = v_ptr[i];
        i++;
    }
    if (i < elementsPerUnitLength)
    {
        y1 += (y0 - y2) * (y0 - y2) / (y0 - 2.0 * y1 + y2);
    }
    return y1;
}   //  end VStarMax()


double FDprofile::VStar_2()
//  v* at the outflow point '2'
{
    return v_ptr[elementsPerUnitLength];
}   //  end VStar_2()


double FDprofile::DVStarDZeta_2()
//  1st derivative of vStar at outflow point '2'
{
    double temp = v_ptr[elementsPerUnitLength-2]
            - 8.0 * v_ptr[elementsPerUnitLength-1]
            + 8.0 * v_ptr[elementsPerUnitLength+1]
            - v_ptr[elementsPerUnitLength+2];
    return temp * float (elementsPerUnitLength) / 12.0;
}   //  end DVStarDZeta2()


double FDprofile::D2VStarDZeta2_2()
//  2nd derivative of vStar at outflow point '2'
{
    double temp = -v_ptr[elementsPerUnitLength-2]
            + 16.0 * v_ptr[elementsPerUnitLength-1]
            - 30.0 * v_ptr[elementsPerUnitLength]
            + 16.0 * v_ptr[elementsPerUnitLength+1]
            - v_ptr[elementsPerUnitLength+2];
    return temp * float(elementsPerUnitLength * elementsPerUnitLength) / 12.0;
}   //  end wStar2dash2()


double FDprofile::IntegralVStarDZeta_12()
//  Integrate (vStar dZeta) from crack tip point '1' to outflow point '2'
//  FIXME: VERY approximately, by Simpson trapezoidal integration
{
    double simpsonSum = 0.5 * (v_ptr[0] + v_ptr[elementsPerUnitLength]);
    for (int i = 1;  i < elementsPerUnitLength; i++)
        simpsonSum += v_ptr[i];
    return simpsonSum / float(elementsPerUnitLength);
}   //  end integral_wStar2()


double FDprofile::ClosureMoment()
//  Find closure-point:crack-tip ratio of dvStar2_dzeta2, using 5-point stencils
{
    //	return (v_ptr[arraySize-3] -4.0 * v_ptr[arraySize-2] + 6.0 * v_ptr[arraySize-1])
    //			/ (6.0 * v_ptr[0] - 4.0 * v_ptr[1] + v_ptr[2]);
    return (-v_ptr[arraySize-2] +  16.0 * v_ptr[arraySize-1])
                                / (16.0 * v_ptr[0] - v_ptr[1]);
}   //  end closureMoment()


void FDprofile::GetBackfillEjectPoint(double& zeta_at_max_dzetadz,
                                      double& vStarDashBackfillEject)
//  Find point within 0 < zeta < 1 at which 2nd derivative of COD changes sign,
//  and get 1st derivative at that point
{
    double interval = 1.0 / float(elementsPerUnitLength);
    short foundIt = false;
//  Second derivative at crack tip, which must be >0:
    double last2ndDeriv = 16.0 * v_ptr[0] - v_ptr[1];
//  Second derivative at node 0
    double this2ndDeriv = -30.0 * v_ptr[0] + 16.0 * v_ptr[1] - v_ptr[2];
    if (this2ndDeriv < 0.0)
    {
        zeta_at_max_dzetadz = (last2ndDeriv
                                    / (last2ndDeriv - this2ndDeriv)) * interval;
        foundIt = true;
    }
    else
    {
        last2ndDeriv = this2ndDeriv;
        this2ndDeriv = 16.0 * v_ptr[0] -30.0 * v_ptr[1]
                                            + 16.0 * v_ptr[2] - v_ptr[3];
        if (this2ndDeriv < 0.0)
        {
            zeta_at_max_dzetadz = interval *
                        (last2ndDeriv / (last2ndDeriv - this2ndDeriv) + 1.0);
            foundIt = true;
        }
        else
        {
            short i = 2;
            zeta_at_max_dzetadz = -1.0;
            while (i < elementsPerUnitLength and not foundIt)
            {
                last2ndDeriv = this2ndDeriv;
                this2ndDeriv = -v_ptr[i-2] + 16.0 * v_ptr[i-1] -30.0 * v_ptr[i]
                                            + 16.0 * v_ptr[i+1] - v_ptr[i+2];
                if (this2ndDeriv < 0.0)
                {
                    zeta_at_max_dzetadz = interval *
                      (last2ndDeriv / (last2ndDeriv - this2ndDeriv) + float(i));
                    foundIt = true;
                }
                i++;
            }
        }
    }
//	NB gradient by 5-pt stencil is [(1)(-8)(0)(8)(-1)] / 12h
//  hence interpolation between two adjacent values is:
//  (1 - FractionElementBackfilled) * [(+1)(-8)(+0)(+8)(-1)(+0)]
//     + FractionElementBackfilled  * [(+0)(+1)(-8)(+0)(+8)(-1)]
//  where 3rd node is 'iMaxBackfilled'.
    double FractionElementBackfilled = zeta_at_max_dzetadz *
                                        float(elementsPerUnitLength);
    short iMaxBackfilled = floor(FractionElementBackfilled);
    FractionElementBackfilled -= float(iMaxBackfilled);
    iMaxBackfilled -= 1;
    vStarDashBackfillEject =
        -8.0 * FractionElementBackfilled * v_ptr[iMaxBackfilled]
        + 8.0 * (1.0 - FractionElementBackfilled) * v_ptr[iMaxBackfilled + 1]
        + (9.0 * FractionElementBackfilled - 1.0) * v_ptr[iMaxBackfilled + 2]
        - FractionElementBackfilled * v_ptr[iMaxBackfilled + 3];
    if (iMaxBackfilled > 0)
        vStarDashBackfillEject +=
            (9.0 * FractionElementBackfilled - 8.0) * v_ptr[iMaxBackfilled - 1];
    if (iMaxBackfilled > 1)
        vStarDashBackfillEject +=
            (1.0 - FractionElementBackfilled) * v_ptr[iMaxBackfilled - 2];
    vStarDashBackfillEject *= float(elementsPerUnitLength) / 12.0;
} // end backfillEject()


short FDprofile::IsUnphysical()
{
    short index = -1;
    for (short i=1; i < NodeAtClosure(); i++)
    {
        if (v_ptr[i] < 0.0)
        {
          index = i;
          break;
        }
    }
    return index;
}   //  end IsUnphysical()


short FDprofile::NodeAtMinimum()
//  Returns first node immediately before a negative minimum in the profile
{
    double lastValue = 0.0;					// Boundary value
    double thisValue = v_ptr[0];
    double lastGrad = thisValue;
    double thisGrad = thisValue;
    for (short i = 1; i < arraySize; i++)
    {
        lastValue = thisValue;
        thisValue = v_ptr[i];
        lastGrad = thisGrad;
        thisGrad = v_ptr[i] - lastValue;
        if (lastValue < 0 and lastGrad * thisGrad < 0)
        {
            if (thisGrad + lastGrad > 0.0)
                return i - 1;
            else
                return i;
        }
    }
    {
        //  check that neg min is not at last point in array,
        //  where next 'thisValue' is boundary zero
        lastValue = thisValue;
        lastGrad = thisGrad;
        thisGrad = -lastValue;
        if (thisGrad > 0 and lastGrad * lastValue > 0)
        {
            if (lastGrad - lastValue > 0.0)
            {
                return arraySize - 1;
            }
            else
            {
                return arraySize;
            }
        }
    }
    return -1;	// signals that there is NO negative minimum
}   //  end nodeAtMinimum()


short FDprofile::NodeAtClosure()
//  return zeta coordinate of RH boundary point in array = array size
{
    return arraySize;
}   //  end nodeAtClosure()


void FDprofile::ShowCODProfile()
//  outputs crack opening displacement profile
{
    l = arraySize;
    for (int i=0; i<arraySize; i++)
    {
        zeta.push_back(float(i + 1) / float(elementsPerUnitLength));
        vptra.push_back(float(v_ptr[i]));
    }
}   //  end ShowCODProfile()
