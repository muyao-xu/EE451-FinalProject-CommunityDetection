#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>
#include <omp.h>
#include <unordered_map>

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

int main(int argc, char* argv[]) {
    stringstream ss(argv[2]);
    string filename = argv[1];
    int N;
    ss >> N;

    vector<int>* adjList = readGraph(filename);
    int* labels = new int[numNodes+1];
    bool* active = new bool[numNodes+1];
    int maxIterations = 10000;
    for (int i=1; i<=numNodes; i++) {
        labels[i] = i;
        active[i] = true;
    }
    
    omp_set_num_threads(N);
    // start measuring time
    struct timespec start, stop;
	double time;
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}


    int numUpdates = numNodes;
    int iterations = 0;
    while (numUpdates > numNodes/1e3 && iterations <= maxIterations) {
        numUpdates = 0;
        // cout << "iteration: " << iterations << endl;
        iterations++;

        #pragma omp parallel shared(numUpdates, labels, numNodes) 
        {
            #pragma omp for schedule(guided)
            for (int i=1; i<=numNodes; i++) {
                // cout << "tid " << tid << ": " << i << endl;
                if (active[i] && !adjList[i].empty()) {
                    // find the most popular label
                    vector<int> neighbors = adjList[i];
                    unordered_map<int, int> labelMap;
                    for (int j=0; j<neighbors.size(); j++) {
                        int l = labels[neighbors[j]];
                        labelMap[l]++;
                    }

                    int maxWeight = -1;
                    int maxLabel = -1;
                    for (auto l : labelMap) { 
                        if (maxWeight < l.second) { 
                            maxLabel = l.first; 
                            maxWeight = l.second; 
                        } 
                    } 
                    if (maxLabel == labels[i]) {
                        active[i] = false;
                    }
                    else {
                        numUpdates++;
                        labels[i] = maxLabel;
                         for (int j=0; j<neighbors.size(); j++) {
                            active[neighbors[j]] = true;
                        }
                    }
                }
            }
        }
        // cout << "updates: " << numUpdates << endl;
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
