/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PitchShifterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor&);
    ~PitchShifterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    std::unique_ptr<juce::Slider> delaySlider;
    std::unique_ptr<juce::Slider> fbSlider;
    std::unique_ptr<Label> label;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> delayAttachment;
    std::unique_ptr<Attachment> fbAttachment;
    

    PitchShifterAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessorEditor)
};
