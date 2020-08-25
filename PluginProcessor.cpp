/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PitchShifterAudioProcessor::PitchShifterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParameters()),  delayBuffer(2, 96000)
#endif
{
    apvts.state.addListener (this);
}

PitchShifterAudioProcessor::~PitchShifterAudioProcessor()
{
}

//==============================================================================
const juce::String PitchShifterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PitchShifterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PitchShifterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PitchShifterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PitchShifterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PitchShifterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PitchShifterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PitchShifterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PitchShifterAudioProcessor::getProgramName (int index)
{
    return {};
}

void PitchShifterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PitchShifterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mSampleRate = sampleRate;
    update();
    
}

void PitchShifterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PitchShifterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PitchShifterAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    if (mustUpdateProcessing)
        update();

    
    // const for the life of this process block
    const float feedback = feedbackLevel.get();
    auto delaySize = delayBuffer.getNumSamples();
    
    //DBG(lastFeedbackLevel);
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    writeToRingBuffer(buffer, 0, 0, writePos, 1.0, 1.0, true);
    
    readPos = roundToInt (writePos - (mSampleRate * delayTime.get() / 1000.0));
                if (readPos < 0)
                    readPos += delaySize;
    readFromRingBuffer(buffer, 0, 0, readPos, 1.0, 1.0, true);
//    for (int j = 0; j < buffer.getNumSamples(); ++j)
//    {
//        if (mustUpdateProcessing)
//            update();
//        auto* writePtr = buffer.getWritePointer(0);
//        auto* readPtr = delayBuffer.getReadPointer(0);
//        readPos = roundToInt (writePos - (mSampleRate * delayTime.get() / 1000.0));
//        if (readPos < 0)
//            readPos += delaySize;
//        /* retrieve sample from ringBuffer */
//        auto sample = readPtr[readPos];
//        /* write to and fill the audio buffer (output) at each index */
//        writePtr[j] += sample;
//        writePos += 1;
//
//        readPos = readPos % delaySize;
//    }
    

    // feedback
    writeToRingBuffer (buffer, 0, 0, writePos, lastFeedbackLevel, feedback, false);
    lastFeedbackLevel = feedback;
    
    writePos += buffer.getNumSamples();
    writePos = writePos % delaySize;
    
    DBG(writePos);

}


void PitchShifterAudioProcessor::writeToRingBuffer(AudioBuffer<float>& buffer, const int channelIn, const int channelOut,
                      const int mWritePos, float startGain, float endGain, bool replacing)
{
    auto buffSize = buffer.getNumSamples();
    auto delaySize = delayBuffer.getNumSamples();
    
    if (mWritePos + buffSize <= delaySize)
    {
        delayBuffer.copyFromWithRamp(channelOut, mWritePos, buffer.getReadPointer(channelIn), buffSize, startGain, endGain);
        
    }
    /*-------------------------------------------------------*/
    else
    {
        const auto midPos  = delaySize - mWritePos;
        
        const auto midGain = jmap (float (midPos) / buffSize, startGain, endGain);
        delayBuffer.copyFromWithRamp(channelOut, mWritePos, buffer.getReadPointer(channelIn), midPos, startGain, midGain);
        delayBuffer.copyFromWithRamp(channelOut, 0, buffer.getReadPointer(channelIn, midPos), buffSize - midPos, midGain, endGain);
        
    }
    
   
}
void PitchShifterAudioProcessor::readFromRingBuffer(AudioBuffer<float>& buffer,
                                                   const int channelIn, const int channelOut,
                                                   const int mReadPos, float startGain, float endGain,
                                                   bool pitchChange)
{
    auto buffSize = buffer.getNumSamples();
    auto delaySize = delayBuffer.getNumSamples();
    auto* writePtr = buffer.getWritePointer(channelOut);
    auto* readPtr = delayBuffer.getReadPointer(channelIn);
    float readFloat = readPos; // allow us to add and store float increments, then round later

    for(int i = 0; i < buffSize; ++i)
    {
        /* retrieve sample from ringBuffer */
        auto sample = readPtr[readPos];
        /* write to and fill the audio buffer (output) at each index */
        writePtr[i] += sample;

        //DBG(readPos);
        readPos += 1;
        readPos = readPos % delaySize;

    }


}

//void PitchShifterAudioProcessor::rectifyPos(float dSamples, float expSamples)
//{
//    if (dSamples == 0)
//        dSamples = -1;
//
//    skipRatio = expSamples / dSamples;
//
//}

//==============================================================================
bool PitchShifterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PitchShifterAudioProcessor::createEditor()
{
    return new PitchShifterAudioProcessorEditor (*this);
}

//==============================================================================
void PitchShifterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PitchShifterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PitchShifterAudioProcessor();
}

void PitchShifterAudioProcessor::update()
{
    
    mustUpdateProcessing = false;
    auto time = apvts.getRawParameterValue ("Time");
    auto fb = apvts.getRawParameterValue("FB");
    
    
    
    using mult = juce::ValueSmoothingTypes::Multiplicative;
    using lin = juce::ValueSmoothingTypes::Linear;
    
    juce::SmoothedValue<float, lin> delay;
    delay.setTargetValue(*time);
    
    juce::SmoothedValue<float, lin> feedback;
    feedback.setTargetValue(*fb);

    
    delayTime = delay.getNextValue();
    feedbackLevel = feedback.getNextValue();

    
}

juce::AudioProcessorValueTreeState::ParameterLayout PitchShifterAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Time", "Delay Time", NormalisableRange<float> (0.0f, 2000.f, 0.1f, 1.0f), 200.f));

    parameters.push_back (std::make_unique<AudioParameterFloat>("FB", "Feedback",
                                                                NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f), 0.0f));
   
    
    return { parameters.begin(), parameters.end() };
}
