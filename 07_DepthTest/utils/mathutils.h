#pragma once

/// @brief Storing a const for Pi
constexpr float CONST_PI = 3.141592654f;

/// @brief Utility function to convert units in degrees to radians
/// @param degs Degrees to convert to radians
/// @return radians
constexpr float degreesToRadians(float degs)
{
    return degs * (CONST_PI / 180.0f);
}

/// @brief Utility function to clamp float values
/// @param in input value
/// @param low low range to clamp to
/// @param high high range to clamp to
/// @return clamped value
constexpr float clamp(const float in, const float low, const float high)
{
    return in < low ? low : in > high ? high
                                      : in;
}
