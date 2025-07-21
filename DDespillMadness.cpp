// DDespillMadness.cpp
static const char* const CLASS = "DDespillMadness";
 
#include "DDImage/PixelIop.h"
#include "DDImage/NukeWrapper.h"
#include "DDImage/Knobs.h"
#include "DDImage/Row.h"
#include "DDImage/Channel.h"
#include "DDImage/DDMath.h"
 
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

    float saturationWeights[3] = { 0.2126f, 0.7152f, 0.0722f };

public:

    DDespillMadness(Node* node) : PixelIop(node)
    {
        screenTypeKnob = 0;
        despillAlgorithmKnob = 0;
        fineTuneKnob = 0.94f;
        restoreSourceLuminanceKnob = true;
        outputSpillMatteKnob = false;

        for (int i=0; i<4; i++) 
        {
            saturationKnob[i] = gammaKnob[i] = 1.0f;
            offsetKnob[i] = 0.0f;
        }
 
    }

    const char* input_label(int n, char*) const override
    {
        switch (n) {
        case 0: return "img";
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

    void _validate(bool for_real) override
    {
        copy_info();

        ChannelSet outChannels = info_.channels();
        
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

        // Temporary variables to hold the output values
        // These will be modified based on the screen type, algorithm, saturation, gamma, and offset knobs
        // and then written to the output buffers.

        float RGBAOutVal[4];

        // Pointer to when the loop is done:
        const float* END = rIn + (r - x);

        // Start the loop:
        while (rIn < END) {

            float RGBexpression;
            float Aexpression;


            // Temporary variables to hold the output values
            // These will be modified based on the screen type, algorithm, saturation, gamma, and offset knobs
            // and then written to the output buffers.
            float RGBAOutVal[4] = { *rIn++, *gIn++, *bIn++, 0.0f };


            if(screenTypeKnob == 0) // Green screen
            {
                switch (despillAlgorithmKnob)
                {
                    case 0: 
                        RGBexpression = (RGBAOutVal[1] > (RGBAOutVal[2]+RGBAOutVal[0]) / 2 * fineTuneKnob) ? ((RGBAOutVal[2]+RGBAOutVal[0])/2*fineTuneKnob) :RGBAOutVal[1];
                        Aexpression = RGBAOutVal[1] - (RGBAOutVal[0]+RGBAOutVal[2]) * fineTuneKnob / 2;
                        break;
                    case 1: 
                        RGBexpression = (RGBAOutVal[1] > (2*RGBAOutVal[2]+RGBAOutVal[0]) / 3 * fineTuneKnob) ? ((2*RGBAOutVal[2]+RGBAOutVal[0])/3*fineTuneKnob) :RGBAOutVal[1];
                        Aexpression = RGBAOutVal[1] - (RGBAOutVal[0]+RGBAOutVal[2]) * fineTuneKnob / 2;
                        break;
                    case 2: 
                        RGBexpression = (RGBAOutVal[1] > (RGBAOutVal[2]+2*RGBAOutVal[0]) / 3 * fineTuneKnob) ? ((RGBAOutVal[2]+2*RGBAOutVal[0]) / 3 * fineTuneKnob) :RGBAOutVal[1];
                        Aexpression = RGBAOutVal[1] - (RGBAOutVal[0]+RGBAOutVal[2]) * fineTuneKnob / 2;
                        break;
                    case 3: 
                        RGBexpression = (RGBAOutVal[1] > RGBAOutVal[0] * fineTuneKnob) ? (RGBAOutVal[0] * fineTuneKnob) : RGBAOutVal[1];
                        Aexpression = RGBAOutVal[1] - RGBAOutVal[0] * fineTuneKnob;
                        break;
                    case 4: 
                        RGBexpression = (RGBAOutVal[1] > RGBAOutVal[2] * fineTuneKnob) ? (RGBAOutVal[2] * fineTuneKnob) : RGBAOutVal[1];
                        Aexpression = RGBAOutVal[1] - RGBAOutVal[2] * fineTuneKnob;
                        break;

                    default:
                        RGBexpression = (RGBAOutVal[1] > (RGBAOutVal[2]+RGBAOutVal[0]) / 2 * fineTuneKnob) ? ((RGBAOutVal[2]+RGBAOutVal[0])/2*fineTuneKnob) :RGBAOutVal[1];
                        Aexpression = RGBAOutVal[1] - (RGBAOutVal[0]+RGBAOutVal[2]) * fineTuneKnob / 2;
                        break;
                }

                RGBAOutVal[1] = RGBexpression;
                RGBAOutVal[3] = Aexpression;

            }
            else // Blue screen
            {
                switch (despillAlgorithmKnob)
                {
                    case 0: 
                        RGBexpression = (RGBAOutVal[2] > (RGBAOutVal[1]+RGBAOutVal[0]) / 2 * fineTuneKnob) ? ((RGBAOutVal[1]+RGBAOutVal[0])/2*fineTuneKnob) :RGBAOutVal[2];
                        Aexpression = RGBAOutVal[2] - (RGBAOutVal[0]+RGBAOutVal[1]) * fineTuneKnob / 2;
                        break;
                    case 1: 
                        RGBexpression = (RGBAOutVal[2] > (2*RGBAOutVal[1]+RGBAOutVal[0]) / 2 * fineTuneKnob) ? ((2*RGBAOutVal[1]+RGBAOutVal[0]) / 2 * fineTuneKnob) :RGBAOutVal[2];
                        Aexpression = RGBAOutVal[2] - (RGBAOutVal[0]+2*RGBAOutVal[1]) * fineTuneKnob / 2;
                        break;
                    case 2: 
                        RGBexpression = (RGBAOutVal[2] > (RGBAOutVal[1]+2*RGBAOutVal[0]) / 2 * fineTuneKnob) ? ((RGBAOutVal[1]+2*RGBAOutVal[0]) / 2 * fineTuneKnob) :RGBAOutVal[2];
                        Aexpression = RGBAOutVal[2] - (2*RGBAOutVal[0]+RGBAOutVal[1]) * fineTuneKnob / 2;
                        break;
                    case 3: 
                        RGBexpression = (RGBAOutVal[2] > RGBAOutVal[0] * fineTuneKnob) ? (RGBAOutVal[0] * fineTuneKnob) : RGBAOutVal[2];
                        Aexpression = RGBAOutVal[2] - RGBAOutVal[0] * fineTuneKnob;
                        break;
                    case 4: 
                        RGBexpression = (RGBAOutVal[2] > RGBAOutVal[1] * fineTuneKnob) ? (RGBAOutVal[1] * fineTuneKnob) : RGBAOutVal[2];
                        Aexpression = RGBAOutVal[2] - RGBAOutVal[1] * fineTuneKnob;
                        break;

                    default:
                        RGBexpression = (RGBAOutVal[2] > RGBAOutVal[0] * fineTuneKnob) ? (RGBAOutVal[0] * fineTuneKnob) : RGBAOutVal[2];
                        Aexpression = RGBAOutVal[2] - RGBAOutVal[0] * fineTuneKnob;
                        break;
                }

                RGBAOutVal[2] = RGBexpression;
                RGBAOutVal[3] = Aexpression;

            }

            RGBAOutVal[3] = clamp(RGBAOutVal[3], 0.0f, 1.0f);

            // Compute luminance with Rec.709 weights
            float luma = 0.2126f * RGBAOutVal[0] +
                        0.7152f * RGBAOutVal[1] +
                        0.0722f * RGBAOutVal[2];

            for (int i = 0; i < 3; ++i)
            {
                float original = RGBAOutVal[i];

                // Saturation
                float grade = luma + (original - luma) * saturationKnob[i];

                // Gamma (skip negatives)
                if (gammaKnob[i] > 0.0f && grade >= 0.0f)
                {
                    grade = powf(grade, 1.0f / gammaKnob[i]);
                }

                // Offset
                grade += offsetKnob[i];

                // Mask blend (using spill matte)
                grade = RGBAOutVal[3] * grade + (1.0f - RGBAOutVal[3]) * original;

                RGBAOutVal[i] = grade;
            }
            
            // Write the output values to the output buffers
            *rOut++ = RGBAOutVal[0];
            *gOut++ = RGBAOutVal[1];
            *bOut++ = RGBAOutVal[2];
            *aOut++ = RGBAOutVal[3];


           
        }
    }


    const char* Class() const override { return CLASS; }
    const char* node_help() const override { return "Do nothing."; }

    static const Description d;
};



static Iop *build(Node *node)
{
  return (new NukeWrapper(new DDespillMadness(node)))->noChannels();
}

const Iop::Description DDespillMadness::d(CLASS, "Color/DDespillMadness", build);
