/*
 * Copyright (c) 2020 Andreas Pohl
 * Licensed under MIT (https://github.com/apohl79/audiogridder/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#pragma once

#include <JuceHeader.h>
#include <set>

#include "Client.hpp"
#include "NumberConversion.hpp"
#include "Utils.hpp"

class AudioGridderAudioProcessor : public AudioProcessor, public e47::LogTagDelegate {
  public:
    AudioGridderAudioProcessor();
    ~AudioGridderAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(AudioBuffer<float>& buf, MidiBuffer& midi) override { processBlockReal(buf, midi); }
    void processBlock(AudioBuffer<double>& buf, MidiBuffer& midi) override { processBlockReal(buf, midi); }
    void processBlockBypassed(AudioBuffer<float>& buf, MidiBuffer& midi) override;
    void processBlockBypassed(AudioBuffer<double>& buf, MidiBuffer& midi) override;

    template <typename T>
    void processBlockReal(AudioBuffer<T>&, MidiBuffer&);

    void updateLatency(int samples);

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;
    StringArray getAlternateDisplayNames() const override { return {"AuGrid", "AuGr", "AG"}; }

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    bool supportsDoublePrecisionProcessing() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String& newName) override;

    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void saveConfig(int numOfBuffers = -1);

    e47::Client& getClient() { return *m_client; }
    std::vector<ServerPlugin> getPlugins(const String& type) const;
    const std::vector<ServerPlugin>& getPlugins() const { return m_client->getPlugins(); }
    std::set<String> getPluginTypes() const;

    struct LoadedPlugin {
        String id;
        String name;
        String settings;
        StringArray presets;
        Array<e47::Client::Parameter> params;
        bool bypassed;
        bool ok;
    };

    auto& getLoadedPlugins() const { return m_loadedPlugins; }
    LoadedPlugin& getLoadedPlugin(int idx) {
        return idx > -1 ? m_loadedPlugins[e47::as<size_t>(idx)] : m_unusedDummyPlugin;
    }
    bool loadPlugin(const String& id, const String& name);
    void unloadPlugin(int idx);
    void editPlugin(int idx);
    void hidePlugin(bool updateServer = true);
    int getActivePlugin() const { return m_activePlugin; }
    bool isBypassed(int idx);
    void bypassPlugin(int idx);
    void unbypassPlugin(int idx);
    void exchangePlugins(int idxA, int idxB);
    bool enableParamAutomation(int idx, int paramIdx, int slot = -1);
    void disableParamAutomation(int idx, int paramIdx);
    void getAllParameterValues(int idx);
    void increaseSCArea();
    void decreaseSCArea();

    const int SCAREA_STEPS = 30;

    void storeSettingsA();
    void storeSettingsB();
    void restoreSettingsA();
    void restoreSettingsB();
    void resetSettingsAB();

    bool getMenuShowCategory() const { return m_menuShowCategory; }
    void setMenuShowCategory(bool b) { m_menuShowCategory = b; }
    bool getMenuShowCompany() const { return m_menuShowCompany; }
    void setMenuShowCompany(bool b) { m_menuShowCompany = b; }
    bool getGenericEditor() const { return m_genericEditor; }
    void setGenericEditor(bool b) { m_genericEditor = b; }

    auto& getServers() const { return m_servers; }
    void addServer(const String& s) { m_servers.add(s); }
    void delServer(const String& s);
    String getActiveServerHost() const { return m_client->getServerHostAndID(); }
    String getActiveServerName() const;
    void setActiveServer(const e47::ServerString& s);
    Array<e47::ServerString> getServersMDNS();

    int getLatencyMillis() const {
        return e47::as<int>(lround(m_client->NUM_OF_BUFFERS * getBlockSize() * 1000 / getSampleRate()));
    }

    // It looks like most hosts do not support dynamic parameter creation or changes to existing parameters. Logic at
    // least allows for the name to be updated. So we create slots at the start.
    class Parameter : public AudioProcessorParameter {
      public:
        Parameter(AudioGridderAudioProcessor& processor, int slot) : m_processor(processor), m_slotId(slot) {}
        float getValue() const override;
        void setValue(float newValue) override;
        float getValueForText(const String& /* text */) const override { return 0; }
        float getDefaultValue() const override { return getParam().defaultValue; }
        String getName(int maximumStringLength) const override;
        String getLabel() const override { return getParam().label; }
        int getNumSteps() const override { return getParam().numSteps; }
        bool isDiscrete() const override { return getParam().isDiscrete; }
        bool isBoolean() const override { return getParam().isBoolean; }
        bool isOrientationInverted() const override { return getParam().isOrientInv; }
        bool isMetaParameter() const override { return getParam().isMeta; }

      private:
        friend AudioGridderAudioProcessor;
        AudioGridderAudioProcessor& m_processor;
        int m_idx = -1;
        int m_paramIdx = 0;
        int m_slotId = 0;

        const LoadedPlugin& getPlugin() const { return m_processor.getLoadedPlugin(m_idx); }
        const e47::Client::Parameter& getParam() const { return getPlugin().params.getReference(m_paramIdx); }

        void reset() {
            m_idx = -1;
            m_paramIdx = 0;
        }
    };

  private:
    Uuid m_instId;
    std::unique_ptr<e47::Client> m_client;
    std::vector<LoadedPlugin> m_loadedPlugins;
    int m_activePlugin = -1;
    StringArray m_servers;

    int m_numberOfAutomationSlots = 16;
    LoadedPlugin m_unusedDummyPlugin;
    e47::Client::Parameter m_unusedParam;

    Array<Array<float>> m_bypassBufferF;
    Array<Array<double>> m_bypassBufferD;
    std::mutex m_bypassBufferMtx;

    String m_settingsA, m_settingsB;

    bool m_menuShowCategory = true;
    bool m_menuShowCompany = true;
    bool m_genericEditor = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioGridderAudioProcessor)
};
