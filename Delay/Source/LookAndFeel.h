/*
  ==============================================================================

    LookAndFeel.h
    Styling the GUI
 
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
/*  Define some colors that will be used by drawing code */

namespace Colors        // Using namespace is good practice for organizing constants
{
const juce::Colour background {245, 240, 235};
const juce::Colour header {40, 40, 40};

namespace Knob
{
    const juce::Colour trackBackground { 205, 200, 195 };
    const juce::Colour trackActive { 177, 101, 135 };
    const juce::Colour outline { 255, 250, 245 };
    const juce::Colour gradientTop { 250, 245, 240 };
    const juce::Colour gradientBottom { 240, 235, 230 };
    const juce::Colour dial { 100, 100, 100 };
    const juce::Colour dropShadow { 195, 190, 185 };
    const juce::Colour label { 80, 80, 80 };
    const juce::Colour textBoxBackground { 80, 80, 80 };
    const juce::Colour value { 240, 240, 240 };
    const juce::Colour caret { 255, 255, 255 };
}

namespace Button
{
    const juce::Colour text { 80, 80, 80 };
    const juce::Colour textToggled { 40, 40, 40 };
    const juce::Colour background { 245, 240, 235 };
    const juce::Colour backgroundToggled { 255, 250, 245 };
    const juce::Colour outline { 235, 230, 225 };
}

namespace LevelMeter
{
    const juce::Colour background { 245, 240, 235 };
    const juce::Colour tickLine { 200, 200, 200 };
    const juce::Colour tickLabel { 80, 80, 80 };
    const juce::Colour tooLoud { 226, 74, 81 };
    const juce::Colour levelOK { 65, 206, 88 };
}

namespace Group
{
    const juce::Colour label {160, 155, 150};
    const juce::Colour outline {235, 230, 225};
}

}

//==============================================================================
// Convenient class to write Fonts::getFont() whenever we want to use imported Font file
class Fonts
{
public:
    static juce::Font getFont(float height = 16.0f);
private:
    static const juce::Typeface::Ptr typeface;
    
    Fonts() = delete; // Both functions are static, we dont need to create an instance of Fonts to use it.
};


//==============================================================================
/*
    Styling the slider's knob
 */
class RotaryKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RotaryKnobLookAndFeel();
    
    // Returns a pointer to a single shared instance of this class
    static RotaryKnobLookAndFeel* get()
    {
        static RotaryKnobLookAndFeel instance;
        return &instance;
    }
    
    // Draws the knob
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider)
    override;
    
    // Gets the font used
    juce::Font getLabelFont(juce::Label&) override;
    
    // Customize the slider text box
    juce::Label* createSliderTextBox(juce::Slider&) override;
    // We dont want to draw an outline, so override inherited function with empty body
    void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override {}
    void fillTextEditorBackground(juce::Graphics&, int width, int height, juce::TextEditor&) override;
    
private:
    juce::DropShadow dropShadow{Colors::Knob::dropShadow, 6, {0,3}};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnobLookAndFeel)
};

//==============================================================================
/*
    Styling the button
 */
class ButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ButtonLookAndFeel();
    
    static ButtonLookAndFeel* get(){
        static ButtonLookAndFeel instance;
        return &instance;
    }
    
    // Draws the button
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColor,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown)  override;
    
    void drawButtonText(juce::Graphics& g, juce::TextButton &button,
                        bool shouldDrawButtonAsHightlighted,
                        bool shouldDrawButtonAsDown) override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ButtonLookAndFeel)
};

//==============================================================================
/*
    Styling the entire editor (except for knobs)
 */
class MainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MainLookAndFeel();
    juce:: Font getLabelFont(juce::Label&) override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainLookAndFeel)
};
