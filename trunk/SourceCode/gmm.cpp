// Gaussian mixture model class
// Hand writing recognition Januari project, MSc AI, University of Amsterdam
// Thijs Kooi, 2011

#include "gmm.h"

//To do:
// Add EM to train priors
// Work on elegant initialisation of the model for 1 or more mixture components
// optimise, work on arrays rather than vectors

double PI = 4.0*atan(1.0);

//Constructors and intialisation functions
GMM::GMM(int d) { mixture_components = 1; data_dimension = d; initialiseParameters(); }
GMM::GMM(int d, int n) { mixture_components = n; data_dimension = d; initialiseParameters(); }
GMM::GMM(vector<double> mu,vector<vector<double> > sigma) 
{ 
	mixture_components = 1;
	priors.push_back(1.0);
	if(mu.size() != sigma.size())
	{
		cout << "Error: size of mean and covariance do not agree." << endl;
		return;
	}
	data_dimension = mu.size();
	means.push_back(mu);
	covariances.push_back(sigma);
}

void GMM::initialiseParameters()
{
	vector<vector<double> > zero_matrix;
	vector<double> zero_vector;
	
	zero_matrix.reserve(data_dimension);
	zero_vector.reserve(data_dimension);
	
	for(size_t i = 0; i < data_dimension; ++i)
		zero_vector.push_back(0.0);
	for(size_t i = 0; i < data_dimension; ++i)
		zero_matrix.push_back(zero_vector);
	
	for(size_t i = 0; i < mixture_components; ++i)
	{
		priors.push_back(0.0);
		means.push_back(zero_vector);
		covariances.push_back(zero_matrix);
	}
}
//end constructors and initialisation functions

double GMM::gmmProb(vector<double> x)
{
	double product = 0.0;
	for(size_t k = 0; k < mixture_components; ++k)
		product+=priors[k]*gmmProb(x,k);
	return product;
}

double GMM::gmmProb(vector<double> x, int component_number)
{
	double normalisation_constant = 1.0/( pow((2.0*PI),data_dimension/2.0) * pow(determinant(covariances[component_number]),0.5) );
	double exponent = -0.5*mahalanobisDistance(x,means[component_number],covariances[component_number]);
	
	return normalisation_constant*exp(exponent);
}

//Math functions
double GMM::mahalanobisDistance(vector<double> x,vector<double> mean,vector<vector<double> > covariance)
{
	vector<double> difference;
	for(size_t d = 0; d < data_dimension; ++d)
		difference.push_back(x[d]-mean[d]);
	
	vector<vector<double> > inverse_covariance = inverse(covariance);
	
	vector<double> distance ;
	for(size_t d = 0; d < data_dimension; ++d)
		distance.push_back(innerProduct(difference,inverse_covariance[d]));
	
	return innerProduct(distance,difference);
}

double GMM::innerProduct(vector<double> a, vector<double> b)
{
	double sum = 0.0;
	for(size_t d = 0; d < a.size(); ++d)
		sum+=a[d]*b[d];
	return sum;
}

double GMM::determinant(vector<vector<double> > A)
{
	if(A.size() == 1)
		return A[0][0];
	else if(A.size() == 2)
		return A[0][0]*A[1][1] - A[1][0]*A[0][1];
	else
	{
		double det = 0.0;
		for(size_t d = 0; d < A.size(); ++d)
		{
			if(d%2 == 0)
				det+=A[0][d]*determinant(getMinor(A,0,d));
			else
				det-=A[0][d]*determinant(getMinor(A,0,d));
		}
		return det;
	}	
}

vector<vector<double> > GMM::getMinor(vector<vector<double> > mat, int m, int n)
{
	mat.erase(mat.begin()+m);
	for(size_t j = 0; j < mat.size(); ++j)
		mat[j].erase(mat[j].begin()+n);
	
	return mat;
}

vector<vector<double> > GMM::inverse(vector<vector<double> > A)
{
	double detA = determinant(A);
	if(detA == 0.0)//Matrix is singular
		return A;
	else
	{
		vector<vector<double> > AInverse;
		vector<double> row;
		for(size_t m = 0; m < A.size(); ++m)
		{
			row.clear();
			for(size_t n = 0; n < A.size(); ++n)
			{
				if((m+n)%2 == 0)
					row.push_back( (1.0/detA)*determinant(getMinor(A,m,n)));
				else
					row.push_back( (-1.0/detA)*determinant(getMinor(A,m,n)));
			}
			AInverse.push_back(row);
		}
		
		return transpose(AInverse);
	}
}

