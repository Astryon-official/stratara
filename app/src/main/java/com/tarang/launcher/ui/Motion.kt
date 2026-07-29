package com.tarang.launcher.ui

import androidx.compose.animation.core.AnimationSpec
import androidx.compose.animation.core.CubicBezierEasing
import androidx.compose.animation.core.spring
import androidx.compose.animation.core.tween
import androidx.compose.ui.graphics.GraphicsLayerScope
import androidx.compose.ui.graphics.TransformOrigin
import com.tarang.launcher.data.AnimStyle
import kotlin.math.abs
import kotlin.math.pow

/**
 * The motion vocabulary for the four big transitions (enter/exit Frame Art, launch/return an app),
 * factored out of [LauncherScreen] so each [AnimStyle] is one coherent, comparable package of
 * timing + transform. The launcher drives two chrome layers on shared timelines (the dock leads, the
 * top bar trails); this file decides, per style, HOW those layers move and how fast.
 *
 * Progress convention everywhere: 0f = home (chrome fully present), 1f = gone (Frame Art / app open).
 */

/** The two chrome layers the launcher animates independently. */
enum class ChromeLayer { TOP_BAR, DOCK }

// Easings shared across styles. StandardEase is Material/iOS-ish accelerate-then-settle; Decel is a
// pure ease-out (rush in, glide to rest — used on returns); Accel is ease-in (start slow, fly away).
private val StandardEase = CubicBezierEasing(0.4f, 0.0f, 0.2f, 1f)
private val DecelEase = CubicBezierEasing(0.0f, 0.0f, 0.2f, 1f)
private val AccelEase = CubicBezierEasing(0.4f, 0.0f, 1f, 1f)

// A gentle ease-out-back (softened from the classic 1.56 overshoot): the value rides just past its
// target and settles back. DEPTH uses it on the *settling* direction so chrome lands home with a
// subtle bounce. The overshoot only reads on the return/exit move — on the way out the chrome has
// already faded to alpha 0 before the tail, so it stays invisible there.
private val OvershootEase = CubicBezierEasing(0.34f, 1.45f, 0.64f, 1f)

// GLIDE's progress (0..1) is multiplied by the box height (~950px) to drive translation, so a spring's
// default 0.01 visibility threshold would let it "settle" a visible ~9px from the target and then snap
// there on the last frame. A sub-pixel threshold makes the tail land smoothly on the resting position.
private const val GLIDE_THRESHOLD = 0.0002f

// GLIDE felt too fast on real TV hardware, so slow every glide spring ~2.5×. A spring's settling time
// scales with 1/√stiffness, so 2.5× slower ≈ stiffness ÷ 2.5² (÷6.25). Dividing here (and keeping the
// original stiffness numbers at the call sites) preserves the relative pacing between the layers.
private const val GLIDE_SLOWDOWN = 6.25f

private fun glideSpring(dampingRatio: Float, stiffness: Float): AnimationSpec<Float> =
    spring(dampingRatio, stiffness / GLIDE_SLOWDOWN, visibilityThreshold = GLIDE_THRESHOLD)

// ---------------------------------------------------------------------------------------------------
// Timing — the AnimationSpecs the launcher's Animatables run on. Frame transitions are calm/slow; app
// launches are quick. BASELINE/DEPTH use tuned tweens; GLIDE uses springs so it decelerates
// naturally and stays interruptible.
// ---------------------------------------------------------------------------------------------------

/** Master progress spec (drives clock reveal, art crossfade + all the gating). */
fun frameMasterSpec(style: AnimStyle): AnimationSpec<Float> = when (style) {
    AnimStyle.BASELINE -> tween(1700, easing = StandardEase)
    AnimStyle.GLIDE -> glideSpring(dampingRatio = 1f, stiffness = 130f)
    AnimStyle.DEPTH -> tween(1100, easing = StandardEase)
}

