// TestSetup_Test.cpp
// Author: Fraser Edwards

// Program to test performance of TestSetup.cpp
// Tests: null constructor, constructor at specified temperature, reads configfile, editing of parameters
//        ,display of data.

#include <iostream>

using namespace std;

#include "ConfigFile.h"
#include "TestSetup.h"

int main()
{
	

	//declare control variables
	int control;	
	
	ConfigFile config("caseInputData.txt");
	
	cout << endl << "Enter the test case: (Display tested with each case)" << endl 
			<< endl << "0: Null Constructor" << endl
			<< "1: Constructor at specified temperature" << endl
			<< "2: Read in from configfile" << endl
			<< "3: Edit Test Parameters" << endl;
	
	cin >> control;

	TestSetup testSetup;	
	
	if(control==0)
		{
		    // declare and initialise material class using 
	        // Null constructor
			TestSetup testSetup;	
			cout << endl << "Null Constructor" << endl;
			testSetup.outputData();		
		}
	else if(control==1)
		{
			// Constructor at specified temperature
			TestSetup testSetup(3, config); 
			cout << endl << "Specified Temperature Constructor" << endl;
			testSetup.outputData();		
		}
	else if(control==2)
		{
			// Read from ConfigFile			
			TestSetup testSetup(7, config);
			cout << endl << "ConfigFile" <<endl;
			pipe.outputData();		
		}
	else if (control==3)
		{
			// Input material data
		    testSetup.outputData();			
			cout << endl << "Input TestSetup Data" << endl;
			testSetup.inputData();
		}	
	else
	{
	cout << "Enter an appropriate test case";	
	}
	
	
	
}