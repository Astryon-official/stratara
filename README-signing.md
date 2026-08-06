# Release signing

Self-update (see `UpdateInstaller`) checks that a downloaded APK has the same
signing certificate as the installed app. This check only works if every
release uses the same key, so release builds must sign with a stable, dedicated
keystore, not the default per-machine debug key.

## One-time setup

1. Generate a release keystore. Keep it outside the repository, or keep it in
   the repository root — `*.jks` and `*.keystore` are gitignored either way.
   ```
   keytool -genkeypair -v -keystore tarang-release.jks -alias tarang \
     -keyalg RSA -keysize 2048 -validity 10000
   ```
2. Create `keystore.properties` in the project root. This file is gitignored.
   ```
   storeFile=tarang-release.jks
   storePassword=<password>
   keyAlias=tarang
   keyPassword=<password>
   ```
   `storeFile` is relative to the project root.
3. Build the release APK as normal. Gradle picks up `keystore.properties`
   automatically. If the file is absent, the build falls back to the debug
   key (with a warning that self-update signature checks will fail against
   past releases).

## After each release build

Record the certificate fingerprint as a manual paper trail (for example, in
the GitHub release notes or the commit message):
```
apksigner verify --print-certs app/build/outputs/apk/release/app-release.apk
```

Confirm the fingerprint matches the one from the previous release. A mismatch
means self-update will reject the new APK on devices running an older build.
