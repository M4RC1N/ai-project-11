// Hidden Markov model class
// Hand writing recognition Januari project, MSc AI, University of Amsterdam
// Thijs Kooi, 2011

#include "hmm.h"

// To do:
// - Do hand calculations on forward/backward probability and probability of observation sequences, for different models (2).
// - Do hand calculations for viterbi sequence
// - check for possible underflow forward/backward probability, normalise/log probabilities
// - Extend to higher dimensional observations
// - Add Gaussian/mixture of Gaussians
// - Check model with HMM toolkit available

int main()
{
	//Testing
	HMM markov_model(4,4);
	int test_obs[6] = {3,1,2,1,0,2};
	
	markov_model.trainModel(test_obs,6);
}

//Constructors and initialisation functions
//When no further model parameters are passed, the model is initialised uniform
//In the case of discrete observations, no takes the number of possible observations
//Please note that the observations should not be passed as actual values, but as indexes of the vocabulary.
HMM::HMM(int ns, int no) 
{ 
	number_of_states = ns;
	number_of_observations = no;
	initialiseUniform();
}

HMM::HMM(int ns, int no, double* p, map<int, map<int, double> > t, map<int, map<int, double> > o)
{
	number_of_states = ns;
	number_of_observations = no;
	prior_probabilities = p;
	transition_probabilities = t;
	observation_probabilities = o;
}

//Initialise the HMM with ns states and a mixture of Guassians, corresponding to every state
HMM::HMM(int ns, vector<GMM> mixture_models)
{
	number_of_states = ns;
	observation_dimension = mixture_models[0].getDimension();
}

