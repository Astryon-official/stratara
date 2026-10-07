#include "Settings.h"

#include <QStandardPaths>
#include <QDir>

namespace Stratara::Core {

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope,
                               "Astryon", "Stratara", this))
{
    initDefaults();
}

void Settings::initDefaults()
{
    m_defaults["glassBlur"] = false;
    m_defaults["columns"] = 4;
    m_defaults["useImageWallpaper"] = false;
    m_defaults["wallpaperImagePath"] = "";
    m_defaults["wallpaperId"] = 0;
    m_defaults["useAppArtwork"] = false;
    m_defaults["artworkApps"] = QStringList();
    m_defaults["theme"] = "Dark";
    m_defaults["hiddenApps"] = QStringList();
    m_defaults["frameSource"] = "Wallpaper";
    m_defaults["frameFolderId"] = "";
    m_defaults["frameFolderName"] = "";
    m_defaults["frameImagePath"] = "";
    m_defaults["frameIntervalSec"] = 60;
    m_defaults["frameAutoStartSec"] = 0;
    m_defaults["frameClock"] = false;
    m_defaults["frameClockPosition"] = "BottomLeft";
    m_defaults["frameClockSize"] = "Medium";
    m_defaults["frameShowDate"] = true;
    m_defaults["frameMotion"] = true;
    m_defaults["frameShuffle"] = false;
    m_defaults["useFrameArtWallpaper"] = false;
    m_defaults["weatherOnHome"] = false;
    m_defaults["frameWeather"] = false;
    m_defaults["weatherUnit"] = "Celsius";
    m_defaults["weatherCity"] = "";
    m_defaults["weatherLat"] = 0.0;
    m_defaults["weatherLon"] = 0.0;
    m_defaults["frameNightDim"] = false;
    m_defaults["nowPlaying"] = false;
    m_defaults["navSounds"] = true;
}

QVariant Settings::get(const QString &key, const QVariant &defaultValue) const
{
    return m_settings->value(key, defaultValue);
}

void Settings::set(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
}

// Glass blur
bool Settings::glassBlur() const
{
    return get("glassBlur", m_defaults["glassBlur"]).toBool();
}

void Settings::setGlassBlur(bool value)
{
    if (glassBlur() != value) {
        set("glassBlur", value);
        emit glassBlurChanged(value);
    }
}

// Columns
int Settings::columns() const
{
    return get("columns", m_defaults["columns"]).toInt();
}

void Settings::setColumns(int value)
{
    value = qBound(4, value, 7);
    if (columns() != value) {
        set("columns", value);
        emit columnsChanged(value);
    }
}

// Wallpaper
bool Settings::useImageWallpaper() const
{
    return get("useImageWallpaper", m_defaults["useImageWallpaper"]).toBool();
}

void Settings::setUseImageWallpaper(bool value)
{
    if (useImageWallpaper() != value) {
        set("useImageWallpaper", value);
        emit useImageWallpaperChanged(value);
    }
}

QString Settings::wallpaperImagePath() const
{
    return get("wallpaperImagePath", m_defaults["wallpaperImagePath"]).toString();
}

void Settings::setWallpaperImagePath(const QString &path)
{
    if (wallpaperImagePath() != path) {
        set("wallpaperImagePath", path);
        emit wallpaperImagePathChanged(path);
    }
}

int Settings::wallpaperId() const
{
    return get("wallpaperId", m_defaults["wallpaperId"]).toInt();
}

void Settings::setWallpaperId(int id)
{
    if (wallpaperId() != id) {
        set("wallpaperId", id);
        emit wallpaperIdChanged(id);
    }
}

// App artwork
bool Settings::useAppArtwork() const
{
    return get("useAppArtwork", m_defaults["useAppArtwork"]).toBool();
}

void Settings::setUseAppArtwork(bool value)
{
    if (useAppArtwork() != value) {
        set("useAppArtwork", value);
        emit useAppArtworkChanged(value);
    }
}

QStringList Settings::artworkApps() const
{
    return get("artworkApps", m_defaults["artworkApps"]).toStringList();
}

