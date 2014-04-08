#include "Program.h"

#include "trace.h"
#include "LukasiewiczTnorm.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <typeinfo>

ostream& operator<<(ostream& out, const Program& program) {
    for(list<Rule*>::const_iterator it = program.rules.begin(); it != program.rules.end(); ++it)
        out << **it << endl;
    return out;
}

Program* Program::instance = NULL;

void Program::init(const Tnorm& tnorm) {
    assert(instance == NULL);
    instance = new Program(tnorm);
}

void Program::free() {
    assert(instance != NULL);
    delete instance;
}

Program::Program(const Tnorm& _tnorm) : tnorm(_tnorm.clone()), nextIdInBilevelProgram(1), processed(0)
{
    getFalseAtom().setName("#0");
}

Program::~Program()
{
    assert(tnorm != NULL);
    delete tnorm;

    for(list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
        delete *it;

    for(list<Rule*>::iterator it = rules.begin(); it != rules.end(); ++it)
        delete *it;
    
    for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ++it)
        delete *it;

    for(list<Atom::Data*>::iterator it = constants.begin(); it != constants.end(); ++it)
        delete *it;

    glp_free_env();
}

void Program::insert(Rule* rule) {
    rules.push_back(rule);
    rule->onIncreaseUpperBound();
}

void Program::addDependency(Atom head, Atom body) {
    dependencyGraph.addEdge(head.getId(), body.getId());
}

void Program::printInterpretation(ostream& out) const {
    for(list<Atom::Data*>::const_iterator it = atomList.begin(); it != atomList.end(); ++it) {
        Atom atom(*it);
        out << atom << "[" << atom.getLowerBound() << ";" << atom.getUpperBound() << "] ";
    }
    //for(list<Atom::Data*>::const_iterator it = constants.begin(); it != constants.end(); ++it) {
    //    Atom atom(*it);
    //    out << atom << "[" << atom.getLowerBound() << ";" << atom.getUpperBound() << "] ";
    //}
    out << endl;
}

void Program::onInconsistency() {
    cout << "INCOHERENT" << endl;
    incoherent = true;
}

void Program::printSourcePointers(ostream& out) const {
    out << "SOURCE POINTERS" << endl;
    for(list<Atom::Data*>::const_iterator it = atomList.begin(); it != atomList.end(); ++it) {
        Atom atom(*it);
        out << atom << "\t";
        if(atom.getSourcePointer() != NULL)
            out << *atom.getSourcePointer();
        else
            out << "NULL";
        out << "\n";
    }
    out << endl;
}

void Program::initInterpretation() {
    trace(std, 1, "Init interpretation\n");

    // FIXME: this is really dirty!
    if(typeid(*tnorm) == typeid(LukasiewiczTnorm))
        computeSCC();

    //if(hasTraceLevel(std, 5))
    //    printSCC(cerr);

    trace(std, 2, "Processing constants\n");
    for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ) {
        list<Atom::Data*>::iterator curr = it++;
        Atom atom(*curr);
        if(atom.isConstant()) {
            trace(std, 3, "Found constant %s\n", atom.getName().c_str());
            atom.initConstant();
            constants.push_back(*curr);
            atomList.erase(curr);
        }
    }
    for(list<Atom::Data*>::iterator it = constants.begin(); it != constants.end(); ++it) {
        Atom atom(*it);
        trace(std, 3, "Processing constant %s\n", atom.getName().c_str());
        if(!atom.processConstant()) {
            onInconsistency();
            return;
        }
    }
    
    if(!propagate())
        onInconsistency();
    
    /*
    trace(std, 3, "Resetting upper bounds\n");
    for(list<Atom::Data*>::iterator it = constants.begin(); it != constants.end(); ++it)
        (**it).upperBound = 0;
    for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ++it)
        (**it).upperBound = 0;

    trace(std, 3, "Computing new upper bounds\n");
    for(list<Atom::Data*>::iterator it = constants.begin(); it != constants.end(); ++it) {
        Atom atom(*it);
        atom.updateSourcePointer(atom.getLowerBound(), NULL);
    }
    for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ++it) {
       Atom atom(*it);
       atom.findSourcePointer();
    }

    for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ++it) {
        Atom atom(*it);
        if(!atom.initSourcePointer()) {
            onInconsistency();
            return;
        }
    }

    for(list<Atom::Data*>::iterator it = constants.begin(); it != constants.end(); ++it) {
        Atom atom(*it);
        if(!atom.checkConstant()) {
            onInconsistency();
            return;
        }
    }
    */

