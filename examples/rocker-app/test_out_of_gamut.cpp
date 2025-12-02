#include <algorithm>
#include <cmath>
#include <iostream>

// Replicate the ColorXYToRGB function to test out-of-gamut behavior
void ColorXYToRGB(uint16_t colorX, uint16_t colorY, uint8_t & red, uint8_t & green, uint8_t & blue)
{
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

    printf("ColorXY X:0x%04X Y:0x%04X (%.4f, %.4f) -> RGB R:%d G:%d B:%d\n", colorX, colorY, x, y, red, green, blue);
}

int main()
{
    uint8_t red, green, blue;
    printf("Testing Out-of-Gamut ColorXY Values:\n\n");

    // Test extreme values
    printf("1. Extreme corners of ColorXY space:\n");
    ColorXYToRGB(0, 0, red, green, blue);         // (0,0) - Invalid
    ColorXYToRGB(65535, 0, red, green, blue);     // (1,0) - Invalid
    ColorXYToRGB(0, 65535, red, green, blue);     // (0,1) - Invalid
    ColorXYToRGB(65535, 65535, red, green, blue); // (1,1) - Invalid (x+y > 1)

    printf("\n2. Highly saturated colors (near spectral locus):\n");
    ColorXYToRGB(45875, 52428, red, green, blue); // Pure red spectral (0.7, 0.8)
    ColorXYToRGB(6554, 58981, red, green, blue);  // Pure green spectral (0.1, 0.9)
    ColorXYToRGB(9830, 3277, red, green, blue);   // Pure blue spectral (0.15, 0.05)

    printf("\n3. Colors outside RGB gamut but within visible spectrum:\n");
    ColorXYToRGB(52428, 45875, red, green, blue); // Highly saturated cyan-green
    ColorXYToRGB(39321, 6554, red, green, blue);  // Highly saturated purple
    ColorXYToRGB(58981, 32768, red, green, blue); // Highly saturated yellow-green

    printf("\n4. Normal RGB gamut colors for comparison:\n");
    ColorXYToRGB(41943, 19660, red, green, blue); // Red (0.64, 0.3)
    ColorXYToRGB(19660, 45875, red, green, blue); // Green (0.3, 0.7)
    ColorXYToRGB(9830, 6554, red, green, blue);   // Blue (0.15, 0.1)
    ColorXYToRGB(20316, 21299, red, green, blue); // White D65 (0.31, 0.325)

    return 0;
}
