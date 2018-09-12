// -----------------------------------------------------------------
// laplace.cpp
// Jeremy Campbell
// Uses the laplace class to model laplace's heat transfer equation.
// -----------------------------------------------------------------
#include <iostream>
#include "laplace.hpp"

//#define DEBUG

using std::cout;
using std::cin;
using std::endl;

int main(int argc, char* argv[])
{
	laplace lp;
	lp.initMPI(argc, argv);
	lp.inputSpecs();

#ifdef DEBUG
	if (lp.pid == 0)
		lp.sheet.print(cout);
#endif

	lp.shareInput();
	lp.distributeRows();

#ifdef DEBUG
	lp.print_all_chunks(1);
#endif
	
	lp.solve();
	lp.collectResults();
	lp.printResults(cout);
	lp.printResults();
	lp.finalize();
}