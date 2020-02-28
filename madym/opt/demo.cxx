#include <stdio.h>
#include <windows.h>
#include "stdafx.h"
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <vcl_compiler.h>
#include <iostream>
#include <iostream>
#include "optimization.h"

#include "LinAlg.h"

using namespace alglib;

double counter()
{
	return 0.001*GetTickCount();
}

void speed_test()
{
	alglib::real_2d_array a, b, c;
	int n = 2000;
	int i, j;
	double timeneeded, flops;

	// Initialize arrays
	a.setlength(n, n);
	b.setlength(n, n);
	c.setlength(n, n);
	for (i = 0; i<n; i++)
		for (j = 0; j<n; j++)
		{
			a[i][j] = alglib::randomreal() - 0.5;
			b[i][j] = alglib::randomreal() - 0.5;
			c[i][j] = 0.0;
		}

	// Set number of worker threads: "4" means "use 4 cores".
	// This line is ignored if AE_OS is UNDEFINED.
	alglib::setnworkers(4);

	// Perform matrix-matrix product.
	// We call function with "smp_" prefix, which means that ALGLIB
	// will try to execute it in parallel manner whenever it is possible.
	flops = 2 * pow((double)n, (double)3);
	timeneeded = counter();
	alglib::smp_rmatrixgemm(
		n, n, n,
		1.0,
		a, 0, 0, 0,
		b, 0, 0, 1,
		0.0,
		c, 0, 0);
	timeneeded = counter() - timeneeded;

	// Evaluate performance
	printf("Performance is %.1f GFLOPS\n", (double)(1.0E-9*flops / timeneeded));

	return;
}

void function1_func(const real_1d_array &x, double &func, void *ptr)
{
	//
	// this callback calculates f(x0,x1) = 100*(x0+3)^4 + (x1-3)^4
	//
	func = 100 * pow(x[0] + 3, 4) + pow(x[1] - 3, 4);
}

void optimisation_test()
{

	//
	// This example demonstrates minimization of f(x,y) = 100*(x+3)^4+(y-3)^4
	// subject to bound constraints -1<=x<=+1, -1<=y<=+1, using BLEIC optimizer.
	//

	std::vector<double> xc = { 0, 0 };
	std::vector<double> bndlc = { -1,-1 };
	std::vector<double> bnduc = { +1,+1 };


	alglib::real_1d_array x;
	real_1d_array bndl;
	real_1d_array bndu;

	x.attach_to_ptr(2, xc.data());
	bndl.attach_to_ptr(2, bndlc.data());
	bndu.attach_to_ptr(2, bnduc.data());
	minbleicstate state;
	minbleicreport rep;

	//
	// These variables define stopping conditions for the optimizer.
	//
	// We use very simple condition - |g|<=epsg
	//
	double epsg = 0.000001;
	double epsf = 0;
	double epsx = 0;
	ae_int_t maxits = 0;

	//
	// This variable contains differentiation step
	//
	double diffstep = 1.0e-6;

	//
	// Now we are ready to actually optimize something:
	// * first we create optimizer
	// * we add boundary constraints
	// * we tune stopping conditions
	// * and, finally, optimize and obtain results...
	//
	minbleiccreatef(x, diffstep, state);
	minbleicsetbc(state, bndl, bndu);
	minbleicsetcond(state, epsg, epsf, epsx, maxits);
	alglib::minbleicoptimize(state, function1_func);
	minbleicresults(state, x, rep);

	std::vector<double> opt(x.getcontent(), x.getcontent()+2);

	//
	// ...and evaluate these results
	//
	printf("%d\n", int(rep.terminationtype)); // EXPECTED: 4
	printf("%s\n", x.tostring(2).c_str()); // EXPECTED: [-1,1]
	std::cout << "[" << opt[0] << ", " << opt[1] << "]" << std::endl;
	return;

}

int main(int argc, char **argv)
{
	speed_test();
	optimisation_test();
	return 0;
}