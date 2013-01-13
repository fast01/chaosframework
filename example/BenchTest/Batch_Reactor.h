/*
 
 il tempo di ciclo della control unit deve essere pari a 10ms (uguale a quello del controllo)
 
 dopo aver inizializzato il Batch_Reactor con le condizioni iniziali le chimate delle funzioni nel loop avvengono con il seguente ordine:
 
 
 compute_output;
 update_output;
 set_controllo;
 compute_state;
 
 
 */
#ifndef ChaosFramework_reactor
#define ChaosFramework_reactor
#include <boost/random.hpp>
#include <math.h>
#include "config.h"


using namespace boost;

class Batch_Reactor {
    double error;
	double y_trasmessa[Q];
	double perturbation[N]; // initial condition
    
	public:
    double x[N]; // initial condition
	double y[Q]; // reactor output
	double u[P]; // control value
	
    Batch_Reactor();
	void compute_output();
	void compute_state(); //non modificare il tempo di campionamente DeltaT!!!
	void update_output();
	void perturbateState(double seed);
};
#endif

