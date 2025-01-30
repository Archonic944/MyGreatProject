/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "JuceHeader.h"
#include "PluginEditor.h"

#define MAX_DELAY_LENGTH 5
#define MAX_FEEDBACK 0.99f

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
    return MAX_DELAY_LENGTH;
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
    auto sampleRateInt = static_cast<unsigned long>(std::ceil(sampleRate));
    //Check if buffer needs to be reallocated | treating buffer_left and right as the same in this case
    unsigned long bufferLength = MAX_DELAY_LENGTH * sampleRateInt;
    if (buffer_left.size() != bufferLength) {
        buffer_left.resize(bufferLength, 0.0f);
        buffer_right.resize(bufferLength, 0.0f);
    }
    refreshDelayLen(sampleRateInt);
}

void MyGreatProjectAudioProcessor::refreshDelayLen(unsigned long sampleRate) {
    delayLengthSmp = sampleRate * length;
}

void MyGreatProjectAudioProcessor::releaseResources()
{
    buffer_left.resize(0);
    buffer_right.resize(0);
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
    for (int channel = 0; channel < 2; ++channel) //TODO take out forced stereo
    {
        float* channelData = buffer.getWritePointer (channel);
        const char side = (channel == 0 ? 'l' : 'r');
        const int samples = buffer.getNumSamples();
        float delayOut[samples];
        pushToBuffer(channelData, delayOut, samples, side);
        for (const float f : delayOut) {
            *channelData += f;
            channelData++;
        }
    }

    if (!output) buffer.applyGain(0.0); //mute if we failed any tests
}

//push to a delay line. side is 'l' or 'r'. takes feedback % into account by summing the discarded block with the end of the delay line. returns a block of equal length that was ejected from the array.
void MyGreatProjectAudioProcessor::pushToBuffer(const float *sampleBlock, float* blockOut, int blockLength,
                                                  const char side) {
    std::vector<float>& toPush = (side == 'l' ? buffer_left : buffer_right);
    std::vector<float> feedbackVec{};
    feedbackVec.reserve(blockLength);
    const float *sampleBlockIter = sampleBlock;
    for (int i = 0; i<blockLength; i++) {
        feedbackVec.push_back(*sampleBlockIter);
        sampleBlockIter++;
    }
    using namespace juce;
    FloatVectorOperations::multiply(feedbackVec.data(), sampleBlock, feedback, blockLength);
    assert(blockLength <= toPush.size());
    auto iterator = toPush.erase(toPush.begin(), toPush.begin() + blockLength); //take the beginning of the buffer out
    for (int j = 0; j<blockLength; j++) {
        //instead of pushing to the back of the vector, we'll push at delay_length - block length
        toPush.insert(toPush.begin() + (delayLengthSmp - blockLength + j), feedbackVec[j]); // NOLINT(*-narrowing-conversions)
        *blockOut = *(iterator++);
        blockOut++;
    } //insert our feedback-modified sample block
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
    if (length > MAX_DELAY_LENGTH) this->length = MAX_DELAY_LENGTH;
    else this->length = length;
}

void MyGreatProjectAudioProcessor::setDelayFeedback(float feedback) {
    this->feedback = std::clamp(feedback, 0.0f, MAX_FEEDBACK);
}

void MyGreatProjectAudioProcessor::test(bool val, std::string message) {
    //my very intricate testing system
    tests.push_back(val);
    if (!val) output = false;
    if (!message.empty()) messages.push_back(message);
}

void MyGreatProjectAudioProcessor::runTests() {
    try {
        const float currentLength = length;
        //first test: delay length: a. can be set; b. is capped properly
        setDelayLength(2.0);
        test(std::abs(length - 2.0) <= 0.0001, "");
        setDelayLength(MAX_DELAY_LENGTH + 1);
        test(length == MAX_DELAY_LENGTH, "");

        //second test: can set feedback amount
        const float currentFeedback = feedback;
        setDelayFeedback(0.4);
        test(std::abs(feedback - 0.4) < 0.0001, "");
        //third test: feedback amount clamps correctly
        setDelayFeedback(1.2); //should clamp
        test(feedback <= MAX_FEEDBACK, "");
        //fourth test: buffer allocates correctly
        int blockSize = 10;
        this->prepareToPlay(100, blockSize);
        test(buffer_left.size() == 100 * MAX_DELAY_LENGTH, ""); // NOLINT(*-narrowing-conversions)
        //fifth test: can push a block of samples to our hypothetical vector, multiple times, and receive the correct value
        delayLengthSmp = 50; //we only use the first half of the array. this value is reset on prepareToPlay so it's fine to modify
        setDelayFeedback(0.5);
        //now we test the echo effect.
        using namespace juce;
        std::vector input = {0.25f, 0.5f, 1.0f, 0.25f, 0.5f, 1.0f, 0.25f, 0.5f, 1.0f, 0.5f}; //dummy "audio"
        std::vector expected = input;
        FloatVectorOperations::multiply(expected.data(), feedback, input.size());
        float finalOutput[blockSize];
        pushToBuffer(input.data(), finalOutput, 10, 'l'); //push initial values
        //that expected data should come back after 5 blocks
        for (int i = 0; i<(delayLengthSmp/blockSize - 1); i++) {
            pushToBuffer(std::vector<float>(10).data(), finalOutput, blockSize, 'l'); //push 4 blocks of zeroes
        }
        bool equal = true;
        //check they're equal
        for (int i = 0; i<blockSize; i++) {
            if (!(finalOutput[i] == expected[i])) {
                equal = false;
                break;
            }
        }
        test(equal, (equal ? "" :
            "Output: " + array_to_string(finalOutput, blockSize) + "\n" + array_to_string(expected.data(), blockSize)).toStdString());
        //reset values back to default
        setDelayFeedback(currentFeedback);
        setDelayLength(currentLength);
    } catch (...) {
        std::cout << "An assertion failed during test runs. Please check the console." << std::endl;
        test(false, ""); //make sure we mute output
    }
}

juce::String MyGreatProjectAudioProcessor::array_to_string(float* arr, int len) {
    juce::String str = "";
    for (int i = 0; i<len; i++) {
        str += juce::String(*(arr++));
    }
    return str;
}

int MyGreatProjectAudioProcessor::getMagicNumber() {
    return 2;
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MyGreatProjectAudioProcessor();
}
