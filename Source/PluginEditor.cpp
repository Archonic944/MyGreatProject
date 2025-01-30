/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MyGreatProjectAudioProcessorEditor::MyGreatProjectAudioProcessorEditor (MyGreatProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

MyGreatProjectAudioProcessorEditor::~MyGreatProjectAudioProcessorEditor()
{
}

//==============================================================================
void MyGreatProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    String str = "";
    for (const bool val : audioProcessor.tests) {
        str = str + String(static_cast<int>(val)) + " ";
    }

    String msg = "Tests: " + str;
    for (std::string str : audioProcessor.messages) {
        msg += "\n" + String(str);
    }
    g.drawFittedText (msg, getLocalBounds(), juce::Justification::centred, 4);
}

void MyGreatProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

int MyGreatProjectAudioProcessorEditor::getMagicNumber() {
    return 1;
}
