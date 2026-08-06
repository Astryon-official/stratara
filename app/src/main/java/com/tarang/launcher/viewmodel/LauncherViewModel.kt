package com.tarang.launcher.viewmodel

import android.app.DownloadManager
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import androidx.lifecycle.viewmodel.initializer
import androidx.lifecycle.viewmodel.viewModelFactory
import com.tarang.launcher.data.ApkDownloader
import com.tarang.launcher.data.AppInfo
import com.tarang.launcher.data.AppListCache
import com.tarang.launcher.data.AppRepository
import com.tarang.launcher.data.FavoritesStore
import com.tarang.launcher.data.IconLoader
import com.tarang.launcher.data.InstallResult
import com.tarang.launcher.data.LauncherSettings
import com.tarang.launcher.data.SettingsStore
import com.tarang.launcher.data.UpdateChecker
import com.tarang.launcher.data.UpdateInstaller
import com.tarang.launcher.data.UpdateResult
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.FlowPreview
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.combine
import kotlinx.coroutines.flow.debounce
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch
import java.io.File

data class LauncherUiState(
    val isLoading: Boolean = true,
    val dockApps: List<AppInfo> = emptyList(),
    val gridApps: List<AppInfo> = emptyList(),
    val allApps: List<AppInfo> = emptyList(),
)

/** State for the "Check for updates" flow in Settings > Diagnostics. */
sealed class UpdateUiState {
    data object Idle : UpdateUiState()
    data object Checking : UpdateUiState()
    data object UpToDate : UpdateUiState()
    data class Available(val versionTag: String, val changelog: String, val apkUrl: String) : UpdateUiState()
    data class Downloading(val progressPercent: Int) : UpdateUiState()
    data class ReadyToInstall(val apkFile: File) : UpdateUiState()
    data class NeedsInstallPermission(val apkFile: File) : UpdateUiState()
    data object SignatureMismatch : UpdateUiState()
    data class Error(val message: String) : UpdateUiState()
}

