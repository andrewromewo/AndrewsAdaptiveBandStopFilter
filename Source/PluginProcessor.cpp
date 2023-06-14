/*
  ==============================================================================
    
  MUSIC 320C - ASSIGNMENT 1 STARTER CODE
  PLUGINPROCESSOR.CPP
  SPRING 2022

  COPYRIGHT (C) 2022 CENTER FOR COMPUTER RESEARCH IN MUSIC AND ACOUSTICS

  ==============================================================================
*/

#include "PluginProcessor.h"

//==============================================================================
// Create some static parameter IDs here
static juce::Identifier fftID {"fftPlot"};
static juce::Identifier fftID_thespot {"fftPlot_thespot"};
static juce::String smoothTime("smoothTime");
static juce::String offset("offset");
static juce::String cv("CentralValue");
static juce::String Freq("Freq");

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout AndrewsAdaptiveFilter::createParameterLayout()
{
  juce::AudioProcessorValueTreeState::ParameterLayout layout;
  // Add your parameters here to your layout...
  layout.add(std::make_unique<juce::AudioParameterFloat>(smoothTime, "Smooth Time (ms)", juce::NormalisableRange<float>(0,500,1), 500));
  layout.add(std::make_unique<juce::AudioParameterFloat>(offset, "Offset (dB)", juce::NormalisableRange<float>(-80,50,0.01), -30));
  layout.add(std::make_unique<juce::AudioParameterFloat>(cv, "Test", juce::NormalisableRange<float>(0,22050,1), 0));
  layout.add(std::make_unique<juce::AudioParameterFloat>(Freq, "Select Frequency (Hz)", juce::NormalisableRange(/* min */ 20.0f, /* max */ 4000.0f, /* step */ 1.0f), /* default */ 1000.0f));

  return layout;
}

//==============================================================================
AndrewsAdaptiveFilter::AndrewsAdaptiveFilter()
  : // Initializations:

#ifndef JucePlugin_PreferredChannelConfigurations
  MagicProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                  ),
#endif
  treeState(*this, nullptr, JucePlugin_Name, createParameterLayout())
  , fftData(NSPEC)
  , fftData_thespot(NSPEC)
  , frequencies(NSPEC),
    forwardFFT(FFT_ORDER),
    window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann)
{ // Constructor body:
  jassert(fftData.size() >= NSPEC);
  jassert(frequencies.size() >= NSPEC);
  for (int i = 0; i < NSPEC; ++i) {
    fftData[i] = 0;
    fftData_thespot[i] = 0;
    frequencies[i] = i; // INITIALIZE frequency axis
  }
  juce::zeromem(oldFftData,sizeof(oldFftData));
  // Hooking up our FFT plot:
  // Tell our parent to create an instance of Visualizer
  // and add fftID to the list of available plot sources in the PGM Editor:
  fftPlot = magicState.createAndAddObject<Visualizer>(fftID, fftData, frequencies);
  fftPlot_thespot = magicState.createAndAddObject<Visualizer>(fftID_thespot, fftData_thespot, frequencies);
  // Attach your parameters to your listeners here...
  treeState.addParameterListener(smoothTime, this);
  treeState.addParameterListener(offset, this);
    
    CentralVal = treeState.getParameterAsValue(cv);
    CentralVal = 0;
    
    CompiledFaustP = std::make_unique<CompiledFaust>();
    CompiledFaustUIP = std::make_unique<MapUI>();
    CompiledFaustP->buildUserInterface(CompiledFaustUIP.get()); // load UI with the DSP parameter pointers
    
  magicState.setGuiValueTree (BinaryData::layout_3_xml, BinaryData::layout_3_xmlSize);
}

AndrewsAdaptiveFilter::~AndrewsAdaptiveFilter()
{
}

//==============================================================================
const juce::String AndrewsAdaptiveFilter::getName() const
{
  return JucePlugin_Name;
}

bool AndrewsAdaptiveFilter::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool AndrewsAdaptiveFilter::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool AndrewsAdaptiveFilter::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double AndrewsAdaptiveFilter::getTailLengthSeconds() const
{
  return 0.0;
}

int AndrewsAdaptiveFilter::getNumPrograms()
{
  return 1;   // NB: some hosts don't cope very well if you tgell them there are 0 programs,
  // so this should be at least 1, even if you're not really implementing programs.
}

