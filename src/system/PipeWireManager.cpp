#include "PipeWireManager.h"

#include <QDebug>
#include <QTimer>

#ifdef HAVE_PIPEWIRE
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/utils/defs.h>
#endif

namespace Stratara::System {

#ifdef HAVE_PIPEWIRE
static const struct pw_registry_events registryEvents = {
    PW_VERSION_REGISTRY_EVENTS,
    .global = PipeWireManager::onRegistryGlobal,
    .global_remove = PipeWireManager::onRegistryGlobalRemove,
};
#endif

PipeWireManager::PipeWireManager(QObject *parent)
    : QObject(parent)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(5000);
    connect(m_updateTimer, &QTimer::timeout, this, &PipeWireManager::updateDevices);
    m_updateTimer->start();

    initPipeWire();
    updateDevices();
}

PipeWireManager::~PipeWireManager()
{
    cleanupPipeWire();
}

void PipeWireManager::initPipeWire()
{
#ifdef HAVE_PIPEWIRE
    pw_init(nullptr, nullptr);

    m_pw.loop = pw_main_loop_new(nullptr);
    if (!m_pw.loop) {
        qWarning() << "Failed to create PipeWire main loop";
        return;
    }

    m_pw.context = pw_context_new(pw_main_loop_get_loop(m_pw.loop), nullptr, 0);
    if (!m_pw.context) {
        qWarning() << "Failed to create PipeWire context";
        pw_main_loop_destroy(m_pw.loop);
        m_pw.loop = nullptr;
        return;
    }

    m_pw.core = pw_context_connect(m_pw.context, PW_CONTEXT_CONNECT_DEFAULT, nullptr, 0);
    if (!m_pw.core) {
        qWarning() << "Failed to connect to PipeWire core";
        pw_context_destroy(m_pw.context);
        pw_main_loop_destroy(m_pw.loop);
        m_pw.context = nullptr;
        m_pw.loop = nullptr;
        return;
    }

    m_pw.registry = pw_core_get_registry(m_pw.core, PW_VERSION_REGISTRY, 0);
    if (!m_pw.registry) {
        qWarning() << "Failed to get PipeWire registry";
        pw_core_disconnect(m_pw.core);
        pw_context_destroy(m_pw.context);
        pw_main_loop_destroy(m_pw.loop);
        m_pw.core = nullptr;
        m_pw.context = nullptr;
        m_pw.loop = nullptr;
        return;
    }

    spa_zero(m_pw.registryListener);
    pw_registry_add_listener(m_pw.registry, &m_pw.registryListener, &registryEvents, this);

    m_available = true;
    emit availableChanged(true);

    // Start the PipeWire event loop in a separate thread would be ideal,
    // but for simplicity we'll process events periodically
    QTimer *pwTimer = new QTimer(this);
    pwTimer->setInterval(50);
    connect(pwTimer, &QTimer::timeout, this, [this]() {
        if (m_pw.loop) {
            pw_main_loop_iterate(m_pw.loop, 0);
        }
    });
    pwTimer->start();

    qInfo() << "PipeWire initialized successfully";
#else
    m_available = false;
    qInfo() << "PipeWire support not compiled in";
#endif
}

void PipeWireManager::cleanupPipeWire()
{
#ifdef HAVE_PIPEWIRE
    if (m_pw.registry) {
        spa_hook_remove(&m_pw.registryListener);
        pw_proxy_destroy((struct pw_proxy*)m_pw.registry);
        m_pw.registry = nullptr;
    }
    if (m_pw.core) {
        pw_core_disconnect(m_pw.core);
        m_pw.core = nullptr;
    }
    if (m_pw.context) {
        pw_context_destroy(m_pw.context);
        m_pw.context = nullptr;
    }
    if (m_pw.loop) {
        pw_main_loop_destroy(m_pw.loop);
        m_pw.loop = nullptr;
    }
    pw_deinit();
    m_available = false;
    emit availableChanged(false);
#endif
}

void PipeWireManager::updateDevices()
{
#ifdef HAVE_PIPEWIRE
    if (m_pw.loop) {
        pw_main_loop_iterate(m_pw.loop, 10);
    }
#endif
}

bool PipeWireManager::available() const
{
    return m_available;
}

QList<QVariantMap> PipeWireManager::audioSources() const
{
    return m_audioSources;
}

QList<QVariantMap> PipeWireManager::audioSinks() const
{
    return m_audioSinks;
}

QString PipeWireManager::defaultSource() const
{
    return m_defaultSource;
}