void HMM::initialiseUniform() 
{ 
	//Initialise elements of transition map
	for(size_t i = 0; i < number_of_states; ++i)
		for(size_t j = 0; j < number_of_states; ++j)
			transition_probabilities[i][j] = 1.0;
		
	//Initialise uniform probabilities
	for(map<int, map<int,double> >::iterator i = transition_probabilities.begin(); i != transition_probabilities.end(); ++i)
		for(map<int, double>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
			(*j).second/=(*i).second.size();
		
	prior_probabilities = new double[number_of_states];
	//Initialise uniform prior probabilities
	for(size_t i = 0; i < number_of_states; ++i)
		prior_probabilities[i] = 1.0/number_of_states;
	
	//Initialise uniform probabilities for observations
	for(size_t i = 0; i < number_of_states; ++i)
		for(size_t j = 0; j < number_of_observations; ++j)
			observation_probabilities[i][j] = 1.0;
		
	for(map<int, map<int,double> >::iterator i = observation_probabilities.begin(); i != observation_probabilities.end(); ++i)
		for(map<int, double>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
			(*j).second/=(*i).second.size();
}
//End constructors and initialisation functions

//Getters and setters
int HMM::getStates(){ return number_of_states;  }
int HMM::getNumberOfObservations(){ return number_of_observations; }
//End getters and setters

//Training functions
void HMM::trainModel(int* o, int l)
{
	observations = o;
	observation_sequence_length = l;
	
	cout << "Training model..." << endl;
	
	double previous_likelihood = 0.0;
	double current_likelihood = observationSequenceProbability(o,l);
	int it = 0;
	
	while(current_likelihood - previous_likelihood > 0.000001)
	{
		eStep();
		mStep();
		previous_likelihood = current_likelihood;
		current_likelihood = observationSequenceProbability(o,l);
		cout << "Likelihood at iteration " << it << current_likelihood << endl;
		++it;
	}
	
	cout << "Converged after " << it+1 << " iterations." << endl;
}

void HMM::eStep() 
{ 
	for(size_t i = 0; i < number_of_states; ++i)
		for(size_t t = 0; t < observation_sequence_length; ++t)
			gamma[i][t] = stateProbability(i,t);
	
	for(size_t i = 0; i < number_of_states; ++i)
		for(size_t j = 0; j < number_of_states; ++j)
			for(size_t t = 0; t < observation_sequence_length; ++t)
				xi[i][j][t] = stateToStateProbability(i,j,t);
}

//Generally denoted alpha in the literature
//Please note that the literature typically numbers states and observations 1-N, 1-K respectively,
//whereas the programming language starts enumerating at 0
double HMM::forwardProbability(int state, int timestep)
{
	if(state == 0 && timestep == 0)
		return 1.0;
	else if(timestep == 0)
		return prior_probabilities[timestep]*observation_probabilities[state][observations[0]];
	
	double sum = 0.0;
	for(size_t i = 0; i < number_of_states; ++i)
		sum+=forwardProbability(i,timestep-1)*transition_probabilities[i][state];
		
	return sum*observation_probabilities[state][observations[timestep+1]];
}
//#805 0x0804a7a8 in HMM::backwardProbability (this=0xbffff218, state=0, timestep=56) at hmm.cpp:146
//Generally denoted beta in the literature
double HMM::backwardProbability(int state, int timestep)
{
	if(timestep == observation_sequence_length-1)
		return 1.0;
	
	double sum = 0.0;
	for(size_t j = 0; j < number_of_states; ++j)
		sum+=transition_probabilities[state][j]*observation_probabilities[j][observations[timestep+1]]*backwardProbability(j,timestep+1);
	
	return sum;
}

//Generally denoted gamma in the literature
double HMM::stateProbability(int state, int timestep)
{
	double normalisation_constant = 0.0;
	
	for(size_t i = 0; i < number_of_states; ++i)
		normalisation_constant+= forwardProbability(i,timestep)*backwardProbability(i,timestep);
	
	return (forwardProbability(state,timestep)*backwardProbability(state,timestep))/normalisation_constant;
}

//Generally denoted xi in the literature
double HMM::stateToStateProbability(int state_i, int state_j, int timestep)
{
	double normalisation_constant = 0.0;
	
	for(size_t k = 0; k < number_of_states; ++k)
		for(size_t l = 0; l < number_of_states; ++l)
			normalisation_constant+= forwardProbability(k,timestep)*transition_probabilities[k][l]*observation_probabilities[k][observations[timestep+1]]*backwardProbability(l,timestep+1);
		
	return (forwardProbability(state_i, timestep)*observation_probabilities[state_j][observations[timestep+1]]*backwardProbability(state_j,timestep+1))/normalisation_constant;
}

void HMM::mStep()
{
	maximisePriors();
	maximiseTransitions();
	maximiseObservationDistribution();
}

void HMM::maximisePriors()
{
	for(size_t i = 0; i < number_of_states; ++i)
		prior_probabilities[i] = gamma[i][1];
}

void HMM::maximiseTransitions()
{
	for(map<int, map<int,double> >::iterator i = transition_probabilities.begin(); i != transition_probabilities.end(); ++i)
		for(map<int,double>::iterator j = i->second.begin(); j != i->second.end(); ++j)
			updateTransition(i->first,j->first);
}

void HMM::updateTransition(int i, int j)
{
	double numerator = 0.0;
	double denominator = 0.0;
	
	for(size_t t = 0; t < observation_sequence_length-1; ++t)
	{
		numerator+=xi[i][j][t];
		denominator+=gamma[i][t];
	}
	
	transition_probabilities[i][j] = numerator/denominator;
}

void HMM::maximiseObservationDistribution()
{
	for(size_t i = 0; i < number_of_states; ++i)
		for(size_t m = 0; m < number_of_observations; ++m)
			updateObservationDistribution(i,m);
}

void HMM::updateObservationDistribution(int state, int observation_index)
{
	double numerator = 0.0;
	double denominator = 0.0;
	
	for(size_t t = 0; t < observation_sequence_length; ++t)
	{
		if(observations[t] == observation_index)
			numerator+=gamma[state][t];
		denominator+=gamma[state][t];
	}
	
	observation_probabilities[state][observation_index]= numerator/denominator;
}
//End training functions

//Model properties
double HMM::stateSequenceProbability(vector<int> sequence)
{
	double probability = prior_probabilities[sequence[0]];
	for(size_t i = 1; i < sequence.size(); ++i)
		probability*=transition_probabilities[sequence[i-1]][sequence[i]];
	
	return probability;
}

//Returns the probability of the sequence under the given model
double HMM::observationSequenceProbability(int *sequence,int length)
{
	double probability = 0.0;
	observation_sequence_length = length;
	observations = sequence;
	
	for(size_t i = 0; i < number_of_states; ++i)
	{
		cout << i << " " << forwardProbability(i,observation_sequence_length-1) << endl;
		probability+=forwardProbability(i,observation_sequence_length-1);
	}
	
	return probability;
}

//These functions need some work in efficiency and readability
int* HMM::viterbiSequence(int* observation_sequence, int l)
{
	observation_sequence_length = l;
	observations = observation_sequence;
	
	//Declare and initialise dynammic programming table
	//{delta,psi}[states,timesteps]
	double **delta = new double*[number_of_states];
	int **psi = new int*[number_of_states];
	
	for(size_t i = 0; i < number_of_states; ++i)
	{
		delta[i] = new double[observation_sequence_length];
		psi[i] = new int[observation_sequence_length];
	}
	
	for(size_t i = 0; i < number_of_states; ++i)
	{
		delta[i][0] = prior_probabilities[i]*observation_probabilities[i][observations[0]];
		psi[i][0] = 0;
	}
	
	//Compute table
	int index;
	for(size_t t = 1; t < observation_sequence_length; ++t)
	{
		for(size_t i = 0; i < number_of_states; ++i)
		{
			delta[i][t] = highestPathProbability(i, t, delta, index);
			psi[i][t] = index;
		}
	}
	
	//Termination
	double max_probability = 0.0;
	for(size_t i = 0; i < number_of_states; ++i)
		if(delta[i][observation_sequence_length-1] > max_probability)
		{
			max_probability = delta[i][observation_sequence_length-1];
			index = i;
		}
	
	//Perform the backtrack
	int *state_sequence = new int[observation_sequence_length];
	for(size_t t = 0; t < observation_sequence_length-1; ++t)
		state_sequence[t] = 0;
	
	state_sequence[observation_sequence_length-1] = index;
	
	for(size_t t = observation_sequence_length-2; t > 0; --t)
		state_sequence[t] = psi[state_sequence[t+1]][t+1];

	state_sequence[0] = psi[state_sequence[1]][1];
	return state_sequence;
	
}

double HMM::highestPathProbability(int state, int timestep, double **delta, int index)
{
	double *probabilities = new double[number_of_states];
	for(size_t i = 0; i < number_of_states; ++i)
		probabilities[i] = delta[i][timestep-1]*transition_probabilities[i][state];
	
	return maxValue(probabilities,index)*observation_probabilities[state][observations[timestep]];
}

double HMM::maxValue(double* array, int index)
{
	double value = 0.0;
	index = 0;
	for(size_t i = 0; i < number_of_states; ++i)
		if(array[i] > value)
		{
			value = array[i];
			index = i;
			
		}
	return value;
}
//End properties




//Print functions
void HMM::printObservations()
{
	cout << "Training on observations: " << endl;
	for(size_t i = 0; i < number_of_observations; ++i)
		cout << observations[i] << " ";
	cout << endl;
}
void HMM::printPriorProbabilities()
{
	cout << "Current prior probabilities: " << endl;
	for(size_t i = 0; i < number_of_states; ++i)
		cout << prior_probabilities[i] << " ";
	cout << endl;
}
void HMM::printTransitionProbabilities()
{
	cout << "Current transition probabilities: " << endl;
	for(map<int, map< int, double> >::iterator i = transition_probabilities.begin(); i != transition_probabilities.end(); ++i)
	{
		for(map<int, double>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
			cout << (*j).second << " ";
		cout << endl;
	}
}
void HMM::printObservationProbabilities()
{
	cout << "Current observation probabilities: " << endl;
	for(map<int, map< int, double> >::iterator i = observation_probabilities.begin(); i != observation_probabilities.end(); ++i)
	{
		for(map<int, double>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
			cout << (*j).second << " ";
		cout << endl;
	}
}
//End print functions