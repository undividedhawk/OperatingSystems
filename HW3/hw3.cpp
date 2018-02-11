#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include<sys/shm.h>
#include <sys/sem.h>
#include<unistd.h>
#include <stdio.h>
#include<stdlib.h>
#include<fstream>
#include <string>
#include<vector>
#include <sstream>
#include <math.h>
using namespace std;

int mainMemory;		//Total number of page frames in main memory
int pageSize;		//size of each page in bytes
int segmentSize;	//size of each segment in number of pages
int frames;		//number of page frames per process
int lookAhead;		//window size for OPT 0 for others
int windowMin;		//min window for working set. 0 for others
int windowMax;		//max window for working set. 0 for others
int totalProcesses;		//total number of processes
int totalPageFaults;

struct Address
{
	int segment;
	int page;
	int displacement;
};

struct Page
{
	bool allocated;
	int frameID;
	int pageID;
	Page(int id)
	{
		pageID = id;
		allocated = false;
		frameID = 0;
	}
};

struct Frame
{
	bool allocated;
	int pageID;
	int processID;
	int frameID;
	int segmentID;
	Frame(int id)
	{
		frameID = id;
	}
	
};

struct PageTable
{
	vector<Page> pages;
};

struct Segment
{
	int segmentID;
	PageTable pageTable;
	Segment(int id)
	{
		segmentID = id;
	}
};

struct SegmentTable
{
	vector<Segment> segments;
};

struct AddressSpace
{
	vector<Page> pageTables;		//vector containing all pages in virtual memory
	vector<SegmentTable> segmentTables;	//Vector containing all segment tables in address space
};

struct Process
{
	int processID;			
	int totalPageFrames;
	AddressSpace addressSpace;		//addressspace object containing all of the addresses
	vector<int> request;	//reference string of addresses converted to int values
	int pageFaults;		//number of page Faults per process
};
struct Process *processes;

vector<string> instructions;		//vector of all the instructions
vector<Frame> framesInDisk;		//vector of frames stored on the disk
vector<Frame> framesInMemory;		//vector of frames in main memory

void allocateToDisk(Process *cProcess)
{
	if(cProcess -> addressSpace.segmentTables.size() == 0)
	{
		cProcess -> addressSpace.segmentTables.push_back(*(new SegmentTable()));
	}
	for(int i = 0; i < cProcess -> totalPageFrames; i++)
	{
		int segment = i/segmentSize;
		int page = i % segmentSize;

		if(segment>=cProcess->addressSpace.segmentTables[0].segments.size())
		{
			cProcess->addressSpace.segmentTables[0].segments.push_back(*(new Segment(segment)));
		}

		if(page >= cProcess->addressSpace.segmentTables[0].segments[segment].pageTable.pages.size())
		{
			cProcess->addressSpace.segmentTables[0].segments[segment].pageTable.pages.push_back(*(new Page(page)));
		}
		
		Frame *cFrame = new Frame(i + framesInDisk.size());
		cFrame->allocated = true;
		cFrame->segmentID = segment;
		cFrame->pageID = page;
		cFrame->processID = cProcess->processID;

		framesInDisk.push_back(*cFrame);
	}	
}

void buildVirtualMemory(int frameSize)
{
	for (int i = 0; i < frameSize; i++)
	{
		framesInMemory.push_back(*(new Frame(i)));
	}
}

void clearMemory()
{				//clear virtual memory
	framesInDisk.clear();
	framesInMemory.clear();
	delete[] processes;
	processes = NULL;
	totalPageFaults = 0;	//reset totalPageFaults back to 0
	
}

