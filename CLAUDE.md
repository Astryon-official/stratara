# Tarang Launcher — guide for Claude

Tarang is a minimal, tvOS-inspired Android TV launcher. It uses Kotlin, Jetpack
Compose for TV, MVVM, and DataStore. You build it from the command line with
Gradle. You deploy it to a real Chromecast with Google TV ("sabrina", Android
14, 32-bit `armeabi-v7a`). The release APK is signed with the debug key, because
this is a personal launcher and not a Play Store app.

## Writing style: ASD-STE100 Simplified Technical English

Write all text in ASD-STE100 Simplified Technical English (STE). This includes
documentation, code comments, commit messages, UI copy, and your replies.

Obey these rules:

- Write short sentences. Use a maximum of 20 words in a procedure sentence.
- Write one instruction in one sentence.
- Use the active voice.
- Start a procedure sentence with the verb (the command).
- Use simple verb tenses: present, past, or future.
- Use one approved meaning for each word. Do not use a word as more than one
  part of speech.
- Do not use slang or unnecessary jargon. Keep technical names and technical
  verbs.
- Use the articles "a", "an", and "the" where possible.

## Build a release APK that does NOT crawl

The Chromecast install is a sideload. It gets no Play install-time AOT
compilation. If you install the APK and do no more, ART can keep the app at
compilation `status=verify` (interpreted). Then the launcher is very slow ("it
crawls"). You must apply the baseline profile and then compile the app.

### Rule 1 — Generate the baseline profile ONLY on the Pixel AVD

NEVER run `generateBaselineProfile` against the Chromecast. The macrobenchmark
does `pm clear` between iterations. This wipes the app DataStore (favourites,
wallpaper, and all settings). You cannot recover that data. It can also leave
the app at `status=verify`, which makes it very slow.

Generate the profile on the `whyberlin_pixel5` AVD. This AVD is a rootable
`-userdebug` build. The profile is device-agnostic, so the AVD profile is
correct for the Chromecast. The `tarang_tv` AVD is a `-user` build and cannot
capture the profile.

Do these steps:

1. Disconnect the Chromecast and stop mDNS auto-connect:
   ```
   adb disconnect
   ADB_MDNS=0 ADB_MDNS_OPENSCREEN=0 adb kill-server && adb start-server
   ```
2. Start the AVD:
   ```
   ~/Library/Android/sdk/emulator/emulator -avd whyberlin_pixel5 -no-snapshot-save &
   ```
3. Wait for the boot to complete:
   ```
   adb wait-for-device shell 'while [[ -z $(getprop sys.boot_completed) ]]; do sleep 2; done'
   ```
4. Generate the profile. Pin the task to the AVD:
   ```
   ANDROID_SERIAL=emulator-5554 ./gradlew :app:generateReleaseBaselineProfile
   ```
5. Build the release APK:
   ```
   ./gradlew :app:assembleRelease
   ```

The APK is at `app/build/outputs/apk/release/app-release.apk`. Confirm that the
profile is in the APK:
```
unzip -l app/build/outputs/apk/release/app-release.apk | grep baseline.prof
```

### Rule 2 — Deploy and force compilation

The Chromecast wireless-debugging port changes. Discover it first.

1. Find the port and connect:
   ```
   adb mdns services            # find the _adb-tls-connect._tcp entry
   adb connect <ip:port>        # e.g. 192.168.0.164:39985
   ```
   If the connection is refused, pair again. Ask Mohit for the 6-digit code and
   the pair port from "Wireless debugging > Pair device with pairing code":
   ```
   adb pair <ip:pairPort> <code>
   ```
2. Install the APK:
   ```
   adb -s <ip:port> install -r app/build/outputs/apk/release/app-release.apk
   ```
3. Apply the baseline profile now. Do not wait for the idle dexopt:
   ```
   adb -s <ip:port> shell am broadcast \
     -a androidx.profileinstaller.action.INSTALL_PROFILE \
     -n com.tarang.launcher/androidx.profileinstaller.ProfileInstallReceiver
   ```
   The result `result=1` means success.
4. Compile the app to native code:
   ```
   adb -s <ip:port> shell cmd package compile -m speed -f com.tarang.launcher
   ```
5. Restart the launcher:
   ```
   adb -s <ip:port> shell am force-stop com.tarang.launcher
   adb -s <ip:port> shell am start -n com.tarang.launcher/.home.HomeActivity
   ```

### Rule 3 — Verify that it will not crawl

Read the compilation status:
```
adb -s <ip:port> shell dumpsys package dexopt | grep -A2 com.tarang.launcher
```

- `status=speed` or `status=speed-profile` means the app is compiled. This is
  good.
- `status=verify` means the app is interpreted. This is bad. Do Rule 2, steps 3
  to 5, again.

### Recovery — the installed app already crawls

You do not need to reinstall. Do Rule 2, steps 3 to 5. Then verify with Rule 3.

`pm clear` and this recovery do not touch external storage. A wallpaper photo in
`/sdcard/Pictures` stays on the device. Ask Mohit to select it again in
Settings > Appearance > Wallpaper.

## Version bump and release

1. Change `versionCode` and `versionName` in `app/build.gradle.kts`.
2. Commit the code first, then the version bump, then the baseline profile
   (`app/src/release/generated/baselineProfiles/`).
3. Create the GitHub release:
   ```
   gh release create vX.Y.Z <apk> --title "vX.Y.Z" --notes "..."
   ```

## Testing on the Chromecast

Ask Mohit to confirm UI changes on the TV. Do not drive the UI with
`adb keyevent` or screenshots. To install and to launch the app is OK.

## Clean up

Kill the AVD when the baseline work is complete:
```
adb -s emulator-5554 emu kill
```
Restart the adb server to restore the default mDNS state:
```
adb kill-server && adb start-server
```