int AndrewsAdaptiveFilter::getCurrentProgram()
{
  return 0;
}

void AndrewsAdaptiveFilter::setCurrentProgram (int index)
{
}

const juce::String AndrewsAdaptiveFilter::getProgramName (int index)
{
  return {};
}

void AndrewsAdaptiveFilter::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AndrewsAdaptiveFilter::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    fftPlot->prepareToPlay(sampleRate, samplesPerBlock);
    // Give our magicState the required sampling rate and samples per block
    magicState.prepareToPlay(sampleRate, samplesPerBlock);

  
    // Might want to init FFT to zero
    fs = sampleRate;
    
    CompiledFaustP->init(sampleRate);
    CompiledFaustUIP->setParamValue("Freq", 200);
    
    leaky = std::exp(-(FFT_SIZE) / (400 * 0.001 * fs));
    
    thePRE = 50;
    thePOST = 50;
    
    preparedToPlay = true;
}

void AndrewsAdaptiveFilter::releaseResources()
{
  // When playback stops, free anything you allocated in prepareToPlay().
  preparedToPlay = false;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AndrewsAdaptiveFilter::isBusesLayoutSupported (const BusesLayout& layouts) const
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

// "Meat" of the function
void AndrewsAdaptiveFilter::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (not preparedToPlay)
        return;

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    jassert(fftData.size() >= NSPEC);

    const float* channelData = buffer.getReadPointer(0);
    
    for (int samp = 0; samp<buffer.getNumSamples();samp++)
    {
        fifo[fifoIndex] = channelData[samp];
        fifoIndex++;
        
        if (fifoIndex == FFT_SIZE)
        {
            juce::zeromem(newFftData,sizeof(newFftData));
            
            memcpy(newFftData, fifo, sizeof(fifo));
            
            fifoIndex = 0;
            
            window.multiplyWithWindowingTable (newFftData, FFT_SIZE);
            
            forwardFFT.performFrequencyOnlyForwardTransform(newFftData);
            
            for (int i = 0; i < FFT_SIZE / 2; i++)
            {
                fftData[i] = newFftData[i] * (1 - leaky) + oldFftData[i] * leaky;
                oldFftData[i] = fftData[i];
                if (i > int(max_val)) {
                    fftData_thespot[i] = fftData[i];
                } else {
                    fftData_thespot[i] = 0;
                }
            }
            
//            std::cout<<max_val<<std::endl;
            
            fftPlot->updatePlot(); // This line will update the plot
            fftPlot_thespot->updatePlot(); // This line will update the plot
        }
    }
    
    // Step size between bins in Hz
    double binSize = fs / fftData.size();
    
    max_val = 1;
    float weighty = 0;
    

    for (std::size_t i = 0; i < fftData.size(); ++i)
    {
        double magnitude = fftData[i];
        
        if (magnitude*i > weighty) {
            max_val = i;
            weighty = magnitude*i;
        }
    }
    
    jassert(buffer.getNumChannels() == 2);
    jassert(CompiledFaustP->getNumInputs() == 2);
    jassert(CompiledFaustP->getNumOutputs() == 2);

    float*const* writePointers = buffer.getArrayOfWritePointers();
    int nChannelsFaust = CompiledFaustP->getNumOutputs();
    jassert(nChannelsFaust == CompiledFaustP->getNumInputs()); // sanity check
    float* bufferPointersFaust[nChannelsFaust];
    for (int i=0; i<nChannelsFaust; i++)
        bufferPointersFaust[i] = writePointers[i];
    CompiledFaustP->compute(buffer.getNumSamples(), bufferPointersFaust, bufferPointersFaust);

    float frequency = max_val*binSize;
    if (frequency > 1500) {
        frequency = 1500;
//        std::cout<<max_val<<std::endl;
        max_val = frequency/binSize;
    }
    CentralVal = frequency;
    CompiledFaustUIP->setParamValue("Freq",frequency - (frequency*0.25));

}

//==============================================================================
// Handle parameter updates here
void AndrewsAdaptiveFilter::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (parameterID == smoothTime)
    {
        leaky = newValue < 0.1 ? 0.0 : std::exp(-(FFT_SIZE) / (newValue * 0.001 * fs));
    }
    if (parameterID == offset)
    {
        off_val = newValue;
        fftPlot->set_db_offset(off_val);
        fftPlot_thespot->set_db_offset(off_val);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new AndrewsAdaptiveFilter();
}
