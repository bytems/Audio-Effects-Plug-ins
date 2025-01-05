/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 31 Dec 2024 10:21:57am
    Author:  mohamed saleh

  ==============================================================================
*/

#include "LookAndFeel.h"

//=========================    Fonts   ==========================================
/* Load the font from the font file data added to BinaryData */
const juce::Typeface::Ptr  Fonts::typeface = juce::Typeface::createSystemTypefaceFor(BinaryData::LatoMedium_ttf, BinaryData::LatoMedium_ttfSize);

/* Return a font based on typeface, with a particular height */
juce::Font Fonts::getFont(float height){
    return juce::FontOptions(typeface)
                .withMetricsKind(juce::TypefaceMetricsKind::legacy)
                .withHeight(height);
}

juce::Font RotaryKnobLookAndFeel::getLabelFont([[maybe_unused]] juce::Label& label){
    return Fonts::getFont();
}


//==============================================================================
RotaryKnobLookAndFeel::RotaryKnobLookAndFeel()
{
    setColour(juce::Label::textColourId, Colors::Knob::label);
    setColour(juce::Slider::textBoxTextColourId, Colors::Knob::label);
    setColour(juce::Slider::rotarySliderFillColourId, Colors::Knob::trackActive);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::CaretComponent::caretColourId, Colors::Knob::caret);
}

void RotaryKnobLookAndFeel::drawRotarySlider(
        juce::Graphics &g,
        int x,int y,int width, [[maybe_unused]] int height,
        float sliderPos,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider &slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, width).toFloat();
    auto knobRect = bounds.reduced(10.0f, 10.0f);
    
    // Add a drop shadow to visually seperate dial from the background
    auto path = juce::Path(); // A path is a series of points that are connected to create a certain shape
    path.addEllipse(knobRect);
    dropShadow.drawForPath(g, path);
    
    g.setColour(Colors::Knob::outline);
    g.fillEllipse(knobRect);
    
    // Draw a subtle gradient inside the Knob (which adds some depth)
    auto innerRect = knobRect.reduced(2.0f, 2.0f);
    auto gradient = juce::ColourGradient(Colors::Knob::gradientTop, 0.0f, innerRect.getY(),
                                         Colors::Knob::gradientBottom, 0.0f, innerRect.getBottom(),
                                         false);
    g.setGradientFill(gradient); // Tells graphic context that from now it should use this gradient
                                 // to fill shapes instead of color that was previoulsy set g.setColor
    g.fillEllipse(innerRect);
    
    // Draw an arc around outside of the Knob. AKA the "track"
    auto center = bounds.getCentre();
    auto radius = bounds.getWidth()/2.0f;
    auto lineWidth = 3.0f;
    auto arcRadius = radius - lineWidth/2.0f;
    
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(center.x, center.y, arcRadius, arcRadius,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);
    auto strokeType = juce::PathStrokeType(lineWidth,
                                           juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded);
    g.setColour(Colors::Knob::trackBackground);
    g.strokePath(backgroundArc, strokeType);
    
    // Drawing the dial
    auto dialRadius = innerRect.getHeight() / 2.0f - lineWidth;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    //juce::Point<float> dialStart(center.x, center.y);
    juce::Point<float> dialStart(center.x + 10.0f * std::sin(toAngle),
                                 center.y - 10.0f * std::cos(toAngle));
    juce::Point<float> dialEnd(center.x + dialRadius*std::sin(toAngle),
                               center.y - dialRadius*std::cos(toAngle));
    // Create new path that descbribes line between dialStart & dialEnd points. Then stroke it
    juce::Path dialPath;
    dialPath.startNewSubPath(dialStart);
    dialPath.lineTo(dialEnd);
    g.setColour(Colors::Knob::dial);
    g.strokePath(dialPath, strokeType);
    
    // Fill the arc "track" to where the dial points with a different color
    if(slider.isEnabled()){
        float fromAngle = rotaryStartAngle;
        if(slider.getProperties()["drawFromMiddle"])
            fromAngle += (rotaryEndAngle - rotaryStartAngle) / 2.0f;
        juce::Path valueArc;
        valueArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f,
                               fromAngle, toAngle, true);
        //g.setColour(Colors::Knob::trackActive);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, strokeType);
    }
    
}

