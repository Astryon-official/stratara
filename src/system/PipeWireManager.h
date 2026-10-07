#pragma once

#include <QObject>
#include <QTimer>
#include <QList>
#include <QVariantMap>

#ifdef HAVE_PIPEWIRE
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#endif

namespace Stratara::System {

class PipeWireManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(QList<QVariantMap> audioSources READ audioSources NOTIFY audioSourcesChanged)
    Q_PROPERTY(QList<QVariantMap> audioSinks READ audioSinks NOTIFY audioSinksChanged)
    Q_PROPERTY(QString defaultSource READ defaultSource NOTIFY defaultSourceChanged)
    Q_PROPERTY(QString defaultSink READ defaultSink NOTIFY defaultSinkChanged)
    Q_PROPERTY(double defaultSourceVolume READ defaultSourceVolume NOTIFY defaultSourceVolumeChanged)
    Q_PROPERTY(double defaultSinkVolume READ defaultSinkVolume NOTIFY defaultSinkVolumeChanged)
    Q_PROPERTY(bool defaultSourceMuted READ defaultSourceMuted NOTIFY defaultSourceMutedChanged)
    Q_PROPERTY(bool defaultSinkMuted READ defaultSinkMuted NOTIFY defaultSinkMutedChanged)

public:
    explicit PipeWireManager(QObject *parent = nullptr);
    ~PipeWireManager() override;

    bool available() const;
    QList<QVariantMap> audioSources() const;
    QList<QVariantMap> audioSinks() const;
    QString defaultSource() const;
    QString defaultSink() const;
    double defaultSourceVolume() const;
    double defaultSinkVolume() const;
    bool defaultSourceMuted() const;
    bool defaultSinkMuted() const;

    Q_INVOKABLE void refreshDevices();
    Q_INVOKABLE void setDefaultSource(const QString &nodeId);
    Q_INVOKABLE void setDefaultSink(const QString &nodeId);
    Q_INVOKABLE void setSourceVolume(const QString &nodeId, double volume);
    Q_INVOKABLE void setSinkVolume(const QString &nodeId, double volume);
    Q_INVOKABLE void setSourceMuted(const QString &nodeId, bool muted);
    Q_INVOKABLE void setSinkMuted(const QString &nodeId, bool muted);

signals:
    void availableChanged(bool available);
    void audioSourcesChanged();
    void audioSinksChanged();
    void defaultSourceChanged(const QString &nodeId);
    void defaultSinkChanged(const QString &nodeId);
    void defaultSourceVolumeChanged(double volume);
    void defaultSinkVolumeChanged(double volume);
    void defaultSourceMutedChanged(bool muted);
    void defaultSinkMutedChanged(bool muted);
    void deviceAdded(const QVariantMap &device);
    void deviceRemoved(const QString &nodeId);

private:
#ifdef HAVE_PIPEWIRE
    struct PipeWireData {
        pw_main_loop *loop = nullptr;
        pw_context *context = nullptr;
        pw_core *core = nullptr;
        pw_registry *registry = nullptr;
        spa_hook registryListener;
        uint32_t defaultSourceId = 0;
        uint32_t defaultSinkId = 0;
    };
    PipeWireData m_pw;
#endif

    QTimer *m_updateTimer = nullptr;

    bool m_available = false;
    QList<QVariantMap> m_audioSources;
    QList<QVariantMap> m_audioSinks;
    QString m_defaultSource;
    QString m_defaultSink;
    double m_defaultSourceVolume = 1.0;
    double m_defaultSinkVolume = 1.0;
    bool m_defaultSourceMuted = false;
    bool m_defaultSinkMuted = false;

    void initPipeWire();
    void cleanupPipeWire();
    void updateDevices();

#ifdef HAVE_PIPEWIRE
    static void onRegistryGlobal(void *data, uint32_t id, uint32_t permissions,
                                 const char *type, uint32_t version,
                                 const struct spa_dict *props);
    static void onRegistryGlobalRemove(void *data, uint32_t id);
    void processRegistryGlobal(uint32_t id, const char *type, const struct spa_dict *props);
    void processRegistryGlobalRemove(uint32_t id);
    QVariantMap nodeToMap(uint32_t id, const struct spa_dict *props);
    bool isAudioSource(const struct spa_dict *props) const;
    bool isAudioSink(const struct spa_dict *props) const;
#endif
};

} // namespace Stratara::System