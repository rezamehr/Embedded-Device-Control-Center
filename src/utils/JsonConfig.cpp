#include "JsonConfig.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace edcc {

QString JsonConfig::configFilePath()
{
    // Store config in a per-user application data location
    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dirPath);
    return dirPath + "/devices.json";
}

QJsonObject JsonConfig::deviceToJson(const DeviceInfo &info)
{
    QJsonObject obj;
    obj["id"]          = info.id;
    obj["name"]        = info.name;
    obj["description"] = info.description;
    obj["portName"]    = info.portName;   // used for Serial
    obj["host"]        = info.host;       // used for TCP
    obj["port"]        = static_cast<int>(info.port);
    obj["baudRate"]    = static_cast<int>(info.baudRate);
    obj["type"]        = info.type;
    return obj;
}

DeviceInfo JsonConfig::deviceFromJson(const QJsonObject &obj)
{
    DeviceInfo info;
    info.id          = obj.value("id").toString();
    info.name        = obj.value("name").toString();
    info.description = obj.value("description").toString();
    info.portName    = obj.value("portName").toString();
    info.host        = obj.value("host").toString();
    info.port        = static_cast<quint16>(obj.value("port").toInt(0));
    info.baudRate    = static_cast<quint16>(obj.value("baudRate").toInt(0));
    info.type        = obj.value("type").toString();
    return info;
}

QVector<DeviceInfo> JsonConfig::loadDevices()
{
    QVector<DeviceInfo> result;

    QFile file(configFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return result;      // file does not exist yet – not an error
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        return result;
    }

    const QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        if (value.isObject()) {
            result.append(deviceFromJson(value.toObject()));
        }
    }

    return result;
}

bool JsonConfig::saveDevices(const QVector<DeviceInfo> &devices)
{
    QJsonArray array;
    for (const DeviceInfo &info : devices) {
        array.append(deviceToJson(info));
    }

    QFile file(configFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

} // namespace edcc
