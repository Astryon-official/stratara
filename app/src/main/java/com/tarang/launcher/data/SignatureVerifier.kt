package com.tarang.launcher.data

import android.content.Context
import android.content.pm.PackageManager
import android.content.pm.SigningInfo
import java.io.File
import java.security.MessageDigest

/** Confirms a downloaded APK is signed with the same certificate(s) as the installed app. */
object SignatureVerifier {

    /**
     * Fails closed: any error (parse failure, missing signature) means "do not install". Compares
     * the full signer set, not just one entry — [android.content.pm.SigningInfo.getApkContentsSigners]
     * does not guarantee ordering, so comparing a single index would let an APK with an extra,
     * attacker-controlled signer slip through. Also rejects an APK whose package name doesn't match
     * this app's, since the system installer applies no such check itself (a differently-named APK
     * installs as a brand new app, bypassing this whole comparison).
     */
    fun isSameSigner(context: Context, apkFile: File): Boolean = try {
        val pm = context.packageManager
        val apkInfo = pm.getPackageArchiveInfo(apkFile.absolutePath, PackageManager.GET_SIGNING_CERTIFICATES)
            ?: error("Could not read the downloaded APK.")
        apkInfo.packageName == context.packageName &&
            signerHashes(pm.getPackageInfo(context.packageName, PackageManager.GET_SIGNING_CERTIFICATES).signingInfo) ==
            signerHashes(apkInfo.signingInfo)
    } catch (e: Exception) {
        false
    }

    private fun signerHashes(info: SigningInfo?): Set<String> {
        val signers = info?.apkContentsSigners
        if (signers.isNullOrEmpty()) error("No signers found.")
        return signers.map { sha256Hex(it.toByteArray()) }.toSet()
    }

    private fun sha256Hex(bytes: ByteArray): String =
        MessageDigest.getInstance("SHA-256").digest(bytes).joinToString("") { "%02x".format(it) }
}
