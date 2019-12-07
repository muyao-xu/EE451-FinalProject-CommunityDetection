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

int main() {
    string filename = "input.graph";
    int N = 8;

    vector<int>* adjList = readGraph(filename);
    int* labels = new int[numNodes+1];
    bool* active = new bool[numNodes+1];
    int maxIterations = 1000;
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
        cout << "iteration: " << iterations << endl;
        iterations++;
        int* newLabels = new int[numNodes+1];

        int tid;
        #pragma omp parallel shared(numUpdates, labels, newLabels, numNodes) private(tid) 
        {
            tid = omp_get_thread_num();
            // cout << "tid: " << tid << endl;
            #pragma omp for schedule(guided)
            for (int i=1; i<=numNodes; i++) {
                // cout << "tid " << tid << ": " << i << endl;
                if (active[i] && !adjList[i].empty()) {
                    // cout << "here " << i << " " << labels[i] << endl;
                    // find the most popular label
                    int* labelWeights = new int[numNodes+1];
                    for (int i=1; i<=numNodes; i++) {
                        labelWeights[i] = 0;
                    }
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
                        newLabels[i] = labels[i];
                        // cout << "new label for node" << i << endl;
                    }
                    else {
                        numUpdates++;
                        newLabels[i] = maxLabel;
                         for (int j=0; j<neighbors.size(); j++) {
                            active[neighbors[j]] = true;
                        }
                    }
                    delete[] labelWeights;
                }
            }
        }
        // print out new labels
        delete[] labels;
        labels = newLabels;
        cout << "updates: " << numUpdates << endl;
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
