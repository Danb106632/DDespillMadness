// D_DespillMadness.cpp
// Minimum boilerplate code required to create a Nuke Op/Plugin
 
// A string to define your class name.
static const char* const CLASS = "D_DespillMadness";
 
// Includes, in this case we need the NoIop class to inherit from it.
#include "DDImage/NoIop.h"
#include "DDImage/Knobs.h"
 
// Namespace. In this case we don't need it, but it's usually convenient to have.
using namespace DD::Image;

static const char* const screenTypeKnobNames[]  = { "green", "blue", 0 };
static const char* const despillAlgorithmKnobNames[]  = { "average", "double green/blue average", "double red average", "red limit", "green/blue average", 0 };
 
// Our class: MyNoOp, inheriting from NoIop.
class D_DespillMadness : public NoIop
{
    int screenTypeKnobKnob;
    int despillAlgorithmKnobKnob;
    float fineTuneKnob;
    bool restoreSourceLuminanceKnob;
    float saturationKnob[4];
    float gammaKnob[4];
    float offsetKnob[4];
    bool outputSpillMatteKnob;
    Channel maskChannel;
    bool invertMask;
    float mix;

public:
    D_DespillMadness(Node* node) : NoIop(node)
    {
        inputs(2);
        screenTypeKnobKnob = 0;
        despillAlgorithmKnobKnob = 0;
        fineTuneKnob = 0.94f;
        restoreSourceLuminanceKnob = true;
        outputSpillMatteKnob = false;
        //maskChannel = Chan_Alpha;
        invertMask = false;
        mix = 1.0f;

        for (int i=0; i<4; i++) 
        {
            saturationKnob[i] = 1.0f;
            gammaKnob[i] = 1.0f;
            offsetKnob[i] = 0.0f;
        }
    }

    const char* input_label(int n, char*) const override
    {
        switch (n) {
        case 0: return "img";
        default: return "mask";
        }
    }
    
    void D_DespillMadness::_validate(bool for_real) override
    {
        NoIop::_validate(for_real);
    }

    void D_DespillMadness::knobs(Knob_Callback f) override
    {
        Enumeration_knob(f, &screenTypeKnobKnob, screenTypeKnobNames, "screenType", "screen type");
        Enumeration_knob(f, &despillAlgorithmKnobKnob, despillAlgorithmKnobNames, "algorithm", "despill algorithm");
        Float_knob(f, &fineTuneKnob, IRange(0.5f, 1.5f), "LimitPercentage", "fine tune");
        Divider(f, "");
        Text_knob(f, "spill area correction"); SetFlags(f, Knob::STARTLINE);
        Text_knob(f, " "); SetFlags(f, Knob::STARTLINE); Spacer(f, 250);
        Bool_knob(f, &restoreSourceLuminanceKnob, "restore source luminance"); Tooltip(f, "Restores luminance of the original image"); SetFlags(f, Knob::STARTLINE);
        AColor_knob(f, saturationKnob, IRange(0.f, 4), "saturation", "saturation");
        AColor_knob(f, gammaKnob, IRange(0.2f, 5), "gamma", "gamma");
        AColor_knob(f, offsetKnob, IRange(-1, 1), "offset", "offset");
        Divider(f, "");
        Bool_knob(f, &outputSpillMatteKnob, "output spill matte in alpha"); SetFlags(f, Knob::STARTLINE);
        Divider(f, "");
        Input_Channel_knob(f, &maskChannel, 1, 2, "maskChannel", "mask channel");
        Bool_knob(f, &invertMask, "invertMask", "invert");
        Float_knob(f, &mix, "mix");

    }

    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return "Do nothing."; }

    static const Description d;
};
 
static Iop* build(Node* node) { return new D_DespillMadness(node); }
const Iop::Description D_DespillMadness::d(CLASS, nullptr, build);