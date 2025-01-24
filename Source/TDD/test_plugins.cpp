#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "../PluginEditor.h"
#include "../PluginProcessor.h"

TEST_CASE("Test plugin class linking with test class"){
  int m = MyGreatProjectAudioProcessorEditor::getMagicNumber();
  int m2 = MyGreatProjectAudioProcessor::getMagicNumber();
  CHECK(m == 1);
  CHECK(m2 == 2);
}