class RotaryKnobLabel : public juce::Label{
public:
    RotaryKnobLabel() : juce::Label() {}
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override {}
    
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler(*this);
    }

    juce::TextEditor* createEditorComponent() override
    {
        auto* ed = new juce::TextEditor(getName());
        ed->applyFontToAllText(getLookAndFeel().getLabelFont(*this));
        copyAllExplicitColoursTo(*ed);

        ed->setBorder(juce::BorderSize<int>());
        ed->setIndents(2, 1);
        ed->setJustification(juce::Justification::centredTop);
        ed->setPopupMenuEnabled(false);
        ed->setInputRestrictions(8);
        return ed;
    }
};

juce::Label* RotaryKnobLookAndFeel::createSliderTextBox(juce::Slider& slider){
    auto l = new RotaryKnobLabel();
    l->setJustificationType(juce::Justification::centred);
    l->setKeyboardType(juce::TextInputTarget::decimalKeyboard);
    l->setColour(juce::Label::textColourId,
                 slider.findColour(juce::Slider::textBoxTextColourId));
    l->setColour(juce::TextEditor::textColourId, Colors::Knob::value);
    l->setColour(juce::TextEditor::highlightedTextColourId,Colors::Knob::value);
    l->setColour(juce::TextEditor::highlightColourId,
                 slider.findColour(juce::Slider::rotarySliderFillColourId));
    l->setColour(juce::TextEditor::backgroundColourId,
                 Colors::Knob::textBoxBackground);
    return l;
}

void RotaryKnobLookAndFeel::fillTextEditorBackground(juce::Graphics& g,
                                                     [[maybe_unused]] int width,
                                                     [[maybe_unused]] int height,
                                                     juce::TextEditor& textEditor)
{
    g.setColour(Colors::Knob::textBoxBackground);
    g.fillRoundedRectangle(textEditor.getLocalBounds().reduced(4,0).toFloat(), 4.0f);
}



//=============================   Button   =====================================
/**
    Constructor allows you to override colors for buttons
 */
ButtonLookAndFeel::ButtonLookAndFeel()
{
    setColour(juce::TextButton::textColourOffId,  Colors::Button::text);
    setColour(juce::TextButton::textColourOnId,   Colors::Button::textToggled);
    setColour(juce::TextButton::buttonColourId,   Colors::Button::background);
    setColour(juce::TextButton::buttonOnColourId, Colors::Button::backgroundToggled);
}

void ButtonLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button,
                                             const juce::Colour &backgroundColor,
                                             [[maybe_unused]]bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown){
    // Create a rectangle that the same as buttons bound but inset by one pixel on all sides
    // so that theres room to draw border around the button. Trim an additional pixel from bottom
    auto bounds = button.getLocalBounds().toFloat();
    auto cornerSize = bounds.getHeight() * 0.25f;
    auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);
    
    // When button is clicked, shift the "buttonRect" one pixel downard, to give impression that
    // that button is literally being pressed down like a mechanical button
    if(shouldDrawButtonAsDown)
        buttonRect.translate(0.0, 1.0f);
    
    g.setColour(backgroundColor);
    g.fillRoundedRectangle(buttonRect, cornerSize);
    
    g.setColour(Colors::Button::outline);
    g.drawRoundedRectangle(buttonRect, cornerSize, 2.0f);
}

void ButtonLookAndFeel::drawButtonText(juce::Graphics &g, juce::TextButton &button,
                                       [[maybe_unused]]bool shouldDrawButtonAsHightlighted,
                                       bool shouldDrawButtonAsDown){
    auto bounds = button.getLocalBounds().toFloat();
    auto buttonRect = bounds.reduced(1.0f, 1.0f).withTrimmedBottom(1.0f);
    
    // Shift buttonRect down when button is pressed
    if(shouldDrawButtonAsDown)
        buttonRect.translate(0.0f, 1.0f);
    
    if(button.getToggleState())
        g.setColour(button.findColour(juce::TextButton::textColourOnId));
    else
        g.setColour(button.findColour(juce::TextButton::textColourOffId));
    
    g.setFont(Fonts::getFont());
    g.drawText(button.getButtonText(), buttonRect, juce::Justification::centred);
}
//==============================================================================
/*  Styling the Editor   */
MainLookAndFeel::MainLookAndFeel(){
    setColour(juce::GroupComponent::textColourId, Colors::Group::label);
    setColour(juce::GroupComponent::outlineColourId, Colors::Group::outline);
}

juce::Font MainLookAndFeel::getLabelFont([[maybe_unused]]juce::Label &label){
    return Fonts::getFont();
}
