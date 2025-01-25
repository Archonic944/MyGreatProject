/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MyGreatProjectAudioProcessor::MyGreatProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    runTests();
}

MyGreatProjectAudioProcessor::~MyGreatProjectAudioProcessor()
{
    output = false; //on destroy, mute
}

//==============================================================================
const juce::String MyGreatProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MyGreatProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MyGreatProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MyGreatProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MyGreatProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MyGreatProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MyGreatProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MyGreatProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MyGreatProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void MyGreatProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MyGreatProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MyGreatProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MyGreatProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MyGreatProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);


    }

    if (!output) buffer.applyGain(0.0); //mute if we failed any tests
}



//==============================================================================
bool MyGreatProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MyGreatProjectAudioProcessor::createEditor()
{
    return new MyGreatProjectAudioProcessorEditor (*this);
}

//==============================================================================
void MyGreatProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MyGreatProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void MyGreatProjectAudioProcessor::setDelayLength(float length) {
    this ->length = length;
}

void MyGreatProjectAudioProcessor::setDelayFeedback(float feedback) {
    feedback = std::clamp(feedback, 0.0f, 1.0f);
    this->feedback = feedback;
}

void MyGreatProjectAudioProcessor::runTests() {
    int currentLength = length;
    //first test: can set delay length
    setDelayLength(2.0);
    if (std::abs(length - 2.0) <= 0.0001) {
        setDelayLength(MAX_LENGTH + 1);
        if (length == MAX_LENGTH) TESTS_SUCCEEDED++;
    }
    setDelayLength(length); //reset delay length


    //second test: can set feedback amount, check feedback amount
    int currentFeedback = feedback;
    setDelayFeedback(0.4);
    if (std::abs(feedback - 0.4) < 0.0001) {
        setDelayFeedback(1.2); //should clamp
        if (feedback <= 1.0) TESTS_SUCCEEDED++;
    }
    setDelayFeedback(currentFeedback);

    if (TESTS_SUCCEEDED < TESTS_NUM) {
        output = false; //mutes the plugin
    }
}

int MyGreatProjectAudioProcessor::getMagicNumber() {
    return 2;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MyGreatProjectAudioProcessor();
}