//    if(__options__.mode != Options::WELL_FOUNDED) {
//        trace(std, 2, "Processing inequalities\n");
//        bool stop = false;
//        while(!stop) {
//            stop = true;
//            for(list<Component*>::iterator it = components.begin(); it != components.end(); ++it) {
//                Component& component = **it;
//                if(!component.hasChangedBounds())
//                    continue;
//                stop = false;
//                if(!(**it).updateLowerBoundsByLinearProgram()) {
//                    onInconsistency();
//                    return;
//                }
//            }
//        }
//    }
}

void Program::setNaiveBounds() {
    trace(std, 2, "Set naive bounds\n");
    for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ) {
        list<Atom::Data*>::iterator curr = it++;
        Atom atom(*curr);
        if(atom.isConstant()) {
            trace(std, 3, "Found constant %s\n", atom.getName().c_str());
            atom.parseBoundsForConstant();
            constants.push_back(*curr);
            atomList.erase(curr);
        }
        else {
            atom.data->upperBound = 1;
        }
    }

}

void Program::computeSCC() {
    /* TO BE REVISED
    vector<int>* component_ = NULL;
    int num = dependencyGraph.computeSCC(component_);
    vector<int>& component = *component_;
    vector<Component*> components(num);
    for(int i = 0; i < num; ++i)
        components[i] = new Component();
    for(unsigned i = 1; i < component.size(); ++i)
        if(atoms.find(i) != atoms.end())
            components[component[i]]->add(Atom(*this, i));
    for(list<int>::const_iterator it = dependencyGraph.selfLoops.begin(); it != dependencyGraph.selfLoops.end(); ++it)
        components[component[*it]]->setRecursive();
    for(int i = 0; i < num; ++i) {
        if(components[i]->isRecursive() && components[i]->hasOddCycles()) {
            components[i]->initLinearProgram();
            this->components.push_back(components[i]);
        }
        else
            delete components[i];
    }
    delete component_;
    */
    Component* c = new Component();
    for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ++it) {
        Atom atom(*it);
        if(!atom.isConstant())
            c->add(atom);
    }
    c->initLinearProgram();
    this->components.push_back(c);
}

void Program::printSCC(ostream& out) const {
    out << "Strongly Connected Componentes (only recursive with odd-cyles)" << endl;
    for(list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
        out << " " << **it << endl;
}

string Program::toString() const {
    stringstream ss;
    ss << *this;
    return ss.str();
}

void Program::computeFuzzyAnswerSet() {
    char tmpFile[] = "/tmp/fasp.XXXXXX";
    mkstemp(tmpFile);
    ofstream out(tmpFile);
    this->printBilevelProgram(out);
    out.close();

    stringstream cmd;
    cmd << "octave " << tmpFile;
    FILE* f = popen(cmd.str().c_str(), "r");
    if(f == 0)
        return;

    char buff[1024];
    while(fgets(buff, 1024, f) != NULL) {
        if(strcmp(buff, "SOLUTION\n") == 0) {
            int i = 1;
            list<Atom::Data*>::const_iterator it = atomList.begin();
            while(fgets(buff, 1024, f) != NULL) {
                Atom atom(*it);
                assert(atom.data->columnInBilevelProgram == i);

                stringstream ss(buff);
                double value;
                ss >> value;

                // Fix problem due to precision
                if(value < atom.getLowerBound())
                    value = atom.getLowerBound();
                else if(value > atom.getUpperBound())
                    value = atom.getUpperBound();

                cout << atom.getName() << "[" << value << "] ";
                ++i;
                ++it;
            }
            cout << endl;
        }
        else if(strcmp(buff, "INFEASIBLE\n") == 0) {
            onInconsistency();
        }
        else if(__options__.octaveTermOut)
            cerr << buff;
    }
    pclose(f);
    remove(tmpFile);
}

bool Program::computeFuzzyAnswerSet2() {
    while(true) {
        double max = 0;
        list<Atom::Data*>::iterator maxIt;
        for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ++it) {
            Atom atom(*it);
            double diff = atom.getBoundsDifference();
            if(diff > max) {
                max = diff;
                maxIt = it;
            }
        }
        
        if(max == 0)
            return true;
            
        unsigned unroll = processed;
        if(Atom(*maxIt).split())
            return true;
        
        trace(std, 2, "Found inconsistency\n");
        while(unroll < assignments.size()) {
            if(assignments.back().second < 10.0)
                assignments.back().first.unrollLowerBound(assignments.back().second);
            else
                assignments.back().first.unrollUpperBound(assignments.back().second - 10);
            assignments.pop_back();
        }
        processed = unroll;
        if(!Atom(*maxIt).split2())
            return false;
    }
    return true;
}

