//     aRCPLan
//     Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//     aRCPlan may be freely distributed under the MIT license.
//     For the underlying model, see http://www.sciencedirect.com/science/article/pii/S0013794412003530

//     A VERY simple class with no exception handling
//     Despite its name it is NOT restricted to symmetrical matrices

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include "SymDoubleMatrix.h"

// Construct n x n symmetric double matrix stored row-wise in 1D array
SymDoubleMatrix::SymDoubleMatrix(short n)
{
	size = n;
	nElements = size * size;
    pointer2elements = new double[nElements];
    pointer2indices = new short[size];				// Array of row exchanges
}

// Null constructor:  create 0 x 0 matrix
SymDoubleMatrix::SymDoubleMatrix()
{
	size = 0;
    nElements = 0;
    pointer2elements = 0;
    pointer2indices = 0;	
}

// Delete array
SymDoubleMatrix::~SymDoubleMatrix()
{
    delete [] pointer2elements;
    delete [] pointer2indices;
}

// Construct matrix which is copy of existing matrix a
SymDoubleMatrix::SymDoubleMatrix(const SymDoubleMatrix& a)
{
    nElements = a.nElements;
    pointer2elements = new double[nElements];
    copy(a); // Copy a's elements
}

// Assign value x to element (i, j), j>=i
void SymDoubleMatrix::setElement(short i, short j, double x)
{
	int index = size * i + j;
	if ((index - size * size + 1) > 0)
        cout << " bad set index\n";
    *(pointer2elements + index) = x;
}

// Add dx to element (i, j), j>=i
void SymDoubleMatrix::incrementElement(short i, short j, double dx)
{
	*(pointer2elements + size * i + j) += dx;	
}

// returns value of element [i, j] in this triangular symm matrix
double SymDoubleMatrix::getElement(short i, short j)
{
	int index = size * i + j;
	if ((index - size * size + 1) > 0)
		cout << " bad get index\n";
	return *(pointer2elements + index);	
}

// Sets all elements of this matrix to rhs by operator '='
SymDoubleMatrix& SymDoubleMatrix::operator=(const double rhs)
{
    double* p = pointer2elements + nElements;
    while (p > pointer2elements) *--p = rhs;
    return *this;
}

// Assign another matrix to this one by operator '='
SymDoubleMatrix& SymDoubleMatrix::operator=(const SymDoubleMatrix& rhs)
{
    if ( pointer2elements != rhs.pointer2elements )
    {
        resetSizeTo(rhs.nElements);
        copy(rhs);
    }
    return *this;
}

// Increment every element of this matrix by dx using operator '+='
SymDoubleMatrix& SymDoubleMatrix::operator+=(const SymDoubleMatrix& rhs)
{
    if ( pointer2elements != rhs.pointer2elements )
        resetSizeTo(rhs.nElements);
    double* p = pointer2elements + nElements;
    double* q = rhs.pointer2elements + nElements;
    while (p > pointer2elements) 
    	*--p += *--q;
    return *this;
}

// Number of rows/columns in matrix
short SymDoubleMatrix::sizeOf()
{
    return size;
}


void SymDoubleMatrix::resetSizeTo(short newSize)
{
    if (newSize != size)
    {
        delete [] pointer2elements;				// Delete old elements,
        size = newSize;                 		// set new count,
        pointer2elements = new double[newSize * newSize];	// and allocate new elements
        pointer2indices = new short[newSize];
    }
}

// Completes the matrix from its upper triangle
void SymDoubleMatrix::makeSymmetrical()
{
	for (short i=1; i<size; i++)
		for (short j=0; j<i; j++)
			*(pointer2elements + size * i + j) = *(pointer2elements + size * j + i);
}

// prints up to 8 rows and columns of matrix
void SymDoubleMatrix::printMatrix()
{
	short max = size <9 ? size : 8;
	cout << "First 8 rows and columns of matrix whose size is "<< size << endl;
	for (short i=0; i<max; i++)
	{
		for (short j=0; j<max; j++)
			cout << setw(9) << setprecision(4) << getElement(i, j) << "  ";
		cout << endl;
	}
	cout << endl;
}

// Copy elements of [a] into this matrix
void SymDoubleMatrix::copy(const SymDoubleMatrix& a)
{
    double* p = pointer2elements + nElements;
    double* q = a.pointer2elements + nElements;
    while (p > pointer2elements) 
    	*--p = *--q;
}

// Replaces symmetric matrix by a row-wise decomposition of itself
void SymDoubleMatrix::decompose(short& error)
{
	const double tiny = 1.0e-20;
	short i, imax, j, k;
	double biggest, dummy, sum, temp;
    double *vv = new double[size];	
	double sign = 1.0;
	
	for (i = 0; i < size; i++)
	{
		biggest = 0.0;
		for (j = 0; j < size; j++)
		{
			temp = fabs(getElement(i, j));
			if (temp > biggest)
				biggest = temp;
		}
		if (biggest  ==  0.0)
			error = 1;
		*(vv + i) = 1.0 / biggest;
	}
// 
	for (j = 0; j < size; j++)
	{
		for (i = 0; i < j; i++)
		{
			sum = getElement(i, j);
			for (k = 0; k < i; k++) 
				sum -=  getElement(i, k) * getElement(k, j);
			setElement(i, j, sum);
		}
		biggest = 0.0;
		for (i = j; i < size; i++)
		{
			sum = getElement(i, j);
			for (k = 0; k < j; k++)
				sum -= getElement(i, k) * getElement(k, j);
			setElement(i, j, sum);
			if ((dummy = *(vv + i) * fabs(sum)) >=  biggest)
			{
				biggest = dummy;
				imax = i;
			}
		}
		if (j != imax)
		{
			for (k = 0; k < size; k++)
			{
				dummy = getElement(imax, k);
				setElement(imax, k, getElement(j, k));
				setElement(j, k, dummy);
			}
			sign  = -sign;
			*(vv + imax) = *(vv + j);
		}
		*(pointer2indices + j) = imax;
		if (getElement(j, j)  ==  0.0)
			setElement(i, j, tiny);
		if (j !=  size-1)
		{
			dummy = 1.0 / getElement(j, j);
			for (i = j + 1; i < size; i++)
				setElement(i, j, getElement(i, j) * dummy);
		}
	}
	delete [] vv;
}

// Solves
void SymDoubleMatrix::backSubstitute(double* b)
{
	short i, ii = 0, ip, j;
	double sum;

	short n = sizeOf();
	for (i = 0; i < n; i++)
	{
		ip = *(pointer2indices + i);
		sum = b[ip];
//		double temp = b[i];
		b[ip] = b[i];
		if (ii != 0)
			for (j = ii - 1; j < i; j++)
				sum -= getElement(i, j) * b[j];
		else if (sum != 0.0)
			ii = i + 1;
		b[i] = sum;
	}
	for (i = n - 1; i >= 0; i--)
	{
		sum = b[i];
		for (j = i + 1; j < n; j++)
			sum -= getElement(i, j) * b[j];
		b[i] = sum / getElement(i, i);
	}
}
