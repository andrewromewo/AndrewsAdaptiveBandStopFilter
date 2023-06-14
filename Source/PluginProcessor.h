/*
  ==============================================================================
    
  MUSIC 320C - ASSIGNMENT 1 STARTER CODE
  PLUGINPROCESSOR.H
  SPRING 2022

  COPYRIGHT (C) 2022 CENTER FOR COMPUTER RESEARCH IN MUSIC AND ACOUSTICS

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
// Import our Visualizer code
#include "Visualizer.h"
#include "CompiledFaust.h"

// Macros for FFT size and order. Try changing these!
static const int FFT_ORDER = 11;
static const int FFT_SIZE = (1 << (FFT_ORDER));
static const int NSPEC = (FFT_SIZE >> 1);

//==============================================================================
/**
 */
class AndrewsAdaptiveFilter  :  public foleys::MagicProcessor,
                                       private juce::AudioProcessorValueTreeState::Listener  // Listener for parameters
{
public:
  //==============================================================================
  AndrewsAdaptiveFilter();
  ~AndrewsAdaptiveFilter() override;

  //==============================================================================
  void prepareToPlay (double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

  void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  //==============================================================================
  // PGM takes this over:
  //    juce::AudioProcessorEditor* createEditor() override;
  //    bool hasEditor() const override;

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
  // Inherited from juce::AudioProcessorValueTreeState::Listener
  void parameterChanged(const juce::String &parameterID, float newValue) override;

private:
  //==============================================================================
  // Parameter listener setup
  juce::AudioProcessorValueTreeState treeState;
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  // ADD YOUR MEMBER VARIABLES AND std::atomic<float>* PARAMETER-POINTERS HERE

    Visualizer* fftPlot = nullptr;   // This will create our FFT plot
    Visualizer* fftPlot_thespot = nullptr;   // This will create our FFT plot
    
    std::unique_ptr<CompiledFaust> CompiledFaustP;
    std::unique_ptr<MapUI> CompiledFaustUIP;
    
    CompiledFaust CompiledFaust_obj;
    MapUI CompiledFaustUI;
    
    std::size_t max_val;


  // FFT SETUP
  // Do your FFT setup here...
    float newFftData[FFT_SIZE * 2];
    float oldFftData[FFT_SIZE / 2];

  // Here are some buffers you can use:
    std::vector<float> fftData; // We use std::vector in case you want to support changing the FFT size
    std::vector<float> fftData_thespot;
    std::vector<float> frequencies;
    
    juce::dsp::FFT forwardFFT;
    juce::Image spectrogramImage;
    
    juce::dsp::WindowingFunction<float> window;
    
    float fifo [FFT_SIZE];                           // [6]
    int fifoIndex;

    
    bool nextFFTBlockReady = false;

    bool preparedToPlay = false;
    
    float leaky;
    
    float thePRE;
    float thePOST;
    
    double fs;
    float off_val;
    juce::Value CentralVal;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndrewsAdaptiveFilter)
};