void readFile(string& fileName)
{
	ifstream iFile(fileName.c_str());
	if(!iFile.good())
	{
		cout << "Error reading file\n";
		exit(1);
	}
	if(iFile.peek()== std::ifstream::traits_type::eof())
	{
		cout << "File is empty\n";
		exit(1);
	}
	stringstream stoi;	//if file is correctly read parse through and convert 
	string cLine;		//all the string values into integer values
	getline(iFile, cLine);
	stoi << cLine << ' ';
	getline(iFile,cLine);
	stoi << cLine << ' ';
	getline(iFile,cLine);
	stoi << cLine << ' ';
	getline(iFile,cLine);
	stoi << cLine << ' ';
	getline(iFile,cLine);
	stoi << cLine << ' ';
	getline(iFile,cLine);
	stoi << cLine << ' ';
	getline(iFile,cLine);
	stoi << cLine << ' ';
	getline(iFile,cLine);
	stoi << cLine << ' ';
	stoi >> mainMemory >> segmentSize >> pageSize >> frames >> lookAhead >> windowMin >> windowMax >> totalProcesses;
	while(getline(iFile,cLine))
	{
		instructions.push_back(cLine);	
	}
}

void FIFO_Replacement()
{
	buildVirtualMemory(mainMemory);
	processes = new Process[totalProcesses];
	for(size_t i = 0; i < totalProcesses; i++)
	{
		istringstream iss(instructions[i]);		//create a reference string of ints
		string str;					//for each process
		iss >> str;
		stringstream stoi;
		stoi << str << ' ';
		stoi >> processes[i].processID;

		iss >> str;
		stoi << str << ' ';
		stoi >> processes[i].totalPageFrames;
		allocateToDisk(&processes[i]);
	}
	for(int i =3; i < instructions.size();i++)
	{
		istringstream iss(instructions[i]);
		string str;
		iss >> str;
		stringstream stoi;
		int temp = 0;
		stoi << str << ' ';
		stoi >> temp;
		unsigned int x = 0;
		iss >> str;
		stoi << std::hex << str << ' ';		//convert the address string to an int
		stoi >> x;
		
		if(processes[0].processID == temp)
			processes[0].request.push_back(static_cast<int>(x));
	
		if(processes[1].processID == temp)
			processes[1].request.push_back(static_cast<int>(x));

		if(processes[2].processID == temp)
			processes[2].request.push_back(static_cast<int>(x));
	}
	for(size_t i = 0; i < totalProcesses; i++)
	{					//run FIFO page replacement
		processes[i].pageFaults = 0;	//for each process
		int temp[frames];
		for(int m = 0; m < frames; m++)
		{	
			temp[m] = -1;
		} 
		for(int m = 0; m < (frames+((int)log2(pageSize)));m++)
		{
			int s = 0;
			for(int n = 0; n < frames; n++)
			{
				if(processes[i].request[m] == temp[n])
				{
					s++;
					processes[i].pageFaults--;
				}
			}
			processes[i].pageFaults++;
			if((processes[i].pageFaults <= frames) && (s == 0))
			{
				temp[m] = processes[i].request[m];
			}
			else if(s == 0)
				temp[(processes[i].pageFaults - 1)%frames] = processes[i].request[m];
		}
		cout << "\nPageFaults:\t" << processes[i].pageFaults << endl;
		totalPageFaults += processes[i].pageFaults;	
	}
	cout << "\n\nTotal Page Faults:\t" << totalPageFaults << endl;

}

void LIFO_Replacement()
{

	buildVirtualMemory(mainMemory);
	processes = new Process[totalProcesses];
	for(size_t i = 0; i < totalProcesses; i++)
	{
		istringstream iss(instructions[i]);
		string str;
		iss >> str;
		stringstream stoi;
		stoi << str << ' ';
		stoi >> processes[i].processID;

		iss >> str;
		stoi << str << ' ';
		stoi >> processes[i].totalPageFrames;
		allocateToDisk(&processes[i]);
	}
	for(int i =3; i < instructions.size();i++)
	{
		istringstream iss(instructions[i]);	//form a reference string of ints
		string str;				//from the string of hex values
		iss >> str;
		stringstream stoi;
		int temp = 0;
		stoi << str << ' ';
		stoi >> temp;
		unsigned int x = 0;
		iss >> str;
		stoi << std::hex << str << ' ';
		stoi >> x;
		
		if(processes[0].processID == temp)
			processes[0].request.push_back(static_cast<int>(x));
	
		if(processes[1].processID == temp)
			processes[1].request.push_back(static_cast<int>(x));

		if(processes[2].processID == temp)
			processes[2].request.push_back(static_cast<int>(x));
	}
	for(size_t i = 0; i < totalProcesses; i++)
	{
		processes[i].pageFaults = 0;
		int temp[frames];
		for(int m = 0; m < frames; m++)		//reverse the FIFO page
		{					//replacement algorithm
			temp[m] = -1;
		} 
		for(int m = (frames+((int)log2(pageSize))-1); m >=0; m--)
		{
			int s = 0;
			for(int n = frames-1; n >= 0; n--)
			{
				if(processes[i].request[m] == temp[n])
				{
					s++;
					processes[i].pageFaults--;
				}
			}
			processes[i].pageFaults++;
			if((processes[i].pageFaults <= frames) && (s == 0))
			{
				temp[m] = processes[i].request[m];
			}
			else if(s == 0)
				temp[(processes[i].pageFaults - 1)%frames] = processes[i].request[m];
		}
		cout << "\nPageFaults:\t" << processes[i].pageFaults << endl;
		totalPageFaults += processes[i].pageFaults;	
	}
	cout << "\n\nTotal Page Faults:\t" << totalPageFaults << endl;

}

