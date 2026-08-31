package com.tarang.launcher.data

import android.content.Context
import android.content.pm.PackageManager
import android.content.pm.SigningInfo
import android.os.Build
import java.io.File
import java.security.MessageDigest

/** Confirms a downloaded APK is signed with the same certificate(s) as the installed app. */
object SignatureVerifier {

    /**
     * Fails closed: any error (parse failure, missing signature) means "do not install". Compares
     * the full signer set, not just one entry — [SigningInfo.apkContentsSigners] does not
     * guarantee ordering, so comparing a single index would let an APK with an extra,
     * attacker-controlled signer slip through. Also rejects an APK whose package name doesn't match
     * this app's, since the system installer applies no such check itself (a differently-named APK
     * installs as a brand new app, bypassing this whole comparison).
     */
    fun isSameSigner(context: Context, apkFile: File): Boolean = try {
        val pm = context.packageManager
        val apkSigners = archiveSignerHashes(pm, apkFile)
            ?: error("Could not read signers from the downloaded APK.")
        val installedSigners = installedSignerHashes(pm, context.packageName)
        val apkPackage = pm.getPackageArchiveInfo(apkFile.absolutePath, 0)?.packageName
            ?: error("Could not read the downloaded APK.")
        apkPackage == context.packageName && installedSigners == apkSigners
    } catch (_: Exception) {
        false
    }

    private fun installedSignerHashes(pm: PackageManager, packageName: String): Set<String> {
        val info = if (Build.VERSION.SDK_INT >= 28) {
            pm.getPackageInfo(packageName, PackageManager.GET_SIGNING_CERTIFICATES)
        } else {
            @Suppress("DEPRECATION")
            pm.getPackageInfo(packageName, PackageManager.GET_SIGNATURES)
        }
        return signerHashesFromPackage(info)
    }

    private fun archiveSignerHashes(pm: PackageManager, apkFile: File): Set<String>? {
        if (Build.VERSION.SDK_INT >= 28) {
            val modern = pm.getPackageArchiveInfo(
                apkFile.absolutePath,
                PackageManager.GET_SIGNING_CERTIFICATES,
            )
            // Some API 28–30 builds leave signingInfo null for on-disk archives unless the
            // applicationInfo paths point at the file.
            modern?.applicationInfo?.let { appInfo ->
                appInfo.sourceDir = apkFile.absolutePath
                appInfo.publicSourceDir = apkFile.absolutePath
            }
            val modernHashes = modern?.let { signerHashesFromPackage(it) }
            if (!modernHashes.isNullOrEmpty()) return modernHashes
        }

        @Suppress("DEPRECATION")
        val legacy = pm.getPackageArchiveInfo(apkFile.absolutePath, PackageManager.GET_SIGNATURES)
        return legacy?.let { signerHashesFromPackage(it) }?.takeIf { it.isNotEmpty() }
    }

    private fun signerHashesFromPackage(info: android.content.pm.PackageInfo): Set<String> {
        if (Build.VERSION.SDK_INT >= 28) {
            val fromSigningInfo = signerHashes(info.signingInfo)
            if (fromSigningInfo.isNotEmpty()) return fromSigningInfo
        }
        @Suppress("DEPRECATION")
        val signatures = info.signatures
        if (signatures.isNullOrEmpty()) error("No signers found.")
        return signatures.map { sha256Hex(it.toByteArray()) }.toSet()
    }

    private fun signerHashes(info: SigningInfo?): Set<String> {
        val signers = info?.apkContentsSigners
        if (signers.isNullOrEmpty()) return emptySet()
        return signers.map { sha256Hex(it.toByteArray()) }.toSet()
    }

    private fun sha256Hex(bytes: ByteArray): String =
        MessageDigest.getInstance("SHA-256").digest(bytes).joinToString("") { "%02x".format(it) }
}
