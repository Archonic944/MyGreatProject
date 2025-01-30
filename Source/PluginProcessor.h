/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

class MyGreatProjectAudioProcessor  : public juce::AudioProcessor
{


public:
    //==============================================================================
    MyGreatProjectAudioProcessor();
    ~MyGreatProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    void refreshDelayLen(unsigned long sampleRate);

    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    void pushToBuffer(const float *sampleBlock, float *blockOut, int blockLength, char side);

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

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
    void setStateInformation (const void* data, int sizeInBytes) override;

    void setDelayLength(float len);

    void setDelayFeedback(float feedback);

    void test(bool val, std::string message);

    void runTests();

    static juce::String array_to_string(float *arr, int len);

    //=========
    static int getMagicNumber();
    float feedback = 0.5; //from 0 to 1
    float length = 1.0;
    unsigned long delayLengthSmp = -1; //set in the prepareToPlay function
    std::vector<std::string> messages = {};

    bool output = true;
    //======
    std::vector<float> buffer_left;
    std::vector<float> buffer_right;
    //======
    std::vector<bool> tests;
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyGreatProjectAudioProcessor)
};
