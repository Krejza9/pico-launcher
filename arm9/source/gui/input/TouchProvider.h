#pragma once
#include "core/math/Point.h"
#include "sharedMemory.h"

/// @brief Provides touch input from shared memory, tracking state transitions.
class TouchProvider
{
public:
    /// @brief Samples the current touch state from shared memory.
    ///        Call this during VBlank.
    void Sample()
    {
        _sampleBuffer[_writePtr].penDown = SHARED_TOUCH_PENDOWN != 0;
        if (_sampleBuffer[_writePtr].penDown)
        {
            u32 xy = SHARED_TOUCH_XY;
            _sampleBuffer[_writePtr].position = Point(xy & 0xFFFF, xy >> 16);
        }
        _writePtr = (_writePtr + 1) & 3;
    }

    /// @brief Processes sampled touch data and updates state transitions.
    ///        Call this before handling touch input each frame.
    void Update()
    {
        _previousPenDown = _currentPenDown;

        // Process all buffered samples, keeping the latest state
        while (_readPtr != _writePtr)
        {
            auto& sample = _sampleBuffer[_readPtr];
            _currentPenDown = sample.penDown;
            if (_currentPenDown)
                _position = sample.position;
            _readPtr = (_readPtr + 1) & 3;
        }

        _triggered = _currentPenDown && !_previousPenDown;
        _released = !_currentPenDown && _previousPenDown;
    }

    /// @brief Returns true if the pen just went down this frame.
    bool Triggered() const { return _triggered; }

    /// @brief Returns true if the pen just went up this frame.
    bool Released() const { return _released; }

    /// @brief Returns true if the pen is currently down.
    bool Current() const { return _currentPenDown; }

    /// @brief Gets the current touch position in screen coordinates.
    Point GetPosition() const { return _position; }

private:
    struct TouchSample
    {
        bool penDown;
        Point position;
    };

    Point _position;
    bool _currentPenDown = false;
    bool _previousPenDown = false;
    bool _triggered = false;
    bool _released = false;

    TouchSample _sampleBuffer[4];
    u8 _readPtr = 0;
    u8 _writePtr = 0;
};
