// DDespillMadness.cpp
static const char* const CLASS = "DDespillMadness";
 
#include "DDImage/PixelIop.h"
#include "DDImage/NukeWrapper.h"
#include "DDImage/Knobs.h"
#include "DDImage/Row.h"
#include "DDImage/Channel.h"
 
// Namespace. In this case we don't need it, but it's usually convenient to have.
using namespace DD::Image;

static const char* const screenTypeKnobNames[]  = { "green", "blue", 0 };
static const char* const despillAlgorithmKnobNames[]  = { "average", "double green/blue average", "double red average", "red limit", "green/blue average", 0 };

class DDespillMadness : public PixelIop
{
    int screenTypeKnob;
    int despillAlgorithmKnob;
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

    DDespillMadness(Node* node) : PixelIop(node)
    {
        inputs(2);
        screenTypeKnob = 0;
        despillAlgorithmKnob = 0;
        fineTuneKnob = 0.94f;
        restoreSourceLuminanceKnob = true;
        outputSpillMatteKnob = false;

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

    // Add our knobs to the user interface. This is called by Nuke to build the control panel.
    void knobs(Knob_Callback f) override
    {
        Divider(f, "");

        Enumeration_knob(f, &screenTypeKnob, screenTypeKnobNames, "screenType", "screen type");
        Enumeration_knob(f, &despillAlgorithmKnob, despillAlgorithmKnobNames, "algorithm", "despill algorithm");
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
    }

    void in_channels(int input_number, ChannelSet& channels) const override
    {
        channels &= Mask_RGB;
    }

    void _validate(bool for_real) override
    {
        copy_info();

        ChannelSet outChannels = channels();
        
        if(!restoreSourceLuminanceKnob)
        {
            if(screenTypeKnob == 0) 
            {
                outChannels &= Mask_Green;
            }  
            else 
            {
                outChannels &= Mask_Blue;
            }
        }
        else
        {
            outChannels &= Mask_RGB;
        }
        
        if(outputSpillMatteKnob)
            outChannels += Chan_Alpha;

        set_out_channels(outChannels);
        info_.turn_on(outChannels);

        PixelIop::_validate(for_real);
    }

    // The pixel_engine function is called by Nuke to process the image data.
    void pixel_engine(const Row& in, int y, int x, int r, ChannelMask channels, Row& out) override
    {
        const float* rIn = in[Chan_Red] + x;
        const float* gIn = in[Chan_Green] + x;
        const float* bIn = in[Chan_Blue] + x;

        float* rOut = out.writable(Chan_Red) + x;
        float* gOut = out.writable(Chan_Green) + x;
        float* bOut = out.writable(Chan_Blue) + x;
        float* aOut = out.writable(Chan_Alpha) + x;

        // Pointer to when the loop is done:
        const float* END = rIn + (r - x);

        // Start the loop:
        while (rIn < END) {

            float rVal = *rIn++;
            float gVal = *gIn++;
            float bVal = *bIn++;

            // Temporary variables to hold the output values
            // These will be modified based on the screen type, algorithm, saturation, gamma, and offset knobs
            // and then written to the output buffers.
            float rOutVal = rVal;
            float gOutVal = gVal;
            float bOutVal = bVal;
            float aOutVal = 0.0f;
            
            float RGBexpression;
            float Aexpression;

            if(screenTypeKnob == 0) // Green screen
            {
                switch (despillAlgorithmKnob)
                {
                    case 0: 
                        RGBexpression = (gVal > (bVal+rVal) / 2 * fineTuneKnob) ? ((bVal+rVal)/2*fineTuneKnob) :gVal;
                        Aexpression = gVal - (rVal+bVal) * fineTuneKnob / 2;
                        break;
                    case 1: 
                        RGBexpression = (gVal > (2*bVal+rVal) / 3 * fineTuneKnob) ? ((2*bVal+rVal)/3*fineTuneKnob) :gVal;
                        Aexpression = gVal - (rVal+bVal) * fineTuneKnob / 2;
                        break;
                    case 2: 
                        RGBexpression = (gVal > (bVal+2*rVal) / 3 * fineTuneKnob) ? ((bVal+2*rVal) / 3 * fineTuneKnob) :gVal;
                        Aexpression = gVal - (rVal+bVal) * fineTuneKnob / 2;
                        break;
                    case 3: 
                        RGBexpression = (gVal > rVal * fineTuneKnob) ? (rVal * fineTuneKnob) : gVal;
                        Aexpression = gVal - rVal * fineTuneKnob;
                        break;
                    case 4: 
                        RGBexpression = (gVal > bVal * fineTuneKnob) ? (bVal * fineTuneKnob) : gVal;
                        Aexpression = gVal - bVal * fineTuneKnob;
                        break;

                    default:
                        RGBexpression = (gVal > (bVal+rVal) / 2 * fineTuneKnob) ? ((bVal+rVal)/2*fineTuneKnob) :gVal;
                        Aexpression = gVal - (rVal+bVal) * fineTuneKnob / 2;
                        break;
                }

            }
            else // Blue screen
            {
                switch (despillAlgorithmKnob)
                {
                    case 0: 
                        RGBexpression = (bVal > (gVal+rVal) / 2 * fineTuneKnob)?((gVal+rVal) / 2 * fineTuneKnob):bVal;
                        Aexpression = bVal - (rVal+gVal) * fineTuneKnob / 2;
                        break;
                    case 1: 
                        RGBexpression = (bVal > (2*gVal+rVal) / 2 * fineTuneKnob)?((2*gVal+rVal) / 2 * fineTuneKnob):bVal;
                        Aexpression = bVal - (rVal+2*gVal) * fineTuneKnob / 2;
                        break;
                    case 2: 
                        RGBexpression = (bVal > (gVal+2*rVal) / 2 * fineTuneKnob)?((gVal+2*rVal) / 2 * fineTuneKnob):bVal;
                        Aexpression = bVal - (2*rVal+gVal) * fineTuneKnob / 2;
                        break;
                    case 3: 
                        RGBexpression = (bVal > rVal * fineTuneKnob) ? (rVal * fineTuneKnob) : bVal;
                        Aexpression = bVal - rVal * fineTuneKnob;
                        break;
                    case 4: 
                        RGBexpression = (bVal > gVal * fineTuneKnob) ? (gVal * fineTuneKnob) : bVal;
                        Aexpression = bVal - gVal * fineTuneKnob;
                        break;

                    default:
                        RGBexpression = (bVal > rVal * fineTuneKnob) ? (rVal * fineTuneKnob) : bVal;
                        Aexpression = bVal - rVal * fineTuneKnob;
                        break;
                }
            }

            if(screenTypeKnob == 0) // Green screen
            {
                rOutVal = rVal;
                gOutVal = RGBexpression;
                bOutVal = bVal;
                aOutVal = Aexpression;
            }
            else // Blue screen
            {
                rOutVal = rVal;
                gOutVal = gVal;
                bOutVal = RGBexpression;
                aOutVal = Aexpression;
            }

            *rOut++ = rOutVal;
            *gOut++ = gOutVal;
            *bOut++ = bOutVal;
            *aOut++ = aOutVal;
        }
    }


    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return "Do nothing."; }

    static const Description d;
};



static Iop* build(Node* node) { return new NukeWrapper(new DDespillMadness(node)); }

const Iop::Description DDespillMadness::d(CLASS, "Color/DDespillMadness", build);
