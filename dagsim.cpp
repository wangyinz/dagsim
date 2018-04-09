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
        size_t s;
        char* block;
    public:
        int remaining; 
        
        DashmmNode (uint64_t id, string type, int priority, size_t s) {
            this->id = id;
            this->type = type;
            this->priority = priority;
            this->s = s;
            this->remaining = 0;
        }
        pair<map<uint64_t, Function>::iterator, bool> insertEdge (uint64_t d, Function f) {
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
        void setSize (size_t d) {
            s=d;
        }
        size_t size () {
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
                int priority;
                size_t size;
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
                    dag.at(idout).remaining++;
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
        vector<uint64_t> getInitialNodes () {
            vector<uint64_t> result;
            for (map<uint64_t, DashmmNode>::iterator it=dag.begin(); it!=dag.end(); ++it)
                if (it->second.remaining == 0)
                    result.push_back(it->first);
            return result;
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
} item;

struct item_comp
{
    bool operator() (item& e1, item& e2) { 
        return e1.priority < e2.priority; 
    }
};


class Frontier {
    private:
        priority_queue<item, vector<item>, item_comp> q;
    public:
        item pop () {
            item r = q.top();
            q.pop(); 
            return r;
        }
        void push (item f) {
            q.push(f);
        }
        void push_edges (uint64_t n, DashmmDag& dag) {
            map<uint64_t, Function> edges(dag.getOutEdges(n));
            for (map<uint64_t, Function>::iterator it=edges.begin(); it!=edges.end(); ++it) {
                item f;
                f.m = n;
                f.n = it->first;
                f.priority = dag.getFunctionPriority(it->first);
                q.push(f);
            }
        }
        bool empty () {
            return q.empty();
        }
        size_t size () {
            return q.size();
        }
        Frontier split () {
            Frontier f;
            for (int i=0;i<this->size()/2;i++) {
                f.push(this->pop());
            }
            return f;
        }
};


void next_step (int i, vector<Frontier> front, vector<int> pc, vector<int> current, vector<vector<bool>> record) {
    if(current[i] == 0) {
        switch (++pc[i]) {
            case 1: {
                
                break;
            }
            //for any other instructions only keep make the processor busy for one cycle
            default: {
                record[i].push_back(true);
                break;
            }
        }
        
    } else {
        current[i]--;
    }
}

void usage () {
    cout<<"usage: dagsim input.txt"<<endl;
}

int main (int argc, char* argv[]) {
    if (argc<2) {usage();return 0;}
    string textin(argv[1]);
    int p = 4; 
    //Read in DASHMM dag
    DashmmDag dag(textin);
    
    //define one frontier for each processor, and add all the items to frontier 0
    vector<Frontier> front(p);
    vector<uint64_t> start_nodes = dag.getInitialNodes();
    for (vector<uint64_t>::iterator it=start_nodes.begin(); it!=start_nodes.end(); ++it) {
        front[0].push_edges(*it,dag);
    }
    
    //matrix to record whether a processor is busy or not
    vector<vector<bool>> record(p);
    
    //program counter for each processor
    vector<int> pc(p, 0);
    
    //keep how many cycles left for currrent instruction for each processor
    vector<int> current(p, 0);
    
    //simulation begin
    bool finished(false);
    while (!finished) {
        //move a step for each processor
        for (int i=0;i<p;i++) {
            next_step(i, front, pc, current, record);
        }
    
        //see whether all the frontiers are empty
        finished = front[0].empty();
        for (int i=1;i<p;i++)
            finished = finished && front[i].empty();
    }
    //Print the maps for verification
    //dag.print();
    
    
    
    
    return 0;
}
