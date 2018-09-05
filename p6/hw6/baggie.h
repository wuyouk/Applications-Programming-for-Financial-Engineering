#include <iostream> 

#define RUNNING 1
#define WAITING 0
#define FINISHED 2 
 

class baggie{
public:
	baggie(int N, int T, double alpha, double pi1, double pi2, double p1, double p2, double rho, int name);
	~baggie(){ printf("worker %d says goodbye\n", name); } 
  void setconsolemutex(HANDLE consolemutexinput);
  void setmastermutex(HANDLE consolemutexinput);
  void baggiecomp();
  void printBag(void);
 
  void setstatustofinished(void){status = FINISHED;}
  int getstatus(void){ return status; }
  void setheavysectionmutex(HANDLE heavysectioninput){heavysectionmutex = heavysectioninput;}
  void setmaxworkersinheavysection(int maxheavy){
	  maxworkersinheavysection = maxheavy;}
  void setnowinheavyaddress(int *paddress){address_of_nowinheavysection = paddress;}
 private:
  int N, T;
  double alpha, pi1, pi2, p1, p2, rho;
  int name;

  int status;
  int maxworkersinheavysection;
  int *address_of_nowinheavysection;  /** this is the address of the integer keeping track of how many workers are busy **/
  HANDLE heavysectionmutex;
  HANDLE consolemutex;
  HANDLE mastermutex;
  double result;
  void letmein(void);
  void seeya(void);
};

using namespace std;

