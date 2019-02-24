#ifndef EZGL_COLOR_HPP
#define EZGL_COLOR_HPP

#include <cstdint>

namespace ezgl {

/**
 * Represents a color as a mixture or red, green, and blue as well as the transparency level.
 *
 * Each color channel and transparency level is an 8-bit value, ranging from 0-255.
 */
struct color {
  /**
   * Create a color.
   *
   * @param r The amount of red.
   * @param g The amount of green.
   * @param b The amount of blue.
   * @param a The level of transparency.
   */
  constexpr color(std::uint_fast8_t r,
      std::uint_fast8_t g,
      std::uint_fast8_t b,
      std::uint_fast8_t a = 255) noexcept
      : red(r), green(g), blue(b), alpha(a)
  {
  }

  /**
   * A red component of the color, between 0 and 255.
   */
  std::uint_fast8_t red;

  /**
   * The green component of the color, between 0 and 255.
   */
  std::uint_fast8_t green;

  /**
   * The blue component of the color, between 0 and 255.
   */
  std::uint_fast8_t blue;

  /**
   * The amount of transparency, between 0 and 255.
   */
  std::uint_fast8_t alpha;

  /**
   * Test for equality.
   */
  bool operator==(const color &rhs) const
  {
    return red == rhs.red && green == rhs.green && blue == rhs.blue && alpha == rhs.alpha;
  }

  /**
   * Test for inequality.
   */
  bool operator!=(const color &rhs) const
  {
    return !(rhs == *this);
  }
};

//custom colors
static constexpr color NONE(0, 0, 0, 0);
static constexpr color BACKGROUND_GREY(230, 230, 230);
static constexpr color WATER_BLUE(117, 207, 241);
static constexpr color PARK_GREEN(182, 229, 158);
static constexpr color BUILDING_GREY(215, 215, 215);
static constexpr color BEACH_YELLOW(249, 242, 200);
static constexpr color BIKE_GREEN(5, 80, 5);

//given colors
static constexpr color WHITE(0xFF, 0xFF, 0xFF);
static constexpr color BLACK(0x00, 0x00, 0x00);
static constexpr color GREY_55(0x8C, 0x8C, 0x8C);
static constexpr color GREY_75(0xBF, 0xBF, 0xBF);
static constexpr color RED(0xFF, 0x00, 0x00);
static constexpr color ORANGE(0xFF, 0xA5, 0x00);
static constexpr color YELLOW(0xFF, 0xFF, 0x00);
static constexpr color GREEN(0x00, 0xFF, 0x00);
static constexpr color CYAN(0x00, 0xFF, 0xFF);
static constexpr color BLUE(0x00, 0x00, 0xFF);
static constexpr color PURPLE(0xA0, 0x20, 0xF0);
static constexpr color PINK(0xFF, 0xC0, 0xCB);
static constexpr color LIGHT_PINK(0xFF, 0xB6, 0xC1);
static constexpr color DARK_GREEN(0x00, 0x64, 0x00);
static constexpr color MAGENTA(0xFF, 0x00, 0xFF);
static constexpr color BISQUE(0xFF, 0xE4, 0xC4);
static constexpr color LIGHT_SKY_BLUE(0x87, 0xCE, 0xFA);
static constexpr color THISTLE(0xD8, 0xBF, 0xD8);
static constexpr color PLUM(0xDD, 0xA0, 0xDD);
static constexpr color KHAKI(0xF0, 0xE6, 0x8C);
static constexpr color CORAL(0xFF, 0x7F, 0x50);
static constexpr color TURQUOISE(0x40, 0xE0, 0xD0);
static constexpr color MEDIUM_PURPLE(0x93, 0x70, 0xDB);
static constexpr color DARK_SLATE_BLUE(0x48, 0x3D, 0x8B);
static constexpr color DARK_KHAKI(0xBD, 0xB7, 0x6B);
static constexpr color LIGHT_MEDIUM_BLUE(0x44, 0x44, 0xFF);
static constexpr color SADDLE_BROWN(0x8B, 0x45, 0x13);
static constexpr color FIRE_BRICK(0xB2, 0x22, 0x22);
static constexpr color LIME_GREEN(0x32, 0xCD, 0x32);
}

#endif //EZGL_COLOR_HPP