vector<vector<double> > GMM::transpose(vector<vector<double> > A)
{
	vector<vector<double> > Atranspose;
	vector<double> row;
	
	for(size_t m = 0; m < A.size(); ++m)
	{
		row.clear();
		for(size_t n = 0; n < A.size(); ++n)
			row.push_back(0.0);
		Atranspose.push_back(row);
	}
	for(size_t m = 0; m < A.size(); ++m)
		for(size_t n = 0; n < A.size(); ++n)
			Atranspose[m][n] = A[n][m];
		
	return Atranspose;
	
}
//End math functions

//Getters and setters
int GMM::getMixtureComponents() { return mixture_components; }
void GMM::setMixtureComponents(int N) { mixture_components =  N; }

int GMM::getDimension() { return data_dimension; }
void GMM::setDimension(int d) { data_dimension = d; }

int GMM::getPrior(int component_number) { return priors[component_number]; }
void GMM::setPrior(int component_number,double probability) { priors[component_number] = probability; }

vector<double> GMM::getMean(int component_number) { return means[component_number]; }
void GMM::setMean(int component_number,vector<double> mean) { means[component_number] = mean; }

vector<vector<double> > GMM::getCovariance(int component_number) { return covariances[component_number]; } 
void GMM::setCovariance(int component_number, vector<vector<double> >covariance) { covariances[component_number] = covariance; }
//End getters and setters

//Print functions
void GMM::printPrior(int component_number)
{
	cout << "Prior for component " << component_number << endl;
	cout << priors[component_number] << endl;
}
void GMM::printMean(int component_number)
{
	cout << "Mean of component " << component_number << endl;
	printMatrix(means[component_number]);
}
void GMM::printCovariance(int component_number)
{
	cout << "Covariance of component " << component_number << endl;
	printMatrix(covariances[component_number]);
}
void GMM::printParameters(int component_number)
{
	cout << "Parameters for component " << component_number << endl;
	cout << "Prior: " << priors[component_number] << endl;
	cout << "Means: " << endl;
	printMatrix(means[component_number]);
	cout << "Covariance: " << endl;
	printMatrix(covariances[component_number]);
}
//end print functions

// Testing and debugging
void GMM::testMahalanobisDistance(vector<double> x)
{
	cout << mahalanobisDistance(x, means[0],covariances[0]) << endl;
}

void GMM::testInnerProduct(vector<double> a,vector<double> b)
{
	cout << innerProduct(a,b) << endl;
}

void GMM::testDeterminant(vector<vector<double> > A)
{
	cout << "Determinant of matrix: " << endl;
	for(size_t i = 0; i < A.size(); ++i)
	{
		for(size_t j = 0; j < A[i].size(); ++j)
			cout << A[i][j] << " ";
		cout << endl;
	}
	cout << "is: " << determinant(A) << endl;
}

void GMM::testInverse(vector<vector<double> > A)
{
	vector<vector<double> > Ainverse = inverse(A);
	cout << "Inverse of :" << endl;
	printMatrix(A);
	cout << "Is: " << endl;
	printMatrix(Ainverse);
}

void GMM::testMVNPDF(vector<double>)
{
}
void GMM::testGMM(vector<double>)
{
}
void GMM::testTranspose(vector<vector<double> > A)
{
	vector<vector<double> > Atranspose = transpose(A);
	cout << "Transpose of :" << endl;
	printMatrix(A);
	cout << "Is: " << endl;
	printMatrix(Atranspose);
}
void GMM::printMatrix(vector<vector<double> > A)
{
	for(size_t i = 0; i < A.size(); ++i)
	{
		for(size_t j = 0; j < A[i].size(); ++j)
			cout << A[i][j] << " ";
		cout << endl;
	}
}
void GMM::printMatrix(vector<double> b)
{
	for(size_t i = 0; i < b.size(); ++i)
		cout << b[i] << " ";
	cout << endl;
}
// vector<double> GMM::makeVector(double[] a)
// {
// 	
// }
// end 