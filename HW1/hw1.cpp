#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;
int main(int argc, char *argv[]){
	ifstream file;
	file.open(argv[1]);
	//if file is not located generate error and kill process
	if(!file.is_open()){
		cout << "Could not open file\n";
		exit(1);
	}
	vector<int> values;	//List to hold all the integer values inside text file.
	vector<string> data; //List to hold all of the data inside text file.
	string s;
	int x = 0;
	int mypipefd[2];	//Flips between read and write of the pipe
	int ret;
	char buf[20];
	ret = pipe(mypipefd);	//create the pipe
	if(ret == -1){
		perror("pipe");	//Generate error and kill process
		exit(1);
	}
	stringstream strValue;
	strValue << argv[2];	//Second argument gets turned into integer numberOfProcesses
	int numberOfProcesses;
	strValue >> numberOfProcesses;
	stringstream strValue2;
	strValue2 << argv[3];	//Third argument gets turned into integer increment
	int increment;
	strValue2 >> increment;
	cout << "Number of processeses: " << numberOfProcesses << endl; //Total number of processes, 1 parent process reads data, while n-1 child processes write data
	cout << "Incremented by: " << increment << endl;
	int parentID = getpid();	//Get the id of the parent process
	while(file >> s){
		data.push_back(s);	//store data from text file into data vector
	}
	int value;
	vector<int> flag;	//flag to check positions of string data and int data
	for (int n = 0; n < data.size(); n++){
		if(stringstream(data.at(n)) >> value){
			values.push_back(value);	//stores the integer variables of the text file into vector values and store the index of the int values into flag
			flag.push_back(1);
		}
		else	
			flag.push_back(0);	//stores index of the string values into flag
	}
	cout << "size of data: " << data.size() << endl;	//word count of text file
	for (int i=0; i < values.size(); i++){

		for (int y = 1; y < numberOfProcesses; y++){
			int cid = fork();
			if(cid == 0){
				values[i] = values[i] + increment;	//spawns n-1 child processes
				write(mypipefd[1],&values[i],sizeof(values[i]));	//each child process increments the values of each integer by increment concurrently
				exit(0);	//kill child process
			}
			else{
				wait(); 	//wait for children to finish
				read(mypipefd[0],&values[i],sizeof(values[i]));	//reads the values inside pipe
			}
		}
	}
	value = 0;
	int b = 0;
	for (int a = 0; a< data.size(); a++){
		if(flag[a] == 1){
			cout << values[b] << " ";	//reiterates through data vector, if flag is set output the new integer stored inside pipe.
			b = b + 1;
		}
		else if(flag[a] == 0){
			cout << data[a] << " ";	//if flag is not set output string
		}
	}
	return 0;
}
	
			
			
		