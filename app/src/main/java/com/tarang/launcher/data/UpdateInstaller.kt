package com.tarang.launcher.data

import android.content.Context
import android.content.Intent
import android.net.Uri
import android.provider.Settings
import android.util.Log
import androidx.core.content.FileProvider
import java.io.File

sealed class InstallResult {
    data object Started : InstallResult()
    data object NeedsInstallPermission : InstallResult()
    data object SignatureMismatch : InstallResult()
    data object CouldNotStart : InstallResult()
}

/** Verifies and launches the system installer for a downloaded update APK. */
class UpdateInstaller(private val context: Context) {

    fun canInstallPackages(): Boolean = context.packageManager.canRequestPackageInstalls()

    /** Opens the system screen where the user grants "install unknown apps" for this app. */
    fun requestInstallPermission() {
        val intent = Intent(
            Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES,
            Uri.parse("package:${context.packageName}"),
        ).addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        runCatching { context.startActivity(intent) }
            .onFailure { Log.w(TAG, "Could not open install-permission settings", it) }
    }

    /**
     * Verifies [apkFile] is signed with the same certificate as the installed app, then starts the
     * system install screen. Returns which of those steps blocked the install, if any.
     */
    fun install(apkFile: File): InstallResult {
        if (!canInstallPackages()) return InstallResult.NeedsInstallPermission
        if (!SignatureVerifier.isSameSigner(context, apkFile)) return InstallResult.SignatureMismatch

        val uri = FileProvider.getUriForFile(context, "${context.packageName}.fileprovider", apkFile)
        val intent = Intent(Intent.ACTION_VIEW).apply {
            setDataAndType(uri, "application/vnd.android.package-archive")
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_GRANT_READ_URI_PERMISSION)
        }
        return runCatching {
            context.startActivity(intent)
            InstallResult.Started
        }.getOrElse {
            Log.w(TAG, "Could not start the install screen", it)
            InstallResult.CouldNotStart
        }
    }

    private companion object {
        const val TAG = "UpdateInstaller"
    }
}