/** Dock layer during a Frame Art enter/exit (the dock leads — shortest/stiffest). */
fun frameDockSpec(style: AnimStyle): AnimationSpec<Float> = when (style) {
    AnimStyle.BASELINE -> tween(1200, easing = StandardEase)
    AnimStyle.GLIDE -> glideSpring(dampingRatio = 1f, stiffness = 190f)
    AnimStyle.DEPTH -> tween(900, easing = OvershootEase)
}

/** Top bar layer during a Frame Art enter/exit (trails the dock — longest/softest). */
fun frameTopBarSpec(style: AnimStyle): AnimationSpec<Float> = when (style) {
    AnimStyle.BASELINE -> tween(1500, easing = StandardEase)
    AnimStyle.GLIDE -> glideSpring(dampingRatio = 1f, stiffness = 110f)
    AnimStyle.DEPTH -> tween(1100, easing = OvershootEase)
}

// Debug multiplier on DEPTH's launch/return timings: 1f in normal use; raise it (e.g. to 4f) to
// slow-motion the dock ripple for on-device inspection. The real durations live in the specs below.
const val DEPTH_LAUNCH_SLOWDOWN = 1f

/** How long [LauncherScreen] waits after starting the DEPTH launch animation before actually starting
 *  the app. 0 = launch immediately (the app's own start-up keeps the ripple visible anyway); raise it
 *  toward the top-bar duration to watch the full move during motion debugging. */
const val DEPTH_LAUNCH_HOLD_MS = 0L

/** Dock layer during an app launch ([entering]) / return (!entering). */
fun launchDockSpec(style: AnimStyle, entering: Boolean): AnimationSpec<Float> = when (style) {
    AnimStyle.BASELINE -> tween(600, easing = if (entering) AccelEase else DecelEase)
    AnimStyle.GLIDE -> glideSpring(dampingRatio = if (entering) 1f else 0.82f, stiffness = 340f)
    // Return dives back with a subtle overshoot; the launch itself still accelerates away. Tuned
    // on-device for the dock ripple (stagger + lift + burst needs more room than a flat dissolve).
    AnimStyle.DEPTH -> tween(
        ((if (entering) 850 else 1000) * DEPTH_LAUNCH_SLOWDOWN).toInt(),
        easing = if (entering) AccelEase else OvershootEase,
    )
}

/** Top bar layer during an app launch / return. */
fun launchTopBarSpec(style: AnimStyle, entering: Boolean): AnimationSpec<Float> = when (style) {
    AnimStyle.BASELINE -> tween(900, easing = if (entering) AccelEase else DecelEase)
    AnimStyle.GLIDE -> glideSpring(dampingRatio = if (entering) 1f else 0.9f, stiffness = 240f)
    AnimStyle.DEPTH -> tween(
        ((if (entering) 930 else 1140) * DEPTH_LAUNCH_SLOWDOWN).toInt(),
        easing = if (entering) AccelEase else OvershootEase,
    )
}

// ---------------------------------------------------------------------------------------------------
// Transforms — applied inside the chrome layers' graphicsLayer blocks. [frameP] and [launchP] are the
// two progresses; in practice only one is non-zero at a time (you can't launch an app mid-frame), so
// styles that want different behaviour for the two moves (DEPTH) just branch on which is active.
// ---------------------------------------------------------------------------------------------------

/**
 * Shape the chrome for the current style. Called from the layer's `graphicsLayer {}` where `size` is
 * the layer's own size.
 */
fun GraphicsLayerScope.applyChrome(
    style: AnimStyle,
    layer: ChromeLayer,
    frameP: Float,
    launchP: Float,
) {
    when (style) {
        AnimStyle.BASELINE -> baseline(layer, maxOf(frameP, launchP))
        AnimStyle.GLIDE -> glide(layer, maxOf(frameP, launchP))
        AnimStyle.DEPTH -> depth(layer, frameP, launchP)
    }
}