@OptIn(FlowPreview::class)
class LauncherViewModel(
    private val repository: AppRepository,
    private val favoritesStore: FavoritesStore,
    private val settingsStore: SettingsStore,
    private val appListCache: AppListCache,
    private val iconLoader: IconLoader,
    private val updateChecker: UpdateChecker,
    private val apkDownloader: ApkDownloader,
    private val updateInstaller: UpdateInstaller,
) : ViewModel() {

    private val apps = MutableStateFlow<List<AppInfo>>(emptyList())
    private val loading = MutableStateFlow(true)
    private val _focusedPackage = MutableStateFlow<String?>(null)
    private val _updateState = MutableStateFlow<UpdateUiState>(UpdateUiState.Idle)
    val updateState: StateFlow<UpdateUiState> = _updateState
    private var downloadPollJob: Job? = null

    /** The currently focused app package — drives the ambient wallpaper glow. Kept OUT of [uiState]
     *  so moving focus doesn't recompute the dock/grid lists (and recompose the grid) on every press. */
    val focusedPackage: StateFlow<String?> = _focusedPackage

    val uiState: StateFlow<LauncherUiState> =
        combine(loading, apps, favoritesStore.favorites) { isLoading, allApps, favorites ->
            val favoriteSet = favorites.toSet()
            val dock = favorites.mapNotNull { pkg -> allApps.firstOrNull { it.packageName == pkg } }
            val grid = allApps.filterNot { it.packageName in favoriteSet }
            LauncherUiState(
                isLoading = isLoading,
                dockApps = dock,
                gridApps = grid,
                allApps = allApps,
            )
        }.stateIn(viewModelScope, SharingStarted.WhileSubscribed(5_000), LauncherUiState())

    /** Null until the first DataStore read lands — the UI holds its (black) first frame on it
     *  instead of flashing default settings (wrong wallpaper/theme) and re-rendering. */
    val settings: StateFlow<LauncherSettings?> =
        settingsStore.settings.stateIn(viewModelScope, SharingStarted.WhileSubscribed(5_000), null)

    private var prefetchJob: Job? = null

    init {
        viewModelScope.launch {
            // Cold start: publish the last-known app list from disk so the dock/grid draw on the
            // first frame instead of waiting out the PackageManager scan. The real scan follows
            // (briefly deferred on a cache hit, so it doesn't contend with first-frame rendering).
            val cached = appListCache.read()
            if (!cached.isNullOrEmpty() && apps.value.isEmpty()) {
                apps.value = cached
                loading.value = false
                delay(COLD_START_SCAN_DELAY_MS)
            }
            scan(showLoading = apps.value.isEmpty())
        }
        // Keep the list live: refresh when apps are installed/removed/updated (debounced to
        // coalesce the burst of broadcasts a single install produces).
        viewModelScope.launch {
            repository.packageEvents().debounce(400).collect { refresh(showLoading = false) }
        }
    }

    /** [showLoading] is false for background refreshes (e.g. install/uninstall) so the grid
     *  doesn't flash the "Loading…" placeholder while the user is looking at it. */
    fun refresh(showLoading: Boolean = true) {
        viewModelScope.launch { scan(showLoading) }
    }

    private suspend fun scan(showLoading: Boolean) {
        if (showLoading) loading.value = true
        val loaded = repository.loadApps()
        apps.value = loaded
        loading.value = false
        appListCache.write(loaded)
        if (!favoritesStore.seeded.first()) {
            favoritesStore.setFavorites(loaded.take(DEFAULT_DOCK_COUNT).map { it.packageName })
            favoritesStore.markSeeded()
        }
        prefetchTiles(loaded)
    }

    /** Warms the tile-art disk cache for every app (sequentially, after a beat) so off-screen grid
     *  tiles — and the whole next cold start — load from disk instead of the slow PM resolve. */
    private fun prefetchTiles(apps: List<AppInfo>) {
        prefetchJob?.cancel()
        prefetchJob = viewModelScope.launch(Dispatchers.IO) {
            delay(PREFETCH_DELAY_MS)
            for (app in apps) runCatching { iconLoader.loadTile(app) }
        }
    }

    fun onAppFocused(packageName: String) {
        if (_focusedPackage.value != packageName) _focusedPackage.value = packageName
    }

    fun launchApp(packageName: String, options: android.os.Bundle? = null): Boolean =
        repository.launch(packageName, options)

    fun openAppInfo(packageName: String) = repository.openAppInfo(packageName)

    fun uninstallApp(packageName: String) = repository.requestUninstall(packageName)

    fun toggleFavorite(packageName: String) {
        viewModelScope.launch { favoritesStore.toggle(packageName) }
    }

    /** Persists a new dock order (used when the user reorders favorites in move mode). */
    fun setFavoritesOrder(packages: List<String>) {
        viewModelScope.launch { favoritesStore.setFavorites(packages) }
    }

    fun setWallpaper(id: Int) = viewModelScope.launch { settingsStore.setWallpaper(id) }.let {}
    fun setGlassBlur(value: Boolean) = viewModelScope.launch { settingsStore.setGlassBlur(value) }.let {}
    fun setColumns(n: Int) = viewModelScope.launch { settingsStore.setColumns(n) }.let {}
    fun setImageWallpaper(path: String) = viewModelScope.launch { settingsStore.setImageWallpaper(path) }.let {}
    fun setUseImageWallpaper(value: Boolean) = viewModelScope.launch { settingsStore.setUseImageWallpaper(value) }.let {}
    fun setUseAppArtwork(value: Boolean) = viewModelScope.launch { settingsStore.setUseAppArtwork(value) }.let {}
    fun setArtworkApp(packageName: String, enabled: Boolean) =
        viewModelScope.launch { settingsStore.setArtworkApp(packageName, enabled) }.let {}
    fun setTheme(mode: com.tarang.launcher.data.ThemeMode) =
        viewModelScope.launch { settingsStore.setTheme(mode) }.let {}
    fun setAppHidden(packageName: String, hidden: Boolean) =
        viewModelScope.launch { settingsStore.setAppHidden(packageName, hidden) }.let {}
    fun setFrameSource(source: com.tarang.launcher.data.FrameSource) =
        viewModelScope.launch { settingsStore.setFrameSource(source) }.let {}
    fun setFrameFolder(id: String, name: String) =
        viewModelScope.launch { settingsStore.setFrameFolder(id, name) }.let {}
    fun setFrameImage(path: String) = viewModelScope.launch { settingsStore.setFrameImage(path) }.let {}
    fun setFrameInterval(sec: Int) = viewModelScope.launch { settingsStore.setFrameInterval(sec) }.let {}
    fun setFrameAutoStart(sec: Int) = viewModelScope.launch { settingsStore.setFrameAutoStart(sec) }.let {}
    fun setFrameClock(value: Boolean) = viewModelScope.launch { settingsStore.setFrameClock(value) }.let {}
    fun setFrameClockPosition(pos: com.tarang.launcher.data.FrameClockPosition) =
        viewModelScope.launch { settingsStore.setFrameClockPosition(pos) }.let {}
    fun setFrameClockSize(size: com.tarang.launcher.data.FrameClockSize) =
        viewModelScope.launch { settingsStore.setFrameClockSize(size) }.let {}
    fun setFrameShowDate(value: Boolean) = viewModelScope.launch { settingsStore.setFrameShowDate(value) }.let {}
    fun setFrameMotion(value: Boolean) = viewModelScope.launch { settingsStore.setFrameMotion(value) }.let {}
    fun setFrameShuffle(value: Boolean) = viewModelScope.launch { settingsStore.setFrameShuffle(value) }.let {}
    fun setUseFrameArtWallpaper(value: Boolean) =
        viewModelScope.launch { settingsStore.setUseFrameArtWallpaper(value) }.let {}
    fun setWeatherOnHome(value: Boolean) = viewModelScope.launch { settingsStore.setWeatherOnHome(value) }.let {}
    fun setFrameWeather(value: Boolean) = viewModelScope.launch { settingsStore.setFrameWeather(value) }.let {}
    fun setWeatherUnit(unit: com.tarang.launcher.data.WeatherUnit) =
        viewModelScope.launch { settingsStore.setWeatherUnit(unit) }.let {}
    fun setWeatherCity(name: String, lat: Double, lon: Double) =
        viewModelScope.launch { settingsStore.setWeatherCity(name, lat, lon) }.let {}
    fun clearWeatherCity() = viewModelScope.launch { settingsStore.clearWeatherCity() }.let {}
    fun setFrameNightDim(value: Boolean) = viewModelScope.launch { settingsStore.setFrameNightDim(value) }.let {}
    fun setNowPlaying(value: Boolean) = viewModelScope.launch { settingsStore.setNowPlaying(value) }.let {}
    fun setNavSounds(value: Boolean) = viewModelScope.launch { settingsStore.setNavSounds(value) }.let {}

    fun checkForUpdate() {
        _updateState.value = UpdateUiState.Checking
        viewModelScope.launch {
            _updateState.value = when (val result = updateChecker.checkForUpdate(System.currentTimeMillis())) {
                is UpdateResult.UpToDate -> UpdateUiState.UpToDate
                is UpdateResult.UpdateAvailable ->
                    UpdateUiState.Available(result.versionTag, result.changelog, result.apkUrl)
                is UpdateResult.Error -> UpdateUiState.Error(result.reason)
            }
        }
    }

    fun downloadUpdate(apkUrl: String) {
        val downloadId = apkDownloader.startDownload(apkUrl)
        _updateState.value = UpdateUiState.Downloading(0)
        downloadPollJob?.cancel()
        downloadPollJob = viewModelScope.launch {
            while (true) {
                val status = apkDownloader.queryStatus(downloadId)
                if (status == DownloadManager.STATUS_SUCCESSFUL) {
                    _updateState.value = UpdateUiState.ReadyToInstall(apkDownloader.downloadedFile())
                    break
                }
                if (status == DownloadManager.STATUS_FAILED) {
                    _updateState.value = UpdateUiState.Error("The download failed.")
                    break
                }
                val (downloaded, total) = apkDownloader.queryProgress(downloadId)
                val percent = if (total > 0) (downloaded * 100 / total) else 0
                _updateState.value = UpdateUiState.Downloading(percent)
                delay(DOWNLOAD_POLL_INTERVAL_MS)
            }
        }
    }

    fun installUpdate(apkFile: File) {
        _updateState.value = when (updateInstaller.install(apkFile)) {
            InstallResult.Started -> UpdateUiState.ReadyToInstall(apkFile)
            InstallResult.NeedsInstallPermission -> UpdateUiState.NeedsInstallPermission(apkFile)
            InstallResult.SignatureMismatch -> UpdateUiState.SignatureMismatch
        }
    }

    fun requestInstallPermission() = updateInstaller.requestInstallPermission()

    companion object {
        private const val DEFAULT_DOCK_COUNT = 5

        /** How long a cache-hit cold start defers the verify scan, keeping the CPU free while the
         *  first frames render. Package broadcasts still trigger an immediate refresh. */
        private const val COLD_START_SCAN_DELAY_MS = 1_500L

        /** How long after a scan the tile prefetch starts (lets the visible tiles load first). */
        private const val PREFETCH_DELAY_MS = 3_000L

        /** How often the download-progress poll loop re-reads DownloadManager's cursor. */
        private const val DOWNLOAD_POLL_INTERVAL_MS = 500L

        fun provideFactory(
            repository: AppRepository,
            favoritesStore: FavoritesStore,
            settingsStore: SettingsStore,
            appListCache: AppListCache,
            iconLoader: IconLoader,
            updateChecker: UpdateChecker,
            apkDownloader: ApkDownloader,
            updateInstaller: UpdateInstaller,
        ): ViewModelProvider.Factory = viewModelFactory {
            initializer {
                LauncherViewModel(
                    repository, favoritesStore, settingsStore, appListCache, iconLoader,
                    updateChecker, apkDownloader, updateInstaller,
                )
            }
        }
    }
}
