/*
  ==============================================================================
    
  MUSIC 320C - ASSIGNMENT 1 STARTER CODE
  VISUALIZER.CPP
  SPRING 2022 

  COPYRIGHT (C) 2022 CENTER FOR COMPUTER RESEARCH IN MUSIC AND ACOUSTICS

  ==============================================================================
*/
#include "Visualizer.h"

// Constructor requires pointer to data array and data array size
Visualizer::Visualizer(std::vector<float>& dataIn, std::vector<float>& freqAxisIn)
  : data(dataIn), freqAxis(freqAxisIn)
{
    off_db_val = 0;
}

// Call this whenever the plot needs to change
void Visualizer::updatePlot()
{
  resetLastDataFlag(); // Calls the resetLastDataFlag() in the parent which triggers a plot update next paint()
}

// Compare to foleys::MagicAnalyser::indexToX
static double binNumberToX (double bin, int nSpec, juce::Rectangle<float> bounds)
{
  float x = juce::jmap ( std::log10(float(bin)+0.00001f),
                        /* source range min */ 0.0f, /* source range max */ std::log10(float(nSpec-1)),
                        /* target range min */ bounds.getX(), /* target range max */ bounds.getRight());
  return x;
}

// Cf. foleys::MagicAnalyser::binToY
static float magnitudeToY (float mag, juce::Rectangle<float> bounds, float minDB = -80.0f, float off_val = 0)
{
  float y = juce::jmap<float> (/* value in dB */ juce::Decibels::gainToDecibels (mag, minDB) + off_val,
                        /* source range min */ minDB, /* source range max */ 10.0f,
                        /* target range min */ bounds.getBottom(), /* target range max */ bounds.getY());
  return y;
}

void Visualizer::prepareToPlay (double sampleRateIn, int maxSamplesPerBlock) {
  sampleRate = sampleRateIn; // needed for frequency axes and labels
}

void Visualizer::set_db_offset (float offset_input) {
    off_db_val = offset_input; // needed for frequency axes and labels
//    std::cout<<off_db_val<<std::endl;
}

// createPlotPaths does the actual drawing for juce::Component::paint().  It looks a lot like PostScript.
void Visualizer::createPlotPaths(juce::Path &path, juce::Path &filledPath, juce::Rectangle<float> bounds,
                                 foleys::MagicPlotComponent &component)
{
  path.clear();
  int N = int(data.size());
  path.preallocateSpace (8 + N * 3);
    
  // Juice Assert: In debug build, can check variables and raise custom errors.
  jassert(int(freqAxis.size()) == N);
  jassert(data[0]>=0);
  path.startNewSubPath (binNumberToX(freqAxis[0], N, bounds),
                        magnitudeToY(data[0], bounds, -80.0f, off_db_val));
  for (int i = 1; i < N; i++) {
    double d = data[i];
    jassert(d>=0.0); // we expect magnitude data, not dB or amplitude
      
      // bin num refers to FFT bin
    double x = binNumberToX(freqAxis[i], N, bounds);
    double y = magnitudeToY(d, bounds, -80.0f, off_db_val);
    path.lineTo (x, y);
  }
  filledPath = path;
  filledPath.lineTo (bounds.getBottomRight());
  filledPath.lineTo (bounds.getBottomLeft());
  filledPath.closeSubPath();
}