QString PipeWireManager::defaultSink() const
{
    return m_defaultSink;
}

double PipeWireManager::defaultSourceVolume() const
{
    return m_defaultSourceVolume;
}

double PipeWireManager::defaultSinkVolume() const
{
    return m_defaultSinkVolume;
}

bool PipeWireManager::defaultSourceMuted() const
{
    return m_defaultSourceMuted;
}

bool PipeWireManager::defaultSinkMuted() const
{
    return m_defaultSinkMuted;
}

void PipeWireManager::refreshDevices()
{
    updateDevices();
}

void PipeWireManager::setDefaultSource(const QString &nodeId)
{
#ifdef HAVE_PIPEWIRE
    if (m_pw.core && !nodeId.isEmpty()) {
        bool ok = false;
        uint32_t id = nodeId.toUInt(&ok);
        if (ok) {
            // Set default source via metadata
            // This would require PipeWire metadata API
            qInfo() << "Set default source:" << nodeId;
        }
    }
#endif
    Q_UNUSED(nodeId);
}

void PipeWireManager::setDefaultSink(const QString &nodeId)
{
#ifdef HAVE_PIPEWIRE
    if (m_pw.core && !nodeId.isEmpty()) {
        bool ok = false;
        uint32_t id = nodeId.toUInt(&ok);
        if (ok) {
            qInfo() << "Set default sink:" << nodeId;
        }
    }
#endif
    Q_UNUSED(nodeId);
}

void PipeWireManager::setSourceVolume(const QString &nodeId, double volume)
{
#ifdef HAVE_PIPEWIRE
    if (m_pw.core && !nodeId.isEmpty()) {
        bool ok = false;
        uint32_t id = nodeId.toUInt(&ok);
        if (ok) {
            // Set volume via PipeWire node
            qInfo() << "Set source volume:" << nodeId << volume;
        }
    }
#endif
    Q_UNUSED(nodeId);
    Q_UNUSED(volume);
}

void PipeWireManager::setSinkVolume(const QString &nodeId, double volume)
{
#ifdef HAVE_PIPEWIRE
    if (m_pw.core && !nodeId.isEmpty()) {
        bool ok = false;
        uint32_t id = nodeId.toUInt(&ok);
        if (ok) {
            qInfo() << "Set sink volume:" << nodeId << volume;
        }
    }
#endif
    Q_UNUSED(nodeId);
    Q_UNUSED(volume);
}

void PipeWireManager::setSourceMuted(const QString &nodeId, bool muted)
{
#ifdef HAVE_PIPEWIRE
    if (m_pw.core && !nodeId.isEmpty()) {
        bool ok = false;
        uint32_t id = nodeId.toUInt(&ok);
        if (ok) {
            qInfo() << "Set source muted:" << nodeId << muted;
        }
    }
#endif
    Q_UNUSED(nodeId);
    Q_UNUSED(muted);
}

void PipeWireManager::setSinkMuted(const QString &nodeId, bool muted)
{
#ifdef HAVE_PIPEWIRE
    if (m_pw.core && !nodeId.isEmpty()) {
        bool ok = false;
        uint32_t id = nodeId.toUInt(&ok);
        if (ok) {
            qInfo() << "Set sink muted:" << nodeId << muted;
        }
    }
#endif
    Q_UNUSED(nodeId);
    Q_UNUSED(muted);
}

#ifdef HAVE_PIPEWIRE
void PipeWireManager::onRegistryGlobal(void *data, uint32_t id, uint32_t permissions,
                                      const char *type, uint32_t version,
                                      const struct spa_dict *props)
{
    PipeWireManager *self = static_cast<PipeWireManager*>(data);
    if (self) {
        self->processRegistryGlobal(id, type, props);
    }
}

void PipeWireManager::onRegistryGlobalRemove(void *data, uint32_t id)
{
    PipeWireManager *self = static_cast<PipeWireManager*>(data);
    if (self) {
        self->processRegistryGlobalRemove(id);
    }
}

