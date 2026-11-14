package com.hollow.game

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.LinearGradient
import android.graphics.Paint
import android.graphics.Path
import android.graphics.RectF
import android.graphics.Shader
import android.view.MotionEvent
import android.view.View
import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min
import kotlin.math.sin

/**
 * First playable prototype of THE HOLLOW.
 *
 * Everything is drawn with Android Canvas so the prototype has no external
 * asset or native-library dependency. World coordinates use a virtual
 * 960x540 landscape canvas and are scaled to the device.
 */
class HollowGameView(context: Context) : View(context) {
    private companion object {
        const val VIEW_W = 960f
        const val VIEW_H = 540f
        const val PLAYER_W = 28f
        const val PLAYER_H = 54f
        const val GRAVITY = 1500f
        const val MOVE_SPEED = 255f
        const val JUMP_SPEED = -610f

        const val CONTROL_LEFT = 1
        const val CONTROL_RIGHT = 2
        const val CONTROL_JUMP = 3
    }

    private enum class GameState { MENU, PLAYING, DEAD, COMPLETE, CHAPTER_COMPLETE }

    // Simulation and gameplay rules live in the C++ core through JNI.
    private val engine = NativeGameEngine()
    private val savePrefs = context.getSharedPreferences("the_hollow_save", Context.MODE_PRIVATE)
    private var nativeCollectedMask = 0

