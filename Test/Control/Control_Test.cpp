// Control_Test.cpp
// Author: Fraser Edwards

// Program to test performance of Control.cpp
// Tests: null constructor, constructor at specified temperature, reads configfile, editing of parameters
//        ,display of data.

#include <iostream>

using namespace std;

#include "ConfigFile.h"
#include "Control.h"

int main()
{
	

	//declare control variables
	int var;
	
	
	ConfigFile config("caseInputData.txt");
	
	cout << endl << "Enter the test case: (Display tested with each case)" << endl 
			<< endl << "0: Null Constructor" << endl
			<< "1: Constructor at specified temperature" << endl
			<< "2: Read in from configfile" << endl
			<< "3: Edit Test Parameters" << endl;
	
	cin >> var;

	Control control;	
	
	if(var==0)
		{
		    // declare and initialise material class using 
	        // Null constructor
			Control control;	
			cout << endl << "Null Constructor" << endl;
			control.outputData();		
		}
	else if(var==1)
		{
			// Constructor at specified temperature
			Control control(3, config); 
			cout << endl << "Specified Temperature Constructor" << endl;
			control.outputData();		
		}
	else if(var==2)
		{
			// Read from ConfigFile			
			Control control(7, config);
			cout << endl << "ConfigFile" <<endl;
			control.outputData();		
		}
	else if (var==3)
		{
			// Input material data
		    control.outputData();			
			cout << endl << "Input Control Data" << endl;
			control.inputData();
		}	
	else
	{
	cout << "Enter an appropriate test case";	
	}
	
	
	
}