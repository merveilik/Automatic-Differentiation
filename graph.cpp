#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include "graph.h"
#include <queue>
#include <sstream>
#include <cmath>
#include <stack>
using namespace std;
template <class Container>

void split1(const string& str, Container& cont)
{
    istringstream iss(str);
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter(cont));
}
// helper function that checks whether the given string is number or not.
bool isNumber(const string& s)
{
    char* end = 0;
    double val = strtod(s.c_str(), &end);
    return end != s.c_str() && val != HUGE_VAL;
}

int Graph::getVariable(string inp){
    int res;
    if( isNumber(inp) ){
        double val = stod(inp.c_str());
        idCount++;
        name[idCount] = inp;
        vars[idCount] = new Variable(idCount, inp, val);
        res = idCount;
        type[idCount] = VARIABLE;
    }
    else{
        if(id.find(inp)==id.end()){
            idCount++;
            id[inp] = idCount;
            name[idCount] = inp;
            vars[idCount] = new Variable(idCount, inp);
            res = idCount;
            type[idCount] = VARIABLE;
        }
        else{
            res = id[inp];
        }
    }
    // printf("%s -> %d\n",inp.c_str(), res);
    return res;
}

int Graph::getFunction(string fnc){
    idCount++;
    name[idCount] = fnc;
    type[idCount] = FUNCTION;
    Function *f;
    // cout << "new Function: " << fnc << endl;
    if(fnc.compare("mult")==0)
        f = new Multiplication(idCount, fnc);
    else if(fnc.compare("add")==0)
        f = new Addition(idCount, fnc);
    else if(fnc.compare("subs")==0)
        f = new Subtraction(idCount, fnc);
    else if(fnc.compare("divide")==0)
        f = new Division(idCount, fnc);
    else if(fnc.compare("sin")==0)
        f = new Sine(idCount, fnc);
    else if(fnc.compare("cos")==0)
        f = new Cosine(idCount, fnc);
    else if(fnc.compare("identity")==0)
        f = new Identity(idCount, fnc);
    else if(fnc.compare("tan")==0)
        f = new Tangent(idCount, fnc);
    else if(fnc.compare("acos")==0)
        f = new ArcCosine(idCount, fnc);
    else if(fnc.compare("asin")==0)
        f = new ArcSine(idCount, fnc);
    else if(fnc.compare("atan")==0)
        f = new ArcTangent(idCount, fnc);
    else if(fnc.compare("exp")==0)
        f = new Exponential(idCount, fnc);
    else if(fnc.compare("log")==0)
        f = new Log(idCount, fnc);
    else if(fnc.compare("log10")==0)
        f = new Log10(idCount, fnc);
    else if(fnc.compare("pow")==0)
        f = new Power(idCount, fnc);
    else if(fnc.compare("sqrt")==0)
        f = new Sqrt(idCount, fnc);
    fncs[idCount] = f;
    return idCount;
}

void Graph::addUnaryFunction(string fnc, string inp, string out){

    int fId = getFunction(fnc);
    int inpId = getVariable(inp);
    int outId = getVariable(out);
    fncs[fId]->addInput(vars[inpId]);
    fncs[fId]->setOutput(vars[outId]);

    vars[inpId]->addTo(fncs[fId]);
    vars[outId]->setFrom(fncs[fId]);
}

void Graph::addBinaryFunction(string fnc, string inp1, string inp2, string out){
    int fId = getFunction(fnc);
    int inpId1 = getVariable(inp1);
    int inpId2= getVariable(inp2);
    int outId = getVariable(out);

    fncs[fId]->addInput(vars[inpId1]);
    fncs[fId]->addInput(vars[inpId2]);

    fncs[fId]->setOutput(vars[outId]);

    vars[inpId1]->addTo(fncs[fId]);
    vars[inpId2]->addTo(fncs[fId]);

    vars[outId]->setFrom(fncs[fId]);

}

void Graph::readGraph(string fileName){
    ifstream infile(fileName);
    string line;
    getline(infile,line);
    vector<string> words;
    split1(line,words);

    while(words[0].compare(" ")!=0){

        if(words[0].compare("input")==0){
           int id=getVariable(words[0]);
            inputNodes.push_back(id);
        }
        else if(words[0].compare("output")==0){
            int id=getVariable(words[0]);
            outputNode=id;
        }
        else /*if(words[0].compare("assignment")==0)*/{
            string s="";
            for(int j=2;j<words.size();j++){
                s=s+words[j]+" ";
            }
            addAssignment(words[0],s);
        }
        getline(infile,line);
        vector<string> words;
        split1(line,words);
    }
}

bool Graph::isCyclic(){

    queue<Node*> q;
    for(int i=0;i<vars.size();i++){
        if(vars[i]->getIncomings().size()==0)
            q.push((Node*)vars[i]);
    }
    Node* n;
    while(!q.empty()){
        n=q.front();
        q.pop();

        for(int i=0;i<n->getOutgoings().size();i++){
            n->getOutgoings()[i]->indegree--;
            if(n->getOutgoings()[i]->indegree==0)
                q.push(n->getOutgoings()[i]);
        }
    }
    if(n->id==outputNode)
        return false;
    else
        return true;
}



double Graph::forwardPass(vector<double> inputValues){

    for(int i=0;i<vars.size();i++){
      this->vars[inputNodes[i]]->value=inputValues[i];
    }

    queue <Variable *> q;
    for(int i=0;i<inputNodes.size();i++){
        q.push(this->vars[inputNodes[i]]);
    }

    while(!q.empty()){

        Variable* v=q.front();
        q.pop();

        for(int i=0;i<v->to.size();i++){
            v->to[i]->indegree--;
            v->to[i]->output->indegree--;
            if(v->to[i]->output->indegree==0)
                q.push(v->to[i]->output);
        }
        for(int i=0;i<v->to.size();i++){
            v->to[i]->doForward();
        }

    }

    return vars[outputNode]->value;

}

vector<double> Graph::backwardPass(){

    vector<double> result;
    queue <Variable *> q;
    queue <Variable*> q2;
    for(int i=0;i<inputNodes.size();i++){
        q.push(this->vars[inputNodes[i]]);
    }

    while(!q.empty()){

        Variable* v=q.front();
        q2.push(v);
        q.pop();
        for(int i=0;i<v->to.size();i++){
            v->to[i]->indegree--;
            v->to[i]->output->indegree--;
            if(v->to[i]->output->indegree==0)
                q.push(v->to[i]->output);
        }
        for(int i=0;i<v->to.size();i++){
            v->to[i]->doForward();
        }
    }
    stack <Variable *> s;
    for(int i=0;q2.size();i++){
        Variable* x=q2.front();
        q2.pop();
        s.push(x);
    }
    for(int i=0;i<s.size();i++){
        Variable* y=s.top();
        s.pop();
        q2.push(y);
    }

    while(!q2.empty()){
        Variable* v=q2.front();
        q2.pop();
        v->from->doBackward();
    }
    for(int i=0;i<inputNodes.size();i++){
        result.push_back(vars[inputNodes[i]]->derivative);
    }
    return result;

}

void Graph::addAssignment(string lvalue, string rvalue) {

    vector<string> right;
    split1(rvalue,right);

    if(right.size()==3)
        addBinaryFunction(right[0],right[1],right[2],lvalue);

    else
        addUnaryFunction(right[0], right[1],lvalue);

}