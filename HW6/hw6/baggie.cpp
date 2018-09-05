#include<vector>
#include<algorithm>

#include <windows.h> 
#include <process.h>
#include "baggie.h"

using namespace std;

double mytimecheck(void);


// implementation file for class baggie

baggie :: baggie(int N_in, int T_in, double alpha_in, double pi1_in, double pi2_in, double p1_in, double p2_in, double rho_in, int name_in)
{
	N = N_in; T = T_in; alpha = alpha_in; pi1 = pi1_in; pi2 = pi2_in; p1 = p1_in; p2 = p2_in; rho = rho_in; 
	name = name_in;
	result = 0;
	status = WAITING;

}

//set mutex
void baggie :: setconsolemutex(HANDLE consolemutexinput)
{
	consolemutex = consolemutexinput;
}
void baggie :: setmastermutex(HANDLE mastermutexinput)
{
	mastermutex = mastermutexinput;
}

//logic to check whether a worker can enter heavysection to start working
void baggie :: letmein(void)
{
	char icangoin;
	int localinheavysection;
	
		icangoin = 0;
		while(icangoin == 0){
			Sleep(1000);
			WaitForSingleObject(heavysectionmutex, INFINITE);
			 
			if( (*address_of_nowinheavysection) < maxworkersinheavysection){
				/** key logic: it checks to see if the number of workers in the heavy section is less than the
				number we want to allow **/
				icangoin = 1;
				++*address_of_nowinheavysection; //** increase the count
				localinheavysection = *address_of_nowinheavysection;  
				// so localinheavysection will have the count of busy workers
			}

			ReleaseMutex(heavysectionmutex);
		}
		WaitForSingleObject(consolemutex, INFINITE);
		cout << "\n******worker" << name <<": I'm in and there are " << localinheavysection <<" total busy workers\n";
		// we can use localinheavysection without protecting it with a mutex, because it is a local variable to this function, i.e.
		// it is not shared with other mutexes
		ReleaseMutex(consolemutex);
}

void baggie :: seeya(void)
{
		WaitForSingleObject(heavysectionmutex, INFINITE);
		--*address_of_nowinheavysection;
		ReleaseMutex(heavysectionmutex); 
}

void baggie :: baggiecomp(void)
{
	letmein();
	double t1 = mytimecheck();  // see the comments below.  mytimecheck() returns the time of day in milliseconds
								// it is defined in mytimer.cpp


	//initialize
	vector<vector<double>> V(T, vector<double>(N + 1, 0));
	vector<vector<int>> x(T, vector<int>(N + 1, 0));

	//base case
	for (int k = 0; k <= N; k++) {
		vector<double> a(k + 1, 0);
		for (int h = 0; h <= k; h++) {
			a[h] = rho*h*(p1*(1 - alpha*pow((double)h, pi1)) + p2*(1 - alpha*pow((double)h, pi2)));
		}

		V[T - 1][k] = *max_element(a.begin(), a.end());
	}
	int x0 = 0;
	//recursion
	for (int t = T - 2; t >= 0; t--) {
		for (int k = 0; k <= N; k++) {
			vector<double> a(k + 1, 0);
			for (int h = 0; h <= k; h++) {
				double temp = (1 - rho)*V[t + 1][k] + rho*(h + V[t + 1][k - h]);
				a[h] = temp*(p1*(1 - alpha*pow((double)h, pi1)) + p2*(1 - alpha*pow((double)h, pi2)));
			}
			V[t][k] = *max_element(a.begin(), a.end());
			if (t == 0) {
				x0 = distance(a.begin(), max_element(a.begin(), a.end()));
			}
		}
		
	}
	result = V[0][N];
	
	double t2 = mytimecheck();  // check out to see how this function works, it's in mytimer.cpp
								// mytimecheck simply returns the time of day in milliseconds
	double tdiff;

	tdiff = t2 - t1;  // t1 was set above 

	WaitForSingleObject(consolemutex, INFINITE);
	printf(" >> worker %d:  I have completed one instance computation in time %g\n", name, tdiff);
	printf("Max=%g\n", V[0][N]);
	printf("x0=%d\n", x0);
	/*
	int s = N;
	for (int t = 0; t < T; t++) {
		printf("x[%d]=%d\n", t, x[t][s]);
		s = s - x[t][s];
	}
	*/

	ReleaseMutex(consolemutex);
	status = FINISHED;
	seeya();
	WaitForSingleObject(consolemutex, INFINITE);
	printf(" >> worker %d:  I am out\n", name);
	ReleaseMutex(consolemutex);

}

void baggie::printBag(void) {
	
	printf("\nBag %d:\n", name);
	printf("N = %d, T = %d, alpha= %g, pi1=%g, pi2=%g, p1=%g, p2=%g, rho=%g", N,
		T, alpha, pi1, pi2, p1, p2, rho);

}


