#include "config/DatabaseConfig.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
Json::Value loadApplicationConfiguration(
    const std::filesystem::path &configPath
)
{
    std::ifstream configFile(
        configPath,
        std::ios::binary
    );

    if (!configFile.is_open())
    {
        throw std::runtime_error(
            "Unable to open Drogon configuration file: " +
            configPath.string()
        );
    }

    Json::CharReaderBuilder readerBuilder;
    readerBuilder["collectComments"] = false;

    Json::Value configuration;
    std::string parsingErrors;

    if (!Json::parseFromStream(
            readerBuilder,
            configFile,
            &configuration,
            &parsingErrors
        ))
    {
        throw std::runtime_error(
            "Invalid Drogon configuration file: " +
            parsingErrors
        );
    }

    configuration["db_clients"] =
        healthcare::config::buildDatabaseClientsConfig();

    return configuration;
}
}

int main(int, char *argv[])
{
    try
    {
        const auto executableDirectory =
            std::filesystem::absolute(argv[0]).parent_path();

        const auto configPath =
            executableDirectory / "config.json";

        if (!std::filesystem::exists(configPath))
        {
            throw std::runtime_error(
                "Drogon configuration file was not found: " +
                configPath.string()
            );
        }

        auto configuration =
            loadApplicationConfiguration(configPath);

        drogon::app()
            .loadConfigJson(std::move(configuration))
            .run();
    }
    catch (const std::exception &exception)
    {
        std::cerr
            << "Failed to start Healthcare AI Agent: "
            << exception.what()
            << '\n';

        return 1;
    }

    return 0;
}