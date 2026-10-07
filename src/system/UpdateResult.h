#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

namespace Stratara::System {

struct UpdateResult {
    enum class Type {
        UpToDate,
        UpdateAvailable,
        Error
    };

    Type type = Type::UpToDate;
    QString versionTag;
    QString changelog;
    QString downloadUrl;
    QString errorMessage;

    UpdateResult() = default;
    explicit UpdateResult(Type t) : type(t) {}

    static UpdateResult upToDate() {
        return UpdateResult{Type::UpToDate};
    }

    static UpdateResult updateAvailable(const QString &versionTag, const QString &changelog, const QString &downloadUrl) {
        UpdateResult result;
        result.type = Type::UpdateAvailable;
        result.versionTag = versionTag;
        result.changelog = changelog;
        result.downloadUrl = downloadUrl;
        return result;
    }

    static UpdateResult error(const QString &message) {
        UpdateResult result;
        result.type = Type::Error;
        result.errorMessage = message;
        return result;
    }

    bool isUpToDate() const { return type == Type::UpToDate; }
    bool isUpdateAvailable() const { return type == Type::UpdateAvailable; }
    bool isError() const { return type == Type::Error; }
};

} // namespace Stratara::System

Q_DECLARE_METATYPE(Stratara::System::UpdateResult)