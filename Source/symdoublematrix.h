//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef SymDoubleMatrixH
#define SymDoubleMatrixH

class SymDoubleMatrix
{
    public:

        SymDoubleMatrix(short n);   //  Create n x n symmetric double matrix
        SymDoubleMatrix();          //  Null constructor:  create 0 x 0 matrix
        ~SymDoubleMatrix();         //  Delete this matrix
        SymDoubleMatrix(const SymDoubleMatrix& a);
                                    //  Create a copy of the existing matrix a
        double getElement(short i, short j);
                                    //  Return value of element (i, j), j>=i
        void setElement(short i, short j, double x);
                                    //  Assign value x to element (i, j), j>=i
        void incrementElement(short i, short j, double dx);
                                    //  Add dx to element (i, j), j>=i
        SymDoubleMatrix& operator=(const double xij);
                                    //  Assignment to scalar by '='
        SymDoubleMatrix& operator=(const SymDoubleMatrix&);
                                    //  Assignment to matrix by '='
        SymDoubleMatrix& operator+=(const SymDoubleMatrix&);
                                    //  Matrix incrementation by operator '+='
        short sizeOf();				//  Returns number of rows/columns in matrix
        void resetSizeTo(short newSize);    // Change size
        void makeSymmetrical();		//  Completes matrix
                                    //  by overwriting lower with upper triangle
        void printMatrix();         // Prints from upper triangular (?  really?) sector
        void decompose(short& error);   // Completion and LU decomposition
        void backSubstitute(double* vVx);   // Solves [A](x)=(b), then (b)=(x)
        void copy(const SymDoubleMatrix& a);// Copy in elements of a

    private:
        short size;                 // Number of rows/columns
        short nElements;            // Number of elements
        double* pointer2elements;   // Pointer to element array
        short* pointer2indices;     // Pointer to matrix
                    //  records row permutations effected by partial pivoting
};

#endif
