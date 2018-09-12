#pragma once
#include <mpi.h>
#include <string>
#include <iostream>
#include <ostream>
#include "matrix.hpp"

using my::matrix;

struct laplace
{
	/* DATA */
	int numProcs;
	int pid;
	int numRows;
	int numCols;
	int numRowsPerProc;
	const double SIG_CHANGE = 0.001;

	// The "extended" values include the extra rows and columns (Given by Dr. Foust)
	int extRows() { return numRows + 2; }
	int extCols() { return numCols + 2; }
	int extRowsPerProc() { return numRowsPerProc + 2; }

	matrix<double> sheet;
	matrix<double> myChunk;
	matrix<double> myChange;

	/* FUNCTIONS */
	laplace() : numProcs(0), pid(0), numRows(0), numCols(0), numRowsPerProc(0) {}
	void initMPI(int argc, char* argv[]);
	void inputSpecs();
	void shareInput();
	void distributeRows();
	void solve();
	int average();
	void shareChangedRows();
	void collectResults();
	void printResults(std::ostream& out) const;

	void print_all_chunks(int tag);

	void log(const std::string& message) {
		std::cout << pid << ": " << message << std::endl;
	}
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
			// non ideal 
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
		for (int i = 1; i < numProcs; i++)
		{
			MPI_Send(sheet[i], extRowsPerProc() * extCols(), MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
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

int laplace::average()
{
	int changed = 0;
	for (int i = 1; i <= numRowsPerProc; i++)
	{
		for (int j = 1; j <= numCols; j++)
		{
			double sum = myChunk[i - 1][j] + myChunk[i][j + 1] + myChunk[i + 1][j] + myChunk[i][j - 1];
			myChange[i][j] = sum / 4;

			if (myChange[i][j] - myChunk[i][j] > SIG_CHANGE)
				changed = 1;
		}
	}
	return changed;
}

void laplace::shareChangedRows()
{
	// Send and Receive the last row that was changed, then the first row that was changed
	// Note, process 0 does nothing with it's first two rows, while process numProcs - 1 does
	// nothing with it's last two rows. 
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

void laplace::printResults(std::ostream& out) const
{
	if (pid == 0)
		sheet.print(out);
}


void laplace::print_all_chunks(int tag) {
	for (int i = 0; i < numProcs; ++i) {
		if (pid == i) {
			std::cout << '[' << tag << "] Process " << pid << " has chunk:\n" << ((tag == 1) ? myChunk : myChange)<< std::endl;
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
}