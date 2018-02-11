#include <unistd.h>
#include<iostream>
#include<iomanip>
#include <string>
#include <algorithm>    // std::sort
#include <vector>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>	//shared memory library

using namespace std;


class process {
public:
	string name;
	int computationTime;
	int deadline;
	int relativeDeadline;
	int laxity;
	int sharedMemorySize = 0;
	key_t sharedMemoryKey = 8792;
	
    static const int numOfProcesses = 3;
    static const int numOfResourceTypes = 2;
    bool finish[numOfProcesses] = {false, false, false};
	int resources[numOfResourceTypes] = {2,3};
    int available[numOfResourceTypes] = {2, 3};

    int need[numOfProcesses][numOfResourceTypes] = {{1, 2},
                                                    {2, 1},
                                                    {2, 3},
                                                    {2, 3}};
    int allocated[numOfProcesses][numOfResourceTypes] = {{0, 0},
                                                         {0, 0},
                                                         {0, 0}};
    int work[numOfResourceTypes] = {0, 0};
    int request[numOfProcesses][numOfResourceTypes] = {{1, 2},
                                                       {2, 1},
                                                       {2, 3},
                                                       {2, 3}};

	int getSharedMemSize(){
		sharedMemorySize +=4;
		for(int i = 0; i < numOfResourceTypes; i++){
			sharedMemorySize += resources[i] * sizeof(resources[i]);
		}
		return sharedMemorySize;
	}
	void createSharedMemory(){
		
		int shmid;
		size_t memorySize;
		memorySize = static_cast<size_t>(getSharedMemSize());
		shmid = shmget(sharedMemoryKey,memorySize,IPC_CREAT | 0666);
		int *shmPointer = (int *) shmat(shmid,nullptr,NULL);
		for (int i = 0; i < numOfResourceTypes; i++){
			*shmPointer++ = resources[i];
		}
		*shmPointer = NULL;
		shmdt(&shmPointer);
	}
	
	void readSharedMemorySpace(){
        int shmid;
        size_t memorySize;
        int* temp;
        int temp2;
        memorySize = static_cast<size_t>(sharedMemorySize);

        //create shared memory, set permissions to allow all
        shmid = shmget(sharedMemoryKey,memorySize,0666);

        //attach a pointer to this share memory
        temp = (int*) shmat(shmid, NULL,0);

        for (int i = 0; i < numOfResourceTypes; i++){
            available[i] = *temp++;
        }

        for (int i = 0; i < numOfResourceTypes; i++){
            cout << available[i] <<endl;
        }

    }

    void writeSharedMemorySpace(){
        int shmid;
        size_t memorySize;

        memorySize = static_cast<size_t>(sharedMemorySize);

        //create shared memory, set permissions to allow all
        shmid = shmget(sharedMemoryKey,memorySize, 0666);

        //attach a pointer to this share memory
        int *shmPointer = (int *) shmat(shmid, nullptr,NULL);

        //fill shared memory with resources
        for (int i = 0;i < numOfResourceTypes; i++)
            *shmPointer++ = available[i];

        *shmPointer = NULL;
        //detach pointer from shared memory once operations complete
        shmdt(&shmPointer);

    }
	
	
    //simulated calculate function that sleeps for n amount of time
    void calculate(int n) {
        sleep(n*100);
    }
    //simulated useresource function that sleeps for n amount of time
    void useresource(int n) {
        sleep(n*100);
    }

