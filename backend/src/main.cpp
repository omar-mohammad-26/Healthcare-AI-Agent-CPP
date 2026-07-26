#include <drogon/drogon.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>

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

        drogon::app()
            .loadConfigFile(configPath.string())
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