void Program::printBilevelProgram(ostream& out) {
    /*
    out << "do_braindead_shortcircuit_evaluation (1);\n";
    out << "warning(\"off\", \"Octave:possible-matlab-short-circuit-operator\");\n";
    out << "addpath(genpath('/home/malvi/workspaces/c/fasp/yalmip'));\n";

    out << "function varargout = ismembc (varargin)\n";
    out << "    varargout = cell (nargout, 1);\n";
    out << "    [varargout{:}] = ismember (varargin{:});\n";
    out << "endfunction\n";
    */

    out << "n = " << atomList.size() << ";\n";

    out << "i = sdpvar(1,n,'full');\n";
    out << "o = sdpvar(1,n,'full');\n";

    out << "OO = sum(o - i);\n";
    stringstream lb, ub;
    lb.precision(15);
    ub.precision(15);
//    if(__options__.bilevelProgram == Options::BILEVEL_PROGRAM_SIMPLE) {
//        lb << 0;
//        ub << 1;
//    }
//    else {
//        assert(__options__.bilevelProgram == Options::BILEVEL_PROGRAM_ENHANCED);

        assert(nextIdInBilevelProgram == 1);
        lb << "[";
        for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ++it) {
            Atom atom(*it);
            atom.getColumnIndexInBilevelProgram(nextIdInBilevelProgram);
            lb << atom.getLowerBound() << " ";
        }
        lb << "]";

        ub << "[";
        for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ++it) {
            Atom atom(*it);
            ub << atom.getUpperBound() << " ";
        }
        ub << "]";
//    }
    out << "CO = [o >= " << lb.str() << ", o <= " << ub.str() << "];\n";

    out << "OI = sum(i);\n";
    out << "CI = [ \n";
    for(list<Rule*>::iterator it = rules.begin(); it != rules.end(); ++it) {
        Rule* rule = *it;
        rule->printBilevelProgram(out, nextIdInBilevelProgram);
    }
    out << "  i >= " << lb.str() << ",\n  i <= o ];\n";

    out << "solvebilevel(CO,OO,CI,OI,i);\n";
    out << "if(double(OO) <= " << EPSILON << ")\n";
    out << "    disp(\"SOLUTION\");\n";
    out << "    od = double(o);\n";
    out << "    for x = od, disp(x); end;\n";
    out << "else\n";
    out << "    disp(\"INFEASIBLE\");\n";
    out << "endif\n";

    /*
    for(list<Atom::Data*>::const_iterator it = atomList.begin(); it != atomList.end(); ++it) {
        Atom atom(*it);
        cerr << atom.data->columnInBilevelProgram << ":" << atom.getName() << " ";
    }
    cerr << endl;
    */
}

bool Program::propagate() {
    while(processed < assignments.size()) {
        pair<Atom, double> next = assignments[processed++];
        if(next.second < 10.0)
        {
            if(!next.first.propagateLowerBound())
                return false;
        }
        else
        {
            if(!next.first.propagateUpperBound())
                return false;
        }
    }
    return true;
}
