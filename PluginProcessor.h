/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <rubberband/RubberBandStretcher.h>
#include <src/base/RingBuffer.h>

//==============================================================================
/**
*/
class PitchShifterAudioProcessor  : public AudioProcessor,
                                    public ValueTree::Listener
{
public:
    //==============================================================================
    PitchShifterAudioProcessor();
    ~PitchShifterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    
    void writeToRingBuffer(AudioBuffer<float>& buffer,
                            const int channelIn, const int channelOut,
                            const int writePos, float startGain, float endGain,
                            bool replacing);
    void readFromRingBuffer (AudioBuffer<float>& buffer,
                              const int channelIn, const int channelOut,
                              const int readPos, float startGain, float endGain,
                              bool replacing);

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Called when user changes parameters
    void update();
    RubberBand::RingBuffer<float> ringBuffer;
    // Store Parameters
    AudioProcessorValueTreeState apvts;
    AudioProcessorValueTreeState::ParameterLayout createParameters();

private:
    
    
    int writePos = 0;
    int readPos = 0;
    int expPos = 0;
    
    double second;
    bool isActive { false };
    bool mustUpdateProcessing { false };
    float lastFeedbackLevel = 0.0f;
    
    double mSampleRate;
    
    Atomic<float>   mGain           {   0.0f };
    Atomic<float>   delayTime       { 200.0f };
    Atomic<float>   feedbackLevel   {  0.0f };
    Atomic<float>   wetLevel        {   50.0f };
    
    void valueTreePropertyChanged(ValueTree& tree, const Identifier& property) override
    {
        mustUpdateProcessing = true;
    }
   
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessor)
};
