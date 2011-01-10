#ifndef GMM_H
#define GMM_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <map>

using namespace std;

class GMM {
	public:
		//Constructors
		GMM();
		GMM(int);
		GMM(vector<double>,vector<vector<double> > );
		//end constructors
		
		//Testing and debugging, will be removed later
		void testMahalanobisDistance(vector<double>);
		void testInnerProduct(vector<double>,vector<double>);
		void testDeterminant(vector<vector<double> >);
		void testInverse(vector<vector<double> >);
		void testTranspose(vector<vector<double> >);
		void testMVNPDF(vector<double>);
		void testGMM(vector<double>);
		void printMatrix(vector<vector<double> >);
		void printMatrix(vector<double>);
		vector<double> makeVector(double[]);
		//end 

		double gmmProb(vector<double> x);			//returns the probability of x under the current mixture model
		double gmmProb(vector<double> x, int component_number); //returns the probability of x under the given mixture component

		//Getters and setters
		int getMixtureComponents();
		void setMixtureComponents(int);
		
		int getDimension();
		void setDimension(int);
		
		int getPrior(int component_number);
		void setPrior(int component_number,double probability);
		
		vector<double> getMean(int component_number);
		void setMean(vector<double> mean);
		void setMean(int component_number,vector<double> mean);
		
		vector<vector<double> > getCovariance(int component_number);
		void setCovariance(vector<vector<double> >);
		void setCovariance(int component_number, vector<vector<double> > covariance);
		//End getters and setters
		
		//Print functions
		void printPrior(int component_number);
		void printMean(int component_number);
		void printCovariance(int component_number);
		//end print functions
		
	private:
		//GMM variables
		int mixture_components;
		int data_dimension;
		vector<double> priors;					//vector of priors for mixture components
		vector<vector<double> > means;				//matrix of means for mixture components
		vector<vector<vector<double> > > covariances;		//tensor of covariances for mixture components
		//End GMM variables
		
		//math functions
		double mahalanobisDistance(vector<double> x,vector<double> mean,vector<vector<double> > covariance);
		double determinant(vector<vector<double> >);								//Tested
		double innerProduct(vector<double>, vector<double>);							//Tested
		vector<vector<double> > getMinor(vector<vector<double> >, int, int); 					//Tested
		vector<vector<double> > inverse(vector<vector<double> >);						//Tested
		vector<vector<double> > transpose(vector<vector<double> >);						//Tested
		//end math functions
};

#endif