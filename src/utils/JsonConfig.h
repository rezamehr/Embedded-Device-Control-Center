#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include "core/Types.h"

namespace edcc {

/**
 * @brief Simple helper for loading and saving device configurations as JSON.
 */
class JsonConfig
{
public:
    /**
     * @brief Returns the full path to the devices configuration file.
     */
    static QString configFilePath();

    /**
     * @brief Load the list of saved devices from disk.
     * @return List of DeviceInfo. Empty if file does not exist or is invalid.
     */
    static QVector<DeviceInfo> loadDevices();

    /**
     * @brief Save the given list of devices to disk.
     * @param devices List of devices to persist
     * @return true on success
     */
    static bool saveDevices(const QVector<DeviceInfo> &devices);

private:
    static QJsonObject deviceToJson(const DeviceInfo &info);
    static DeviceInfo deviceFromJson(const QJsonObject &obj);
};

} // namespace edcc
