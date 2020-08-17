/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PitchShifterAudioProcessorEditor::PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    delaySlider = std::make_unique<Slider>(Slider::SliderStyle::RotaryVerticalDrag, Slider::TextBoxBelow);
    delaySlider->setBounds(100, 100, 100, 100);
    addAndMakeVisible (delaySlider.get());
    
    fbSlider = std::make_unique<Slider>(Slider::SliderStyle::RotaryVerticalDrag, Slider::TextBoxBelow);
    fbSlider->setBounds(250, 100, 100, 100);
    addAndMakeVisible (fbSlider.get());
    
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    delayAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "Time", *delaySlider);
    fbAttachment = std::make_unique<Attachment>(audioProcessor.apvts, "FB", *fbSlider);
 
    setSize (400, 300);
}

PitchShifterAudioProcessorEditor::~PitchShifterAudioProcessorEditor()
{
}

//==============================================================================
void PitchShifterAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto black = juce::Colours::black;
    auto bounds = getLocalBounds().toFloat();
    Point<float> centre(bounds.getCentre().toFloat());
    /* dummy value because gradient is radial */
    Point<float> right(bounds.getTopRight().toFloat());
    
    g.setColour(black);
    juce::ColourGradient fillGradient(black.brighter(), centre, black, right, true);
    g.setGradientFill(fillGradient);
    g.fillAll();
}

void PitchShifterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
