#include <iostream>
#include <string>
#include <mpi.h>
#include "matrix.hpp"
#include "laplace.hpp"

#define DEBUG

using std::cout;
using std::cin;
using std::endl;
using std::string;
using my::matrix;

int main(int argc, char* argv[])
{
	laplace lp;
	lp.initMPI(argc, argv);
	lp.inputSpecs();

#ifdef DEBUG
	if (lp.pid == 0)
	{
		lp.sheet.print(cout);
	}
#endif

	lp.shareInput();
	lp.distributeRows();
	lp.solve();
	lp.collectResults();
	lp.printResults(cout);

#ifdef DEBUG
	lp.print_all_chunks(1);
#endif


	// Cant stay here
	MPI_Finalize();
}