void OPT_Replacement()
{
	int delta = mainMemory;	
	buildVirtualMemory(mainMemory);
	processes = new Process[totalProcesses];
	for(size_t i = 0; i < totalProcesses; i++)
	{
		istringstream iss(instructions[i]);
		string str;
		iss >> str;
		stringstream stoi;
		stoi << str << ' ';
		stoi >> processes[i].processID;

		iss >> str;
		stoi << str << ' ';
		stoi >> processes[i].totalPageFrames;
		allocateToDisk(&processes[i]);
	}
	for(int i =3; i < instructions.size();i++)
	{
		istringstream iss(instructions[i]);
		string str;
		iss >> str;
		stringstream stoi;
		int temp = 0;
		stoi << str << ' ';
		stoi >> temp;
		unsigned int x = 0;
		iss >> str;
		stoi << std::hex << str << ' ';		//convert string of hex values
		stoi >> x;				//to ints and store in a vector
							//called request
		if(processes[0].processID == temp)
			processes[0].request.push_back(static_cast<int>(x));
	
		if(processes[1].processID == temp)
			processes[1].request.push_back(static_cast<int>(x));

		if(processes[2].processID == temp)
			processes[2].request.push_back(static_cast<int>(x));
	}
	for(size_t i = 0; i < totalProcesses; i++)
	{
		processes[i].pageFaults = 0;		//run OPT replacement for 
		int frame[delta], interval[delta];	//every process
		int m,n,temp,flag,found;
		int position, previous_frame = -1;
		int maximum_interval;
		for(m = 0;m<frames;m++)
			frame[m] = -1;
		
		for (m=0; m < (frames+((int)log2(pageSize))); m++)
		{
			flag = 0;
			for(n = 0; n < frames;n++)
			{
				if(frame[n] == processes[i].request[m])
				{
					flag = 1;
					break;
				}
			}
			if(flag == 0)
			{
				if(previous_frame == frames-1)
				{
					for(n=0;n<frames;n++)
					{
						for(temp=m+1;temp<segmentSize;temp++)
						{
							interval[n] = 0;
							if(frame[n] == processes[i].request[temp])
							{
								interval[n]=temp-m;
								break;
							}
						}
					}
					found = 0;
					for(n=0;n<frames;n++)
					{
						if(interval[n] == 0)
						{
							position = n;
							found = 1;
							break;
						}
					}
				}
				else
				{
					position = ++previous_frame;
					found = 1;
				}
				if(found == 0)
				{
					maximum_interval = interval[0];
					position = 0;
					for(n = 1;n<frames;n++)
					{
						if(maximum_interval < interval[n])
						{
							maximum_interval = interval[n];
							position = n;
						}
					}

				}
				frame[position] = processes[i].request[m];
				processes[i].pageFaults++;
			}
			
		}
		
		cout << "\nPageFaults:\t" << processes[i].pageFaults << endl;
		totalPageFaults += processes[i].pageFaults;	
	
	}
	cout << "\n Total Page Faults:\t" << totalPageFaults << endl;							
}

