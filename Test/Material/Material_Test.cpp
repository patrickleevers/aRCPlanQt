// Material_Test.cpp
// Author: Fraser Edwards

// Program to test performance of Material.cpp
// Tests: null constructor, constructor at specified temperature, reads configfile, editing of parameters
//        ,display of data.

#include <iostream>

using namespace std;

#include "ConfigFile.h"
#include "Material.h"

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

	Material material;	
	
	if(control==0)
		{
		    // declare and initialise material class using 
	        // Null constructor
			Material material;	
			cout << endl << "Null Constructor" << endl;
			material.outputData();		
		}
	else if(control==1)
		{
			// Constructor at specified temperature
			Material material(3, config); 
			cout << endl << "Specified Temperature Constructor" << endl;
			material.outputData();		
		}
	else if(control==2)
		{
			// Read from ConfigFile			
			Material material(7, config);
			cout << endl << "ConfigFile" <<endl;
			material.outputData();		
		}
	else if (control==3)
		{
			// Input material data
		    material.outputData();			
			cout << endl << "Input Material Data" << endl;
			material.inputData();
		}	
	else
	{
	cout << "Enter an appropriate test case";	
	}
	
	
	
}