#include "Program.h"

#include "trace.h"
#include "LukasiewiczTnorm.h"

#include <sstream>
#include <typeinfo>

ostream& operator<<(ostream& out, const Program& program) {
    for(list<Rule*>::const_iterator it = program.rules.begin(); it != program.rules.end(); ++it)
        out << **it << endl;
    return out;
}

Program::Program(const Tnorm& _tnorm) : tnorm(_tnorm.clone())
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
    out << endl;
}

void Program::onInconsistency() {
    cout << "INCOHERENT" << endl;
}

void Program::printSourcePointers(ostream& out) const {
    out << "SOURCE POINTERS" << endl;
    for(list<Atom::Data*>::const_iterator it = atomList.begin(); it != atomList.end(); ++it) {
        Atom atom(*it);
        out << atom << "\t";
        if(atom.getSourcePointer() != NULL)
            out << *atom.getSourcePointer();
        out << "\n";
    }
    out << endl;
}

void Program::initInterpretation() {
    trace(std, 1, "Init interpretation\n");

    // FIXME: this is really dirty!
    if(typeid(*tnorm) == typeid(LukasiewiczTnorm))
        computeSCC();

    if(hasTraceLevel(std, 5))
        printSCC(cerr);

    trace(std, 2, "Processing constants\n");
    for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ) {
        list<Atom::Data*>::iterator curr = it++;
        Atom atom(*curr);
        if(atom.isConstant()) {
            trace(std, 3, "Found constant %s\n", atom.getName().c_str());
            if(!atom.initConstant()) {
                onInconsistency();
                return;
            }
            constants.push_back(*curr);
            atomList.erase(curr);
        }
    }
    for(list<Atom::Data*>::iterator it = atomList.begin(); it != atomList.end(); ++it) {
        Atom atom(*it);
        if(!atom.initSourcePointer()) {
            onInconsistency();
            return;
        }
    }

    trace(std, 2, "Processing inequalities\n");
    bool stop = false;
    while(!stop) {
        stop = true;
        for(list<Component*>::iterator it = components.begin(); it != components.end(); ++it) {
            Component& component = **it;
            if(!component.hasChangedBounds())
                continue;
            stop = false;
            if(!(**it).updateLowerBoundsByLinearProgram()) {
                onInconsistency();
                return;
            }
        }
    }
}

void Program::computeSCC() {
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
