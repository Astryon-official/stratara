#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <QMap>

namespace Stratara::Core {

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool glassBlur READ glassBlur WRITE setGlassBlur NOTIFY glassBlurChanged)
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    Q_PROPERTY(bool useImageWallpaper READ useImageWallpaper WRITE setUseImageWallpaper NOTIFY useImageWallpaperChanged)
    Q_PROPERTY(QString wallpaperImagePath READ wallpaperImagePath WRITE setWallpaperImagePath NOTIFY wallpaperImagePathChanged)
    Q_PROPERTY(int wallpaperId READ wallpaperId WRITE setWallpaperId NOTIFY wallpaperIdChanged)
    Q_PROPERTY(bool useAppArtwork READ useAppArtwork WRITE setUseAppArtwork NOTIFY useAppArtworkChanged)
    Q_PROPERTY(QStringList artworkApps READ artworkApps WRITE setArtworkApps NOTIFY artworkAppsChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QStringList hiddenApps READ hiddenApps WRITE setHiddenApps NOTIFY hiddenAppsChanged)
    Q_PROPERTY(QString frameSource READ frameSource WRITE setFrameSource NOTIFY frameSourceChanged)
    Q_PROPERTY(QString frameFolderId READ frameFolderId WRITE setFrameFolderId NOTIFY frameFolderIdChanged)
    Q_PROPERTY(QString frameFolderName READ frameFolderName WRITE setFrameFolderName NOTIFY frameFolderNameChanged)
    Q_PROPERTY(QString frameImagePath READ frameImagePath WRITE setFrameImagePath NOTIFY frameImagePathChanged)
    Q_PROPERTY(int frameIntervalSec READ frameIntervalSec WRITE setFrameIntervalSec NOTIFY frameIntervalSecChanged)
    Q_PROPERTY(int frameAutoStartSec READ frameAutoStartSec WRITE setFrameAutoStartSec NOTIFY frameAutoStartSecChanged)
    Q_PROPERTY(bool frameClock READ frameClock WRITE setFrameClock NOTIFY frameClockChanged)
    Q_PROPERTY(QString frameClockPosition READ frameClockPosition WRITE setFrameClockPosition NOTIFY frameClockPositionChanged)
    Q_PROPERTY(QString frameClockSize READ frameClockSize WRITE setFrameClockSize NOTIFY frameClockSizeChanged)
    Q_PROPERTY(bool frameShowDate READ frameShowDate WRITE setFrameShowDate NOTIFY frameShowDateChanged)
    Q_PROPERTY(bool frameMotion READ frameMotion WRITE setFrameMotion NOTIFY frameMotionChanged)
    Q_PROPERTY(bool frameShuffle READ frameShuffle WRITE setFrameShuffle NOTIFY frameShuffleChanged)
    Q_PROPERTY(bool useFrameArtWallpaper READ useFrameArtWallpaper WRITE setUseFrameArtWallpaper NOTIFY useFrameArtWallpaperChanged)
    Q_PROPERTY(bool weatherOnHome READ weatherOnHome WRITE setWeatherOnHome NOTIFY weatherOnHomeChanged)
    Q_PROPERTY(bool frameWeather READ frameWeather WRITE setFrameWeather NOTIFY frameWeatherChanged)
    Q_PROPERTY(QString weatherUnit READ weatherUnit WRITE setWeatherUnit NOTIFY weatherUnitChanged)
    Q_PROPERTY(QString weatherCity READ weatherCity WRITE setWeatherCity NOTIFY weatherCityChanged)
    Q_PROPERTY(double weatherLat READ weatherLat WRITE setWeatherLat NOTIFY weatherLatChanged)
    Q_PROPERTY(double weatherLon READ weatherLon WRITE setWeatherLon NOTIFY weatherLonChanged)
    Q_PROPERTY(bool frameNightDim READ frameNightDim WRITE setFrameNightDim NOTIFY frameNightDimChanged)
    Q_PROPERTY(bool nowPlaying READ nowPlaying WRITE setNowPlaying NOTIFY nowPlayingChanged)
    Q_PROPERTY(bool navSounds READ navSounds WRITE setNavSounds NOTIFY navSoundsChanged)