void Settings::setArtworkApps(const QStringList &apps)
{
    if (artworkApps() != apps) {
        set("artworkApps", apps);
        emit artworkAppsChanged(apps);
    }
}

// Theme
QString Settings::theme() const
{
    return get("theme", m_defaults["theme"]).toString();
}

void Settings::setTheme(const QString &newTheme)
{
    if (theme() != newTheme) {
        set("theme", newTheme);
        emit themeChanged(newTheme);
    }
}

// Hidden apps
QStringList Settings::hiddenApps() const
{
    return get("hiddenApps", m_defaults["hiddenApps"]).toStringList();
}

void Settings::setHiddenApps(const QStringList &apps)
{
    if (hiddenApps() != apps) {
        set("hiddenApps", apps);
        emit hiddenAppsChanged(apps);
    }
}

// Frame Art
QString Settings::frameSource() const
{
    return get("frameSource", m_defaults["frameSource"]).toString();
}

void Settings::setFrameSource(const QString &source)
{
    if (frameSource() != source) {
        set("frameSource", source);
        emit frameSourceChanged(source);
    }
}

QString Settings::frameFolderId() const
{
    return get("frameFolderId", m_defaults["frameFolderId"]).toString();
}

void Settings::setFrameFolderId(const QString &id)
{
    if (frameFolderId() != id) {
        set("frameFolderId", id);
        emit frameFolderIdChanged(id);
    }
}

QString Settings::frameFolderName() const
{
    return get("frameFolderName", m_defaults["frameFolderName"]).toString();
}

void Settings::setFrameFolderName(const QString &name)
{
    if (frameFolderName() != name) {
        set("frameFolderName", name);
        emit frameFolderNameChanged(name);
    }
}

QString Settings::frameImagePath() const
{
    return get("frameImagePath", m_defaults["frameImagePath"]).toString();
}

void Settings::setFrameImagePath(const QString &path)
{
    if (frameImagePath() != path) {
        set("frameImagePath", path);
        emit frameImagePathChanged(path);
    }
}

int Settings::frameIntervalSec() const
{
    return get("frameIntervalSec", m_defaults["frameIntervalSec"]).toInt();
}

void Settings::setFrameIntervalSec(int sec)
{
    if (frameIntervalSec() != sec) {
        set("frameIntervalSec", sec);
        emit frameIntervalSecChanged(sec);
    }
}

int Settings::frameAutoStartSec() const
{
    return get("frameAutoStartSec", m_defaults["frameAutoStartSec"]).toInt();
}

void Settings::setFrameAutoStartSec(int sec)
{
    if (frameAutoStartSec() != sec) {
        set("frameAutoStartSec", sec);
        emit frameAutoStartSecChanged(sec);
    }
}

bool Settings::frameClock() const
{
    return get("frameClock", m_defaults["frameClock"]).toBool();
}

void Settings::setFrameClock(bool value)
{
    if (frameClock() != value) {
        set("frameClock", value);
        emit frameClockChanged(value);
    }
}

QString Settings::frameClockPosition() const
{
    return get("frameClockPosition", m_defaults["frameClockPosition"]).toString();
}

void Settings::setFrameClockPosition(const QString &position)
{
    if (frameClockPosition() != position) {
        set("frameClockPosition", position);
        emit frameClockPositionChanged(position);
    }
}

QString Settings::frameClockSize() const
{
    return get("frameClockSize", m_defaults["frameClockSize"]).toString();
}

void Settings::setFrameClockSize(const QString &size)
{
    if (frameClockSize() != size) {
        set("frameClockSize", size);
        emit frameClockSizeChanged(size);
    }
}

bool Settings::frameShowDate() const
{
    return get("frameShowDate", m_defaults["frameShowDate"]).toBool();
}

void Settings::setFrameShowDate(bool value)
{
    if (frameShowDate() != value) {
        set("frameShowDate", value);
        emit frameShowDateChanged(value);
    }
}

