package com.hollow.game

/**
 * Thin JNI bridge to the C++ simulation. The native core owns gameplay state;
 * Android Canvas is only responsible for drawing the snapshot.
 */
class NativeGameEngine {
    companion object {
        init {
            System.loadLibrary("hollow")
        }
    }

    private var handle: Long = nativeCreate()

    private external fun nativeCreate(): Long
    private external fun nativeDestroy(handle: Long)
    private external fun nativeStart(handle: Long)
    private external fun nativeSetInput(
        handle: Long,
        left: Boolean,
        right: Boolean,
        jump: Boolean
    )
    private external fun nativeUpdate(handle: Long, deltaSeconds: Float)
    private external fun nativeRestart(handle: Long, fromComplete: Boolean)
    private external fun nativeSnapshot(handle: Long): FloatArray

    fun start() {
        if (handle != 0L) nativeStart(handle)
    }

    fun setInput(left: Boolean, right: Boolean, jump: Boolean) {
        if (handle != 0L) nativeSetInput(handle, left, right, jump)
    }

    fun update(deltaSeconds: Float) {
        if (handle != 0L) nativeUpdate(handle, deltaSeconds)
    }

    fun restart(fromComplete: Boolean) {
        if (handle != 0L) nativeRestart(handle, fromComplete)
    }

    fun snapshot(): FloatArray {
        return if (handle != 0L) nativeSnapshot(handle) else FloatArray(13)
    }

    fun close() {
        if (handle != 0L) {
            nativeDestroy(handle)
            handle = 0L
        }
    }
}
