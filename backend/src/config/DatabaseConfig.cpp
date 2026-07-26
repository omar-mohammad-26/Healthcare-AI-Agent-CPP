#include "config/DatabaseConfig.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace
{
std::string getEnvironmentValue(
    const char *name,
    const std::string &defaultValue
)
{
    const char *value = std::getenv(name);

    if (value == nullptr || std::string(value).empty())
    {
        return defaultValue;
    }

    return value;
}

int getEnvironmentInteger(
    const char *name,
    const int defaultValue
)
{
    const char *value = std::getenv(name);

    if (value == nullptr || std::string(value).empty())
    {
        return defaultValue;
    }

    try
    {
        const std::string stringValue(value);
        std::size_t parsedCharacters = 0;

        const int parsedValue =
            std::stoi(stringValue, &parsedCharacters);

        if (parsedCharacters != stringValue.size() ||
            parsedValue <= 0)
        {
            throw std::invalid_argument("Invalid integer");
        }

        return parsedValue;
    }
    catch (const std::exception &)
    {
        throw std::runtime_error(
            std::string("Invalid integer environment variable: ") +
            name
        );
    }
}
}

namespace healthcare::config
{
Json::Value buildDatabaseClientsConfig()
{
    Json::Value databaseClients(Json::arrayValue);
    Json::Value databaseClient;

    databaseClient["name"] = "default";
    databaseClient["rdbms"] = "postgresql";

    databaseClient["host"] =
        getEnvironmentValue(
            "POSTGRES_HOST",
            "postgres"
        );

    databaseClient["port"] =
        getEnvironmentInteger(
            "POSTGRES_PORT",
            5432
        );

    databaseClient["dbname"] =
        getEnvironmentValue(
            "POSTGRES_DB",
            "healthcare_db"
        );

    databaseClient["user"] =
        getEnvironmentValue(
            "POSTGRES_USER",
            "healthcare_user"
        );

    databaseClient["passwd"] =
        getEnvironmentValue(
            "POSTGRES_PASSWORD",
            "healthcare_pass"
        );

    databaseClient["is_fast"] = false;

    databaseClient["number_of_connections"] =
        getEnvironmentInteger(
            "POSTGRES_CONNECTIONS",
            2
        );

    databaseClient["timeout"] = 5.0;
    databaseClient["auto_batch"] = false;

    databaseClients.append(databaseClient);

    return databaseClients;
}
}