// DDespillMadness.cpp
// Minimum boilerplate code required to create a Nuke Op/Plugin
 
// A string to define your class name.
static const char* const CLASS = "DDespillMadness";
 
// Includes, in this case we need the NoIop class to inherit from it.
#include "DDImage/NoIop.h"
#include "DDImage/Knobs.h"
#include "DDImage/Row.h"
#include "DDImage/Channel.h"
 
// Namespace. In this case we don't need it, but it's usually convenient to have.
using namespace DD::Image;

static const char* const screenTypeKnobNames[]  = { "green", "blue", 0 };
static const char* const despillAlgorithmKnobNames[]  = { "average", "double green/blue average", "double red average", "red limit", "green/blue average", 0 };
 
// Our class: MyNoOp, inheriting from NoIop.
class DDespillMadness : public NoIop
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
    DDespillMadness(Node* node) : NoIop(node)
    {
        inputs(2);
        screenTypeKnobKnob = 0;
        despillAlgorithmKnobKnob = 0;
        fineTuneKnob = 0.94f;
        restoreSourceLuminanceKnob = true;
        outputSpillMatteKnob = false;
        maskChannel = Chan_Black;
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
        case 1: return "mask";
        default: return 0;
        }
    }
    
    void DDespillMadness::_validate(bool for_real) override
    {
        copy_info();

        ChannelSet outChannels = channels();

        outChannels &= Mask_RGB;

        if(!restoreSourceLuminanceKnob)
        {
            if(screenTypeKnobKnob == 0) // green
            {
                outChannels &= Mask_Green;
            }
            else // blue
            {
                outChannels &= Mask_Blue;
            }

        }
        
        if(outputSpillMatteKnob)
        {
            outChannels += Chan_Alpha;
        }

        set_out_channels(outChannels);
        info_.turn_on(outChannels);

        Iop::_validate(for_real);
    }

    void DDespillMadness::knobs(Knob_Callback f) override
    {
        Enumeration_knob(f, &screenTypeKnobKnob, screenTypeKnobNames, "screenType", "screen type");
        Enumeration_knob(f, &despillAlgorithmKnobKnob, despillAlgorithmKnobNames, "algorithm", "despill algorithm");
        Float_knob(f, &fineTuneKnob, IRange(0.5f, 1.5f), "LimitPercentage", "fine tune");

        Divider(f, "");
        
        Text_knob(f, "spill area correction"); SetFlags(f, Knob::STARTLINE);
        Text_knob(f, " "); SetFlags(f, Knob::STARTLINE); Spacer(f, 250);

        Bool_knob(f, &restoreSourceLuminanceKnob, "sourceLuma", "restore source luminance"); Tooltip(f, "Restores luminance of the original image"); SetFlags(f, Knob::STARTLINE);
        
        AColor_knob(f, saturationKnob, IRange(0.f, 4), "saturation", "saturation");
        AColor_knob(f, gammaKnob, IRange(0.2f, 5), "gamma", "gamma");
        AColor_knob(f, offsetKnob, IRange(-1, 1), "offset", "offset");

        Divider(f, "");

        Bool_knob(f, &outputSpillMatteKnob, "spillMatteOut", "output spill matte in alpha"); 
        SetFlags(f, Knob::STARTLINE);

        Divider(f, "");

        Input_Channel_knob(f, &maskChannel, 1, 1, "maskChannel", "mask channel");
        Bool_knob(f, &invertMask, "invertMask", "invert");
        Float_knob(f, &mix, "mix");
    }

    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return "Do nothing."; }

    static const Description d;
};
 
static Iop* build(Node* node) { return new DDespillMadness(node); }
const Iop::Description DDespillMadness::d(CLASS, nullptr, build);