    private data class Platform(val left: Float, val top: Float, val right: Float)

    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        textAlign = Paint.Align.CENTER
        typeface = android.graphics.Typeface.create("sans-serif", android.graphics.Typeface.BOLD)
    }
    private val platforms = listOf(
        Platform(0f, 430f, 690f),
        Platform(770f, 430f, 1160f),
        Platform(1245f, 390f, 1585f),
        Platform(1660f, 430f, 2180f),
        Platform(2260f, 430f, 2820f)
    )
    private var hazards = floatArrayOf(900f, 1040f, 1500f, 2385f)
    private val shardPositions = floatArrayOf(555f, 1370f, 1905f)
    private val collectedShards = HashSet<Int>()
    private val pointers = HashMap<Int, Int>()

    private var state = GameState.MENU
    private var playerX = 90f
    private var playerY = 365f
    private var velocityX = 0f
    private var velocityY = 0f
    private var grounded = false
    private var leftPressed = false
    private var rightPressed = false
    private var jumpPressed = false
    private var jumpLatch = false
    private var cameraX = 0f
    private var enemyX = 1110f
    private var crateX = 1720f
    private var crateOnPlate = false
    private var chapterIndex = 0
    private var switchActivated = false
    private var checkpointX = 90f
    private var elapsed = 0f
    private var lastFrameNanos = 0L
    private var deaths = 0
    private var screenScaleX = 1f
    private var screenScaleY = 1f

    init {
        engine.restoreProgress(
            checkpointX = savePrefs.getFloat("checkpoint_x", 90f),
            collectedMask = savePrefs.getInt("collected_mask", 0),
            switchActive = savePrefs.getBoolean("switch_active", false),
            crateX = savePrefs.getFloat("crate_x", 1720f),
            deaths = savePrefs.getInt("deaths", 0),
            chapterIndex = savePrefs.getInt("chapter_index", 0)
        )
        syncNativeSnapshot()
        isFocusable = true
        isClickable = true
        setLayerType(View.LAYER_TYPE_SOFTWARE, null)
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        if (width <= 0 || height <= 0) return

        screenScaleX = width / VIEW_W
        screenScaleY = height / VIEW_H

        val now = System.nanoTime()
        val delta = if (lastFrameNanos == 0L) 0f
            else ((now - lastFrameNanos) / 1_000_000_000f).coerceIn(0f, 0.033f)
        lastFrameNanos = now
        elapsed += delta

        if (state == GameState.PLAYING) updateWorld(delta)

        canvas.save()
        canvas.scale(screenScaleX, screenScaleY)
        drawBackground(canvas)

        canvas.save()
        canvas.translate(-cameraX, 0f)
        drawWorld(canvas)
        canvas.restore()

        when (state) {
            GameState.MENU -> drawMenu(canvas)
            GameState.PLAYING -> drawHudAndControls(canvas)
            GameState.DEAD -> drawEndOverlay(canvas, "A ESCURIDÃO ALCANÇOU-TE", "TOCA PARA TENTAR NOVAMENTE")
            GameState.CHAPTER_COMPLETE -> drawEndOverlay(
                canvas,
                "CAPÍTULO CONCLUÍDO",
                "TOCA PARA CONTINUAR • CAPÍTULO " + (chapterIndex + 2)
            )
            GameState.COMPLETE -> drawEndOverlay(canvas, "O FIM DO VAZIO", "TOCA PARA RECOMEÇAR")
        }
        canvas.restore()
        postInvalidateOnAnimation()
    }

    private fun updateWorld(dt: Float) {
        if (dt <= 0f) return
        engine.setInput(leftPressed, rightPressed, jumpPressed)
        engine.update(dt)
        syncNativeSnapshot()
    }

    private fun syncNativeSnapshot() {
        val snapshot = engine.snapshot()
        if (snapshot.size < 16) return

        playerX = snapshot[0]
        playerY = snapshot[1]
        velocityX = snapshot[2]
        velocityY = snapshot[3]
        grounded = snapshot[4] > 0.5f
        cameraX = snapshot[5]
        enemyX = snapshot[6]
        switchActivated = snapshot[7] > 0.5f
        checkpointX = snapshot[10]
        deaths = snapshot[11].toInt()
        crateX = snapshot[13]
        crateOnPlate = snapshot[14] > 0.5f
        chapterIndex = snapshot[15].toInt().coerceIn(0, 4)
        hazards = when (chapterIndex) {
            1 -> floatArrayOf(850f, 1080f, 1430f, 2370f)
            2 -> floatArrayOf(930f, 1110f, 1515f, 2450f)
            3 -> floatArrayOf(820f, 1010f, 1460f, 2300f)
            4 -> floatArrayOf(890f, 1085f, 1510f, 2490f)
            else -> floatArrayOf(900f, 1040f, 1500f, 2385f)
        }

        val mask = snapshot[12].toInt()
        nativeCollectedMask = mask
        collectedShards.clear()
        for (i in 0 until 3) {
            if ((mask and (1 shl i)) != 0) collectedShards.add(i)
        }

        state = when (snapshot[9].toInt()) {
            1 -> GameState.PLAYING
            2 -> GameState.DEAD
            3 -> GameState.COMPLETE
            4 -> GameState.CHAPTER_COMPLETE
            else -> GameState.MENU
        }
        saveProgress()
    }

    private fun saveProgress() {
        val unchanged =
            savePrefs.getFloat("checkpoint_x", 90f) == checkpointX &&
            savePrefs.getInt("collected_mask", 0) == nativeCollectedMask &&
            savePrefs.getBoolean("switch_active", false) == switchActivated &&
            savePrefs.getFloat("crate_x", 1720f) == crateX &&
            savePrefs.getInt("deaths", 0) == deaths &&
            savePrefs.getInt("chapter_index", 0) == chapterIndex
        if (!unchanged) {
            savePrefs.edit()
                .putFloat("checkpoint_x", checkpointX)
                .putInt("collected_mask", nativeCollectedMask)
                .putBoolean("switch_active", switchActivated)
                .putFloat("crate_x", crateX)
                .putInt("deaths", deaths)
                .putInt("chapter_index", chapterIndex)
                .apply()
        }
    }

    private fun resetGame(fromComplete: Boolean = false) {
        engine.restart(fromComplete)
        clearControls()
        syncNativeSnapshot()
        state = GameState.PLAYING
    }

    private fun clearControls() {
        pointers.clear()
        leftPressed = false
        rightPressed = false
        jumpPressed = false
        jumpLatch = false
        engine.setInput(false, false, false)
    }

    private fun drawBackground(canvas: Canvas) {
        val skyTop = when (chapterIndex) {
            1 -> Color.rgb(38, 34, 33)
            2 -> Color.rgb(42, 46, 49)
            3 -> Color.rgb(18, 22, 29)
            4 -> Color.rgb(24, 18, 35)
            else -> Color.rgb(31, 34, 39)
        }
        val skyBottom = when (chapterIndex) {
            1 -> Color.rgb(8, 8, 9)
            2 -> Color.rgb(5, 7, 8)
            3 -> Color.rgb(3, 5, 9)
            4 -> Color.rgb(1, 1, 5)
            else -> Color.rgb(5, 6, 8)
        }
        paint.shader = LinearGradient(
            0f, 0f, 0f, VIEW_H,
            skyTop,
            skyBottom,
            Shader.TileMode.CLAMP
        )
        canvas.drawRect(0f, 0f, VIEW_W, VIEW_H, paint)
        paint.shader = null

        // A pale, distant moon and sparse stars establish the monochrome mood.
        paint.color = Color.rgb(174, 178, 181)
        canvas.drawCircle(760f, 112f, 35f, paint)
        paint.color = Color.rgb(31, 34, 38)
        canvas.drawCircle(775f, 101f, 33f, paint)

        paint.color = Color.rgb(155, 160, 165)
        val stars = arrayOf(
            90f to 78f, 180f to 130f, 300f to 56f, 420f to 112f,
            540f to 65f, 650f to 155f, 850f to 70f, 925f to 190f
        )
        for ((x, y) in stars) canvas.drawCircle(x, y, 1.5f, paint)

        val farHill = Path().apply {
            moveTo(0f, 350f)
            lineTo(115f, 270f)
            lineTo(235f, 338f)
            lineTo(390f, 250f)
            lineTo(545f, 345f)
            lineTo(700f, 260f)
            lineTo(860f, 335f)
            lineTo(960f, 285f)
            lineTo(960f, 540f)
            lineTo(0f, 540f)
            close()
        }
        paint.color = Color.rgb(20, 22, 25)
        canvas.drawPath(farHill, paint)

        val nearHill = Path().apply {
            moveTo(0f, 410f)
            lineTo(150f, 350f)
            lineTo(295f, 418f)
            lineTo(470f, 340f)
            lineTo(640f, 420f)
            lineTo(815f, 355f)
            lineTo(960f, 420f)
            lineTo(960f, 540f)
            lineTo(0f, 540f)
            close()
        }
        paint.color = Color.rgb(12, 14, 17)
        canvas.drawPath(nearHill, paint)

        paint.color = Color.argb(18, 220, 225, 230)
        for (i in 0..5) {
            val y = 330f + i * 30f
            canvas.drawRect(0f, y, VIEW_W, y + 18f, paint)
        }
    }

    private fun drawWorld(canvas: Canvas) {
        // Platforms and the black void beneath them.
        paint.color = Color.rgb(5, 6, 7)
        canvas.drawRect(0f, 430f, 2820f, VIEW_H + 700f, paint)

        for (platform in platforms) {
            paint.color = Color.rgb(13, 15, 18)
            canvas.drawRect(platform.left, platform.top, platform.right, 560f, paint)
            paint.color = Color.rgb(86, 91, 96)
            paint.strokeWidth = 2f
            canvas.drawLine(platform.left, platform.top, platform.right, platform.top, paint)
            paint.color = Color.rgb(27, 30, 34)
            for (x in platform.left.toInt()..platform.right.toInt() step 42) {
                canvas.drawLine(x.toFloat(), platform.top + 9f, x.toFloat() + 18f, platform.top + 9f, paint)
            }
        }

        drawChapterProps(canvas)
        drawHazards(canvas)
        drawShards(canvas)
        drawPressurePlate(canvas)
        drawCrate(canvas)
        drawGate(canvas)
        drawShadow(canvas)
        drawPlayer(canvas)
    }

    private fun drawChapterProps(canvas: Canvas) {
        when (chapterIndex) {
            1 -> drawRuinedCity(canvas)
            2 -> drawFactory(canvas)
            3 -> drawCavern(canvas)
            4 -> drawVoidMonoliths(canvas)
            else -> drawDeadTrees(canvas)
        }
    }

    private fun drawRuinedCity(canvas: Canvas) {
        val positions = floatArrayOf(120f, 420f, 760f, 1080f, 1360f, 1760f, 2070f, 2440f, 2660f)
        for (i in positions.indices) {
            val x = positions[i]
            val height = 105f + (i % 3) * 37f
            val width = 92f + (i % 2) * 25f
            paint.color = Color.rgb(7, 8, 10)
            canvas.drawRect(x, 430f - height, x + width, 430f, paint)
            paint.color = Color.rgb(44, 48, 53)
            for (row in 0..3) {
                for (col in 0..2) {
                    val wx = x + 14f + col * 23f
                    val wy = 430f - height + 18f + row * 24f
                    if (wy < 420f) canvas.drawRect(wx, wy, wx + 7f, wy + 10f, paint)
                }
            }
        }
    }

    private fun drawFactory(canvas: Canvas) {
        val chimneys = floatArrayOf(220f, 690f, 1190f, 1580f, 2050f, 2510f)
        for (i in chimneys.indices) {
            val x = chimneys[i]
            val h = 120f + (i % 3) * 45f
            paint.color = Color.rgb(5, 7, 9)
            canvas.drawRect(x, 430f - h, x + 34f, 430f, paint)
            canvas.drawRect(x - 6f, 430f - h, x + 40f, 430f - h + 12f, paint)
            paint.color = Color.argb(45, 190, 196, 202)
            for (r in 0..2) {
                val drift = sin(elapsed * 0.7f + i + r) * 7f
                canvas.drawCircle(x + 18f + drift, 430f - h - 25f - r * 23f, 10f + r * 3f, paint)
            }
        }
        paint.color = Color.rgb(8, 10, 12)
        canvas.drawRect(0f, 330f, 2820f, 343f, paint)
        paint.color = Color.rgb(54, 59, 64)
        for (x in 0..2800 step 180) canvas.drawRect(x.toFloat(), 343f, x + 5f, 430f, paint)
    }

    private fun drawCavern(canvas: Canvas) {
        paint.color = Color.rgb(5, 6, 9)
        for (x in 80..2780 step 170) {
            val length = 50f + (x % 4) * 16f
            val stalactite = Path().apply {
                moveTo(x.toFloat(), 0f)
                lineTo(x + 28f, 0f)
                lineTo(x + 14f, length)
                close()
            }
            canvas.drawPath(stalactite, paint)
            canvas.drawRect(x + 4f, 405f, x + 23f, 430f, paint)
        }
        paint.color = Color.rgb(25, 28, 34)
        for (x in 300..2600 step 560) {
            canvas.drawRect(x.toFloat(), 0f, x + 8f, 430f, paint)
        }
    }

    private fun drawVoidMonoliths(canvas: Canvas) {
        val positions = floatArrayOf(170f, 560f, 990f, 1450f, 1880f, 2320f, 2640f)
        for (i in positions.indices) {
            val x = positions[i]
            val height = 150f + (i % 3) * 55f
            paint.color = Color.rgb(2, 2, 5)
            canvas.drawRect(x, 430f - height, x + 30f, 430f, paint)
            paint.color = Color.argb(55, 180, 185, 205)
            canvas.drawLine(x + 15f, 430f - height + 15f, x + 15f, 420f, paint)
        }
    }

    private fun drawDeadTrees(canvas: Canvas) {
        val treePositions = floatArrayOf(150f, 360f, 610f, 820f, 1120f, 1430f, 1780f, 2040f, 2460f, 2620f)
        for (i in treePositions.indices) {
            val x = treePositions[i]
            val baseY = if (x > 1200f && x < 1600f) 390f else 430f
            paint.color = Color.rgb(8, 9, 11)
            paint.strokeWidth = 7f
            canvas.drawLine(x, baseY, x - 10f, baseY - 105f, paint)
            canvas.drawLine(x - 7f, baseY - 68f, x - 58f, baseY - 112f, paint)
            canvas.drawLine(x - 4f, baseY - 86f, x + 42f, baseY - 138f, paint)
            canvas.drawLine(x - 8f, baseY - 54f, x + 38f, baseY - 78f, paint)
        }
    }

    private fun drawHazards(canvas: Canvas) {
        for (x in hazards) {
            val baseY = if (x >= 1245f && x < 1585f) 390f else 430f
            val tipY = baseY - 40f
            val path = Path().apply {
                moveTo(x, baseY)
                lineTo(x + 20f, tipY)
                lineTo(x + 43f, baseY)
                close()
            }
            paint.color = Color.rgb(2, 2, 3)
            canvas.drawPath(path, paint)
            paint.color = Color.rgb(90, 94, 98)
            paint.strokeWidth = 1.5f
            canvas.drawLine(x + 20f, tipY + 1f, x + 30f, tipY + 23f, paint)
        }
    }

    private fun drawShards(canvas: Canvas) {
        for (i in shardPositions.indices) {
            if (collectedShards.contains(i)) continue
            val x = shardPositions[i]
            val pulse = 1f + sin(elapsed * 3f + i) * 0.12f
            canvas.save()
            canvas.translate(x, 350f)
            canvas.scale(pulse, pulse)
            val shard = Path().apply {
                moveTo(0f, -16f)
                lineTo(11f, 0f)
                lineTo(0f, 18f)
                lineTo(-11f, 0f)
                close()
            }
            paint.color = Color.rgb(205, 208, 211)
            canvas.drawPath(shard, paint)
            paint.color = Color.argb(90, 255, 255, 255)
            canvas.drawCircle(-2f, -2f, 3f, paint)
            canvas.restore()
        }
    }

    private fun drawPressurePlate(canvas: Canvas) {
        paint.color = if (switchActivated) Color.rgb(190, 194, 197) else Color.rgb(45, 49, 54)
        canvas.drawRoundRect(RectF(1760f, 423f, 1815f, 430f), 2f, 2f, paint)
        paint.color = Color.argb(70, 225, 230, 235)
        canvas.drawRect(1770f, 409f, 1805f, 423f, paint)
    }

    private fun drawCrate(canvas: Canvas) {
        // Heavy wooden crate with pale metal corners; its position comes from C++.
        paint.color = Color.rgb(8, 9, 11)
        canvas.drawRoundRect(RectF(crateX, 391f, crateX + 42f, 430f), 2f, 2f, paint)
        paint.color = if (crateOnPlate) Color.rgb(150, 155, 160) else Color.rgb(76, 81, 86)
        paint.style = Paint.Style.STROKE
        paint.strokeWidth = 2f
        canvas.drawRect(crateX + 4f, 395f, crateX + 38f, 426f, paint)
        canvas.drawLine(crateX + 5f, 397f, crateX + 37f, 424f, paint)
        canvas.drawLine(crateX + 37f, 397f, crateX + 5f, 424f, paint)
        paint.style = Paint.Style.FILL
    }

    private fun drawGate(canvas: Canvas) {
        if (switchActivated) {
            paint.color = Color.argb(90, 170, 176, 182)
            canvas.drawRect(2115f, 350f, 2158f, 430f, paint)
            return
        }
        paint.color = Color.rgb(3, 4, 5)
        canvas.drawRect(2115f, 300f, 2158f, 430f, paint)
        paint.color = Color.rgb(100, 105, 111)
        paint.strokeWidth = 4f
        canvas.drawLine(2120f, 303f, 2120f, 429f, paint)
        canvas.drawLine(2152f, 303f, 2152f, 429f, paint)
        paint.strokeWidth = 2f
        for (y in 315..420 step 22) {
            canvas.drawLine(2118f, y.toFloat(), 2155f, y.toFloat(), paint)
        }
    }

    private fun drawShadow(canvas: Canvas) {
        val bob = sin(elapsed * 4f) * 4f
        paint.color = Color.rgb(1, 1, 2)
        canvas.drawOval(RectF(enemyX - 23f, 360f + bob, enemyX + 25f, 432f + bob), paint)
        canvas.drawCircle(enemyX, 350f + bob, 19f, paint)
        paint.color = Color.rgb(120, 124, 128)
        canvas.drawCircle(enemyX - 7f, 348f + bob, 2.5f, paint)
        canvas.drawCircle(enemyX + 7f, 348f + bob, 2.5f, paint)
        paint.color = Color.rgb(2, 2, 3)
        canvas.drawRect(enemyX - 29f, 402f + bob, enemyX - 18f, 438f + bob, paint)
        canvas.drawRect(enemyX + 17f, 402f + bob, enemyX + 29f, 438f + bob, paint)
    }

    private fun drawPlayer(canvas: Canvas) {
        val run = if (abs(velocityX) > 1f && grounded) sin(elapsed * 15f) * 5f else 0f
        val centerX = playerX + PLAYER_W / 2f
        val headY = playerY + 11f
        paint.color = Color.rgb(1, 2, 3)
        canvas.drawOval(RectF(playerX + 1f, playerY + 8f, playerX + PLAYER_W - 1f, playerY + PLAYER_H), paint)
        canvas.drawCircle(centerX, headY, 11f, paint)
        paint.color = Color.rgb(175, 180, 185)
        canvas.drawCircle(centerX + 4f, headY - 1f, 1.8f, paint)
        paint.color = Color.rgb(2, 3, 4)
        canvas.drawLine(centerX - 5f, playerY + 39f, centerX - 8f - run, playerY + 53f, paint)
        canvas.drawLine(centerX + 5f, playerY + 39f, centerX + 9f + run, playerY + 53f, paint)
    }

    private fun drawHudAndControls(canvas: Canvas) {
        textPaint.textAlign = Paint.Align.LEFT
        textPaint.textSize = 17f
        textPaint.color = Color.argb(210, 235, 238, 240)
        canvas.drawText("THE HOLLOW", 28f, 36f, textPaint)
        textPaint.textSize = 12f
        textPaint.color = Color.rgb(182, 187, 192)
        val chapterTitles = arrayOf("A FLORESTA MORTA", "AS RUÍNAS", "A FÁBRICA", "O SUBSOLO", "O VAZIO")
        canvas.drawText("CAPÍTULO " + (chapterIndex + 1) + " — " + chapterTitles[chapterIndex], 180f, 36f, textPaint)
        textPaint.textSize = 13f
        textPaint.color = Color.rgb(160, 166, 171)
        canvas.drawText("FRAGMENTOS: " + collectedShards.size + " / 3", 29f, 58f, textPaint)
        if (!switchActivated) {
            textPaint.textAlign = Paint.Align.RIGHT
            canvas.drawText("EMPURRA A CAIXA ATÉ À PLACA", 930f, 38f, textPaint)
        } else {
            textPaint.textAlign = Paint.Align.RIGHT
            canvas.drawText("O PORTÃO ABRIU", 930f, 38f, textPaint)
        }

        drawControl(canvas, 82f, 465f, "◀", leftPressed)
        drawControl(canvas, 190f, 465f, "▶", rightPressed)
        drawControl(canvas, 865f, 465f, "▲", jumpPressed)
        textPaint.textAlign = Paint.Align.CENTER
        textPaint.textSize = 11f
        textPaint.color = Color.argb(140, 225, 228, 230)
        canvas.drawText("MOVER", 136f, 525f, textPaint)
        canvas.drawText("SALTAR", 865f, 525f, textPaint)
    }

    private fun drawControl(canvas: Canvas, x: Float, y: Float, symbol: String, active: Boolean) {
        paint.color = if (active) Color.argb(110, 220, 225, 230) else Color.argb(48, 220, 225, 230)
        canvas.drawCircle(x, y, 38f, paint)
        paint.style = Paint.Style.STROKE
        paint.strokeWidth = 1.5f
        paint.color = Color.argb(125, 220, 225, 230)
        canvas.drawCircle(x, y, 38f, paint)
        paint.style = Paint.Style.FILL
        textPaint.textAlign = Paint.Align.CENTER
        textPaint.textSize = 29f
        textPaint.color = Color.rgb(235, 238, 240)
        canvas.drawText(symbol, x, y + 10f, textPaint)
    }

    private fun drawMenu(canvas: Canvas) {
        paint.color = Color.argb(125, 0, 0, 0)
        canvas.drawRect(0f, 0f, VIEW_W, VIEW_H, paint)
        textPaint.textAlign = Paint.Align.CENTER
        textPaint.color = Color.rgb(232, 235, 238)
        textPaint.textSize = 58f
        canvas.drawText("THE HOLLOW", 480f, 205f, textPaint)
        textPaint.textSize = 16f
        textPaint.color = Color.rgb(166, 171, 176)
        canvas.drawText("NÃO SIGAS A LUZ. ESCUTA O SILÊNCIO.", 480f, 246f, textPaint)
        textPaint.textSize = 19f
        textPaint.color = Color.WHITE
        canvas.drawText("TOCA PARA COMEÇAR", 480f, 350f, textPaint)
        textPaint.textSize = 12f
        textPaint.color = Color.rgb(145, 150, 155)
        canvas.drawText("UM JOGO DE EXPLORAÇÃO E SOBREVIVÊNCIA", 480f, 390f, textPaint)
    }

    private fun drawEndOverlay(canvas: Canvas, title: String, subtitle: String) {
        paint.color = Color.argb(190, 0, 0, 0)
        canvas.drawRect(0f, 0f, VIEW_W, VIEW_H, paint)
        textPaint.textAlign = Paint.Align.CENTER
        textPaint.textSize = 30f
        textPaint.color = Color.rgb(230, 233, 235)
        canvas.drawText(title, 480f, 225f, textPaint)
        textPaint.textSize = 17f
        textPaint.color = Color.rgb(180, 185, 190)
        canvas.drawText(subtitle, 480f, 285f, textPaint)
        textPaint.textSize = 12f
        canvas.drawText("FRAGMENTOS: " + collectedShards.size + " / 3    •    QUEDAS: " + deaths, 480f, 325f, textPaint)
    }

    private fun controlAt(x: Float, y: Float): Int {
        if (y < 385f) return 0
        return when {
            x < 135f -> CONTROL_LEFT
            x < 270f -> CONTROL_RIGHT
            x > 720f -> CONTROL_JUMP
            else -> 0
        }
    }

    private fun refreshControls() {
        leftPressed = pointers.values.contains(CONTROL_LEFT)
        rightPressed = pointers.values.contains(CONTROL_RIGHT)
        jumpPressed = pointers.values.contains(CONTROL_JUMP)
        engine.setInput(leftPressed, rightPressed, jumpPressed)
    }

    override fun onDetachedFromWindow() {
        clearControls()
        saveProgress()
        engine.close()
        super.onDetachedFromWindow()
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val action = event.actionMasked
        if (state == GameState.MENU && action == MotionEvent.ACTION_DOWN) {
            engine.start()
            syncNativeSnapshot()
            state = GameState.PLAYING
            lastFrameNanos = 0L
            return true
        }
        if (state == GameState.CHAPTER_COMPLETE && action == MotionEvent.ACTION_DOWN) {
            engine.nextChapter()
            syncNativeSnapshot()
            lastFrameNanos = 0L
            return true
        }
        if ((state == GameState.DEAD || state == GameState.COMPLETE) &&
            action == MotionEvent.ACTION_DOWN
        ) {
            resetGame(fromComplete = state == GameState.COMPLETE)
            lastFrameNanos = 0L
            return true
        }

        when (action) {
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                val index = event.actionIndex
                val x = event.getX(index) / screenScaleX
                val y = event.getY(index) / screenScaleY
                pointers[event.getPointerId(index)] = controlAt(x, y)
                refreshControls()
            }
            MotionEvent.ACTION_MOVE -> {
                for (index in 0 until event.pointerCount) {
                    val x = event.getX(index) / screenScaleX
                    val y = event.getY(index) / screenScaleY
                    pointers[event.getPointerId(index)] = controlAt(x, y)
                }
                refreshControls()
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                pointers.remove(event.getPointerId(event.actionIndex))
                refreshControls()
            }
            MotionEvent.ACTION_CANCEL -> clearControls()
        }
        return true
    }
}
