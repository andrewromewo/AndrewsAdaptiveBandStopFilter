/*
  ==============================================================================
    
  MUSIC 320C - ASSIGNMENT 1 STARTER CODE
  VISUALIZER.H
  SPRING 2022

  COPYRIGHT (C) 2022 CENTER FOR COMPUTER RESEARCH IN MUSIC AND ACOUSTICS

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>

// Visualizer is a custom class that can be used to plot x vs data
class Visualizer : public foleys::MagicPlotSource
{
public:
  //==============================================================================
  // Constructor requires pointer to data and data array size
  Visualizer(std::vector<float>& dataR, std::vector<float>& freqAxisR); // data to plot
  ~Visualizer() {}; // Nothing to deallocate
  void prepareToPlay (double samplingRate, int maxSamplesPerBlock) override; // we save the sampling rate for freq displays
  void pushSamples (const juce::AudioBuffer<float>& buffer) override {};  // NOT USED - We use a shared array instead of this copy approach
  void updatePlot(); // tells us data has changed and a new plot is needed
  //==============================================================================
  // MAIN THING TO WRITE: Plot drawing is done in createPlotPaths:
  void createPlotPaths (juce::Path& path, juce::Path& filledPath, juce::Rectangle<float> bounds, foleys::MagicPlotComponent& component) override;
    //My functions:
  void set_db_offset (float offset_input);
private:
  //==============================================================================
  // Private members point to data arrays and provide data array size
  std::vector<float>& data;
  std::vector<float>& freqAxis;
  double sampleRate;
  float off_db_val;

    
  juce::Label xAxisLabel;
  juce::Label yAxisLabel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualizer)
};
