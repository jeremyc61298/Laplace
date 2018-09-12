#include <iostream>
#include <string>
#include <mpi.h>
#include "heatmap.hpp"
#include "matrix.hpp"
#include "laplace.hpp"

//#define DEBUG

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

	MPI_Finalize();
}