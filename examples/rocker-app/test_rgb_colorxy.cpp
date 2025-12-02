#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

// Simplified version of the symmetric RGB ↔ ColorXY conversion functions
void RGBToColorXY(uint8_t red, uint8_t green, uint8_t blue, uint16_t & colorX, uint16_t & colorY)
{
    // Convert RGB (0-255) to ColorXY using symmetric CIE 1931 transformation

    // Convert RGB to floating point (0.0-1.0)
    float r = static_cast<float>(red) / 255.0f;
    float g = static_cast<float>(green) / 255.0f;
    float b = static_cast<float>(blue) / 255.0f;

    // Apply gamma correction (inverse of gamma 2.2)
    auto gammaCorrect = [](float val) -> float { return (val <= 0.04045f) ? (val / 12.92f) : powf((val + 0.055f) / 1.055f, 2.4f); };

    r = gammaCorrect(r);
    g = gammaCorrect(g);
    b = gammaCorrect(b);

    // sRGB to XYZ transformation matrix (D65 illuminant)
    float X = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
    float Y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
    float Z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;

    // Convert XYZ to xy chromaticity coordinates
    float sum = X + Y + Z;
    float x, y;

    if (sum > 0.0f)
    {
        x = X / sum;
        y = Y / sum;
    }
    else
    {
        // Default to warm white if sum is zero
        x = 0.3127f; // D65 white point
        y = 0.3290f;
    }

    // Clamp to valid CIE 1931 gamut
    x = fmaxf(0.0f, fminf(1.0f, x));
    y = fmaxf(0.0f, fminf(1.0f, y));

    // Convert to Matter format (0-65535)
    colorX = static_cast<uint16_t>(x * 65535.0f);
    colorY = static_cast<uint16_t>(y * 65535.0f);
}

void ColorXYToRGB(uint16_t colorX, uint16_t colorY, uint8_t & red, uint8_t & green, uint8_t & blue)
{
    // Convert ColorXY to RGB using symmetric CIE 1931 transformation

    // Convert from Matter format to floating point
    float x = static_cast<float>(colorX) / 65535.0f;
    float y = static_cast<float>(colorY) / 65535.0f;
    float z = 1.0f - x - y;

    // Convert xy to XYZ (assuming Y=1 for maximum brightness)
    float Y = 1.0f;
    float X = (Y / y) * x;
    float Z = (Y / y) * z;

    // XYZ to sRGB transformation matrix (D65 illuminant)
    float r = 3.2404542f * X - 1.5371385f * Y - 0.4985314f * Z;
    float g = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
    float b = 0.0556434f * X - 0.2040259f * Y + 1.0572252f * Z;

    // Apply gamma 2.2 correction
    auto gammaCorrect = [](float val) -> float {
        return (val <= 0.0031308f) ? (val * 12.92f) : (1.055f * powf(val, 1.0f / 2.4f) - 0.055f);
    };

    r = gammaCorrect(r);
    g = gammaCorrect(g);
    b = gammaCorrect(b);

    // Clamp to valid range
    r = fmaxf(0.0f, fminf(1.0f, r));
    g = fmaxf(0.0f, fminf(1.0f, g));
    b = fmaxf(0.0f, fminf(1.0f, b));

    // Convert to RGB (0-255)
    red   = static_cast<uint8_t>(r * 255.0f);
    green = static_cast<uint8_t>(g * 255.0f);
    blue  = static_cast<uint8_t>(b * 255.0f);
}

int main()
{
    std::cout << "Testing Symmetric RGB ↔ ColorXY Conversion\n";
    std::cout << "==========================================\n\n";

    // Test colors
    struct TestColor
    {
        uint8_t r, g, b;
        const char * name;
    };

    TestColor testColors[] = { { 255, 0, 0, "Pure Red" },  { 0, 255, 0, "Pure Green" }, { 0, 0, 255, "Pure Blue" },
                               { 255, 255, 255, "White" }, { 255, 255, 0, "Yellow" },   { 255, 0, 255, "Magenta" },
                               { 0, 255, 255, "Cyan" },    { 128, 128, 128, "Gray" },   { 255, 165, 0, "Orange" },
                               { 128, 0, 128, "Purple" } };

    std::cout << std::setw(12) << "Color" << std::setw(15) << "Original RGB" << std::setw(15) << "ColorXY" << std::setw(15)
              << "Back to RGB" << std::setw(10) << "Error\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto & color : testColors)
    {
        // Original RGB
        uint8_t origR = color.r, origG = color.g, origB = color.b;

        // Convert to ColorXY
        uint16_t colorX, colorY;
        RGBToColorXY(origR, origG, origB, colorX, colorY);

        // Convert back to RGB
        uint8_t backR, backG, backB;
        ColorXYToRGB(colorX, colorY, backR, backG, backB);

        // Calculate error
        int errorR   = abs(static_cast<int>(origR) - static_cast<int>(backR));
        int errorG   = abs(static_cast<int>(origG) - static_cast<int>(backG));
        int errorB   = abs(static_cast<int>(origB) - static_cast<int>(backB));
        int maxError = std::max(errorR, std::max(errorG, errorB));

        std::cout << std::setw(12) << color.name << std::setw(15)
                  << ("(" + std::to_string(origR) + "," + std::to_string(origG) + "," + std::to_string(origB) + ")")
                  << std::setw(15) << ("(" + std::to_string(colorX) + "," + std::to_string(colorY) + ")") << std::setw(15)
                  << ("(" + std::to_string(backR) + "," + std::to_string(backG) + "," + std::to_string(backB) + ")")
                  << std::setw(10) << maxError << "\n";
    }

    std::cout << "\nNote: Error values represent the maximum difference in any RGB component.\n";
    std::cout << "Values ≤ 2 indicate excellent symmetry for 8-bit RGB precision.\n";

    return 0;
}
