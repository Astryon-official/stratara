package com.tarang.launcher.media

import android.content.Context
import android.media.AudioAttributes
import android.media.SoundPool
import com.tarang.launcher.R

/**
 * The launcher's tvOS-style navigation sounds, played through one shared [SoundPool]:
 * [navigate] on a D-pad move, [click] on activating something, [back] on closing/leaving a surface.
 *
 * Playing while a sound is still loading (only possible in the first moments of a cold start) is a
 * silent no-op, and [enabled] (driven by the "Navigation sounds" setting) gates everything.
 */
class UiSounds(context: Context) {

    var enabled: Boolean = true

    private val pool = SoundPool.Builder()
        .setMaxStreams(3) // rapid D-pad ticks may overlap; let them ring out over each other
        .setAudioAttributes(
            AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_ASSISTANCE_SONIFICATION)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build(),
        )
        .build()

    private val navigateId = pool.load(context, R.raw.navigate, 1)
    private val clickId = pool.load(context, R.raw.click, 1)
    private val backId = pool.load(context, R.raw.back, 1)

    fun navigate() = play(navigateId)
    fun click() = play(clickId)
    fun back() = play(backId)

    private fun play(id: Int) {
        if (enabled) pool.play(id, VOLUME, VOLUME, 1, 0, 1f)
    }

    private companion object {
        // The ticks should sit under the content's audio, not compete with it.
        const val VOLUME = 0.6f
    }
}
