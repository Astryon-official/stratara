package com.tarang.launcher.data

import android.app.DownloadManager
import android.content.Context
import android.os.Environment
import java.io.File

/**
 * Downloads an update APK using the system [DownloadManager].
 *
 * The file goes under the app's external files dir (not private cache). DownloadManager runs in a
 * separate process and cannot write to the app's private cache via a `file://` URI — that throws
 * [SecurityException].
 */
class ApkDownloader(private val context: Context) {

    /** Starts the download, replacing any previous one, and returns the download's id. */
    fun startDownload(apkUrl: String, previousDownloadId: Long? = null): Long {
        val manager = context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
        if (previousDownloadId != null && previousDownloadId != 0L) {
            runCatching { manager.remove(previousDownloadId) }
        }
        downloadedFile().delete()

        val request = DownloadManager.Request(android.net.Uri.parse(apkUrl))
            .setTitle("Tarang update")
            .setDescription("Downloading update")
            .setDestinationInExternalFilesDir(
                context,
                Environment.DIRECTORY_DOWNLOADS,
                APK_FILE_NAME,
            )
            .setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)

        return manager.enqueue(request)
    }

    fun downloadedFile(): File =
        File(context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS), APK_FILE_NAME)

    /** Returns (bytesDownloaded, totalBytes), or (0, 0) if the download can't be found. */
    fun queryProgress(downloadId: Long): Pair<Int, Int> {
        val manager = context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
        val query = DownloadManager.Query().setFilterById(downloadId)
        manager.query(query).use { cursor ->
            if (cursor.moveToFirst()) {
                val downloaded = cursor.getInt(
                    cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR),
                )
                val total = cursor.getInt(
                    cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_TOTAL_SIZE_BYTES),
                )
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

    private companion object {
        const val APK_FILE_NAME = "update.apk"
    }
}
