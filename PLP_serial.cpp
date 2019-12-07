#include "PLP.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>


using namespace std;

int numNodes;

vector<int>* readGraph(const string& filename) {
    // read a line from the file
    ifstream file;
    file.open(filename);
    string line;

    int m=0, n=0;
    getline(file, line);
    stringstream ss(line);
    ss >> n >> m;
    numNodes = n;
    vector<int> *adjList = new vector<int>[n+1];

    int node = 1;
    while (getline(file, line)) {
        int u;
        ss.clear();
        ss.str(line);
        while (ss >> u) {
            adjList[node].push_back(u);
        }
        node++;
    }

    return adjList;
}

int main(int arvc, char* argv[]) {
    stringstream ss(argv[2]);
    string filename = argv[1];
    int N;
    ss >> N;
    cout << filename << " " << N << endl; 

    vector<int>* adjList = readGraph(filename);
    int* labels = new int[numNodes+1];
    bool* active = new bool[numNodes+1];
    for (int i=1; i<=numNodes; i++) {
        labels[i] = i;
        active[i] = true;
    }

    // start measuring time
    struct timespec start, stop;
	double time;
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}

    // serial implementation
    int numUpdates = numNodes;
    int iterations = 0;
    while (numUpdates > numNodes/1e5) {
        cout << "iteration: " << iterations << endl;
        numUpdates = 0;
        int* newLabels = new int[numNodes+1];
        for (int i=1; i<=numNodes; i++) {
            newLabels[i] = labels[i];
        }
        for (int i=1; i<=numNodes; i++) {
            if (active[i] && !adjList[i].empty()) {
                // find the most popular label
                int* labelWeights = new int[numNodes+1];
                for (int i=1; i<=numNodes; i++) {
                    labelWeights[i] = 0;
                }
                vector<int> neighbors = adjList[i];
                for (int j=0; j<neighbors.size(); j++) {
                    labelWeights[neighbors[j]] ++; 
                }
                int maxWeight = -1;
                int maxLabel = -1;
                for (int j=1; j<=numNodes; j++) {
                    if (labelWeights[j] > maxWeight) {
                        maxWeight = labelWeights[j];
                        maxLabel = j;
                    }
                }
                if (maxLabel == labels[i]) {
                    active[i] = false;
                }
                else {
                    numUpdates++;
                    newLabels[i] = maxLabel;
                }
                delete[] labelWeights;
            }
        }
        delete[] labels;
        labels = newLabels;
        iterations++;
    }

    // measure the end time here
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	// print out the execution time here
	printf("Execution time = %f sec\n", time);

    ofstream outFile;
    outFile.open("output.dat");

    for (int i=1; i<=numNodes; i++) {
        outFile << i << " " << labels[i] << endl;
    }
    outFile.close();
    
    return 0;
}
