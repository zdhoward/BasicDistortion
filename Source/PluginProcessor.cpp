/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicDistortionAudioProcessor::BasicDistortionAudioProcessor()
     : foleys::MagicProcessor (AudioProcessor::BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
         apvts( *this, nullptr, "Parameters", createParameterLayout())
{
    FOLEYS_SET_SOURCE_PATH(__FILE__);



    magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);
}

BasicDistortionAudioProcessor::~BasicDistortionAudioProcessor()
{
}

//==============================================================================
const juce::String BasicDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicDistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicDistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicDistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicDistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicDistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicDistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicDistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicDistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    magicState.prepareToPlay(sampleRate, samplesPerBlock);
}

void BasicDistortionAudioProcessor::parameterChanged(const String& param, float value)
{

}

void BasicDistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void BasicDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    
    auto chainSettings = getChainSettings(apvts);
    
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto gain = Decibels::decibelsToGain(chainSettings.gain);

        // ..do something to the data...
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            float cleanSignal = *channelData;

            *channelData *= chainSettings.drive * chainSettings.range;

            *channelData = (((((2.f / float_Pi) * atan(*channelData)) * chainSettings.blend) + (cleanSignal * (1.f - chainSettings.blend))) / 2) * gain;

            channelData++;
        }

    }
}

//==============================================================================
/*
bool BasicDistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}
*/

/*
juce::AudioProcessorEditor* BasicDistortionAudioProcessor::createEditor()
{
    return new foleys::MagicPluginEditor(magicState);
    //return new GenericAudioProcessorEditor(this);
    //return new BasicDistortionAudioProcessorEditor (*this);
}
*/

//==============================================================================
void BasicDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void BasicDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
    }
}

ChainSettings getChainSettings(AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.drive = apvts.getRawParameterValue("Drive")->load(); 
    settings.range = apvts.getRawParameterValue("Range")->load();
    settings.blend = apvts.getRawParameterValue("Blend")->load();
    settings.gain = apvts.getRawParameterValue("Gain")->load();

    return settings;
}

AudioProcessorValueTreeState::ParameterLayout
BasicDistortionAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>( "Drive", 
                                                            "Drive", 
                                                            juce::NormalisableRange<float>(0.f, 1.f, 0.001f, 1.f), 
                                                            1.f,
                                                            juce::String(),
                                                            juce::AudioProcessorParameter::genericParameter,
                                                            [](float value, int maxStringLength) { return juce::String(value * 100, 0) << "%"; },
                                                            [](juce::String text) { return text.getFloatValue() / 100; }));

    layout.add(std::make_unique<juce::AudioParameterFloat>( "Range", 
                                                            "Range", 
                                                            juce::NormalisableRange<float>(0.f, 3000.f, 0.001f, 1.f), 
                                                            1000.f,
                                                            juce::String(),
                                                            juce::AudioProcessorParameter::genericParameter,
                                                            [](float value, int) { return (value < 1000) ?
                                                            juce::String(value, 1) + " Hz" :
                                                            juce::String(value / 1000.0, 1) + " kHz"; },
                                                            [](juce::String text) { return text.endsWith(" kHz") ?
                                                            text.getFloatValue() * 1000.0f :
                                                            text.getFloatValue(); }));

    layout.add(std::make_unique<juce::AudioParameterFloat>( "Blend", 
                                                            "Blend", 
                                                            juce::NormalisableRange<float>(0.f, 1.f, 0.001f, 1.f), 
                                                            1.f,
                                                            juce::String(),
                                                            juce::AudioProcessorParameter::genericParameter,
                                                            [](float value, int maxStringLength) { return juce::String(value * 100, 0) << "%"; },
                                                            [](juce::String text) { return text.getFloatValue() / 100; }));

    layout.add(std::make_unique<juce::AudioParameterFloat>( "Gain", 
                                                            "Gain", 
                                                            juce::NormalisableRange<float>(-12.f, 12.f, 0.5f, 1.f),
                                                            0.f,
                                                            juce::String(),
                                                            juce::AudioProcessorParameter::genericParameter,
                                                            [](float value, int maxStringLength) { return juce::String(value,1) << " dB"; },
                                                            [](juce::String text) { return text.getFloatValue(); }));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicDistortionAudioProcessor();
}
