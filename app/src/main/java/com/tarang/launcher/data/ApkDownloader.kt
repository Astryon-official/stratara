package com.tarang.launcher.data

import android.app.DownloadManager
import android.content.Context
import android.net.Uri
import androidx.core.net.toUri
import java.io.File

/** Downloads an update APK into the app cache using the system [DownloadManager]. */
class ApkDownloader(private val context: Context) {

    /** Starts the download, replacing any previous one, and returns the download's id. */
    fun startDownload(apkUrl: String): Long {
        val destFile = downloadedFile()
        destFile.delete()

        val request = DownloadManager.Request(apkUrl.toUri())
            .setDestinationUri(Uri.fromFile(destFile))
            .setNotificationVisibility(DownloadManager.Request.VISIBILITY_HIDDEN)

        val manager = context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
        return manager.enqueue(request)
    }

    fun downloadedFile(): File = File(context.cacheDir, "update.apk")

    /** Returns (bytesDownloaded, totalBytes), or (0, 0) if the download can't be found. */
    fun queryProgress(downloadId: Long): Pair<Int, Int> {
        val manager = context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
        val query = DownloadManager.Query().setFilterById(downloadId)
        manager.query(query).use { cursor ->
            if (cursor.moveToFirst()) {
                val downloaded = cursor.getInt(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR))
                val total = cursor.getInt(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_TOTAL_SIZE_BYTES))
                return downloaded to total
            }
        }
        return 0 to 0
    }

    /** The download's current [DownloadManager] status, or [DownloadManager.STATUS_FAILED] if it
     *  can't be found (e.g. cleared by the system). */
    fun queryStatus(downloadId: Long): Int {
        val manager = context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
        val query = DownloadManager.Query().setFilterById(downloadId)
        manager.query(query).use { cursor ->
            if (cursor.moveToFirst()) {
                return cursor.getInt(cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_STATUS))
            }
        }
        return DownloadManager.STATUS_FAILED
    }
}