bool Settings::frameMotion() const
{
    return get("frameMotion", m_defaults["frameMotion"]).toBool();
}

void Settings::setFrameMotion(bool value)
{
    if (frameMotion() != value) {
        set("frameMotion", value);
        emit frameMotionChanged(value);
    }
}

bool Settings::frameShuffle() const
{
    return get("frameShuffle", m_defaults["frameShuffle"]).toBool();
}

void Settings::setFrameShuffle(bool value)
{
    if (frameShuffle() != value) {
        set("frameShuffle", value);
        emit frameShuffleChanged(value);
    }
}

bool Settings::useFrameArtWallpaper() const
{
    return get("useFrameArtWallpaper", m_defaults["useFrameArtWallpaper"]).toBool();
}

void Settings::setUseFrameArtWallpaper(bool value)
{
    if (useFrameArtWallpaper() != value) {
        set("useFrameArtWallpaper", value);
        emit useFrameArtWallpaperChanged(value);
    }
}

// Weather
bool Settings::weatherOnHome() const
{
    return get("weatherOnHome", m_defaults["weatherOnHome"]).toBool();
}

void Settings::setWeatherOnHome(bool value)
{
    if (weatherOnHome() != value) {
        set("weatherOnHome", value);
        emit weatherOnHomeChanged(value);
    }
}

bool Settings::frameWeather() const
{
    return get("frameWeather", m_defaults["frameWeather"]).toBool();
}

void Settings::setFrameWeather(bool value)
{
    if (frameWeather() != value) {
        set("frameWeather", value);
        emit frameWeatherChanged(value);
    }
}

QString Settings::weatherUnit() const
{
    return get("weatherUnit", m_defaults["weatherUnit"]).toString();
}

void Settings::setWeatherUnit(const QString &unit)
{
    if (weatherUnit() != unit) {
        set("weatherUnit", unit);
        emit weatherUnitChanged(unit);
    }
}

QString Settings::weatherCity() const
{
    return get("weatherCity", m_defaults["weatherCity"]).toString();
}

void Settings::setWeatherCity(const QString &city)
{
    if (weatherCity() != city) {
        set("weatherCity", city);
        emit weatherCityChanged(city);
    }
}

double Settings::weatherLat() const
{
    return get("weatherLat", m_defaults["weatherLat"]).toDouble();
}

void Settings::setWeatherLat(double lat)
{
    if (qFuzzyCompare(weatherLat(), lat) == false) {
        set("weatherLat", lat);
        emit weatherLatChanged(lat);
    }
}

double Settings::weatherLon() const
{
    return get("weatherLon", m_defaults["weatherLon"]).toDouble();
}

void Settings::setWeatherLon(double lon)
{
    if (qFuzzyCompare(weatherLon(), lon) == false) {
        set("weatherLon", lon);
        emit weatherLonChanged(lon);
    }
}

// Frame night dim
bool Settings::frameNightDim() const
{
    return get("frameNightDim", m_defaults["frameNightDim"]).toBool();
}

void Settings::setFrameNightDim(bool value)
{
    if (frameNightDim() != value) {
        set("frameNightDim", value);
        emit frameNightDimChanged(value);
    }
}

// Media
bool Settings::nowPlaying() const
{
    return get("nowPlaying", m_defaults["nowPlaying"]).toBool();
}

void Settings::setNowPlaying(bool value)
{
    if (nowPlaying() != value) {
        set("nowPlaying", value);
        emit nowPlayingChanged(value);
    }
}

// Navigation sounds
bool Settings::navSounds() const
{
    return get("navSounds", m_defaults["navSounds"]).toBool();
}

void Settings::setNavSounds(bool value)
{
    if (navSounds() != value) {
        set("navSounds", value);
        emit navSoundsChanged(value);
    }
}

void Settings::resetToDefaults()
{
    m_settings->clear();
    for (auto it = m_defaults.begin(); it != m_defaults.end(); ++it) {
        m_settings->setValue(it.key(), it.value());
    }
    m_settings->sync();
}

} // namespace Stratara::Core