package com.tarang.launcher.data

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.longPreferencesKey
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

private val Context.updateDataStore: DataStore<Preferences> by preferencesDataStore(name = "tarang_updates")

/** A newer release found earlier, kept so a launch notice can show during the check cooldown. */
data class PendingUpdate(
    val versionTag: String,
    val changelog: String,
    val apkUrl: String,
)

/** Persists update-check timing, a pending release, and which notice the user dismissed. */
class UpdateStore(context: Context) {

    private val dataStore = context.applicationContext.updateDataStore

    val lastCheckedAtMillis: Flow<Long> = dataStore.data.map { it[LAST_CHECKED_KEY] ?: 0L }

    val pendingUpdate: Flow<PendingUpdate?> = dataStore.data.map { prefs ->
        val tag = prefs[PENDING_TAG_KEY] ?: return@map null
        val apkUrl = prefs[PENDING_APK_URL_KEY] ?: return@map null
        PendingUpdate(
            versionTag = tag,
            changelog = prefs[PENDING_CHANGELOG_KEY] ?: "",
            apkUrl = apkUrl,
        )
    }

    val dismissedNoticeTag: Flow<String?> = dataStore.data.map { it[DISMISSED_NOTICE_TAG_KEY] }

    suspend fun markCheckedNow(nowMillis: Long) {
        dataStore.edit { it[LAST_CHECKED_KEY] = nowMillis }
    }

    suspend fun setPendingUpdate(versionTag: String, changelog: String, apkUrl: String) {
        dataStore.edit {
            it[PENDING_TAG_KEY] = versionTag
            it[PENDING_CHANGELOG_KEY] = changelog
            it[PENDING_APK_URL_KEY] = apkUrl
        }
    }

    suspend fun clearPendingUpdate() {
        dataStore.edit {
            it.remove(PENDING_TAG_KEY)
            it.remove(PENDING_CHANGELOG_KEY)
            it.remove(PENDING_APK_URL_KEY)
        }
    }

    suspend fun dismissNotice(versionTag: String) {
        dataStore.edit { it[DISMISSED_NOTICE_TAG_KEY] = versionTag }
    }

    private companion object {
        val LAST_CHECKED_KEY = longPreferencesKey("last_checked_at_millis")
        val PENDING_TAG_KEY = stringPreferencesKey("pending_version_tag")
        val PENDING_CHANGELOG_KEY = stringPreferencesKey("pending_changelog")
        val PENDING_APK_URL_KEY = stringPreferencesKey("pending_apk_url")
        val DISMISSED_NOTICE_TAG_KEY = stringPreferencesKey("dismissed_notice_tag")
    }
}