/** The shipped motion: dock scales up 1.28× and drops off the bottom, top bar rises off the top. */
private fun GraphicsLayerScope.baseline(layer: ChromeLayer, p: Float) {
    when (layer) {
        ChromeLayer.TOP_BAR -> {
            translationY = -p * (size.height + 48f)
            alpha = 1f - (p * 1.7f).coerceAtMost(1f)
        }
        ChromeLayer.DOCK -> {
            val s = 1f + 0.28f * p
            scaleX = s
            scaleY = s
            translationY = p * size.height * 0.55f
            alpha = 1f - (p * 1.7f).coerceAtMost(1f)
            transformOrigin = TransformOrigin(0.5f, 0.85f)
        }
    }
}

/** Fluid glide: pure slide-off + fade (no scale-up), the spring timing does the work. */
private fun GraphicsLayerScope.glide(layer: ChromeLayer, p: Float) {
    when (layer) {
        ChromeLayer.TOP_BAR -> {
            translationY = -p * (size.height + 48f)
            alpha = 1f - (p * 1.6f).coerceAtMost(1f)
        }
        ChromeLayer.DOCK -> {
            // A whisper of scale-down as it leaves, so it settles rather than just translating.
            val s = 1f - 0.04f * p
            scaleX = s
            scaleY = s
            translationY = p * size.height * 0.95f
            alpha = 1f - (p * 1.5f).coerceAtMost(1f)
            transformOrigin = TransformOrigin(0.5f, 1f)
        }
    }
}

/** Z-axis depth: recede (scale <1) toward Frame Art; approach (scale >1) into an app. */
private fun GraphicsLayerScope.depth(layer: ChromeLayer, frameP: Float, launchP: Float) {
    val p = maxOf(frameP, launchP)
    alpha = 1f - (p * 1.5f).coerceAtMost(1f)
    // No blur: an animated RenderEffect blur on these full-screen layers is far too heavy on weak TV
    // GPUs (it re-blurs every frame — janks badly on the Chromecast). The scale recede/approach + fade,
    // plus the art rising forward, carry the depth on their own at essentially no GPU cost.
    // recede on frame-enter (down to 0.9), approach on launch (up to ~1.14). Mutually exclusive.
    val recede = 1f - 0.10f * frameP
    val approach = 1f + 0.14f * launchP
    val s = recede * approach
    scaleX = s
    scaleY = s
    when (layer) {
        ChromeLayer.TOP_BAR -> {
            translationY = -frameP * 24f - launchP * 30f
            transformOrigin = TransformOrigin(0.5f, 0f)
        }
        ChromeLayer.DOCK -> {
            translationY = frameP * 16f + launchP * 26f
            transformOrigin = TransformOrigin(0.5f, 0.55f)
        }
    }
}

/**
 * The incoming Frame Art's entrance scale for DEPTH (the art rises forward from a hair larger). [p] is
 * the master frame progress. Returns 1f (no scale) for styles that just crossfade the art.
 */
fun artEntryScale(style: AnimStyle, p: Float): Float = when (style) {
    AnimStyle.DEPTH -> 1.06f - 0.06f * p
    else -> 1f
}

// ---------------------------------------------------------------------------------------------------
// DEPTH dock ripple — launching from the dock, the chosen tile leads: it rises toward the viewer
// first, then its neighbours ring by ring (distance 1 on each side, then 2, …), and the dock chrome
// (with the grid) trails last. On the return the same mapping runs backwards, so the chrome re-forms
// first and the launched tile lands last. Grid launches keep the uniform transform.
// ---------------------------------------------------------------------------------------------------

/** Fraction of the launch timeline across which the ripple's start times are spread; every element
 *  then ramps over the remaining (1 - spread), so the last one still finishes exactly on time. */
private const val RIPPLE_SPREAD = 0.45f

/** Floor for the master's return overshoot (OvershootEase dips the progress a hair below 0), kept so
 *  the landing bounce survives the per-slot clamping. */
private const val RIPPLE_DIP = -0.2f

/**
 * Maps the master dock-launch progress (read lazily via [progress], so it can be sampled per frame
 * in a graphicsLayer block) onto staggered per-tile ramps. [origin] is the launched tile's index in
 * a dock of [count] tiles.
 */
