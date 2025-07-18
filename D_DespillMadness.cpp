// D_DespillMadness.cpp
// Minimum boilerplate code required to create a Nuke Op/Plugin
 
// A string to define your class name.
static const char* const RCLASS = "D_DespillMadness";
 
// Includes, in this case we need the NoIop class to inherit from it.
#include "DDImage/NoIop.h"
 
// Namespace. In this case we don't need it, but it's usually convenient to have.
using namespace DD::Image;
 
// Our class: MyNoOp, inheriting from NoIop.
class D_DespillMadness : public NoIop
{
public:
    // Constructor
    D_DespillMadness(Node* node) : NoIop(node) {}
    // Class member function, must return the class name.
    const char* Class() const override { return RCLASS; }
    // Help string, optional, but good to get into the habit of defining it.
    const char* node_help() const override { return "Do nothing."; }
    // Define the class description object. At this point it's not initialized yet.
    static Iop::Description d;
};
 
// Create a build function. It is responsible for initializing our Op with a Node object, that Nuke will create.
static Iop* build(Node* node) { return new D_DespillMadness(node); }
// Initialize our class description. 
// The first argument MUST be the same object as the return value of our Op's Class() function.
// The second argument is a leftover from Nuke4, back before nuke menus were made with TCL, then Python. It was representing where the menu should be created. Nuke still expects a value but doesn't use it.
// The third argument is a pointer to our build function.
Iop::Description D_DespillMadness::d(RCLASS, "Other/D_DespillMadness", build);