    //process scheduling function that sorts processes by computation time (SJF), and then deadline if there is a draw
    void processschedSJF(vector<processObject> processList) {

        //test print if correct order
        for (int i = 0; i < processList.size(); i++) {
            cout << processList[i].name << endl;
        }

        //sort processes by computation time
        sort(processList.begin(), processList.end(),
             [](processObject a, processObject b) { return a.computationTime < b.computationTime; });

        // if computation time is the same, use earliest deadline first
        for (int i = 0; i < processList.size() - 1; i++) {
            if (processList[i].computationTime == processList[i + 1].computationTime) {
                processObject temp{"one", 1, 1};
                if (processList[i].deadline > processList[i + 1].deadline) {
                    temp = processList[i];
                    processList[i] = processList[i + 1];
                    processList[i + 1] = temp;
                }
            }
        }
        //test print if correct order
        for (int i = 0; i < processList.size(); i++) {
            cout << processList[i].name << endl;
        }

    }
    //process scheduling function that sorts processes by computation time (SJF), and then deadline if there is a draw
    void processschedLLF(vector<processObject> processList) {

        //test print if correct order
        for (int i = 0; i < processList.size(); i++) {
            cout << processList[i].name << endl;
        }
        sort(processList.begin(), processList.end(),
             [](processObject a, processObject b) { return a.laxity < b.laxity; });

        for (int i = 0; i < processList.size() - 1; i++) {
            if (processList[i].laxity == processList[i + 1].laxity) {
                processObject temp{"one", 1, 1};
                if (processList[i].computationTime > processList[i + 1].computationTime) {
                    temp = processList[i];
                    processList[i] = processList[i + 1];
                    processList[i + 1] = temp;
                }
            }
        }
    }

    //safety check that makes sure processes are not in a deadlock state
    bool safetyCheck(int i) {

        bool finish[numOfProcesses] = {true};
        int work[numOfResourceTypes];
        for (int i = 0; i < numOfResourceTypes; i++) {
            work[i] = available[i];
        }
        for (i; i < numOfProcesses; i++) {
            finish[i] = false;
        }
        bool flag = true;
        for (int r = 0; r < numOfResourceTypes; r++) {
            while (flag && i < numOfProcesses) {
                if (!finish[i] && need[i] <= work) {
                    work[r] = work[r] + allocated[i][r];
                    finish[i] = true;
                    i++;
                } else
                    flag = false;
            }
        }
        return flag;


    }
	process(string n, int cpuTime, int d1){
		name = n;
		computationTime = cpuTime;
		deadline = d1;
	}


    int getCPUTime(){
        return computationTime;
    }

    void setLaxity(){
        int laxity = relativeDeadline - computationTime;
    }
	int getLaxity(){
		return laxity;
	}
	
	void requestResources(int numResourcesWanted[]){
		
	}

    //bankers algorithm that ensures that processes complete while using safety check to ensure deadlock does not occur while processes are executing

    void bankers() {
        for (int i = 0; i < numOfProcesses; i++) {
            for (int r = 0; r < numOfResourceTypes; r++) {
                if (request[i][r] > need[i][r]) {
                    cout << "error" << endl;
                } else if (request[i][r] > available[r]) {
                    cout << "process " << i << " must wait\n";
                } else {
                    available[r] = available[r] - request[i][r];
                    allocated[i][r] = allocated[i][r] + request[i][r];
                    need[i][r] = need[i][r] - request[i][r];
                    if (safetyCheck(i)) {
                        cout << "complete transaction " << i << " \n";
                        available[r] = available[r] + request[i][r];

                    } else if (!safetyCheck(i)) {
                        cout << "process" << i << " must wait.\n";
                        available[r] = available[r] + request[i][r];
                        allocated[i][r] = allocated[i][r] - request[i][r];
                        need[i][r] = need[i][r] + request[i][r];
                    }
                }
            }
        }

            //Test Print to ensure bankers is working
        for (int r = 0; r < numOfResourceTypes; r++) {
            cout << "available = " << available[r] << endl;
        }
        for (int i = 0; i < numOfProcesses; i++) {
            for (int r = 0; r < numOfResourceTypes; r++) {
                cout << "Process " << i << "needs " << need[i][r] << "of type " << r << endl;
                cout << "Process " << i << "allocated " << allocated[i][r] << "of type " << r << endl;
                cout << "Process " << i << "requests" << request[i][r] << "of type " << r << endl;
                cout << "----------------------------------------------\n";
            }
        }

    }


};

    int main() {

        process test;
        test.processschedSJF(test.processes);
        test.bankers();

        return 0;
    }