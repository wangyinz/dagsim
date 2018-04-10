#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <utility> 
#include <list>
#include <map>
#include <queue>
#include <cstdint>

#define K 500 //credit to run heartbeat 
#define FN 10 //normalization factor for functions 
#define BN 16 //normalization factor for data blocks 

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
    int remaining;
    DashmmDag (string textin) {
      remaining = 0;
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
          remaining++;
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
    DashmmNode& getNode (uint64_t id) {
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
    int priority;
  public:
    Frontier () {
      priority = 0;
    }
    item pop () {
      item r = q.top();
      q.pop();
      priority -= r.priority;
      return r;
    }
    void push (item f) {
      q.push(f);
      priority += f.priority;
    }
    void pushEdges (uint64_t n, DashmmDag& dag) {
      map<uint64_t, Function> edges(dag.getOutEdges(n));
      for (map<uint64_t, Function>::iterator it=edges.begin(); it!=edges.end(); ++it) {
        item f;
        f.m = n;
        f.n = it->first;
        f.priority = dag.getFunctionPriority(it->first);
        this->push(f);
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
    int getPriority () {
      return priority;
    }
};

struct Frontier_comp
{
  bool operator() (Frontier& e1, Frontier& e2) { 
    return e1.getPriority() < e2.getPriority(); 
  }
};

class Pool {
  private:
    priority_queue<Frontier, vector<Frontier>, Frontier_comp> q;
  public:
    Frontier pop () {
      Frontier r = q.top();
      q.pop();
      return r;
    }
    void push (Frontier f) {
      q.push(f);
    }
    bool empty () {
      return q.empty();
    }
};

class Vertex {
  public:
    Frontier f;
    void run(DashmmDag& dag, Pool& pool) {
      while (dag.remaining > 0) {
        while (!pool.empty()) {   //line 1
          f = pool.pop();     //line 2
          int credit = 0;     
          while (!f.empty()) {  //line 3
            item it = f.pop();  //line 4
            //b' := f(m,n,b);
            credit += dag.getFunctionCycles(it.m, it.n);   //line 5
            bool should_push(false);
            //atomic
              //n.b+=b'                   //line 6
              //should_push = (--n.remaining == 0);
            if (should_push) f.pushEdges(it.n, dag);    //line 7
            if (credit > K) {
              Frontier f_new = f.split();         //line 8
              credit = 0;
              //atomic
                pool.push(f_new);             //line 9
                pool.push(f);
                f = pool.pop();
            }
          }
        }
      }
    };
};

void next_step (Pool& pool, int i, vector<Vertex>& vertex, 
  vector<int>& pc, vector<int>& current, 
    vector<int>& credit, vector<vector<short>>& record, 
      DashmmDag& dag, vector<item>& current_it, 
        vector<vector<short>>& priority) {
  if (current[i] == 0) {
    //cout << "Core: " << i << " at line: " << pc[i] << endl;
    switch (++pc[i]) {
      case 1: {
        if (!pool.empty()) {
          record[i].push_back(1);
          priority[i].push_back(-1);
          //the next line has to be done here
          vertex[i].f = pool.pop();
        }
        else {
          record[i].push_back(0);
          priority[i].push_back(0);
          pc[i]--;
        }
        break;
      }
      case 2: {
        credit[i] = 0;
        record[i].push_back(1);
        priority[i].push_back(-1);
        break;
      }
      case 3: {
        if (!vertex[i].f.empty()) {
          record[i].push_back(1);
          priority[i].push_back(-1);
        }
        else {
          record[i].push_back(0);
          priority[i].push_back(0);
          pc[i] -= 3;
        }
        break;
      }
      //4 is to run dashmm function
      case 4: {
        item it = vertex[i].f.pop();
        current[i] = dag.getFunctionCycles(it.m, it.n) / FN -1;
        record[i].push_back(4);
        priority[i].push_back(dag.getFunctionPriority(it.n) + 1);
        current_it[i] = it;
        break;
      }
      case 5: {
        credit[i] = dag.getFunctionCycles(current_it[i].m, current_it[i].n) / FN;
        record[i].push_back(1);
        priority[i].push_back(-1);
        break;
      }
      case 6: {
        bool blocked(false);
        for (int ii=0; ii<pc.size(); ii++) 
          if(pc[ii] == 6 && ii != i)
            blocked = true;
        if (!blocked) {
          current[i] = dag.getNode(current_it[i].n).size() / BN - 1;
          dag.getNode(current_it[i].n).remaining--;
          record[i].push_back(3);
          priority[i].push_back(-1);
        }
        else {
          record[i].push_back(0);
          priority[i].push_back(0);
          pc[i]--;
        }
        break;
      }
      case 7: {
        if (dag.getNode(current_it[i].n).remaining == 0) {
          vertex[i].f.pushEdges(current_it[i].n, dag);
          dag.remaining--;
          //cout << dag.remaining << endl;
        }
        record[i].push_back(1);
        priority[i].push_back(-1);
        break;
      }
      case 8: {
        if (credit[i] > K) {
          Frontier f_new = vertex[i].f.split(); 
          credit[i] = 0;
          record[i].push_back(2);
          priority[i].push_back(-1);
          //the next is in line 9 and is atomic, but it's easier to do it here
          pool.push(f_new);
        }
        else {
          record[i].push_back(1);
          priority[i].push_back(-1);
          pc[i] = 2;
        }
        break;
      }
      case 9: {
        bool blocked(false);
        for (int ii=0; ii<pc.size(); ii++) 
          if(pc[ii] == 9 && ii != i)
            blocked = true;
        if (!blocked) {
          pool.push(vertex[i].f);
          vertex[i].f = pool.pop();
          record[i].push_back(2);
          priority[i].push_back(-1);
          pc[i] = 2;
        }
        else {
          record[i].push_back(0);
          priority[i].push_back(0);
          pc[i]--;
        }
        break;
      }
      //for any other instructions only keep make the processor busy for one cycle
      default: {
        record[i].push_back(1);
        priority[i].push_back(-1);
        break;
      }
    }
  } else {
    current[i]--;
    record[i].push_back(record[i].back());
    priority[i].push_back(priority[i].back());
    //if(current[i] == 0)
      //cout << "Core: " << i << " remain: " << current[i]  << " credit: " << credit[i] << endl;
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
  
  //define one vertex for each processor, and add all the items to a new frontier in the pool
  vector<Vertex> vertex(p);
  Frontier f;
  Pool pool;
  vector<uint64_t> start_nodes = dag.getInitialNodes();
  for (vector<uint64_t>::iterator it=start_nodes.begin(); it!=start_nodes.end(); ++it) {
    f.pushEdges(*it,dag);
    dag.remaining--;
  }
  pool.push(f);
  
  //matrix to record what a processor is doing: 
  //0 for idle, 1 for other instructions, 2 for heartbeat, 3 for reduction, 4 for dashmm function
  vector<vector<short>> record(p);
  
  //matrix to see the priority of current task
  vector<vector<short>> priority(p);
  
  //program counter for each processor
  vector<int> pc(p, 0);
  
  //keep how many cycles left for currrent instruction for each processor
  vector<int> current(p, 0);
  
  //keep how many credits accumulated for each processor
  vector<int> credit(p, 0);
  
  //keep track of which dashmm edge it is processing
  vector<item> current_it(p);
  
  //simulation begin
  while (dag.remaining > 0) {
    //move a step for each processor
    for (int i=0;i<p;i++) {
      next_step(pool, i, vertex, pc, current, credit, record, dag, current_it, priority);
    }
  }
  
  for (vector<vector<short>>::iterator it=record.begin(); it!=record.end(); ++it) {
    for (vector<short>::iterator iit=it->begin(); iit!=it->end(); ++iit) {
      cout << *iit << "\t";
    }
    cout << endl;
  }
  
  for (vector<vector<short>>::iterator it=priority.begin(); it!=priority.end(); ++it) {
    for (vector<short>::iterator iit=it->begin(); iit!=it->end(); ++iit) {
      cout << *iit << "\t";
    }
    cout << endl;
  }
  
  //Print the maps for verification
  //dag.print();
  
  
  
  
  return 0;
}
