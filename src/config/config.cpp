#include "config.h"
#include "yaml_cpp_converts.h"
#include <filesystem>
#include <fstream>
#include "logger/logger.h"
#include "orbit/tle_manager.h"

std::shared_ptr<ConfigManager> configManager;

ConfigData getDefaultConfig()
{
    ConfigData config;

    config.station_name = "My Station";
    config.station = {0.0, 0.0, 0.0};

    SatelliteConfig noaa15Config = {(int)25338,
                                    (float)10.0f,
                                    (int)1,
                                    (std::vector<DownlinkConfig>){
                                        {(std::string) "APT",
                                         (long)137.620e6,
                                         (long)50e3,
                                         (bool)false,
                                         (std::string) "apt-noaa.lua",
                                         (std::string) "wav",
                                         (ModemType)ModemType::FM,
                                         11025}}};
    config.satelliteConfigs.push_back(noaa15Config);

    config.tle_update = "0 0 * * *";

    config.sdrConfig = {(long)137.500e6, (long)1e6, (int)0};

    config.dataDirectory = "data";

    return config;
}

void initConfig()
{
    logger->info("Initializing config...");

    std::string filename = "config.yml";

    if (std::filesystem::exists(filename))
    {
        logger->debug("Config found! Loading " + filename);
        configManager = std::make_shared<ConfigManager>(filename);
        configManager->loadConfigFile();
    }
    else
    {
        logger->debug("Config not found! Writing defaults to " + filename);
        configManager = std::make_shared<ConfigManager>(filename, getDefaultConfig());
        configManager->saveConfigFile();
    }
    logger->info("Done!");
}

ConfigManager::ConfigManager(std::string filename) : filename_m(filename)
{
}

ConfigManager::ConfigManager(std::string filename, ConfigData config) : filename_m(filename), config_m(config)
{
}

ConfigData &ConfigManager::getConfig()
{
    return config_m;
}

void ConfigManager::loadConfigFile()
{
    try
    {
        configFile = YAML::LoadFile(filename_m);
    }
    catch (YAML::Exception e)
    {
        logger->critical("Fatal error reading config! Aborting!");
        logger->critical(e.what());
        exit(1);
    }

    config_m.station_name = configFile["station_name"].as<std::string>();
    config_m.station = configFile["station"].as<SatelliteStation>();
    config_m.satelliteConfigs = configFile["satellites"].as<std::vector<SatelliteConfig>>();
    config_m.tle_update = configFile["tle_update"].as<std::string>();
    config_m.sdrConfig = configFile["rtlsdr"].as<SDRConfig>();
    config_m.dataDirectory = configFile["data_directory"].as<std::string>();
}

void ConfigManager::saveConfigFile()
{
    configFile["station_name"] = (std::string)config_m.station_name;
    configFile["station"] = (SatelliteStation)config_m.station;
    configFile["satellites"] = (std::vector<SatelliteConfig>)config_m.satelliteConfigs;
    configFile["tle_update"] = (std::string)config_m.tle_update;
    configFile["rtlsdr"] = (SDRConfig)config_m.sdrConfig;
    configFile["data_directory"] = (std::string)config_m.dataDirectory;

    std::ofstream outFile(filename_m);
    outFile << configFile << '\n';
}

SatelliteConfig ConfigData::getSatelliteConfigFromNORAD(int norad)
{
    std::vector<SatelliteConfig>::iterator value = std::find_if(satelliteConfigs.begin(), satelliteConfigs.end(), [&](const SatelliteConfig &c) { return c.norad == norad; });
    if (value != satelliteConfigs.end())
        return *value;
    else
        return SatelliteConfig();
}

std::string SatelliteConfig::getName()
{
    return getTLEFromNORAD(norad).name;
}