class DockRipple(val origin: Int, count: Int, private val progress: () -> Float) {

    // Ring slots 0..maxRing for the tiles; one more slot after them for the dock chrome.
    private val slots = maxOf(origin, count - 1 - origin) + 1

    private fun staged(slot: Int): Float {
        val p = progress()
        // At/past home (including the return's overshoot dip) everything moves together.
        if (p <= 0f) return p.coerceAtLeast(RIPPLE_DIP)
        val start = RIPPLE_SPREAD * slot / slots
        return ((p - start) / (1f - RIPPLE_SPREAD)).coerceIn(0f, 1f)
    }

    /** The staggered progress for dock tile [index]. */
    fun tileProgress(index: Int): Float = staged(abs(index - origin))

    /** How much tile [index] grows: the launched tile is the hero (1.5×); each ring outward carries
     *  less energy than the one before, so the far tiles mostly drift and dissolve. */
    fun tileGrowth(index: Int): Float = growthOfRing(abs(index - origin))

    /**
     * Signed horizontal spread for tile [index], in tile widths: each inner ring contributes its
     * progress weighted by the mean growth of the pair it separates, which pushes the tile outward
     * by at least the room its inner neighbours' growth consumes — the dock bursts open around the
     * launched app and the rising tiles never merge. Because inner rings always lead outer ones,
     * the push can only widen the gaps (and the return re-packs in the same order).
     */
    fun tileSpread(index: Int): Float {
        val d = abs(index - origin)
        var sum = 0f
        for (k in 0 until d) {
            val pairGrowth = (growthOfRing(k) + growthOfRing(k + 1)) / 2f
            sum += pairGrowth * staged(k).coerceAtLeast(0f)
        }
        return if (index >= origin) sum else -sum
    }

    // The wave loses energy as it spreads: ring 1 grows by RIPPLE_GROWTH, each further ring by
    // RIPPLE_DECAY of the previous one (0.50 → 0.22 → 0.14 → 0.09 → …).
    private fun growthOfRing(ring: Int): Float = when (ring) {
        0 -> RIPPLE_GROWTH_ORIGIN
        else -> RIPPLE_GROWTH * RIPPLE_DECAY.pow(ring - 1)
    }

    /** The trailing progress for the dock chrome layer (frosted bar + grid). */
    fun chromeProgress(): Float = staged(slots)
}

/** How much the first ring (the hero's direct neighbours) grows (scale goes to 1 + this). */
private const val RIPPLE_GROWTH = 0.22f

/** Growth carried over from each ring to the next — the wave's energy falloff. */
private const val RIPPLE_DECAY = 0.65f

/** How much the launched tile itself grows — the hero of the move. */
private const val RIPPLE_GROWTH_ORIGIN = 0.50f

/** Per-tile transform for the dock ripple: the tile lifts up out of the dock plane while it grows —
 *  staying fully opaque through the rise so the depth reads — then dissolves on the way out. MULTIPLIES
 *  into the layer's current scale/alpha and must run inside the tile's OWN graphicsLayer (the one that
 *  applies the focus scale): alpha < 1 makes a layer composite offscreen clipped to its bounds, so a
 *  separate wrapper layer would crop the focus-scale overflow into a square. */
fun GraphicsLayerScope.applyDockRippleTile(p: Float, spread: Float, growth: Float) {
    val visP = p.coerceAtLeast(0f)
    // Opaque through the first 35% of the rise, fully dissolved at 80% — growth first, then the fade.
    alpha *= 1f - ((visP - 0.35f) / 0.45f).coerceIn(0f, 1f)
    val s = 1f + growth * p
    scaleX *= s
    scaleY *= s
    // The lift off the dock: rise by a third of the tile height as it grows (and dip past home on the
    // return's overshoot, which is the landing bounce). The spread pushes the tile clear of its
    // growing inner neighbours (see DockRipple.tileSpread).
    translationY += -0.33f * size.height * p
    translationX += spread * size.width
}
