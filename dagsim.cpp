#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <utility> 
#include <list>
#include <map>
#include <queue>
#include <cstdint>

using namespace std;

/*typedef struct {
    uint64_t id;
} MyID;*/

typedef struct {
    int cycles;
} Function;

class DashmmNode {
    private:
        map<uint64_t, Function> out_edges;
        uint64_t id;
        string type;
        int priority;
        int s;
        char* block;
    public:
        int remaining; 
        
        DashmmNode (uint64_t id, string type, int priority, int s) {
            this->id = id;
            this->type = type;
            this->priority = priority;
            this->s = s;
            this->remaining = 0;
        }
        pair<map<uint64_t, Function>::iterator, bool> insertEdge (uint64_t d, Function f) {
            remaining++;
            return out_edges.insert (pair<uint64_t, Function>(d, f));
        }
        map<uint64_t, Function> outs () {
            return out_edges;
        }
        void setID (uint64_t d) {
            id=d;
        }
        uint64_t getID () {
            return id;
        }
        void setType (string d) {
            type=d;
        }
        string getType () {
            return type;
        }
        void setPriority (int d) {
            priority=d;
        }
        int getPriority () {
            return priority;
        }
        void setSize (int d) {
            s=d;
        }
        int size () {
            return s;
        }
};

class DashmmDag {
    private:
        map<uint64_t, DashmmNode> dag;
    public:
        DashmmDag (string textin) {
            ifstream infile;
            infile.open(textin.c_str());
            string line;
            while (getline(infile, line)) {
                istringstream iss(line);
                string type;
                int priority, size;
                uint64_t id, idout;
                Function fun;
                iss >> std::hex >> id;
                iss >> std::hex >> idout;
                if (iss.fail()) {
                    iss.clear();
                    iss >> type;
                    iss >> std::dec >> priority >> size;
                    DashmmNode node(id, type, priority, size);
                    dag.insert(pair<uint64_t, DashmmNode>(id, node));
                    //cout << id  << "\t" << type  << "\t" << priority  << "\t" << size << endl; 
                } else {
                    iss >> std::dec >> fun.cycles;
                    dag.at(id).insertEdge(idout, fun);
                    //cout << id  << "\t" << idout << "\t" << fun.cycles << endl; 
                }
            }
        }
        void print () {
            for (map<uint64_t, DashmmNode>::iterator it=dag.begin(); it!=dag.end(); ++it) {
                cout << std::hex << it->first << " " << std::dec << it->second.remaining;
                cout << " => ";
                map<uint64_t, Function> edges = it->second.outs();
                for (map<uint64_t, Function>::iterator iit=edges.begin(); iit!=edges.end(); ++iit) {
                    cout << std::hex << iit->first;
                    cout << " " << std::dec << iit->second.cycles << endl;
                    cout << "\t\t  ";
                }
                cout << endl;
            }
        }
        DashmmNode getNode (uint64_t id) {
            try {
                return dag.at(id);
            } catch (const std::out_of_range& oor) {
                cerr << "Out of Range error in DashmmNode::getNode(uint64_t id) for " 
                    << id << endl;
            }
        }
        map<uint64_t, Function> getOutEdges (uint64_t id) {
            return this->getNode(id).outs();
        }
        Function getFunction (uint64_t id1, uint64_t id2) {
            try {
                return this->getOutEdges(id1).at(id2);
            } catch (const std::out_of_range& oor) {
                cerr << "Out of Range error in DashmmNode::getFunction(uint64_t id1, uint64_t id2) for " 
                    << id1 << " and " << id2 << endl;
            }
        }

        int getFunctionCycles (uint64_t m, uint64_t n) {
            return this->getFunction(m, n).cycles;
        }

        //The destination node of the function determines the priority.
        int getFunctionPriority (uint64_t dest) {
            return dag.at(dest).getPriority();
        }
};

typedef struct {
    uint64_t m;
    uint64_t n;
    int priority;
} frame;

auto frame_comp =
    [](frame& e1, frame& e2) 
    { return e1.priority < e2.priority; };

class Frontier {
    private:
        priority_queue<frame, vector<frame>, decltype(frame_comp)> q;
    public:
        frame pop () {
            frame r = q.top();
            q.pop(); 
            return r;
        }
        void push_edges (uint64_t n, DashmmDag& dag) {
            map<uint64_t, Function> edges(dag.getOutEdges(n));
            for (map<uint64_t, Function>::iterator it=edges.begin(); it!=edges.end(); ++it) {
                frame f;
                f.m = n;
                f.n = it->first;
                f.priority = dag.getFunctionPriority(it->first);
                q.push(f);
            }
        }
};

void usage() {
    cout<<"usage: dagsim input.txt"<<endl;
}

int main(int argc, char* argv[]) {
    if (argc<2) {usage();return 0;}
    string textin(argv[1]);

    //Read in DASHMM dag
    DashmmDag dag(textin);
    
    //Print the maps for verification
    //dag.print();
    
    
    
    
    return 0;
}