public:
    enum class ThemeMode { Dark, Light, Auto };
    enum class FrameSource { Wallpaper, Folder, Single };
    enum class FrameClockPosition { BottomLeft, BottomCenter, BottomRight, Center };
    enum class FrameClockSize { Small, Medium, Large };
    enum class WeatherUnit { Celsius, Fahrenheit };
    Q_ENUM(ThemeMode)
    Q_ENUM(FrameSource)
    Q_ENUM(FrameClockPosition)
    Q_ENUM(FrameClockSize)
    Q_ENUM(WeatherUnit)

    explicit Settings(QObject *parent = nullptr);
    ~Settings() override = default;

    // Glass blur
    bool glassBlur() const;
    void setGlassBlur(bool value);

    // Columns
    int columns() const;
    void setColumns(int value);

    // Wallpaper
    bool useImageWallpaper() const;
    void setUseImageWallpaper(bool value);
    QString wallpaperImagePath() const;
    void setWallpaperImagePath(const QString &path);
    int wallpaperId() const;
    void setWallpaperId(int id);

    // App artwork
    bool useAppArtwork() const;
    void setUseAppArtwork(bool value);
    QStringList artworkApps() const;
    void setArtworkApps(const QStringList &apps);

    // Theme
    QString theme() const;
    void setTheme(const QString &theme);

    // Hidden apps
    QStringList hiddenApps() const;
    void setHiddenApps(const QStringList &apps);

    // Frame Art
    QString frameSource() const;
    void setFrameSource(const QString &source);
    QString frameFolderId() const;
    void setFrameFolderId(const QString &id);
    QString frameFolderName() const;
    void setFrameFolderName(const QString &name);
    QString frameImagePath() const;
    void setFrameImagePath(const QString &path);
    int frameIntervalSec() const;
    void setFrameIntervalSec(int sec);
    int frameAutoStartSec() const;
    void setFrameAutoStartSec(int sec);
    bool frameClock() const;
    void setFrameClock(bool value);
    QString frameClockPosition() const;
    void setFrameClockPosition(const QString &position);
    QString frameClockSize() const;
    void setFrameClockSize(const QString &size);
    bool frameShowDate() const;
    void setFrameShowDate(bool value);
    bool frameMotion() const;
    void setFrameMotion(bool value);
    bool frameShuffle() const;
    void setFrameShuffle(bool value);
    bool useFrameArtWallpaper() const;
    void setUseFrameArtWallpaper(bool value);

    // Weather
    bool weatherOnHome() const;
    void setWeatherOnHome(bool value);
    bool frameWeather() const;
    void setFrameWeather(bool value);
    QString weatherUnit() const;
    void setWeatherUnit(const QString &unit);
    QString weatherCity() const;
    void setWeatherCity(const QString &city);
    double weatherLat() const;
    void setWeatherLat(double lat);
    double weatherLon() const;
    void setWeatherLon(double lon);

    // Frame night dim
    bool frameNightDim() const;
    void setFrameNightDim(bool value);

    // Media
    bool nowPlaying() const;
    void setNowPlaying(bool value);

    // Navigation sounds
    bool navSounds() const;
    void setNavSounds(bool value);

    Q_INVOKABLE void resetToDefaults();

signals:
    void glassBlurChanged(bool value);
    void columnsChanged(int value);
    void useImageWallpaperChanged(bool value);
    void wallpaperImagePathChanged(const QString &path);
    void wallpaperIdChanged(int id);
    void useAppArtworkChanged(bool value);
    void artworkAppsChanged(const QStringList &apps);
    void themeChanged(const QString &theme);
    void hiddenAppsChanged(const QStringList &apps);
    void frameSourceChanged(const QString &source);
    void frameFolderIdChanged(const QString &id);
    void frameFolderNameChanged(const QString &name);
    void frameImagePathChanged(const QString &path);
    void frameIntervalSecChanged(int sec);
    void frameAutoStartSecChanged(int sec);
    void frameClockChanged(bool value);
    void frameClockPositionChanged(const QString &position);
    void frameClockSizeChanged(const QString &size);
    void frameShowDateChanged(bool value);
    void frameMotionChanged(bool value);
    void frameShuffleChanged(bool value);
    void useFrameArtWallpaperChanged(bool value);
    void weatherOnHomeChanged(bool value);
    void frameWeatherChanged(bool value);
    void weatherUnitChanged(const QString &unit);
    void weatherCityChanged(const QString &city);
    void weatherLatChanged(double lat);
    void weatherLonChanged(double lon);
    void frameNightDimChanged(bool value);
    void nowPlayingChanged(bool value);
    void navSoundsChanged(bool value);

private:
    QSettings *m_settings;
    QMap<QString, QVariant> m_defaults;

    void initDefaults();
    QVariant get(const QString &key, const QVariant &defaultValue) const;
    void set(const QString &key, const QVariant &value);
};

} // namespace Stratara::Core