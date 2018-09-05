#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h> 
#include <process.h>
#include <stdio.h>
#include <stdlib.h> 

#include "baggie.h"


unsigned _stdcall comp_wrapper(void *foo);

int main(int argc, char* argv[])
{	//data.txt N 8 W   
	baggie **ppbaggies;
	HANDLE *pThread;
	unsigned *pthreadID;
	HANDLE consolemutex;
	HANDLE *mastermutexes;

	int retcode = 0;
	FILE *in = NULL, *out = NULL;
	char mybuffer[100];

	if (argc != 5) {
		printf("Usage: main.exe datafilename instance_num 8 W\n"); retcode = 100; goto BACK;
	}

	//open the file to read
	in = fopen(argv[1], "r");
	if (in == NULL) {
		printf("could not open %s for reading\n", argv[1]);
		retcode = 200; goto BACK;
	}

	int instance_num = atoi(argv[2]);
	int param_num = atoi(argv[3]);
	int W = atoi(argv[4]);
	
	double * data = (double *)calloc(instance_num*param_num, sizeof(double));
	if (!data) {
		printf("cannot allocate space for data\n");
		retcode = 200; goto BACK;
	}

	//read in data
	for (int i = 0; i < instance_num; i++) {
		for (int j = 0; j < param_num; j++) {
			fscanf(in, "%[^,],", mybuffer);
			data[i*param_num + j] = atof(mybuffer);
		}
	}

	printf("Data read done!");
	fclose(in);

	//allocate memory to create instance # of baggies
	ppbaggies = (baggie **)calloc(instance_num, sizeof(baggie *));
	if (ppbaggies == NULL) {
		printf("cannot allocate %d \n", instance_num);
		retcode = 1; goto BACK;
	}
	//instance # of threads
	pThread = (HANDLE *)calloc(instance_num, sizeof(HANDLE));
	pthreadID = (unsigned *)calloc(instance_num, sizeof(unsigned));
	mastermutexes = (HANDLE *)calloc(instance_num, sizeof(HANDLE));
	if ((pThread == NULL) || (pthreadID == NULL) || (mastermutexes == NULL)) {
		printf("cannot allocate %d handles and thread ids\n", instance_num);
		retcode = 1; goto BACK;
	}

	//store input into baggies
	for (int p = 0; p < instance_num; p++) {
		ppbaggies[p] = new baggie((int)data[p*param_num + 0], (int)data[p*param_num + 1], data[p*param_num + 2], data[p*param_num + 3],
			data[p*param_num + 4], data[p*param_num + 5], data[p*param_num + 6], data[p*param_num + 7],p);
		ppbaggies[p]->printBag();
	}
	
	//create mutex to control console
	consolemutex = CreateMutex(NULL, 0, NULL);

	for (int j = 0; j < instance_num; j++) {
		ppbaggies[j]->setconsolemutex(consolemutex); // consolemutex shared across workers plus master
	}

	//mutex to work 
	HANDLE heavymutex;
	heavymutex = CreateMutex(NULL, 0, NULL);

	int nowinheavy = 0;
	
	//set mutex in baggies
	for (int j = 0; j < instance_num; j++) {
		mastermutexes[j] = CreateMutex(NULL, 0, NULL);
		ppbaggies[j]->setmastermutex(mastermutexes[j]);

		ppbaggies[j]->setmaxworkersinheavysection(W);
		ppbaggies[j]->setheavysectionmutex(heavymutex);
		ppbaggies[j]->setnowinheavyaddress(&nowinheavy);
	}
	
	//start to work
	for (int j = 0; j < instance_num; j++) {
		pThread[j] = (HANDLE)_beginthreadex(NULL, 0, &comp_wrapper, (void *)ppbaggies[j],
			0, &pthreadID[j]);
	}

	int numberrunning = instance_num;

	//check status of all workers
	for (; numberrunning > 0;) {
		Sleep(5000);
		printf("master will now check on workers\n"); fflush(stdout);

		for (int j = 0; j < instance_num; j++) {
			char jstatus = RUNNING;

			WaitForSingleObject(mastermutexes[j], INFINITE);
			//get status of workers
			jstatus = ppbaggies[j]->getstatus();

			ReleaseMutex(mastermutexes[j]);

			if (jstatus == FINISHED) {
				--numberrunning;
				WaitForSingleObject(consolemutex, INFINITE);
				printf("master: worker %d has done its compuation\n", j);
				ReleaseMutex(consolemutex);
			}
		}
	}
	//work finished and delete baggies
	for (int j = 0; j < instance_num; j++) {
		WaitForSingleObject(pThread[j], INFINITE);
		printf("--> thread %d done\n", j);
		delete ppbaggies[j]; // calls destructor
	}
	free(ppbaggies);
	
BACK:
	return retcode;
}



unsigned _stdcall comp_wrapper(void *genericaddress)
{
	baggie *pbaggie = (baggie *)genericaddress;

	//	comp(pbag->v1, pbag->v2, pbag->v3, &(pbag->result));
	//comp(pbag);

	pbaggie->baggiecomp();

	return 0;
}