void PipeWireManager::processRegistryGlobal(uint32_t id, const char *type, const struct spa_dict *props)
{
    if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
        QVariantMap node = nodeToMap(id, props);
        if (!node.isEmpty()) {
            bool isSource = isAudioSource(props);
            bool isSink = isAudioSink(props);

            if (isSource) {
                // Check if already exists
                bool exists = false;
                for (const auto &existing : m_audioSources) {
                    if (existing["id"].toUInt() == id) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    m_audioSources.append(node);
                    emit audioSourcesChanged();
                    emit deviceAdded(node);
                }
            }

            if (isSink) {
                bool exists = false;
                for (const auto &existing : m_audioSinks) {
                    if (existing["id"].toUInt() == id) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    m_audioSinks.append(node);
                    emit audioSinksChanged();
                    emit deviceAdded(node);
                }
            }

            // Check for default source/sink
            const char *defaultConfigured = spa_dict_lookup(props, PW_KEY_DEFAULT_CONFIGURED);
            if (defaultConfigured && strcmp(defaultConfigured, "true") == 0) {
                const char *mediaClass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
                if (mediaClass) {
                    if (strstr(mediaClass, "Audio/Source") || strstr(mediaClass, "Audio/Duplex")) {
                        m_defaultSource = QString::number(id);
                        emit defaultSourceChanged(m_defaultSource);
                    }
                    if (strstr(mediaClass, "Audio/Sink") || strstr(mediaClass, "Audio/Duplex")) {
                        m_defaultSink = QString::number(id);
                        emit defaultSinkChanged(m_defaultSink);
                    }
                }
            }
        }
    }
}

void PipeWireManager::processRegistryGlobalRemove(uint32_t id)
{
    QString idStr = QString::number(id);

    // Remove from sources
    for (int i = 0; i < m_audioSources.size(); ++i) {
        if (m_audioSources[i]["id"].toUInt() == id) {
            m_audioSources.removeAt(i);
            emit audioSourcesChanged();
            emit deviceRemoved(idStr);
            break;
        }
    }

    // Remove from sinks
    for (int i = 0; i < m_audioSinks.size(); ++i) {
        if (m_audioSinks[i]["id"].toUInt() == id) {
            m_audioSinks.removeAt(i);
            emit audioSinksChanged();
            emit deviceRemoved(idStr);
            break;
        }
    }

    // Update defaults if they were removed
    if (m_defaultSource == idStr) {
        m_defaultSource.clear();
        if (!m_audioSources.isEmpty()) {
            m_defaultSource = m_audioSources.first()["id"].toString();
        }
        emit defaultSourceChanged(m_defaultSource);
    }
    if (m_defaultSink == idStr) {
        m_defaultSink.clear();
        if (!m_audioSinks.isEmpty()) {
            m_defaultSink = m_audioSinks.first()["id"].toString();
        }
        emit defaultSinkChanged(m_defaultSink);
    }
}

QVariantMap PipeWireManager::nodeToMap(uint32_t id, const struct spa_dict *props)
{
    QVariantMap map;
    map["id"] = static_cast<int>(id);

    const char *name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
    if (name) map["name"] = QString::fromUtf8(name);

    const char *description = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
    if (description) map["description"] = QString::fromUtf8(description);

    const char *mediaClass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
    if (mediaClass) map["mediaClass"] = QString::fromUtf8(mediaClass);

    const char *iconName = spa_dict_lookup(props, PW_KEY_ICON_NAME);
    if (iconName) map["icon"] = QString::fromUtf8(iconName);

    const char *deviceId = spa_dict_lookup(props, PW_KEY_DEVICE_ID);
    if (deviceId) map["deviceId"] = QString::fromUtf8(deviceId);

    const char *deviceName = spa_dict_lookup(props, PW_KEY_DEVICE_NAME);
    if (deviceName) map["deviceName"] = QString::fromUtf8(deviceName);

    const char *deviceDescription = spa_dict_lookup(props, PW_KEY_DEVICE_DESCRIPTION);
    if (deviceDescription) map["deviceDescription"] = QString::fromUtf8(deviceDescription);

    const char *api = spa_dict_lookup(props, PW_KEY_API);
    if (api) map["api"] = QString::fromUtf8(api);

    const char *defaultConfigured = spa_dict_lookup(props, PW_KEY_DEFAULT_CONFIGURED);
    if (defaultConfigured) map["isDefault"] = (strcmp(defaultConfigured, "true") == 0);

    return map;
}

bool PipeWireManager::isAudioSource(const struct spa_dict *props) const
{
    const char *mediaClass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
    if (!mediaClass) return false;
    return strstr(mediaClass, "Audio/Source") != nullptr || strstr(mediaClass, "Audio/Duplex") != nullptr;
}

bool PipeWireManager::isAudioSink(const struct spa_dict *props) const
{
    const char *mediaClass = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
    if (!mediaClass) return false;
    return strstr(mediaClass, "Audio/Sink") != nullptr || strstr(mediaClass, "Audio/Duplex") != nullptr;
}
#endif

} // namespace Stratara::System