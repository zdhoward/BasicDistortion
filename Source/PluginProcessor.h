/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct ChainSettings {
    float drive{ 0 }, range{ 0 }, blend{ 0 }, gain{ 0 };
};

ChainSettings getChainSettings(AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class BasicDistortionAudioProcessor  : public foleys::MagicProcessor,
    private AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    BasicDistortionAudioProcessor();
    ~BasicDistortionAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    void parameterChanged(const String& param, float value);

    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    //juce::AudioProcessorEditor* createEditor() override;
    //bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    //void setStateInformation (const void* data, int sizeInBytes) override;

    static AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();

    AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicDistortionAudioProcessor)
};