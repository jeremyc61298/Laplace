// -----------------------------------------------------------
// laplace.hpp
// Jeremy Campbell
// This class will model Laplace's equation in parallel. 
// The main data structure is Dr.Foust's matrix, which he gave
// to us for use in this project.
// -----------------------------------------------------------
#pragma once
#include <mpi.h>
#include <iostream>
#include <ostream>
#include "matrix.hpp"
#include "heatmap.hpp"

//#define DEBUG

using my::matrix;

struct laplace
{
	/* DATA */
	int numProcs;
	int pid;
	int numRows;
	int numCols;
	int numRowsPerProc;
	const double SIG_CHANGE;
	matrix<double> sheet;
	matrix<double> myChunk;
	matrix<double> myChange;

	// The "extended" values include the extra rows and columns (Given by Dr. Foust)
	int extRows() { return numRows + 2; }
	int extCols() { return numCols + 2; }
	int extRowsPerProc() { return numRowsPerProc + 2; }

	/* FUNCTIONS */
	laplace() : numProcs(0), pid(0), numRows(0), numCols(0), 
		numRowsPerProc(0), SIG_CHANGE(0.000001) {}
	void initMPI(int argc, char* argv[]);
	void inputSpecs();
	void shareInput();
	void distributeRows();
	void solve();
	int	 average();
	void shareChangedRows();
	void collectResults();
	void printResults();
	void printResults(std::ostream& out) const;
	void print_all_chunks(int tag);
	double absolute(double num);
	void finalize();
};


void laplace::initMPI(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	// Which process am I?
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);

	// How many processes are there?
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
}

void laplace::inputSpecs()
{
	if (pid == 0)
	{
		int numHeat;
		std::cin >> numRows >> numCols >> numHeat;

		// Resize the sheet
		sheet.resize(extRows(), extCols());
		sheet.fill(0);
		numRowsPerProc = numRows / numProcs;

		for (int i = 0; i < numHeat; i++)
		{
			// Make Robust Later!
			int x1, y1, x2, y2;
			double heatlvl;
			std::cin >> x1 >> y1 >> x2 >> y2 >> heatlvl;
			
			// non ideal, don't want to pass sheet to it's own function,
			// however, using *this in that function doesn't allow the 
			// grid to fill properly
			sheet.fill_region(heatlvl, sheet, x1, y1, x2, y2);
		}
	}
}

void laplace::shareInput()
{
	MPI_Bcast(&numCols, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&numRowsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);
	myChunk.resize(extRowsPerProc(), extCols());
	myChange.resize(extRowsPerProc(), extCols());
}

void laplace::distributeRows()
{
	if (pid == 0)
	{
		// Give process 0 his data
		for (int i = 0; i < extRowsPerProc() * extCols(); i++)
		{
			myChunk.data()[i] = sheet.data()[i];
		}
		// Send
		for (int i = numRowsPerProc; i < numRows; i += numRowsPerProc)
		{
			MPI_Send(sheet[i], extRowsPerProc() * extCols(), MPI_DOUBLE, i / numRowsPerProc, 0, MPI_COMM_WORLD);
		}
	}
	else
	{
		// Recv
		MPI_Recv(myChunk.data(), extRowsPerProc() * extCols(), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	myChange = myChunk;
}

void laplace::solve()
{
	int someoneChanged;
	do
	{
		int changed = average();
		shareChangedRows();
		std::swap(myChunk, myChange);
		MPI_Allreduce(&changed, &someoneChanged, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	} while (someoneChanged == 1);
}

// Each process averages each slot in 
// the matrix with it's surrounding slots
int laplace::average()
{
	int changed = 0;
	for (int i = 1; i <= numRowsPerProc; i++)
	{
		for (int j = 1; j <= numCols; j++)
		{
			double sum = myChunk[i - 1][j] + myChunk[i][j + 1] + myChunk[i + 1][j] + myChunk[i][j - 1];
			myChange[i][j] = sum / 4;

			if (absolute(myChange[i][j] - myChunk[i][j]) > SIG_CHANGE)
				changed = 1;
		}
	}
	return changed;
}

// Send and Receive the last row that was changed, then the first row that was changed
// Note, process 0 does nothing with it's first two rows, while process numProcs - 1 does
// nothing with it's last two rows. 
void laplace::shareChangedRows()
{
	if (pid != numProcs - 1)
		MPI_Send(myChange[numRowsPerProc], extCols(), MPI_DOUBLE, pid + 1, 0, MPI_COMM_WORLD);

	if (pid != 0)
	{
		MPI_Recv(myChange[0], extCols(), MPI_DOUBLE, pid - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(myChange[1], extCols(), MPI_DOUBLE, pid - 1, 0, MPI_COMM_WORLD);
	}
		
	if (pid != numProcs - 1)
		MPI_Recv(myChange[numRowsPerProc + 1], extCols(), MPI_DOUBLE, pid + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void laplace::collectResults()
{
	MPI_Gather(myChunk[1], numRowsPerProc * extCols(), MPI_DOUBLE, 
		sheet[1], numRowsPerProc * extCols(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

// Prints to an ostream
void laplace::printResults(std::ostream& out) const
{
	if (pid == 0)
		sheet.print(out);
}

// Prints a picture
void laplace::printResults()
{
	if (pid == 0)
	{
		my::heatmap map;
		map.sheet = sheet.data();           // pointer to the 2-dimensional array
		map.num_cols = extCols();          // total number of columns in each row
		map.top_row = 1;                            // 
		map.left_col = 1;                           //  |__ The part of the array to include in the image
		map.bottom_row = extRows() - 1;    //  |   (includes top & left, excludes bottom & right)
		map.right_col = extCols() - 1;     // /
		map.scale = 8;                              // How much to scale the image by
		map.file_name = "sheet.png";                // File to save it in
		map.print_to_file();
	}	
}

// Debug function
void laplace::print_all_chunks(int tag) {
	for (int i = 0; i < numProcs; ++i) {
		if (pid == i) {
			std::cout << '[' << tag << "] Process " << pid << " has chunk:\n" << ((tag == 1) ? myChunk : myChange)<< std::endl;
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
}

// Calculates the absolute value of a double
double laplace::absolute(double num)
{
	double result = num;
	if (num < 0)
		result *= -1;
	return result;
}

void laplace::finalize()
{
	MPI_Finalize();
}