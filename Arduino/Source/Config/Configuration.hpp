#pragma once 

class Configuration {
public:
    Configuration()
    {
        LoadDefaultConfig();
        LoadFromFile();
    }

    ~Configuration()
    {
        SaveToFile();
    }

    void LoadDefaultConfig()
    {
        channelApps.clear();
        channelVolumes.clear();
        for (UINT i = 0; i < MAX_CABLES; ++i)
        {
            channelApps[i] = std::vector<std::string>{};
            channelVolumes[i] = 1.0f;
        }
    }

    bool LoadFromFile()
    {
        if (!std::filesystem::exists(CONFIG_FILE))
        {
            return false;
        }

        std::ifstream file(CONFIG_FILE);
        if (!file.is_open())
        {
            return false;
        }

        try
        {
            json config;
            file >> config;
            file.close();

            channelApps.clear();
            channelVolumes.clear();

            for (UINT i = 0; i < MAX_CABLES; ++i)
            {
                channelApps[i] = std::vector<std::string>{};
                channelVolumes[i] = 1.0f;
            }

            if (config.contains("channels"))
            {
                for (const auto& [channelStr, data] : config["channels"].items())
                {
                    UINT channel = std::stoi(channelStr);
                    if (channel >= MAX_CABLES) 
                        continue;

                    channelApps[channel] = data.value("apps", std::vector<std::string>{});
                    channelVolumes[channel] = data.value("volume", 1.0f);
                }
            }
            return true;
        }
        catch (const json::exception& e)
        {
            LoadDefaultConfig();
            return false;
        }
    }

    bool SaveToFile() const
    {
        try
        {
            json config;
            for (const auto& [channel, apps] : channelApps)
            {
                config["channels"][std::to_string(channel)]["apps"] = apps;
                config["channels"][std::to_string(channel)]["volume"] = channelVolumes.at(channel);
            }

            std::ofstream file(CONFIG_FILE);
            if (!file.is_open())
            {
                return false;
            }
            file << config.dump(4);
            file.close();
            return true;
        }
        catch (const json::exception& e)
        {
            return false;
        }
    }

    void AssignAppToChannel(UINT channel, const std::string& appName)
    {
        if (channel >= MAX_CABLES) return;
        auto& apps = channelApps[channel];
        if (std::find(apps.begin(), apps.end(), appName) == apps.end())
        {
            apps.push_back(appName);
            SaveToFile();
        }
    }

    void RemoveAppFromChannel(UINT channel, const std::string& appName)
    {
        if (channel >= MAX_CABLES) return;
        auto& apps = channelApps[channel];
        apps.erase(std::remove(apps.begin(), apps.end(), appName), apps.end());
        SaveToFile();
    }

    void SetChannelVolume(UINT channel, float volume)
    {
        if (channel >= MAX_CABLES || volume < 0.0f || volume > 1.0f) return;
        channelVolumes[channel] = volume;
        SaveToFile();
    }

    std::vector<std::string> GetChannelApps(UINT channel) const
    {
        if (channel >= MAX_CABLES) return {};
        auto it = channelApps.find(channel);
        return it != channelApps.end() ? it->second : std::vector<std::string>{};
    }

    float GetChannelVolume(UINT channel) const
    {
        if (channel >= MAX_CABLES) return 1.0f;
        auto it = channelVolumes.find(channel);
        return it != channelVolumes.end() ? it->second : 1.0f;
    }

    void SyncWithVirtualCableManager()
    {
        if (!g_VirtualCableManager)
        {
            return;
        }

        for (UINT i = 0; i < MAX_CABLES; ++i)
        {
            auto appMap = g_VirtualCableManager->GetAssignedAppNamesPerCable();
            auto it = appMap.find(i);
            channelApps[i].clear();
            if (it != appMap.end())
            {
                channelApps[i].insert(channelApps[i].end(), it->second.begin(), it->second.end());
            }

            float volume;
            if (SUCCEEDED(g_VirtualCableManager->GetCableVolume(i, &volume)))
            {
                channelVolumes[i] = volume;
            }
            else
            {
                channelVolumes[i] = 1.0f;
            }
        }
        SaveToFile();
    }
private:
    using json = nlohmann::json;
    std::map<UINT, std::vector<std::string>> channelApps;
    std::map<UINT, float> channelVolumes;
    static constexpr const char* CONFIG_FILE = "default_config.json";
};

inline Configuration* g_Configuration;