void LRU_Replacement()
{
	
	buildVirtualMemory(mainMemory);
	processes = new Process[totalProcesses];
	for(size_t i = 0; i < totalProcesses; i++)
	{
		istringstream iss(instructions[i]);
		string str;
		iss >> str;
		stringstream stoi;
		stoi << str << ' ';
		stoi >> processes[i].processID;

		iss >> str;
		stoi << str << ' ';
		stoi >> processes[i].totalPageFrames;
		allocateToDisk(&processes[i]);
	}
	for(int i =3; i < instructions.size();i++)
	{
		istringstream iss(instructions[i]);
		string str;
		iss >> str;
		stringstream stoi;
		int temp = 0;
		stoi << str << ' ';
		stoi >> temp;
		unsigned int x = 0;
		iss >> str;
		stoi << std::hex << str << ' ';		//convert string of hex values
		stoi >> x;				//to ints and store in a vector
							//called request
		if(processes[0].processID == temp)
			processes[0].request.push_back(static_cast<int>(x));
	
		if(processes[1].processID == temp)
			processes[1].request.push_back(static_cast<int>(x));

		if(processes[2].processID == temp)
			processes[2].request.push_back(static_cast<int>(x));
	}
	for(size_t i = 0; i < totalProcesses; i++)
	{
		processes[i].pageFaults = 0;		//run LRU replacement for 
		int frame[frames], temp[frames];	//each process
		int m,n,a,b,k,l,position;
		for(m = 0; m < frames; m++)
		{
			frame[m] = -1;
		}
		for (n=0; n < (frames+((int)log2(pageSize))); n++)
		{
			a = 0, b = 0;
			for(m = 0; m < frames; m++)
			{
				if(frame[m]==processes[i].request[n])
				{
					a = 1;
					b = 1;
					break;
				}
			}
			if(a==0)
			{
				for(m=0; m < frames; m++)
				{
					if(frame[m] == -1)
					{
						frame[m] = processes[i].request[n];
						b = 1;
						break;
					}

				}
			}
			if(b==0)
			{
				for(m = 0; m < frames; m++)
				{
					temp[m] = 0;
				}
				for(k = n-1,l=1;l<=frames-1;l++,k--)
				{
					for(m=0;m<frames;m++)
					{
						if(frame[m]==processes[i].request[k])
						{
							temp[m]=1;
						}
					}

				}
				for(m=0;m<frames;m++)
				{
					if(temp[m] == 0)
						position = m;
				}
				frame[position] = processes[i].request[n];
				processes[i].pageFaults++;
			}
		}
			
		cout << "\nPageFaults:\t" << processes[i].pageFaults << endl;
		totalPageFaults += processes[i].pageFaults;	

	}
	cout << "\n Total Number of Page Faults:\t" << totalPageFaults << endl;
	

}

int main(int argc, char* argv[])
{	
	string FName(argv[1]);
	readFile(FName);	//read the input file parsed as an argument
	cout << "Total Pages:\t" << mainMemory << endl;
	cout << "Max Segment Size:\t" << segmentSize << endl;
	cout << "Page Size:\t" << pageSize << endl;
	cout << "Number of Page Frames:\t" << frames << endl;
	cout << "Look ahead Window:\t" << lookAhead << endl;
	cout << "Window Min:\t" << windowMin << endl;
	cout << "Window Max:\t" << windowMax << endl;
	cout << "Total Processes:\t" << totalProcesses << endl << endl << endl;
	
	cout << "-------------------FIFO Replacement--------------------\n\n";
	FIFO_Replacement();	//Run the FIFO replacement method

	clearMemory();		//clear the virtual memory for the next replacement
	
	cout << "------------------LIFO Replacement-------------------\n\n";			//method
	LIFO_Replacement();	//Run the LIFO replacement method
	
	clearMemory();		//clear the virtual memory for the next replacement
				//method
	cout << "--------------------LRU Replacement-------------------\n\n";
	LRU_Replacement();	//Run the LRU replacement method

	clearMemory();		//clear the virtual memory for the next replacement
				//method
	cout << "=======================OPT Replacement-------------------\n\n";
	OPT_Replacement();	//Run the OPT_Replacement method
	clearMemory();		//clear the virutal memory for the next replacement 
				//method

	return 0;
} 
