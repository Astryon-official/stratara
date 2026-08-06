package com.tarang.launcher.data

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.longPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

private val Context.updateDataStore: DataStore<Preferences> by preferencesDataStore(name = "tarang_updates")

/** Persists when the app last checked GitHub for a new release, to respect the API's rate limit. */
class UpdateStore(context: Context) {

    private val dataStore = context.applicationContext.updateDataStore

    val lastCheckedAtMillis: Flow<Long> = dataStore.data.map { it[LAST_CHECKED_KEY] ?: 0L }

    suspend fun markCheckedNow(nowMillis: Long) {
        dataStore.edit { it[LAST_CHECKED_KEY] = nowMillis }
    }

    private companion object {
        val LAST_CHECKED_KEY = longPreferencesKey("last_checked_at_millis")
    }
}
