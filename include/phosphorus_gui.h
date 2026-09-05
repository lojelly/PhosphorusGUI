#pragma once

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "raylib.h"

#ifdef _WIN32
	#ifdef PHOS_GUI_DLL
		#ifdef PHOS_GUI_EXPORTS
			#define PHOS_GUI_API __declspec(dllexport)
		#else
			#define PHOS_GUI_API __declspec(dllimport)
		#endif
	#else
		#define PHOS_GUI_API
	#endif
#else
	#define PHOS_GUI_API
#endif

/**
  Writes into a string buffer on an object with a valid
  char buffer (ex: char ID[...]).
*/
#define phos_gui_write_str(dest, ...) \
	do { \
		snprintf((dest), sizeof((dest)), __VA_ARGS__); \
	} while(0)

/**
  The max number of elements within a single
  phos_gui instance.
*/
#define PHOS_GUI_MAX_ELEMS 128

/**
  The max number of child elements a parent element
  can contain.
*/
#define PHOS_GUI_MAX_CHILDREN 48

/**
  The max number of icons an element can store.
*/
#define PHOS_GUI_MAX_ICONS 16

/**
  The max number of event listeners that a
  single phos_gui_elem can hold.
*/
#define PHOS_GUI_MAX_EVENT_LISTENERS 64

/**
  The max number of timers that a single
  phos_gui can hold.
*/
#define PHOS_GUI_MAX_TIMERS 32

/**
  The max number of animations that a single
  phos_gui can hold.
*/
#define PHOS_GUI_MAX_ANIMATIONS 32

/**
  The max length of an element's ID.
*/
#define PHOS_GUI_MAX_ID_LEN 32

/**
  Alias for "<auto>" ID.

  Use this to auto-generate an objects ID.
*/
#define PHOS_GUI_AUTO_ID "<auto>"
/**
  Alias for "<no-text>" modifier.

  The "<no-text>" modifier is used when
  initializing elements with possible
  text or label components, and it
  indicates the element should not
  be initialized with the text/label
  component.
*/
#define PHOS_GUI_NO_TEXT "<no-text>"

/**
  The max length of a text component's string.
*/
#define PHOS_GUI_MAX_TEXT_LEN 256

/**
  The max number of checkboxes a checkbox
  list component can use.
*/
#define PHOS_GUI_MAX_CHECKBOXES 24

/**
  The default font size for a text component.
*/
#define PHOS_GUI_FONT_SIZE_DEFAULT PHOS_GUI_FONT_SIZE_LARGE
/**
  Small font size.
*/
#define PHOS_GUI_FONT_SIZE_SMALL 8.0f
/**
  Medium font size.
*/
#define PHOS_GUI_FONT_SIZE_MED 16.0f
/**
  Large font size.
*/
#define PHOS_GUI_FONT_SIZE_LARGE 32.0f
/**
  Extremely large font size.
*/
#define PHOS_GUI_FONT_SIZE_XLARGE 48.0f
/**
  Huge font size.
*/
#define PHOS_GUI_FONT_SIZE_HUGE 64.0f
/**
  Extremely huge font size.
*/
#define PHOS_GUI_FONT_SIZE_XHUGE 80.0f
/**
  Gigantic font size.
*/
#define PHOS_GUI_FONT_SIZE_GIGANTIC 128.0f
/**
  Extremely gigantic font size.
*/
#define PHOS_GUI_FONT_SIZE_XGIGANTIC 256.0f
/**
  Largest pre-defined font size in PhosphorusGUI.
*/
#define PHOS_GUI_FONT_SIZE_LARGEST PHOS_GUI_FONT_SIZE_XGIGANTIC

/**
  The default size of icons in PhosphorusGUI.
*/
#define PHOS_GUI_ICON_SIZE_DEFAULT PHOS_GUI_ICON_SIZE_MED
/**
  Small icon size.
*/
#define PHOS_GUI_ICON_SIZE_SMALL 16.0f
/**
  Medium icon size.
*/
#define PHOS_GUI_ICON_SIZE_MED 32.0f
/**
  Large icon size.
*/
#define PHOS_GUI_ICON_SIZE_LARGE 64.0f
/**
  Largest icon size.
*/
#define PHOS_GUI_ICON_SIZE_LARGEST 128.0f

/**
  The max length of an icon name in PhosphorusGUI.

  An example of an icon name is 'CHECK_MARK.' Note
  that PHOS_GUI_CHECK_MARK is not the icon name.
  PhosphorusGUI does not use the 'PHOS_GUI' prefix
  in icon names.
*/
#define PHOS_GUI_MAX_ICON_NAME_LEN 64

/**
  The window's origin.
*/
#define PHOS_GUI_WINDOW_ORIGIN (Vector2) { 0.0f, 0.0f }
/**
  The window's current size.
*/
#define PHOS_GUI_WINDOW_SIZE (Vector2) { GetRenderWidth(), GetRenderHeight() }
/**
  A Vector2 representing the window's origin and size.

  @note This macro uses GetRenderWidth() and GetRenderHeight()
  for the window size.
*/
#define PHOS_GUI_WINDOW_RECT (Rectangle) { 0.0f, 0.0f, GetRenderWidth(), GetRenderHeight() }
/**
  The width of the mouse cursor in pixels.
*/
#define PHOS_GUI_CURSOR_WIDTH 32.0f
/**
  The height of the mouse cursor in pixels.
*/
#define PHOS_GUI_CURSOR_HEIGHT PHOS_GUI_CURSOR_WIDTH


// colors:

/**
  The default alpha value for all tint colors in PhosphorusGUI.
*/
#define PHOS_GUI_COLOR_TINT_STRENGTH 50
/**
  The default brightness increase/decrease factor for
  PhosphorusGUI's built-in colors.
*/
#define PHOS_GUI_COLOR_BRIGHTNESS_FACTOR 0.5f
/**
  The default contrast increase factor for
  PhosphorusGUI's built-in colors.
*/
#define PHOS_GUI_COLOR_CONTRAST_UP_FACTOR 0.4f
/**
  The default contrast decrease factor for PhosphorusGUI's
  built-in colors.
*/
#define PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR -0.3f
/**
  Mixes two colors.
*/
#define PHOS_GUI_COLOR_MIX(c1, c2) ColorLerp(c1, c2, 0.5f)

/**
  Black.
*/
#define PHOS_GUI_COLOR_BLACK (Color) { 15, 15, 15, 255 }

/**
  Gray.
*/
#define PHOS_GUI_COLOR_GRAY (Color) { 125, 125, 125, 255 }
/**
  Light gray.
*/
#define PHOS_GUI_COLOR_LIGHT_GRAY ColorBrightness(PHOS_GUI_COLOR_GRAY, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark gray.
*/
#define PHOS_GUI_COLOR_DARK_GRAY ColorBrightness(PHOS_GUI_COLOR_GRAY, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)

/**
  Red.
*/
#define PHOS_GUI_COLOR_RED (Color) { 255, 0, 0, 255 }
/**
  Light red.
*/
#define PHOS_GUI_COLOR_LIGHT_RED ColorBrightness(PHOS_GUI_COLOR_RED, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark red.
*/
#define PHOS_GUI_COLOR_DARK_RED ColorBrightness(PHOS_GUI_COLOR_RED, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright red.
*/
#define PHOS_GUI_COLOR_BRIGHT_RED ColorContrast(PHOS_GUI_COLOR_RED, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim red.
*/
#define PHOS_GUI_COLOR_DULL_RED ColorContrast(PHOS_GUI_COLOR_RED, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Green.
*/
#define PHOS_GUI_COLOR_GREEN (Color) { 0, 255, 0, 255 }
/**
  Light green.
*/
#define PHOS_GUI_COLOR_LIGHT_GREEN ColorBrightness(PHOS_GUI_COLOR_GREEN, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark green.
*/
#define PHOS_GUI_COLOR_DARK_GREEN ColorBrightness(PHOS_GUI_COLOR_GREEN, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright green.
*/
#define PHOS_GUI_COLOR_BRIGHT_GREEN ColorContrast(PHOS_GUI_COLOR_GREEN, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim green.
*/
#define PHOS_GUI_COLOR_DULL_GREEN ColorContrast(PHOS_GUI_COLOR_GREEN, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Blue.
*/
#define PHOS_GUI_COLOR_BLUE (Color) { 0, 0, 255, 255 }
/**
  Light blue.
*/
#define PHOS_GUI_COLOR_LIGHT_BLUE ColorBrightness(PHOS_GUI_COLOR_BLUE, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark blue.
*/
#define PHOS_GUI_COLOR_DARK_BLUE ColorBrightness(PHOS_GUI_COLOR_BLUE, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright blue.
*/
#define PHOS_GUI_COLOR_BRIGHT_BLUE ColorContrast(PHOS_GUI_COLOR_BLUE, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim blue.
*/
#define PHOS_GUI_COLOR_DULL_BLUE ColorContrast(PHOS_GUI_COLOR_BLUE, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Orange.
*/
#define PHOS_GUI_COLOR_ORANGE (Color) { 255, 125, 0, 255 }
/**
  Light orange.
*/
#define PHOS_GUI_COLOR_LIGHT_ORANGE ColorBrightness(PHOS_GUI_COLOR_ORANGE, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark orange (brown).
*/
#define PHOS_GUI_COLOR_DARK_ORANGE ColorBrightness(PHOS_GUI_COLOR_ORANGE, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright orange.
*/
#define PHOS_GUI_COLOR_BRIGHT_ORANGE ColorContrast(PHOS_GUI_COLOR_ORANGE, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim orange.
*/
#define PHOS_GUI_COLOR_DULL_ORANGE ColorContrast(PHOS_GUI_COLOR_ORANGE, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Yellow.
*/
#define PHOS_GUI_COLOR_YELLOW (Color) { 255, 255, 0, 255 }
/**
  Light yellow.
*/
#define PHOS_GUI_COLOR_LIGHT_YELLOW ColorBrightness(PHOS_GUI_COLOR_YELLOW, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark yellow.
*/
#define PHOS_GUI_COLOR_DARK_YELLOW ColorBrightness(PHOS_GUI_COLOR_YELLOW, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright yellow.
*/
#define PHOS_GUI_COLOR_BRIGHT_YELLOW ColorContrast(PHOS_GUI_COLOR_YELLOW, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim yellow.
*/
#define PHOS_GUI_COLOR_DULL_YELLOW ColorContrast(PHOS_GUI_COLOR_YELLOW, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Violet.
*/
#define PHOS_GUI_COLOR_VIOLET (Color) { 140, 0, 255, 255 }
/**
  Light violet.
*/
#define PHOS_GUI_COLOR_LIGHT_VIOLET ColorBrightness(PHOS_GUI_COLOR_VIOLET, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark violet.
*/
#define PHOS_GUI_COLOR_DARK_VIOLET ColorBrightness(PHOS_GUI_COLOR_VIOLET, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright violet.
*/
#define PHOS_GUI_COLOR_BRIGHT_VIOLET ColorContrast(PHOS_GUI_COLOR_VIOLET, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim violet.
*/
#define PHOS_GUI_COLOR_DULL_VIOLET ColorContrast(PHOS_GUI_COLOR_VIOLET, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Indigo.
*/
#define PHOS_GUI_COLOR_INDIGO (Color) { 75, 0, 175, 255 }
/**
  Light indigo.
*/
#define PHOS_GUI_COLOR_LIGHT_INDIGO ColorBrightness(PHOS_GUI_COLOR_INDIGO, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark indigo.
*/
#define PHOS_GUI_COLOR_DARK_INDIGO ColorBrightness(PHOS_GUI_COLOR_INDIGO, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright indigo.
*/
#define PHOS_GUI_COLOR_BRIGHT_INDIGO ColorContrast(PHOS_GUI_COLOR_INDIGO, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim indigo.
*/
#define PHOS_GUI_COLOR_DULL_INDIGO ColorContrast(PHOS_GUI_COLOR_INDIGO, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Cyan.
*/
#define PHOS_GUI_COLOR_CYAN (Color) { 0, 255, 255, 255 }
/**
  Light cyan.
*/
#define PHOS_GUI_COLOR_LIGHT_CYAN ColorBrightness(PHOS_GUI_COLOR_CYAN, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark cyan.
*/
#define PHOS_GUI_COLOR_DARK_CYAN ColorBrightness(PHOS_GUI_COLOR_CYAN, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright cyan.
*/
#define PHOS_GUI_COLOR_BRIGHT_CYAN ColorContrast(PHOS_GUI_COLOR_CYAN, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim cyan.
*/
#define PHOS_GUI_COLOR_DULL_CYAN ColorContrast(PHOS_GUI_COLOR_CYAN, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Mint.
*/
#define PHOS_GUI_COLOR_MINT (Color) { 115, 255, 145, 255 }
/**
  Light mint.
*/
#define PHOS_GUI_COLOR_LIGHT_MINT ColorBrightness(PHOS_GUI_COLOR_MINT, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark mint.
*/
#define PHOS_GUI_COLOR_DARK_MINT ColorBrightness(PHOS_GUI_COLOR_MINT, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright mint.
*/
#define PHOS_GUI_COLOR_BRIGHT_MINT ColorContrast(PHOS_GUI_COLOR_MINT, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim mint.
*/
#define PHOS_GUI_COLOR_DULL_MINT ColorContrast(PHOS_GUI_COLOR_MINT, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Teal.
*/
#define PHOS_GUI_COLOR_TEAL (Color) { 0, 130, 130, 255 }
/**
  Light teal.
*/
#define PHOS_GUI_COLOR_LIGHT_TEAL ColorBrightness(PHOS_GUI_COLOR_TEAL, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark teal.
*/
#define PHOS_GUI_COLOR_DARK_TEAL ColorBrightness(PHOS_GUI_COLOR_TEAL, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright teal.
*/
#define PHOS_GUI_COLOR_BRIGHT_TEAL ColorContrast(PHOS_GUI_COLOR_TEAL, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim teal.
*/
#define PHOS_GUI_COLOR_DULL_TEAL ColorContrast(PHOS_GUI_COLOR_TEAL, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Pink.
*/
#define PHOS_GUI_COLOR_PINK (Color) { 255, 100, 200, 255 }
/**
  Light pink.
*/
#define PHOS_GUI_COLOR_LIGHT_PINK ColorBrightness(PHOS_GUI_COLOR_PINK, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark pink.
*/
#define PHOS_GUI_COLOR_DARK_PINK ColorBrightness(PHOS_GUI_COLOR_PINK, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright pink.
*/
#define PHOS_GUI_COLOR_BRIGHT_PINK ColorContrast(PHOS_GUI_COLOR_PINK, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim pink.
*/
#define PHOS_GUI_COLOR_DULL_PINK ColorContrast(PHOS_GUI_COLOR_PINK, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Magenta.
*/
#define PHOS_GUI_COLOR_MAGENTA (Color) { 255, 0, 255, 255 }
/**
  Light magenta.
*/
#define PHOS_GUI_COLOR_LIGHT_MAGENTA ColorBrightness(PHOS_GUI_COLOR_MAGENTA, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark magenta.
*/
#define PHOS_GUI_COLOR_DARK_MAGENTA ColorBrightness(PHOS_GUI_COLOR_MAGENTA, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright magenta;
*/
#define PHOS_GUI_COLOR_BRIGHT_MAGENTA ColorContrast(PHOS_GUI_COLOR_MAGENTA, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim magenta.
*/
#define PHOS_GUI_COLOR_DULL_MAGENTA ColorContrast(PHOS_GUI_COLOR_MAGENTA, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Crystal.
*/
#define PHOS_GUI_COLOR_CRYSTAL (Color) { 255, 250, 125, 255 }
/**
  Light crystal.
*/
#define PHOS_GUI_COLOR_LIGHT_CRYSTAL ColorBrightness(PHOS_GUI_COLOR_CRYSTAL, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark crystal.
*/
#define PHOS_GUI_COLOR_DARK_CRYSTAL ColorBrightness(PHOS_GUI_COLOR_CRYSTAL, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright crystal.
*/
#define PHOS_GUI_COLOR_BRIGHT_CRYSTAL ColorContrast(PHOS_GUI_COLOR_CRYSTAL, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim crystal.
*/
#define PHOS_GUI_COLOR_DULL_CRYSTAL ColorContrast(PHOS_GUI_COLOR_CRYSTAL, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Sky blue.
*/
#define PHOS_GUI_COLOR_SKY_BLUE (Color) { 0, 175, 255, 255 }
/**
  Light sky blue.
*/
#define PHOS_GUI_COLOR_LIGHT_SKY_BLUE ColorBrightness(PHOS_GUI_COLOR_SKY_BLUE, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark sky blue.
*/
#define PHOS_GUI_COLOR_DARK_SKY_BLUE ColorBrightness(PHOS_GUI_COLOR_SKY_BLUE, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright sky blue.
*/
#define PHOS_GUI_COLOR_BRIGHT_SKY_BLUE ColorContrast(PHOS_GUI_COLOR_SKY_BLUE, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim sky blue.
*/
#define PHOS_GUI_COLOR_DULL_SKY_BLUE ColorContrast(PHOS_GUI_COLOR_SKY_BLUE, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Coral.
*/
#define PHOS_GUI_COLOR_CORAL (Color) { 255, 125, 90, 255 }
/**
  Light coral.
*/
#define PHOS_GUI_COLOR_LIGHT_CORAL ColorBrightness(PHOS_GUI_COLOR_CORAL, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark coral.
*/
#define PHOS_GUI_COLOR_DARK_CORAL ColorBrightness(PHOS_GUI_COLOR_CORAL, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright coral.
*/
#define PHOS_GUI_COLOR_BRIGHT_CORAL ColorContrast(PHOS_GUI_COLOR_CORAL, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim coral.
*/
#define PHOS_GUI_COLOR_DULL_CORAL ColorContrast(PHOS_GUI_COLOR_CORAL, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)

/**
  Amber.
*/
#define PHOS_GUI_COLOR_AMBER (Color) { 255, 190, 0, 255 }
/**
  Light amber.
*/
#define PHOS_GUI_COLOR_LIGHT_AMBER ColorBrightness(PHOS_GUI_COLOR_AMBER, PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Dark amber.
*/
#define PHOS_GUI_COLOR_DARK_AMBER ColorBrightness(PHOS_GUI_COLOR_AMBER, -PHOS_GUI_COLOR_BRIGHTNESS_FACTOR)
/**
  Bright amber.
*/
#define PHOS_GUI_COLOR_BRIGHT_AMBER ColorContrast(PHOS_GUI_COLOR_AMBER, PHOS_GUI_COLOR_CONTRAST_UP_FACTOR)
/**
  Dim amber.
*/
#define PHOS_GUI_COLOR_DULL_AMBER ColorContrast(PHOS_GUI_COLOR_AMBER, PHOS_GUI_COLOR_CONTRAST_DOWN_FACTOR)


// pre-built themes:

/**
  The default outline thickness
  for pre-built themes in PhosphorusGUI.
*/
#define PHOS_GUI_THEME_DEFAULT_OUTLINE_THICKNESS 2.5f

/**
  A theme built around Autumn colors.
*/
#define PHOS_GUI_THEME_AUTUMN (phos_gui_theme) { \
	.bg_color = PHOS_GUI_COLOR_ORANGE, \
	.outline_color = ColorBrightness(PHOS_GUI_COLOR_RED, -0.2f), \
	.bg_hover_color = ColorBrightness(PHOS_GUI_COLOR_ORANGE, -0.1f), \
	.bg_press_color = ColorBrightness(PHOS_GUI_COLOR_ORANGE, -0.2f), \
	.bg_focus_color = PHOS_GUI_COLOR_ORANGE, \
	.outline_hover_color = ColorBrightness(PHOS_GUI_COLOR_RED, -0.2f), \
	.outline_press_color = ColorBrightness(PHOS_GUI_COLOR_RED, -0.2f), \
	.outline_focus_color = ColorBrightness(PHOS_GUI_COLOR_RED, -0.2f), \
	.decoration_color = ColorContrast(PHOS_GUI_COLOR_ORANGE, -0.2f), \
	.text_color = ColorBrightness(PHOS_GUI_COLOR_DULL_RED, -0.5f), \
	.icon_color = PHOS_GUI_COLOR_DULL_RED, \
	.window_bg_color = ColorBrightness(PHOS_GUI_COLOR_YELLOW, -0.2f), \
	.outline_thickness = PHOS_GUI_THEME_DEFAULT_OUTLINE_THICKNESS }
/**
  A theme similar to PHOS_GUI_THEME_AUTUMN but includes green.
*/
#define PHOS_GUI_THEME_AUTUMN_FIELD phos_gui_create_theme_full(PHOS_GUI_COLOR_BRIGHT_ORANGE, PHOS_GUI_COLOR_RED, PHOS_GUI_COLOR_DULL_ORANGE, PHOS_GUI_COLOR_DARK_ORANGE, PHOS_GUI_COLOR_BRIGHT_YELLOW, ColorContrast(PHOS_GUI_COLOR_DARK_GREEN, -0.2f))
/**
  A simple blue theme.
*/
#define PHOS_GUI_THEME_BLUE phos_gui_create_theme_accented(PHOS_GUI_COLOR_DULL_BLUE, PHOS_GUI_COLOR_DARK_BLUE)
/**
  A theme built around white and blue.
*/
#define PHOS_GUI_THEME_BLUE_LIGHT phos_gui_create_theme_accented(RAYWHITE, PHOS_GUI_COLOR_BRIGHT_BLUE)
/**
  A theme built on sky blue and cyan.
*/
#define PHOS_GUI_THEME_BRIGHT_BLUE_SKY phos_gui_create_theme_accented(PHOS_GUI_COLOR_CYAN, PHOS_GUI_COLOR_SKY_BLUE)
/**
  A theme built around pink.
*/
#define PHOS_GUI_THEME_BUBBLEGUM (phos_gui_theme) { \
	.bg_color = PHOS_GUI_COLOR_PINK, \
	.outline_color = PHOS_GUI_COLOR_DARK_PINK, \
	.bg_hover_color = ColorBrightness(PHOS_GUI_COLOR_PINK, -0.1f), \
	.bg_press_color = ColorBrightness(PHOS_GUI_COLOR_PINK, -0.2f), \
	.bg_focus_color = PHOS_GUI_COLOR_PINK, \
	.outline_hover_color = PHOS_GUI_COLOR_DARK_PINK, \
	.outline_press_color = PHOS_GUI_COLOR_DARK_PINK, \
	.outline_focus_color = PHOS_GUI_COLOR_DARK_PINK, \
	.decoration_color = PHOS_GUI_COLOR_DULL_PINK, \
	.text_color = PHOS_GUI_COLOR_DARK_RED, \
	.icon_color = PHOS_GUI_COLOR_DULL_RED, \
	.window_bg_color = ColorContrast(PHOS_GUI_COLOR_DULL_PINK, -0.1f), \
	.outline_thickness = PHOS_GUI_THEME_DEFAULT_OUTLINE_THICKNESS }
/**
  A theme built around blue and white.
*/
#define PHOS_GUI_THEME_CLOUDS phos_gui_create_theme_accented(RAYWHITE, ColorBrightness(PHOS_GUI_COLOR_DULL_TEAL, -0.1f))
/**
  A theme built around white and yellow.
*/
#define PHOS_GUI_THEME_CORN phos_gui_create_theme_full(PHOS_GUI_COLOR_YELLOW, PHOS_GUI_COLOR_DULL_YELLOW, PHOS_GUI_COLOR_DULL_AMBER, PHOS_GUI_COLOR_BLACK, PHOS_GUI_COLOR_DULL_GREEN, ColorContrast(PHOS_GUI_COLOR_DULL_GREEN, -0.4f))
/**
  A theme built around vibrant blue and mint.
*/
#define PHOS_GUI_THEME_COTTON_CANDY phos_gui_create_theme_accented(PHOS_GUI_COLOR_BRIGHT_SKY_BLUE, PHOS_GUI_COLOR_BRIGHT_MINT)
/**
  A simple dark theme.
*/
#define PHOS_GUI_THEME_DARK phos_gui_create_theme_full(PHOS_GUI_COLOR_GRAY, PHOS_GUI_COLOR_DARK_GRAY, PHOS_GUI_COLOR_LIGHT_GRAY, WHITE, RAYWHITE, PHOS_GUI_COLOR_BLACK)
/**
  A theme built on dull oranges and yellows.
*/
#define PHOS_GUI_THEME_DESERT phos_gui_create_theme_full(PHOS_GUI_COLOR_CRYSTAL, PHOS_GUI_COLOR_DARK_ORANGE, PHOS_GUI_COLOR_DULL_YELLOW, PHOS_GUI_COLOR_DARK_GREEN, PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_DULL_AMBER)
/**
  An alternate version of PHOS_GUI_THEME_KNIGHT where the base and accent colors are reversed and
  slightly modified to make text more readable.
*/
#define PHOS_GUI_THEME_DRAGON phos_gui_create_theme_accented(PHOS_GUI_COLOR_GRAY, PHOS_GUI_COLOR_BRIGHT_RED)
/**
  A theme revolving around yellow.
*/
#define PHOS_GUI_THEME_ELECTRIC phos_gui_create_theme_basic(PHOS_GUI_COLOR_BRIGHT_YELLOW)
/**
  A theme revolving around orange and pink.
*/
#define PHOS_GUI_THEME_FLOWERS phos_gui_create_theme_accented(PHOS_GUI_COLOR_BRIGHT_ORANGE, PHOS_GUI_COLOR_BRIGHT_PINK)
/**
  A theme revolving around green.
*/
#define PHOS_GUI_THEME_FOREST phos_gui_create_theme_accented(PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_DARK_GREEN)
/**
  A simple theme built on grays.
*/
#define PHOS_GUI_THEME_GRAPHITE phos_gui_create_theme_basic(RAYWHITE)
/**
  A simple gray theme.
*/
#define PHOS_GUI_THEME_GRAY phos_gui_create_theme_basic(PHOS_GUI_COLOR_LIGHT_GRAY)
/**
  A theme revolving around typical Halloween colors.
*/
#define PHOS_GUI_THEME_HALLOWEEN phos_gui_create_theme_full(PHOS_GUI_COLOR_BRIGHT_ORANGE, PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_LIGHT_BLUE, PHOS_GUI_COLOR_BRIGHT_YELLOW, PHOS_GUI_COLOR_DULL_INDIGO, PHOS_GUI_COLOR_DULL_VIOLET)
/**
  A theme revolving around neon blue and orange.
*/
#define PHOS_GUI_THEME_HONG_KONG phos_gui_create_theme_full(GetColor(0xFC440FFF), PHOS_GUI_COLOR_CYAN, PHOS_GUI_COLOR_DULL_RED, PHOS_GUI_COLOR_DARK_SKY_BLUE, PHOS_GUI_COLOR_DARK_RED, ColorContrast(PHOS_GUI_COLOR_DULL_VIOLET, -0.3f))
/**
  A theme revolving around blue and yellow.
*/
#define PHOS_GUI_THEME_HURRICANE phos_gui_create_theme_full(PHOS_GUI_COLOR_DULL_BLUE, PHOS_GUI_COLOR_BRIGHT_YELLOW, ColorBrightness(PHOS_GUI_COLOR_SKY_BLUE, 0.1f), ColorBrightness(ColorLerp(PHOS_GUI_COLOR_BLUE, PHOS_GUI_COLOR_BRIGHT_YELLOW, 0.5f), 0.6f), PHOS_GUI_COLOR_DULL_YELLOW, ColorBrightness(RAYWHITE, -0.75f))
/**
  A theme built on crystal and indigo.
*/
#define PHOS_GUI_THEME_INDIGO_COSMOS phos_gui_create_theme_accented(PHOS_GUI_COLOR_LIGHT_BLUE, PHOS_GUI_COLOR_DULL_VIOLET)
/**
  A theme built on red and yellow.
*/
#define PHOS_GUI_THEME_INFERNO phos_gui_create_theme_accented(PHOS_GUI_COLOR_YELLOW, PHOS_GUI_COLOR_RED)
/**
  A theme built on yellow, green, and blue.
*/
#define PHOS_GUI_THEME_JELLY_BEANS phos_gui_create_theme_full(PHOS_GUI_COLOR_CRYSTAL, PHOS_GUI_COLOR_MINT, PHOS_GUI_COLOR_DULL_YELLOW, PHOS_GUI_COLOR_TEAL, PHOS_GUI_COLOR_DULL_BLUE, PHOS_GUI_COLOR_DULL_INDIGO)
/**
  A theme built around red and black.
*/
#define PHOS_GUI_THEME_KNIGHT phos_gui_create_theme_accented(PHOS_GUI_COLOR_DULL_RED, PHOS_GUI_COLOR_BLACK)
/**
  A simple light theme.
*/
#define PHOS_GUI_THEME_LIGHT phos_gui_create_theme_full(WHITE, PHOS_GUI_COLOR_GRAY, ColorBrightness(PHOS_GUI_COLOR_DARK_GRAY, 0.1f), PHOS_GUI_COLOR_DARK_GRAY, PHOS_GUI_COLOR_BLACK, PHOS_GUI_COLOR_LIGHT_GRAY)
/**
  A theme revolving around red and magenta.
*/
#define PHOS_GUI_THEME_MARS phos_gui_create_theme_accented(PHOS_GUI_COLOR_DULL_RED, PHOS_GUI_COLOR_MAGENTA)
/**
  A theme revolving around mint and black.
*/
#define PHOS_GUI_THEME_MINT_CANDY phos_gui_create_theme_accented(PHOS_GUI_COLOR_MINT, PHOS_GUI_COLOR_BLACK)
/**
  A theme revolving around the Mojave desert.
*/
#define PHOS_GUI_THEME_MOJAVE phos_gui_saturate_theme(phos_gui_create_theme_accented(ColorLerp(PHOS_GUI_COLOR_ORANGE, PHOS_GUI_COLOR_RED, 0.4f), PHOS_GUI_COLOR_DULL_GREEN), -0.2f)
/**
  The default theme of PhosphorusGUI.

  A simple black and white theme.
*/
#define PHOS_GUI_THEME_MONOTONE phos_gui_create_theme_full(PHOS_GUI_COLOR_LIGHT_GRAY, PHOS_GUI_COLOR_BLACK, PHOS_GUI_COLOR_MIX(RAYWHITE, PHOS_GUI_COLOR_DARK_GRAY), PHOS_GUI_COLOR_BLACK, PHOS_GUI_COLOR_DARK_GRAY, PHOS_GUI_COLOR_GRAY)
/**
  A theme revolving around violet and green.
*/
#define PHOS_GUI_THEME_MUTATION phos_gui_create_theme_accented(PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_DULL_VIOLET)
/**
  A theme revolving around blues.
*/
#define PHOS_GUI_THEME_NAUTICAL phos_gui_create_theme_accented(PHOS_GUI_COLOR_DULL_BLUE, PHOS_GUI_COLOR_DARK_BLUE)
/**
  A theme revolving around dark and vibrant blues.
*/
#define PHOS_GUI_THEME_NEON_TECH phos_gui_create_theme_basic(PHOS_GUI_COLOR_SKY_BLUE)
/**
  A theme revolving around blue and green.
*/
#define PHOS_GUI_THEME_OASIS phos_gui_create_theme_accented(PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_SKY_BLUE)
/**
  A theme revolving around green and yellow.
*/
#define PHOS_GUI_THEME_PHOSPHORUS phos_gui_create_theme_full(ColorLerp(WHITE, PHOS_GUI_COLOR_GREEN, 0.2f), ColorLerp(PHOS_GUI_COLOR_YELLOW, PHOS_GUI_COLOR_GREEN, 0.3f), PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_BLACK, PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_DARK_GRAY)
/**
  A theme revolving around bright yellows and greens.
*/
#define PHOS_GUI_THEME_PINEAPPLE phos_gui_create_theme_full(PHOS_GUI_COLOR_BRIGHT_YELLOW, ColorLerp(PHOS_GUI_COLOR_BRIGHT_ORANGE, PHOS_GUI_COLOR_DULL_YELLOW, 0.5f), PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_DARK_GREEN, PHOS_GUI_COLOR_DULL_GREEN, ColorContrast(PHOS_GUI_COLOR_LIGHT_GREEN, -0.3f))
/**
  A theme revolving around the punk aesthetic (pinks, blues, and yellows).
*/
#define PHOS_GUI_THEME_PUNK (phos_gui_theme) { \
	.bg_color = PHOS_GUI_COLOR_BRIGHT_PINK, \
	.outline_color = PHOS_GUI_COLOR_DULL_VIOLET, \
	.bg_hover_color = ColorBrightness(PHOS_GUI_COLOR_BRIGHT_PINK, -0.1f), \
	.bg_press_color = ColorBrightness(PHOS_GUI_COLOR_BRIGHT_PINK, -0.2f), \
	.bg_focus_color = PHOS_GUI_COLOR_BRIGHT_PINK, \
	.outline_hover_color = PHOS_GUI_COLOR_DULL_VIOLET, \
	.outline_press_color = PHOS_GUI_COLOR_DULL_VIOLET, \
	.outline_focus_color = PHOS_GUI_COLOR_DULL_VIOLET, \
	.decoration_color = PHOS_GUI_COLOR_DULL_PINK, \
	.text_color = PHOS_GUI_COLOR_YELLOW, \
	.icon_color = PHOS_GUI_COLOR_BRIGHT_INDIGO, \
	.window_bg_color = ColorContrast(PHOS_GUI_COLOR_DULL_CYAN, -0.15f), \
	.outline_thickness = PHOS_GUI_THEME_DEFAULT_OUTLINE_THICKNESS }
/**
  A theme revolving around dull blues.
*/
#define PHOS_GUI_THEME_QUIET phos_gui_create_theme_full(ColorContrast(PHOS_GUI_COLOR_DULL_BLUE, -0.2f), PHOS_GUI_COLOR_DULL_TEAL, PHOS_GUI_COLOR_LIGHT_BLUE, ColorContrast(PHOS_GUI_COLOR_DARK_BLUE, -0.2f), PHOS_GUI_COLOR_DULL_SKY_BLUE, ColorContrast(PHOS_GUI_COLOR_LIGHT_TEAL, -0.2f))
/**
  A simple red theme.
*/
#define PHOS_GUI_THEME_RED phos_gui_create_theme_accented(PHOS_GUI_COLOR_DULL_RED, PHOS_GUI_COLOR_DARK_RED)
/**
  A theme revolving around retro terminals.
*/
#define PHOS_GUI_THEME_RETRO_TERMINAL phos_gui_saturate_theme((phos_gui_theme) { \
	.bg_color = PHOS_GUI_COLOR_DULL_GREEN, \
	.outline_color = PHOS_GUI_COLOR_DARK_GREEN, \
	.bg_hover_color = ColorBrightness(PHOS_GUI_COLOR_DULL_GREEN, -0.1f), \
	.bg_press_color = ColorBrightness(PHOS_GUI_COLOR_DULL_GREEN, -0.2f), \
	.bg_focus_color = PHOS_GUI_COLOR_DULL_GREEN, \
	.outline_hover_color = PHOS_GUI_COLOR_DARK_GREEN, \
	.outline_press_color = PHOS_GUI_COLOR_DARK_GREEN, \
	.outline_focus_color = PHOS_GUI_COLOR_DARK_GREEN, \
	.decoration_color = ColorContrast(PHOS_GUI_COLOR_DULL_GREEN, -0.2f), \
	.text_color = ColorBrightness(PHOS_GUI_COLOR_DARK_GREEN, -0.2f), \
	.icon_color = ColorContrast(PHOS_GUI_COLOR_DARK_GREEN, -0.2f), \
	.window_bg_color = ColorBrightness(PHOS_GUI_COLOR_DARK_GREEN, -0.5f), \
	.outline_thickness = PHOS_GUI_THEME_DEFAULT_OUTLINE_THICKNESS }, 0.05f)
/**
  A theme revolving around marine ecosystems.
*/
#define PHOS_GUI_THEME_SEAWEED phos_gui_create_theme_full(PHOS_GUI_COLOR_CORAL, PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_DARK_CRYSTAL, PHOS_GUI_COLOR_DULL_BLUE, PHOS_GUI_COLOR_DARK_SKY_BLUE, PHOS_GUI_COLOR_DARK_TEAL)
/**
  A theme revolving around orange and yellow.
*/
#define PHOS_GUI_THEME_SUNLIGHT phos_gui_create_theme_accented(PHOS_GUI_COLOR_ORANGE, PHOS_GUI_COLOR_YELLOW)
/**
  A theme revolving around blue, red, and yellow.
*/
#define PHOS_GUI_THEME_SUNSET (phos_gui_theme) { \
	.bg_color = GetColor(0xDB504AFF), \
	.outline_color = GetColor(0xE3B505FF), \
	.bg_hover_color = ColorBrightness(GetColor(0xDB504AFF), -0.1f), \
	.bg_press_color = ColorBrightness(GetColor(0xDB504AFF), -0.2f), \
	.bg_focus_color = GetColor(0xDB504AFF), \
	.outline_hover_color = GetColor(0xE3B505FF), \
	.outline_press_color = GetColor(0xE3B505FF), \
	.outline_focus_color = GetColor(0xE3B505FF), \
	.decoration_color = ColorContrast(GetColor(0xDB504AFF), -0.2f), \
	.text_color = PHOS_GUI_COLOR_DARK_TEAL, \
	.icon_color = ColorContrast(PHOS_GUI_COLOR_DULL_CRYSTAL, 0.2f), \
	.window_bg_color = GetColor(0x4F6D7AFF), \
	.outline_thickness = PHOS_GUI_THEME_DEFAULT_OUTLINE_THICKNESS }
/**
  A theme revolving around violet and pink.
*/
#define PHOS_GUI_THEME_TOKYO phos_gui_create_theme_accented(PHOS_GUI_COLOR_DULL_VIOLET, PHOS_GUI_COLOR_BRIGHT_PINK)
/**
  A theme revolving around dull violets and blues.
*/
#define PHOS_GUI_THEME_URBAN phos_gui_create_theme_full(ColorLerp(PHOS_GUI_COLOR_LIGHT_GRAY, PHOS_GUI_COLOR_DULL_BLUE, 0.5f), PHOS_GUI_COLOR_DULL_VIOLET, PHOS_GUI_COLOR_DULL_TEAL, PHOS_GUI_COLOR_INDIGO, PHOS_GUI_COLOR_DULL_INDIGO, ColorContrast(PHOS_GUI_COLOR_LIGHT_SKY_BLUE, -0.4f))
/**
  A theme revolving around light yellows.
*/
#define PHOS_GUI_THEME_VANILLA phos_gui_create_theme_accented(GetColor(0xFFDDA1FF), GetColor(0xFFD151FF))
/**
  A theme revolving around greens and blues.
*/
#define PHOS_GUI_THEME_WILDERNESS phos_gui_create_theme_full(PHOS_GUI_COLOR_DULL_GREEN, PHOS_GUI_COLOR_SKY_BLUE, PHOS_GUI_COLOR_DARK_GREEN, PHOS_GUI_COLOR_DARK_BLUE, PHOS_GUI_COLOR_TEAL, PHOS_GUI_COLOR_DARK_MINT)
/**
  A theme revolving around bright blues and violets.
*/
#define PHOS_GUI_THEME_XENON phos_gui_create_theme_accented(PHOS_GUI_COLOR_BRIGHT_BLUE, PHOS_GUI_COLOR_DULL_VIOLET)
/**
  A theme revolving around orange and blue.
*/
#define PHOS_GUI_THEME_YOYO phos_gui_create_theme_accented(PHOS_GUI_COLOR_SKY_BLUE, PHOS_GUI_COLOR_BRIGHT_ORANGE)
/**
  A theme revolving around fruity and tropical colors.
*/
#define PHOS_GUI_THEME_ZEST phos_gui_create_theme_full(PHOS_GUI_COLOR_BRIGHT_YELLOW, PHOS_GUI_COLOR_BRIGHT_ORANGE, PHOS_GUI_COLOR_DULL_YELLOW, PHOS_GUI_COLOR_DARK_GREEN, PHOS_GUI_COLOR_DULL_RED, PHOS_GUI_COLOR_SKY_BLUE)


/**
  The different types of elements.

  An element type indicates how it should be updated
  in the main program loop.

  To change how an element is rendered,
  see phos_gui_elem_render_mode.

  @see phos_gui_elem_render_mode
*/
typedef enum phos_gui_elem_type
{
	/**
	  The default element type.
	  
	  Indicates the element is in an invalid state.
	*/
	PHOS_GUI_TYPE_INVALID,
	/**
	  The most basic element type. This type
	  indicates the element has no functionality.
	*/
	PHOS_GUI_TYPE_BLANK,
	/**
	  The primary element type. This type
	  indicates the element can be interacted
	  with, such as being clicked, dragged,
	  etc.
	*/
	PHOS_GUI_TYPE_INTERACTIVE
} phos_gui_elem_type;

/**
  The different shapes of objects.
*/
typedef enum phos_gui_shape
{
	/**
	  The rectangle shape.
	*/
	PHOS_GUI_SHAPE_RECT,
	/**
	  The ellipse shape.
	*/
	PHOS_GUI_SHAPE_ELLIPSE,
	/**
	  The rounded-rectangle shape.

	  Change the objects's corner-radius attribute
	  to modify the roundness of the rectangle.
	*/
	PHOS_GUI_SHAPE_ROUND_RECT
} phos_gui_shape;

/**
  The different render modes of elements.
*/
typedef enum phos_gui_elem_render_mode
{
	/**
	  The default type of all elements.

	  Results in the element's shape being filled,
	  then outlined.
	*/
	PHOS_GUI_RENDER_FILL_OUTLINE,
	/**
	  Results in just the element's shape being filled.
	*/
	PHOS_GUI_RENDER_FILL,
	/**
	  Results in just the element's outline being rendered.
	*/
	PHOS_GUI_RENDER_OUTLINE,
	/**
	  Indicates the element should not be rendered.

	  In most cases, this will be combined with PHOS_GUI_TYPE_BLANK
	  to create invisible elements.
	*/
	PHOS_GUI_RENDER_BLANK,
	/**
	  Results in the element's shape not being rendered.

	  The element's shape will be used for other things,
	  but the main content of the element will not be
	  rendered using its shape. Instead, PhosphorusGUI
	  now expects the element to have a texture component
	  attached to it with a valid Texture2D set.
	*/
	PHOS_GUI_RENDER_TEXTURE
} phos_gui_elem_render_mode;

/**
  The different alignments.
*/
typedef enum phos_gui_alignment
{
	/**
	  The default alignment. Indicates an invalid alignment.
	*/
	PHOS_GUI_ALIGN_INVALID,

	/**
	  Indicates the targeted item should be aligned to the inner left
	  of an element. The inner left represents the visual left
	  of the element.
	*/
	PHOS_GUI_ALIGN_INNER_LEFT,
	/**
	  Indicates the targeted item should be aligned to the inner
	  top of an element. The inner top represents the visual top
	  of the element.
	*/
	PHOS_GUI_ALIGN_INNER_TOP,
	/**
	  Indicates the targeted item should be aligned to the inner right
	  of an element. The inner right represents the visual right
	  of the element.
	*/
	PHOS_GUI_ALIGN_INNER_RIGHT,
	/**
	  Indicates the targeted item should be aligned to the inner bottom
	  of an element. The inner bottom represents the visual bottom of
	  the element.
	*/
	PHOS_GUI_ALIGN_INNER_BOTTOM,
	/**
	  Indicates the targeted item should be placed in the center
	  of an element.
	*/
	PHOS_GUI_ALIGN_INNER_CENTER,
	/**
	  Indicates the targeted item should be placed in the inner top-left
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_INNER_TOP_LEFT,
	/**
	  Indicates the targeted item should be placed in the inner top-right
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_INNER_TOP_RIGHT,
	/**
	  Indicates the targeted item should be placed in the inner bottom-left
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_INNER_BOTTOM_LEFT,
	/**
	  Indicates the targeted item should be placed in the inner bottom-right
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_INNER_BOTTOM_RIGHT,
	/**
	  Indicates the targeted item should be placed directly to the left
	  of an element.
	*/
	PHOS_GUI_ALIGN_LEFT,
	/**
	  Indicates the targeted item should be placed directly above
	  an element.
	*/
	PHOS_GUI_ALIGN_TOP,
	/**
	  Indicates the targeted item should be placed directly to the right
	  of an element.
	*/
	PHOS_GUI_ALIGN_RIGHT,
	/**
	  Indicates the targeted item should be placed directly below
	  an element.
	*/
	PHOS_GUI_ALIGN_BOTTOM,
	/**
	  Indicates the targeted item should be placed in the outer top-left
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_TOP_LEFT_CORNER,
	/**
	  Indicates the targeted item should be placed in the outer top-right
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_TOP_RIGHT_CORNER,
	/**
	  Indicates the targeted item should be placed in the outer bottom-left
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_BOTTOM_LEFT_CORNER,
	/**
	  Indicates the targeted item should be placed in the outer bottom-right
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_BOTTOM_RIGHT_CORNER,
	/**
	  Indicates the targeted item should be placed in the outer top-left
	  corner of the element, but with aligned left edges.
	*/
	PHOS_GUI_ALIGN_TOP_LEFT_EDGE,
	/**
	  Indicates the targeted item should be placed in the outer top-right
	  corner of the element, but with aligned right edges.
	*/
	PHOS_GUI_ALIGN_TOP_RIGHT_EDGE,
	/**
	  Indicates the targeted item should be placed in the outer bottom-left
	  corner of the element, but with aligned left edges.
	*/
	PHOS_GUI_ALIGN_BOTTOM_LEFT_EDGE,
	/**
	  Indicates the targeted item should be placed in the outer bottom-right
	  corner of the element, but with aligned right edges.
	*/
	PHOS_GUI_ALIGN_BOTTOM_RIGHT_EDGE
} phos_gui_alignment;

/**
  Defines the different types of rectangles/bounding boxes
  that an element has.
*/
typedef enum phos_gui_elem_bounding_box
{
	/**
	  Indicates invalid bounds or
	  a selection of no bounds.
	*/
	PHOS_GUI_ELEM_BOUNDS_NONE,

	/**
	  The default element bounding box.

	  Represents the actual position and size
	  of the element stored inside the element's
	  'pos' and 'size' fields.
	*/
	PHOS_GUI_ELEM_BOUNDS_REAL,
	/**
	  The rectangle defining the element's total
	  content area.

	  The total content area represents all of the
	  space inside of the element, which takes
	  the element's outline and padding values
	  into account. The total content area also
	  ignores any components or child elements
	  within the element.
	*/
	PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL,
	/**
	  The rectangle defining the element's free
	  content area.

	  Like the element's total content area, the free
	  content area is a bounding box where any non-components
	  can reside within the element, such as child elements.
	*/
	PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE,
	/**
	  The largest bounding box for an element.

	  This bounding box includes the element's
	  margins.
	*/
	PHOS_GUI_ELEM_BOUNDS_TOTAL,
} phos_gui_elem_bounding_box;

/**
  Additional and optional settings for some phos_gui functions.

  @important Some options are documented as 'immediate' or
  'one-time.' Those options are only executed on the initial
  or first element encountered.
*/
typedef uint32_t phos_gui_opts;

enum
{
	/**
	  Indicates there are no extra options to apply.
	*/
	PHOS_GUI_OPTS_NONE = 0,
	/**
	  Indicates that when resizing an element, its text component
	  (if it has one) should be modified to fit the new size of the
	  element.

	  @note This only takes effect when the element becomes too small
	  to contain its text.
	*/
	PHOS_GUI_OPTS_FIT_TEXT = 1u << 0,
	/**
	  Indicates that when resizing an element, its text component
	  (if it has one) should be realigned.

	  @important By default, text components already realign
	  themselves in most cases. But for explicity you can use
	  this option.
	*/
	PHOS_GUI_OPTS_REALIGN_TEXT = 1u << 1,
	/**
	  Indicates that when moving an element, collisions between the
	  element and other elements should be resolved.
	*/
	PHOS_GUI_OPTS_CHECK_ELEM_COLLISIONS = 1u << 2,
	/**
	  Indicates that when moving an element, collisions between the
	  element and the window's edges should be resolved.
	*/
	PHOS_GUI_OPTS_CHECK_WINDOW_COLLISIONS = 1u << 3,
	/**
	  Indicates the child should be resized to fit the
	  parent's size.

	  @note This option is immediate and only affects
	  the child specifically targeted with this option.
	*/
	PHOS_GUI_OPTS_CHILD_MAXIMIZED = 1u << 4,
	/**
	  Indicates the child's position should be interpreted
	  as relative to the parent's position.

	  @note This option is immediate and only affects
	  the child specifically targeted with this option.
	*/
	PHOS_GUI_OPTS_CHILD_HAS_RELATIVE_POS = 1u << 5,
};

/**
  The available component types.
*/
typedef enum phos_gui_component_type
{
	/**
	  @see phos_gui_shadow_component
	*/
	PHOS_GUI_COMPONENT_SHADOW,
	/**
	  @see phos_gui_texture_component
	*/
	PHOS_GUI_COMPONENT_TEXTURE,
	/**
	  @see phos_gui_mouse_listener_component
	*/
	PHOS_GUI_COMPONENT_MOUSE_LISTENER,
	/**
	  @see phos_gui_text_component
	*/
	PHOS_GUI_COMPONENT_TEXT,
	/**
	  An extension of the text component.

	  @see phos_gui_text_component
	  @see phos_gui_placeholder_text_extension
	*/
	PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT,
	/**
	  @see phos_gui_label_component
	*/
	PHOS_GUI_COMPONENT_LABEL,
	/**
	  @see phos_gui_layout_component
	*/
	PHOS_GUI_COMPONENT_LAYOUT,
	/**
	  @see phos_gui_scroll_pane_component
	*/
	PHOS_GUI_COMPONENT_SCROLL_PANE,
	/**
	  @see phos_gui_drag_pane_component
	*/
	PHOS_GUI_COMPONENT_DRAG_PANE,
	/**
	  @see phos_gui_drop_down_component
	*/
	PHOS_GUI_COMPONENT_DROP_DOWN,
	/**
	  @see phos_gui_checkbox_list_component
	*/
	PHOS_GUI_COMPONENT_CHECKBOX_LIST,
	/**
	  @see phos_gui_value_bar_component
	*/
	PHOS_GUI_COMPONENT_VALUE_BAR,
	/**
	  @see phos_gui_icon_list_component
	*/
	PHOS_GUI_COMPONENT_ICON_LIST,

	/**
	  Represents the last component ID in PhosphorusGUI.
	  
	  @note PhosphorusGUI does not register a component using this ID.

	  If using custom components in your own program, set the first
	  component ID to this exact value. It will ensure your components
	  have unique IDs.

	  Example:

	  @code
	  enum my_components
	  {
		  C1 = PHOS_GUI_COMPONENT_MAX,
		  C2,
		  C3,
		  ...
	  }
	  @endcode
	*/
	PHOS_GUI_COMPONENT_MAX
} phos_gui_component_type;

/**
  The different positions and orientations for a shadow component.
*/
typedef enum phos_gui_shadow_edges
{
	/**
	  Indicates the shadow will be on the left edge only.
	*/
	PHOS_GUI_SHADOW_LEFT,
	/**
	  Indicates the shadow will be on the top edge only.
	*/
	PHOS_GUI_SHADOW_TOP,
	/**
	  Indicates the shadow will be on the right edge only.
	*/
	PHOS_GUI_SHADOW_RIGHT,
	/**
	  Indicates the shadow will be on the bottom edge only.
	*/
	PHOS_GUI_SHADOW_BOTTOM,
	/**
	  Indicates the shadow will be on the top and left edges of the element.
	*/
	PHOS_GUI_SHADOW_TOP_LEFT,
	/**
	  Indicates the shadow will be on the top and right edges of the element.
	*/
	PHOS_GUI_SHADOW_TOP_RIGHT,
	/**
	  Indicates the shadow will be on the bottom and left edges of the element.
	*/
	PHOS_GUI_SHADOW_BOTTOM_LEFT,
	/**
	  Indicates the shadow will be on the bottom and right edges of the element.
	*/
	PHOS_GUI_SHADOW_BOTTOM_RIGHT,
	/**
	  Indicates every edge should have a shadow.
	*/
	PHOS_GUI_SHADOW_ALL
} phos_gui_shadow_edges;
/**
  A phos_gui_shadow_component provides an element
  with a shadow on one of its edges.
*/
typedef struct phos_gui_shadow_component
{
	/**
	  The edges on the element where the shadow will be rendered.

	  The default value is PHOS_GUI_SHADOW_BOTTOM_RIGHT.
	*/
	phos_gui_shadow_edges edges;
	/**
	  The length of the shadow from the element's edge.

	  This is 10.0f by default.
	*/
	float length;
	/**
	  The initial color of the shadow.

	  This is PHOS_GUI_COLOR_DARK_GRAY by default.
	*/
	Color initial_color;
	/**
	  The fade color of the shadow.

	  As the shadow moves out, it becomes this
	  color.

	  This is PHOS_GUI_LIGHT_GRAY by default.
	*/
	Color fade_color;
} phos_gui_shadow_component;

/**
  A phos_gui_texture_component provides an element
  with a texture.

  When giving an element a texture, its shape and outline
  will no longer be rendered. The texture becomes the
  element's shape and outline. However, for calculations,
  the element's shape will still be used.
*/
typedef struct phos_gui_texture_component
{
	/**
	  The actual texture.

	  @note You should use phos_gui_load_texture(...)
	  when setting this field.

	  This is NULL by default. If the texture is still
	  NULL in the program loop, a warning message
	  is printed.
	*/
	Texture2D *src;
} phos_gui_texture_component;

/**
  The different types of mouse listeners.
*/
typedef enum phos_gui_mouse_listener_component_type
{
	/**
	  The default mouse listener type.
	*/
	PHOS_GUI_MOUSE_LISTENER_DEFAULT,
	/**
	  Results in the mouse listener being toggled
	  on/off instead of being clicked.

	  @note When using the toggle-style mouse listener,
	  the colors of the element are used differently.
	  The element's normal colors indicate that it is
	  toggled off. The colors on the mouse listener
	  indicate that it is toggled on.
	*/
	PHOS_GUI_MOUSE_LISTENER_TOGGLED
} phos_gui_mouse_listener_component_type;

/**
  A phos_gui_mouse_listener_component provides an element
  with the ability to receive mouse input.

  @important Mouse listener components give elemements the
  ability to gain and lose focus as well. Some other components
  may require the element to be focusable, such as an editable text
  component.
*/
typedef struct phos_gui_mouse_listener_component
{
	/**
	  The type of the mouse listener.
	*/
	phos_gui_mouse_listener_component_type type;

	/**
	  The background hover color.

	  This color is used when the mouse is hovering over the element.
	*/
	Color bg_hover_color;
	/**
	  The background press color.

	  This color is used when the mouse is held down while hovered over
	  the element.
	*/
	Color bg_press_color;
	/**
	  The background focus color.

	  This color is used when the element currently has focus.

	  @important This color is not used on toggle-style
	  mouse listeners.
	*/
	Color bg_focus_color;

	/**
	  The outline hover color.

	  This color is used when the mouse is hovering over the element.
	*/
	Color outline_hover_color;
	/**
	  The outline press color.

	  This color is used when the mouse is held down while hovered over
	  the element.
	*/
	Color outline_press_color;
	/**
	  The outline focus color.

	  This color is used when the element currently has focus.

	  @important This color is not used on toggle-style
	  mouse listeners.
	*/
	Color outline_focus_color;

	/**
	  Whether or not the element was clicked this frame.
	*/
	bool clicked;
	/**
	  Whether or not the element is being held down.
	*/
	bool pressed;
	/**
	  Whether or not the element was released this frame.
	*/
	bool released;
	/**
	  Whether or not the element was toggled this frame.

	  @note For a mouse listener to be toggled on/off, set the mouse listeners's
	  type to PHOS_GUI_MOUSE_LISTENER_TOGGLED.
	*/
	bool toggled;
	/**
	  Whether or not the element is toggled on.

	  @note For a mouse listener to be toggled on/off, set the mouse listener's
	  type to PHOS_GUI_MOUSE_LISTENER_TOGGLED.

	  This is false by default.
	*/
	bool toggled_on;
	/**
	  Whether or not the user is hovering over the element.
	*/
	bool hovered;

	/**
	  Whether or not the element has focus.
	*/
	bool has_focus;
	/**
	  Whether or not the element gained focus this frame.
	*/
	bool gained_focus;
	/**
	  Indicates whether or not the element should have focus
	  when its phos_gui is switched to.

	  @note Only one element in a phos_gui should have this
	  attriubte set to true. PhosphorusGUI only gives
	  focus to the first element added to a phos_gui
	  with this set to true.
	*/
	bool focus_on_start;
} phos_gui_mouse_listener_component;

/**
  In some PhosphorusGUI functions, a string
  is required for alignments, rendering, etc.
  For those functions, it needs to know the target
  string on an object. The two primary target strings
  are the main text (a phos_gui_text_component) or
  placeholder text (a phos_gui_placeholder_text_extension).
*/
typedef enum phos_gui_target_text_string
{
	/**
	  Indicates the main string on an object should
	  be used (the phos_gui_text_component).
	*/
	PHOS_GUI_TARGET_MAIN_TEXT,
	/**
	  Indicates the placeholder text on an object
	  should be used (the phos_gui_placeholder_text_extension).

	  @note The object must have a phos_gui_placeholder_text_extension
	  to use PHOS_GUI_TARGET_PLACEHOLDER_TEXT.
	*/
	PHOS_GUI_TARGET_PLACEHOLDER_TEXT,
	/**
	  Indicates that PhosphorusGUI should read from both
	  the main and placeholder text of an object and automatically
	  determine which one to use. The string with a longer length
	  is automatically chosen when using PHOS_GUI_TARGET_AUTO_TEXT.
	*/
	PHOS_GUI_TARGET_AUTO_TEXT
} phos_gui_target_text_string;

/**
  Determines how a text component should be wrapped.

  Text wrapping is when the characters the user types
  are forced onto the next line because there's no
  more space on the current line to store those characters.
*/
typedef enum phos_gui_text_wrap_mode
{
	/**
	  Indicates there's no wrapping.

	  This is the default wrap mode.
	*/
	PHOS_GUI_TEXT_WRAP_NONE,
	/**
	  Indicates that wrapping should
	  be performed on individual characters
	  only.
	*/
	PHOS_GUI_TEXT_WRAP_CHAR,
	/**
	  Indicates that wrapping should
	  be performed on whole words.
	*/
	PHOS_GUI_TEXT_WRAP_WORD
} phos_gui_text_wrap_mode;

/**
  A phos_gui_text_component represents a piece of
  text within an element.

  @note A text component can be combined with
  a scroll pane component for scrolling text.

  @see phos_gui_scroll_pane_component
*/
typedef struct phos_gui_text_component
{
	/**
	  This text component's actual string contents.

	  This buffer allocates (PHOS_GUI_MAX_TEXT_LEN + 1) bytes.
	*/
	char str[PHOS_GUI_MAX_TEXT_LEN + 1];

	/**
	  This text component's font.

	  @important If you load the font yourself,
	  it is up to you to unload it later. However,
	  if you use phos_gui_load_font(...) instead,
	  PhosphorusGUI will handle it all for you.
	*/
	Font *font;

	/**
	  The max number of chars that can be typed into this text component.

	  @note This value should be less than or equal to PHOS_GUI_MAX_TEXT_LEN.
	*/
	size_t max_len;
	/**
	  The current number of chars in this text component's current line.

	  The current line is the one the user is typing into.
	*/
	size_t curr_line_len;
	/**
	  The current number of chars that have been typed into this text component.

	  @note This value should be less than or equal to PHOS_GUI_MAX_TEXT_LEN or
	  the text component's 'max_len' field.
	*/
	size_t len;
	/**
	  The number of lines entered into the text.
	*/
	size_t num_lines;

	/**
	  The position of the cursor in this text component. This position is
	  used as an index into the 'str' field of the text component.
	*/
	size_t cursor_pos;

	/**
	  The number of spaces one single tab character represents
	  in this text component.

	  This is 4 by default.
	*/
	size_t spaces_per_tab;

	/**
	  The relative position of this text within its parent element.
	*/
	Vector2 offset;

	/**
	  This text component's font size.

	  The default font size for all text components
	  is equal to PHOS_GUI_FONT_SIZE_DEFAULT (32.0f).
	*/
	float font_size;

	/**
	  The wrap mode for this text component.

	  This is PHOS_GUI_TEXT_WRAP_NONE by default.
	*/
	phos_gui_text_wrap_mode wrap_mode;
	/**
	  The alignment the text component is using.

	  @important This alignment must always be one of the
	  'INNER' alignments. This is because when text is rendered,
	  it is clipped so that it fits into its owner's visual
	  bounds.

	  By default, this is equal to PHOS_GUI_ALIGN_INNER_CENTER.
	*/
	phos_gui_alignment alignment;
	/**
	  As the user edits the text, these options
	  are executed.

	  This is equal to PHOS_GUI_OPTS_NONE by default.

	  @important For the most part, PhosphorusGUI
	  automatically realigns text components, except
	  for when a text component is edited. You must
	  explicitly apply the PHOS_GUI_OPTS_REALIGN_TEXT
	  option here if desired.
	*/
	phos_gui_opts edit_opts;

	/**
	  The color of the text component's main contents ('str').
	*/
	Color color;

	/**
	  The last key the user typed into this text component.

	  If there was no character typed, it will be equal to KEY_NULL.
	*/
	int key_typed;
	/**
	  The last char the user typed into this text component.

	  If there was no character typed, it will be equal to KEY_NULL.
	*/
	char char_typed;

	/**
	  Whether or not the user can edit this text component.

	  This is false by default.
	*/
	bool editable;
	/**
	  Whether or not the text was edited by the user this frame.

	  Hitting a key like the enter key does not mark the text
	  as edited. Only updating the contents of the text marks
	  it as edited.
	*/
	bool edited;
	/**
	  Whether or not letters are accepted in this text's input.
	*/
	bool accept_letters;
	/**
	  Whether or not numbers are accepted in this text's input.
	*/
	bool accept_nums;
	/**
	  Whether or not special characters are accepted in this text's input.
	*/
	bool accept_specials;
	/**
	  Whether or not the ENTER key inserts a new line into the text.

	  By default, this is false.
	*/
	bool enter_inserts_new_line;
	/**
	  Whether or not new lines should automatically be indented
	  to match the indent of the previous line.

	  @note The user must be able to enter new lines
	  into the text component for this to work.

	  This is false by default.

	  @see enter_inserts_new_line
	*/
	bool auto_indent;
} phos_gui_text_component;

/**
  An extension of the text component.
*/
typedef struct phos_gui_placeholder_text_extension
{
	/**
	  The string contents of the placeholder text.
	*/
	char str[PHOS_GUI_MAX_TEXT_LEN + 1];

	/**
	  The text component being extended.
	*/
	phos_gui_text_component *host;

	/**
	  The color of the placeholder text.
	*/
	Color color;
} phos_gui_placeholder_text_extension;

/**
  A phos_gui_label_component is very similar
  to a text component but the text is never editable
  and can reside outside of an element's content bounds.
*/
typedef struct phos_gui_label_component
{
	/**
	  The label's text.
	*/
	char str[PHOS_GUI_MAX_TEXT_LEN + 1];

	/**
	  This label component's font.

	  @important If you load the font yourself,
	  it is up to you to unload it later. However,
	  if you use phos_gui_load_font(...) instead,
	  PhosphorusGUI will handle it all for you.
	*/
	Font *font;

	/**
	  The position of this label relative to its owner.
	*/
	Vector2 offset;

	/**
	  The size of the label's font.
	*/
	float font_size;

	/**
	  The alignment of the label.

	  This is equal to PHOS_GUI_ALIGN_LEFT
	  by default.
	*/
	phos_gui_alignment alignment;

	/**
	  The color of the label.
	*/
	Color color;
} phos_gui_label_component;

/**
  Indicates how layouts should resolve row and column
  additions.
*/
typedef enum phos_gui_layout_flow
{
	/**
	  The default resolution for layouts. This results
	  in a row being filled before moving
	  to the next row.
	*/
	PHOS_GUI_LAYOUT_ROW_MAJOR,
	/**
	  Results in each column being filled before moving
	  to the next column.
	*/
	PHOS_GUI_LAYOUT_COLUMN_MAJOR
} phos_gui_layout_flow;
/**
  A phos_gui_layout_component provides an element
  with a specific layout and formatting technique.

  @important When adding a layout component to an element,
  it is very important to remember that layout components
  operate on ALL children inside the element. If you are
  working with an element and its layout, and you want to
  have a child in the element not be included in the layout,
  you must either place all layout children in a subcontainer,
  or the other element you do not want to format should be a
  separate element in the GUI.
*/
typedef struct phos_gui_layout_component
{
	/**
	  The number of rows in the layout.
	*/
	size_t rows;
	/**
	  The number of columns in the layout.
	*/
	size_t cols;

	/**
	  The amount of pixels on the x-axis between each element.
	*/
	float spacing_x;
	/**
	  The amount of pixels on the y-axis between each element.
	*/
	float spacing_y;

	/**
	  The total amount of pixels this layout component takes up
	  horizontally.

	  @important PhosphorusGUI sets this value automatically during
	  phos_gui_format_children(...).
	*/
	float total_content_width;
	/**
	  The total amount of pixels this layout component takes up
	  vertically.

	  @important PhosphorusGUI sets this value automatically during
	  phos_gui_format_children(...).
	*/
	float total_content_height;

	/**
	  Indicates how the layout should place the elements inside of it.
	*/
	phos_gui_layout_flow flow;

	/**
	  Indicates whether or not phos_gui_format_children(...) forces
	  all the layout's children to fit into the layout's total content area.
	  By default, this is true.
	*/
	bool auto_fit_children;
	/**
	  Indicates whether or not phos_gui_format_children(...) clamps
	  the parent element's size to match the total amount of space
	  its children take up. By default, this is false.
	*/
	bool clamp_parent;
} phos_gui_layout_component;

/**
  A phos_gui_scroll_bar is used in the phos_gui_scroll_pane_component
  to provide additional scrolling capabilities.

  @see phos_gui_scroll_pane_component
*/
typedef struct phos_gui_scroll_bar
{
	/**
	  The background color of the entire bar.
	*/
	Color bg_color;
	/**
	  The color of the scroll thumb.
	*/
	Color thumb_color;
	/**
	  The color of the scroll thumb when the user is
	  interacting with it.
	*/
	Color thumb_focus_color;
	/**
	  The shape of the scroll thumb. By default, this
	  is PHOS_GUI_SHAPE_RECT.
	*/
	phos_gui_shape thumb_shape;
	/**
	  The roundness of the scroll thumb.

	  @note This value is only used if the scroll thumb's shape is PHOS_GUI_SHAPE_ROUND_RECT.

	  @important This value should remain in between 0.0f (no roundess)
	  and 1.0f (full roundness).

	  By default, this is 0.0f.
	*/
	float thumb_corner_radius;
	/**
	  The width/height of the scroll bar based on its orientation.

	  This is 15.0f by default.
	*/
	float span;
	/**
	  Used when the user grabs the scroll thumb of this scroll bar.

	  Represents the distance between the user's initial mouse click
	  and the scroll thumb's origin.
	*/
	float thumb_grab_offset;
	/**
	  Whether or not the scroll thumb has focus.
	*/
	bool thumb_has_focus;
	/**
	  Whether or not the user has the scroll
	  thumb grabbed.
	*/
	bool thumb_grabbed;
	/**
	  Whether or not this bar is rendered in the scroll pane.

	  This is true by default.
	*/
	bool rendered;
	/**
	  Whether or not this bar can be used to scroll.

	  This is true by default.
	*/
	bool active;
} phos_gui_scroll_bar;

/**
  A phos_gui_scroll_pane_component provides an element
  with the ability to be scrolled. Scroll panes result
  in the element's children being clipped if not in the
  visible region. The way the pane is scrolled can also
  be customized.

  @important When adding a scroll pane component to an element,
  it is very important to remember that scroll panes operate
  on ALL children inside the element. If you want to make an
  element scrollable but that element contains other children
  you do not want to include in the scroll pane, you must
  either place all children in a subcontainer, or the
  non-scrollable elements must be separate elements in the GUI.
*/
typedef struct phos_gui_scroll_pane_component
{
	/**
	  The vertical scroll bar for the scroll pane.
	*/
	phos_gui_scroll_bar v_bar;
	/**
	  The horizontal scroll bar for the scroll pane.
	*/
	phos_gui_scroll_bar h_bar;

	/**
	  Determines how many pixels are in one scroll tick
	  when using the mouse wheel to scroll.

	  By default, this is equal to 10.0f, for 10 pixels per tick.

	  @note This should remain positive, but making it negative
	  inverts the scrolling direction.
	*/
	float px_per_tick;

	/**
	  Scroll amount on the x-axis.
	*/
	float scroll_x;
	/**
	  Max amount of scroll on the x-axis.
	*/
	float max_scroll_x;
	/**
	  Scroll amount on the y-axis.
	*/
	float scroll_y;
	/**
	  Max amount of scroll on the y-axis.
	*/
	float max_scroll_y;

	/**
	  Whether or not mouse wheel input will modify the scroll
	  pane. This is true by default.
	*/
	bool use_mouse_wheel_input;
} phos_gui_scroll_pane_component;

/**
  The different drag bar orientations.
*/
typedef enum phos_gui_drag_bar_orientation
{
	/**
	  The default drag bar orientation:
	  the drag bar resides at the element's top
	  edge and spans its width.
	*/
	PHOS_GUI_DRAG_BAR_HORIZONTAL_TOP,
	/**
	  The drag bar will be the left edge of the
	  element and span its height.
	*/
	PHOS_GUI_DRAG_BAR_VERTICAL_LEFT,
	/**
	  The drag bar will be at the right edge of the
	  element and span its height.
	*/
	PHOS_GUI_DRAG_BAR_VERTICAL_RIGHT,
	/**
	  The drag bar will be at the bottom edge of
	  the element and span its width.
	*/
	PHOS_GUI_DRAG_BAR_HORIZONTAL_BOTTOM,
} phos_gui_drag_bar_orientation;

/**
  A phos_gui_drag_pane_component provides an element with the ability
  to be dragged around the screen using the mouse.
*/
typedef struct phos_gui_drag_pane_component
{
	/**
	  The amount of pixels the drag pane has been moved
	  since the last frame.
	*/
	Vector2 drag_delta;
	/**
	  If the drag pane uses a drag bar, this determines
	  the drag bar's orientation.
	*/
	phos_gui_drag_bar_orientation drag_bar_orientation;
	/**
	  If the drag pane uses a drag bar, this color
	  determines the color of the drag bar. The drag
	  bar color is PHOS_GUI_COLOR_LIGHT_GRAY by default.
	*/
	Color drag_bar_color;

	/**
	  Provides this drag pane with additional options
	  when the element is dragged.

	  By default, this is set to PHOS_GUI_OPTS_NONE which means the drag
	  pane will not have any options.

	  @see phos_gui_opts
	*/
	phos_gui_opts drag_opts;

	/**
	  The width/height of the drag bar based on its orientation.

	  This is 35.0f by default.
	*/
	float span;

	/**
	  Determines how the user drags the element.

	  By default, a drag pane is dragged when the
	  user grabs anywhere within the element. However,
	  setting this value to true provides the
	  element with a drag bar. If the drag pane uses a
	  drag bar, the user must grab the drag bar to drag
	  the element.
	*/
	bool use_drag_bar;

	/**
	  Indicates whether or not the user is currently dragging
	  this drag pane.
	*/
	bool grabbed;
} phos_gui_drag_pane_component;

/**
  A phos_gui_drop_down_component provides an element with the ability
  to have a list of options for the user to select from.

  You should add a drop-down component to the element representing
  the drop-down button.
*/
typedef struct phos_gui_drop_down_component
{
	/**
	  The container the dropdown will use.

	  @important This should point to a child element
	  on the dropdown representing the main container
	  holding the dropdown's options.
	*/
	struct phos_gui_elem *container;
	/**
	  The element in 'container' the user has chosen.

	  If no selection has been made, this will be NULL.
	*/
	struct phos_gui_elem *selection;
	
	/**
	  Whether or not the dropdown is expanded.
	  By default, this is false.
	*/
	bool expanded;
} phos_gui_drop_down_component;

/**
  A phos_gui_checkbox_list_component provides an element
  with the ability to have a list of options for the
  user to select from.
*/
typedef struct phos_gui_checkbox_list_component
{
	/**
	  The elements in 'container' the user has chosen.

	  The elements are added to this array in the order
	  they are selected.
	*/
	struct phos_gui_elem *selections[PHOS_GUI_MAX_CHECKBOXES];

	/**
	  The number of options the user can select
	  within the list.

	  By default, this is equal to 1, which means
	  the user can only select one option at a time.

	  Increase this value to let the user select more
	  than one option at a time.

	  @see PHOS_GUI_MAX_CHECKBOXES
	*/
	size_t num_available_options;
	/**
	  The current number of options the user has chosen.
	*/
	size_t num_options_selected;
} phos_gui_checkbox_list_component;

/**
  A phos_gui_value_bar_component provides an element
  with a visual representation of a specific value,
  percentage, etc, using a min/max value.
*/
typedef struct phos_gui_value_bar_component
{
	/**
	  The minimum value in the value bar.

	  This is 0.0f by default.
	*/
	float min_value;
	/**
	  The maximum value in the value bar.

	  This is 100.0f by default.
	*/
	float max_value;
	/**
	  The current value in the value bar.

	  This is 0.0f by default.
	*/
	float curr_value;

	/**
	  The shape of the value bar's slider knob.

	  This shape is only used when the value
	  bar becomes a slider.

	  This is PHOS_GUI_SHAPE_RECT by default.

	  @see editable
	*/
	phos_gui_shape slider_knob_shape;
	/**
	  The corner radius of the slider knob (if the
	  slider knob is being used and its shape
	  is PHOS_GUI_SHAPE_ROUND_RECT).

	  This is 0.0f by default.

	  @see slider_knob_shape
	*/
	float slider_knob_corner_radius;
	/**
	  The width and height of the slider knob.

	  By default, this is equal to the height of the
	  element's free content bounds *
	  3.0f.
	*/
	float slider_knob_span;
	/**
	  Where the user grabbed the slider knob.
	*/
	float slider_knob_grab_offset;

	/**
	  The color of the slider knob (if the
	  slider knob is being used).

	  This is WHITE by default.

	  @see slider_knob_shape
	*/
	Color slider_knob_color;
	/**
	  The color of the slider knob (if the
	  slider knob is being used) when the
	  user hovers over or grabs the slider
	  knob.

	  This is PHOS_GUI_COLOR_GRAY by default.

	  @see slider_knob_shape
	*/
	Color slider_knob_focus_color;
	/**
	  The color of the current value, or
	  the progress bar.

	  When rendering a value bar,
	  the element's background color
	  is used as the background of the
	  value bar. This color will be used
	  to render the rectangle representing
	  the value bar's progress.

	  This is PHOS_GUI_COLOR_GREEN by default.
	*/
	Color progress_color;

	/**
	  Whether or not the user can directly
	  edit this value bar.

	  When a value bar becomes editable,
	  it is referred to as a slider.
	  After a value bar becomes a slider,
	  its slider knob is rendered for the user.
	  The slider knob is what the user grabs
	  to change the current value of the value bar.

	  By default, this is false.
	*/
	bool editable;

	/**
	  Whether or not the slider knob on the value
	  bar was grabbed this frame.
	*/
	bool slider_knob_grabbed;
	/**
	  Whether or not the slider knob on the value
	  bar was released this frame.
	*/
	bool slider_knob_released;
	/**
	  Whether or not the sldier knob on the bar
	  has focus.
	*/
	bool slider_knob_has_focus;

	/**
	  Whether or not the slider knob
	  should snap to each individual
	  value in the value bar.

	  This is false by default.
	*/
	bool slider_knob_snapping;
} phos_gui_value_bar_component;

/**
  The list of different icons for the program.

  All icons are stored in the 'icons' directory.
  Because of this, PhosphorusGUI only needs
  the actual icon file's name, and it automatically
  looks for that icon in the 'icons' directory.
*/
typedef enum phos_gui_icon_id
{
	/**
	  The check mark icon.
	*/
	PHOS_GUI_ICON_CHECK_MARK,
	/**
	  The vertical bars icon.
	*/
	PHOS_GUI_ICON_VERTICAL_BARS,
	/**
	  The horizontal bars icon.
	*/
	PHOS_GUI_ICON_HORIZONTAL_BARS,
	/**
	  The vertical dots icon.
	*/
	PHOS_GUI_ICON_VERTICAL_DOTS,
	/**
	  The horizontal dots icon.
	*/
	PHOS_GUI_ICON_HORIZONTAL_DOTS,
	/**
	  The down arrow icon.
	*/
	PHOS_GUI_ICON_ARROW_DOWN,
	/**
	  The max number of icons.
	*/
	PHOS_GUI_ICON_MAX
} phos_gui_icon_id;

/**
  Represents an actual icon object.

  @see phos_gui_icon_id
*/
typedef struct phos_gui_icon
{
	/**
	  The position and size of the icon.
	*/
	Rectangle bounds;
	/**
	  The icon ID to use.
	*/
	phos_gui_icon_id ID;
	/**
	  The color of the icon.
	*/
	Color color;
	/**
	  The alignment of the icon.

	  This is equal to PHOS_GUI_ALIGN_LEFT
	  by default.
	*/
	phos_gui_alignment alignment;
	/**
	  Whether or not the icon should
	  be rendered.
	*/
	bool visible;
} phos_gui_icon;

/**
  A phos_gui_icon_list_component gives an
  element the ability to store and render
  icons.
*/
typedef struct phos_gui_icon_list_component
{
	/**
	  The actual list of icons on the element.
	*/
	phos_gui_icon icons[PHOS_GUI_MAX_ICONS];

	/**
	  The number of icons currently stored
	  on the element.
	*/
	size_t num_icons;
} phos_gui_icon_list_component;

/**
  Represents an actual bounding box for an element.
*/
typedef struct phos_gui_elem_rect
{
	/**
	  The actual rectangle for the element.
	*/
	Rectangle rect;
	/**
	  Determines whether or not this rectangle
	  should be recalculated for caching.

	  @important PhosphorusGUI automatically
	  sets this field when necessary, so do
	  not modify this field. If you explicitly
	  want to request for the library to recalculate
	  the rectangle, use phos_gui_prepare_rects_for_calculating(phos_gui_elem*).
	*/
	bool should_calculate;
} phos_gui_elem_rect;

/**
  The different types of events an event listener
  can listen for.
*/
typedef enum phos_gui_event_type
{
	/**
	  An invalid/null event.
	*/
	PHOS_GUI_EVENT_NONE,
	/**
	  Used to listen for a single mouse click.
	*/
	PHOS_GUI_EVENT_MOUSE_CLICK,
	/**
	  Used to listen for the mouse being held down.
	*/
	PHOS_GUI_EVENT_MOUSE_DOWN,
	/**
	  Used to listen for the mouse being released.
	*/
	PHOS_GUI_EVENT_MOUSE_RELEASE,
	/**
	  Used to listen for a single key press.
	*/
	PHOS_GUI_EVENT_KEY_CLICK,
	/**
	  Used to listen for a key being held down.
	*/
	PHOS_GUI_EVENT_KEY_DOWN,
	/**
	  Used to listen for a hover event.
	*/
	PHOS_GUI_EVENT_HOVER,
} phos_gui_event_type;

/**
  Provides an event listener with executable code.

  The function returns nothing and takes in the target
  object, or NULL if the target object was the window,
  as well as additional options.

  @see phos_gui_event_listener
*/
typedef void (*phos_gui_event_listener_action) (struct phos_gui_elem *elem, phos_gui_opts opts);

/**
  A phos_gui_event_listener gives PhosphorusGUI information
  about an event, such as a mouse click or key press, and then
  an action to execute when that event occurs.
*/
typedef struct phos_gui_event_listener
{
	/**
	  The action to execute when the event occurs.
	*/
	phos_gui_event_listener_action action;

	/**
	  The target element.

	  The target element is what will be receiving
	  the user's input.

	  @note This can be NULL but when it is NULL,
	  PhosphorusGUI assumes the target of the event
	  is the window.
	*/
	struct phos_gui_elem *elem;

	/**
	  The type of event to listen for.
	*/
	phos_gui_event_type event;

	/**
	  If wanting to pass additional options into
	  the action function, add the options here.
	*/
	phos_gui_opts opts;

	/**
	  Indicates the target button or key the event
	  listener should listen for.

	  This could be MOUSE_BUTTON_LEFT, KEY_ESCAPE,
	  etc.
	*/
	int target_btn;
} phos_gui_event_listener;

/**
  A phos_gui_elem represents a single UI element
  within a phos_gui.

  @important When modifying an element, you should
  use any functions available to do so. For example, to move
  an element, do not directly modify its 'bounds' field. Instead
  use phos_gui_move_elem(...) or phos_gui_set_elem_pos(...), etc.
  These functions will automatically calculate all of the element's
  rectangles, whereas directly changing its position won't.
*/
typedef struct phos_gui_elem
{
	/**
	  The event listeners added to this GUI.
	*/
	phos_gui_event_listener listeners[PHOS_GUI_MAX_EVENT_LISTENERS];

	/**
	  This element's children.
	*/
	struct phos_gui_elem *children[PHOS_GUI_MAX_CHILDREN];

	/**
	  This UI element's ID.

	  The ID should be unique.

	  @important If you assign the ID to
	  '<auto>' then PhosphorusGUI will
	  automatically generate an ID for
	  the element.
	*/
	char ID[PHOS_GUI_MAX_ID_LEN + 1];

	/**
	  This element's total bounds.

	  An element's total bounds is the total amount
	  of space it takes up on screen, taking the
	  element's margins into account.
	*/
	phos_gui_elem_rect total_bounds;
	/**
	  This element's total content bounds.

	  An element's total content bounds is the inner
	  area on an element, but it only takes padding
	  and outline thickness into account.
	*/
	phos_gui_elem_rect content_total_bounds;
	/**
	  This element's free content bounds.

	  An element's free content bounds is the second
	  inner area on an element. After padding and outline
	  thickness, the free content bounds rect also
	  adds offsets derived from decorations on the element
	  such as scroll or drag bars. This area is also where
	  children on the element are stored.
	*/
	phos_gui_elem_rect content_free_bounds;
	/**
	  This element's position and size before
	  any additional calculations or formatting
	  options.
	*/
	Rectangle bounds;

	/**
	  The phos_gui instance this element belongs to.
	*/
	struct phos_gui *gui;

	/**
	  This element's parent.

	  If the element has no parent, this is equal to NULL.
	*/
	struct phos_gui_elem *parent;

	/**
	  This element's number of children.
	*/
	size_t num_children;
	/**
	  The current amount of event listeners added to this GUI.
	*/
	size_t num_listeners;

	/**
	  The type of this element.
	*/
	phos_gui_elem_type type;
	/**
	  The shape of this element.

	  This is PHOS_GUI_SHAPE_RECT by default.
	*/
	phos_gui_shape shape;
	/**
	  How this element should be rendered.

	  @important If this is set to PHOS_GUI_OUTLINE,
	  PhosphorusGUI will not use the element's outline color field when
	  rendering it. It will instead use the element's 'color' field (while
	  also still taking mouse hover and press effects into account).
	*/
	phos_gui_elem_render_mode render_mode;
	/**
	  The alignment this element used last.
	*/
	phos_gui_alignment alignment;
	/**
	  If this element was added as a child to another
	  element, this includes the options given to
	  phos_gui_add_child(...).
	*/
	phos_gui_opts child_opts;
	/**
	  When an element is tested for mouse input,
	  this rectangle determines where on the element
	  the mouse becomes hovered over the element,
	  and can interact with the element.

	  This is equal to PHOS_GUI_ELEM_BOUNDS_REAL by default.
	*/
	phos_gui_elem_bounding_box input_test_bounds;

	/**
	  The element's background color.
	*/
	Color bg_color;
	/**
	  The element's outline color.
	*/
	Color outline_color;
	/**
	  The color of the element when it is disabled.

	  @see disabled
	*/
	Color disabled_color;

	/**
	  The thickness of the element's outline.

	  @note If set to 0, it will not be visible.
	*/
	float outline_thickness;
	/**
	  The roundness of the element's corners.

	  @note This value is only used for elements with the
	  PHOS_GUI_ROUND_RECT shape.

	  @important This value should remain in between
	  0.0f (no roundness) and 1.0f (full roundness).

	  By default, this is 0.0f.
	*/
	float corner_radius;

	/**
	  This value represents the amount of space inside of the element on the left.
	*/
	float left_padding;
	/**
	  This value represents the amount of space inside of the element on the top.
	*/
	float top_padding;
	/**
	  This value represents the amount of space inside of the element on the right.
	*/
	float right_padding;
	/**
	  This value represents the amount of space inside of the element on the bottom.
	*/
	float bottom_padding;

	/**
	  This value represents the amount of space on the left side of the element.
	*/
	float left_margin;
	/**
	  This value represents the amount of space above the element.
	*/
	float top_margin;
	/**
	  This value represents the amount of space on the right side of the element.
	*/
	float right_margin;
	/**
	  This value represents the amount of space below the element.
	*/
	float bottom_margin;

	/**
	  Indicates whether or not this element is currently
	  disabled.

	  A disabled element cannot be interacted with, but it
	  is still rendered. However, when it is rendered,
	  it uses the 'disabled_color' to tint the element
	  a specific color letting the user know they cannot
	  interact with it. Additionally, if the element's
	  outline is rendered, the normal color on its outline
	  color set is used.

	  This is false by default.
	*/
	bool disabled;

	/**
	  Indicates whether or not PhosphorusGUI automatically
	  renders this element in the program loop.
	*/
	bool visible;

	/**
	  Indicates whether or not this element should be clipped
	  when rendered.

	  This is false by default.
	*/
	bool clipped;
} phos_gui_elem;

/**
  Provides a timer with executable code.

  Whenever a timer's target time is reached,
  this function is executed.

  @param args The function should take in a
  void pointer representing any generic arguments.
  It is up to the user to handle these arguments
  and how the timer uses them.

  @see phos_gui_timer
*/
typedef void (*phos_gui_timer_action) (void *args);

/**
  A phos_gui_timer allows the user to execute
  events over and over based on a time interval
  in seconds.
*/
typedef struct phos_gui_timer
{
	/**
	  The action this timer should
	  execute when its target time
	  is reached.
	*/
	phos_gui_timer_action action;
	/**
	  The arguments the timer should
	  use in the action function.
	*/
	void *args;

	/**
	  The number of times this timer should execute.

	  Set this to -1 to indicate the timer should
	  loop forever.
	*/
	int execution_count;

	/**
	  The target time for the timer.

	  For example, to repeat an action
	  every second, this should be 1.0f.
	*/
	float target_time;
	/**
	  The timer's current time in seconds;
	*/
	float curr_time;
} phos_gui_timer;

/**
  The different interpretations of an animation's
  end value.
*/
typedef enum phos_gui_animation_end_value_interpretation
{
	/**
	  The default interpretation.

	  Indicates that the end value is absolute,
	  meaning the end value is exactly what the
	  user sets on the animation.
	*/
	PHOS_GUI_ANIMATION_END_VALUE_ABSOLUTE,
	/**
	  Indicates the end value is relative
	  to the starting value.

	  For example, if the starting value is 100 and the
	  end value is 200, the actual end value
	  is 300.
	*/
	PHOS_GUI_ANIMATION_END_VALUE_RELATIVE
} phos_gui_animation_end_value_interpretation;

/**
  Animations are used to modify a float value
  over time.
*/
typedef struct phos_gui_animation
{
	/**
	  If this animation modifies
	  an element directly, make this
	  point to that element.

	  PhosphorusGUI will need to reload
	  that element as the animation
	  modifies it so that all of its
	  rectangles are updated correctly.

	  If this is NULL, PhosphorusGUI
	  assumes you are not modifying an element
	  and therefore no reloading occurs.
	*/
	phos_gui_elem *elem;

	/**
	  The starting float value.
	*/
	float *curr_value;

	/**
	  The target end value.
	*/
	float end_value;

	/**
	  The total amount of seconds that should
	  elapse during the animation.
	*/
	float duration;

	/**
	  The current time elapsed for this animation.

	  @important Do not modify this value. PhosphorusGUI
	  automatically updates it and tracks it.
	*/
	float curr_time;
	/**
	  The amount of time that must pass before an update
	  to the animation is required.

	  @important Do not modify this value. PhosphorusGUI
	  automatically updates it and tracks it.
	*/
	float target_time;
	/**
	  The amount that 'curr_value' changes each frame.

	  @note You can modify this value to tweak how fast
	  or slow the animation moves.
	*/
	float step;

	/**
	  The number of times this animation
	  should execute.

	  Set this to -1 to indicate the animation
	  should loop forever.
	*/
	int execution_count;

	/**
	  As the animation updates, this
	  options are used in any necessary
	  function calls.
	*/
	phos_gui_opts opts;
} phos_gui_animation;

/**
  A phos_gui_theme represents a custom and global set of
  styles for elements in a phos_gui.

  To apply a theme to a phos_gui, you have to create the
  theme first, then use phos_gui_apply_theme_to_gui(phos_gui*, phos_gui_theme)
  to use it.

  PHOS_GUI_THEME_MONOTONE is the default theme of PhosphorusGUI.
*/
typedef struct phos_gui_theme
{
	/**
	  Global background color for all elements in the theme.
	*/
	Color bg_color;
	/**
	  Global outline color for all elements in the theme.
	*/
	Color outline_color;
	/**
	  Global background hover color for all elements in the theme.

	  @note Only applies to elements with mouse listener components.
	*/
	Color bg_hover_color;
	/**
	  Global background press color for all elements in the theme.

	  @note Only applies to elements with mouse listener components.
	*/
	Color bg_press_color;
	/**
	  Global background focus color for all elements in the theme.

	  @note Only applies to elements with mouse listener components.
	*/
	Color bg_focus_color;
	/**
	  Global outline hover color for all elements in the theme.

	  @note Only applies to elements with mouse listener components.
	*/
	Color outline_hover_color;
	/**
	  Global outline press color for all elements in the theme.

	  @note Only applies to elements with mouse listener components.
	*/
	Color outline_press_color;
	/**
	  Global outline focus color for all elements in the theme.

	  @note Only applies to elements with mouse listener components.
	*/
	Color outline_focus_color;
	/**
	  Global color for decorations.

	  Decorations are things like scroll bars, drag bars, etc.

	  @important This always applies to the background of decorations.
	  If a component has multiple colors for a decoration, then the
	  other colors are automatically generated using this color as the
	  base color. For example, the scroll bar decoration has 3 colors:
	  one for the scroll bar's background, and then one for when the thumb
	  is in its default state, and one for when the thumb has focus. The
	  theme's decoration color applies to the scroll bar's background color.
	  Then the scroll thumb's colors are automatically generated.
	*/
	Color decoration_color;
	/**
	  Global text color.
	*/
	Color text_color;
	/**
	  Global icon color.
	*/
	Color icon_color;
	/**
	  Global background color for the window.
	*/
	Color window_bg_color;
	/**
	  Global outline thickness in the theme.
	*/
	float outline_thickness;
} phos_gui_theme;

/**
  A phos_gui is used to store and organize UI elements.

  It represents the scene, or the context, in which the
  elements live in. This makes a phos_gui the top-level
  container.
*/
typedef struct phos_gui
{
	/**
	  The elements inside the GUI.
	*/
	phos_gui_elem *elems[PHOS_GUI_MAX_ELEMS];
	/**
	  The timers added to this GUI.
	*/
	phos_gui_timer timers[PHOS_GUI_MAX_TIMERS];
	/**
	  The animations added to this GUI.
	*/
	phos_gui_animation anims[PHOS_GUI_MAX_ANIMATIONS];

	/**
	  This GUI's ID.

	  The ID should be unique.

	  @important If you assign the ID to
	  '<auto>' then PhosphorusGUI will
	  automatically generate an ID for
	  the element.
	*/
	char ID[PHOS_GUI_MAX_ID_LEN + 1];

	/**
	  The current amount of elements inside this GUI.
	*/
	size_t num_elems;
	/**
	  The current amount of timers added to this GUI.
	*/
	size_t num_timers;
	/**
	  The current amount of animations added to this GUI.
	*/
	size_t num_anims;
} phos_gui;


/**
  Initializes the PhosphorusGUI library.

  @important You must call pluto_cs_init()
  before calling phos_gui_init(). PhosphorusGUI
  expects PlutoniumCS's component system to be working
  when it is called so that component types can be
  automatically registered for you. PhosphorusGUI
  will not call pluto_cs_init() or pluto_cs_shutdown()
  for you. Do not forget to call phos_gui_shutdown() before
  the end of the program and before CloseWindow()!

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_init(void);
/**
  Frees all resources used by the PhosphorusGUI library.
*/
PHOS_GUI_API void phos_gui_shutdown(void);
/**
  Automatically closes and disposes of system resources
  and exits the program with the given exit code.

  @important Only call this function once PlutoniumCS,
  PhosphorusGUI, and Raylib have all been initialized!
*/
PHOS_GUI_API void phos_gui_exit(int exit_code);

/**
  Sets the current phos_gui to use for updating
  and rendering.

  Because a program can have multiple different
  screens and scenes, it is best to create and set
  them up in your main function, then just
  manage switching between them when necessary.
  PhosphorusGUI will handle everything else.

  @note Every time PhosphorusGUI reads a new phos_gui
  pointer here, it will automatically register the phos_gui
  instance. It means you can later use phos_gui_get_gui_by_id(const char *ID)
  to obtain that phos_gui instance from anywhere in the program.

  @important You can pass NULL into function
  to signal to PhosphorusGUI there is no GUI
  to render at the moment.
*/
PHOS_GUI_API void phos_gui_set_gui(phos_gui *new_gui);
/**
  Sets the current phos_gui to use for updating
  and rendering.

  This function searches for the phos_gui matching the given ID.

  @see phos_gui_set_gui(phos_gui*)
*/
PHOS_GUI_API void phos_gui_set_gui_by_id(const char *ID);
/**
  Returns the current phos_gui instance PhosphorusGUI
  is working on.
*/
PHOS_GUI_API phos_gui *phos_gui_get_curr_gui(void);
/**
  Returns the previous phos_gui instance PhosphorusGUI
  was working on. If there was no previous phos_gui,
  then this function returns NULL instead.
*/
PHOS_GUI_API phos_gui *phos_gui_get_prev_gui(void);
/**
  Returns the phos_gui instance with the given ID,
  or NULL if no phos_gui is found.
*/
PHOS_GUI_API phos_gui *phos_gui_get_gui(const char *ID);

/**
  Centers a UI element inside the given bounds.

  @param elem The UI element to center.
  @param origin The origin of the container the element is being centered in.
  @param bounds The size of the container the element is being centered in.
*/
PHOS_GUI_API void phos_gui_center_elem(phos_gui_elem *elem, Vector2 origin, Vector2 size);

/**
  Moves an element x pixels horizontally and y pixels vertically.
*/
PHOS_GUI_API void phos_gui_move_elem(phos_gui_elem *elem, float x, float y, phos_gui_opts opts);
/**
  Resizes an element by w pixels horizontally and h pixels vertically.

  For example, if 'w' is 200 and 'h' is -100, the element's
  width grows by 200 pixels and its height shrinks by 100 pixels.

  @param elem The element to resize.
  @param w The amount of pixels the element's width should change.
  @param h The amount of pixels the element's height should change.
  @param opts Any additional optional arguments you want to pass.
  To add additional arguments, use OPT1 | OPT2 | OPT3... and so on.
*/
PHOS_GUI_API void phos_gui_resize_elem(phos_gui_elem *elem, float w, float h, phos_gui_opts opts);
/**
  Resizes an element by a percentage instead of direct pixels.

  @note A percentage of 0.0f results in no change for the element,
  while a percentage of 1.0f results in the element's size being
  doubled. A percentage above 1.0f results in an even larger element
  size.

  @see phos_gui_resize_elem(phos_gui_elem*, float, float, phos_gui_opts)
*/
PHOS_GUI_API void phos_gui_scale_elem(phos_gui_elem *elem, float percentage, phos_gui_opts opts);

/**
  Returns the center of an element.
*/
PHOS_GUI_API Vector2 phos_gui_get_elem_center(phos_gui_elem *elem);
/**
  Returns the center of an element while taking its text component into account.

  @note Make sure you call this function after the element's text component's contents
  have been set.
*/
PHOS_GUI_API Vector2 phos_gui_get_elem_center_with_text(phos_gui_elem *elem);
/**
  Returns the requested rectangle for an element.
*/
PHOS_GUI_API Rectangle phos_gui_get_elem_rect(phos_gui_elem *elem, phos_gui_elem_bounding_box bounds);
/**
  Forces an element's rectangles to be calculated.
*/
PHOS_GUI_API void phos_gui_reload_elem(phos_gui_elem *elem);
/**
  Forces all elements in a phos_gui to be reloaded.

  @see phos_gui_reload_elem(phos_gui_elem*)
*/
PHOS_GUI_API void phos_gui_reload_gui(phos_gui *gui);
/**
  Scales all elements within a GUI by the scale factor given.

  @see phos_gui_scale_elem(phos_gui_elem*, float, phos_gui_opts)
*/
PHOS_GUI_API void phos_gui_scale_gui(phos_gui *gui, float scale, phos_gui_opts opts);

/**
  Returns the bounds of a text component.

  This function will insert the bounds of the
  'str' field on the text component into
  'out_main_bounds,' and if the text component
  is paired with a phos_gui_placeholder_text_extension
  component, 'out_placeholder_bounds' will contain
  the bounds of the placeholder text. If at
  any point an error occurs, the rectangles
  will be zero-initialized.
*/
PHOS_GUI_API void phos_gui_get_text_bounds(const phos_gui_text_component *const text_component, Rectangle *out_main_bounds, Rectangle *out_placeholder_bounds);
/**
  Returns the bounds of a text component in the form of a Vector2.
  The vector returned only contains the width and height of the text.

  @see phos_gui_get_text_bounds(const phos_gui_text_component *const, Rectangle*, Rectangle*)
*/
PHOS_GUI_API void phos_gui_get_text_bounds_v(const phos_gui_text_component *const text_component, Vector2 *out_main_bounds, Vector2 *out_placeholder_bounds);

/**
  Initializes an element's text component.

  @note This function initializes the main
  text of the element ('str'). To initialize placeholder
  text, use phos_gui_init_placeholder_text(...).
*/
PHOS_GUI_API void phos_gui_init_text(phos_gui_text_component *text, const char *str, float font_size, Color color);
/**
  Initializes an element's placeholder text extension component.
*/
PHOS_GUI_API void phos_gui_init_placeholder_text(phos_gui_placeholder_text_extension *placeholder_text, const char *str, Color color);

/**
  Converts a Rectangle's position into a Vector2.
*/
PHOS_GUI_API Vector2 phos_gui_get_rect_pos(Rectangle r);
/**
  Converts a Rectangle's size into a Vector2.
*/
PHOS_GUI_API Vector2 phos_gui_get_rect_size(Rectangle r);

/**
  Determines if a rectangle is valid for PhosphorusGUI.

  A valid rectangle has all non-zero values.
*/
PHOS_GUI_API bool phos_gui_is_rect_valid(Rectangle r);

/**
  Quickly sets the position of an element.
*/
PHOS_GUI_API void phos_gui_set_elem_pos(phos_gui_elem *elem, float x, float y, phos_gui_opts opts);
/**
  Quickly sets the size of an element.
*/
PHOS_GUI_API void phos_gui_set_elem_size(phos_gui_elem *elem, float w, float h, phos_gui_opts opts);
/**
  Quickly sets the bounds of an element (its position and size).
*/
PHOS_GUI_API void phos_gui_set_elem_bounds(phos_gui_elem *elem, float x, float y, float w, float h, phos_gui_opts opts);
/**
  Sets the bounds of an element using a Rectangle.
*/
PHOS_GUI_API void phos_gui_set_elem_bounds_r(phos_gui_elem *elem, Rectangle r, phos_gui_opts opts);
/**
  Sets the padding values on an element.
*/
PHOS_GUI_API void phos_gui_set_elem_paddings(phos_gui_elem *elem, float left, float top, float right, float bottom);
/**
  Sets the padding values on an element all to the same value.
*/
PHOS_GUI_API void phos_gui_set_elem_padding(phos_gui_elem *elem, float padding);
/**
  Adds each amount to each respective padding value on an element.
*/
PHOS_GUI_API void phos_gui_add_elem_paddings(phos_gui_elem *elem, float left, float top, float right, float bottom);
/**
  Adds the same amount to all of the padding values on an element.
*/
PHOS_GUI_API void phos_gui_add_elem_padding(phos_gui_elem *elem, float padding);
/**
  Sets the margins on an element.
*/
PHOS_GUI_API void phos_gui_set_elem_margins(phos_gui_elem *elem, float left, float top, float right, float bottom);
/**
  Sets the margin values on an element all to the same value.
*/
PHOS_GUI_API void phos_gui_set_elem_margin(phos_gui_elem *elem, float margin);
/**
  Adds each amount to each respective margin value on an element.
*/
PHOS_GUI_API void phos_gui_add_elem_margins(phos_gui_elem *elem, float left, float top, float right, float bottom);
/**
  Adds the same amount to all of the margin values on an element.
*/
PHOS_GUI_API void phos_gui_add_elem_margin(phos_gui_elem *elem, float margin);

/**
  Sets the contents of the given element's text component.

  @param text_component The text component to modify.
  @param target_str The specific string buffer to set on the text component.
  See phos_gui_target_text_string.
  @param str The string that should occupy the target string given.
  @param opts Additional options.

  @see phos_gui_target_text_string
*/
PHOS_GUI_API void phos_gui_set_text_contents(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, const char *new_contents, phos_gui_opts opts);

/**
  Aligns the given text component using its owner as a reference element.
*/
PHOS_GUI_API Vector2 phos_gui_align_elem_text(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, phos_gui_alignment alignment);
/**
  Realigns text using the same alignment originally used to align it.
*/
PHOS_GUI_API Vector2 phos_gui_realign_elem_text(phos_gui_text_component *text_component, phos_gui_target_text_string target_str);
/**
  Aligns the given label using the reference element and alignment.

  @note When aligning a label, the element's total bounds are used (includes the element's margins).
*/
PHOS_GUI_API Vector2 phos_gui_align_elem_label(phos_gui_label_component *label_component, phos_gui_alignment alignment);
/**
  Realigns the label using the same alignment originally used to align it.
*/
PHOS_GUI_API Vector2 phos_gui_realign_elem_label(phos_gui_label_component *label_component);
/**
  Aligns the given icon using a reference element and alignment.
*/
PHOS_GUI_API Vector2 phos_gui_align_elem_icon(phos_gui_icon *icon, phos_gui_elem *reference_elem, phos_gui_alignment alignment);
/**
  Realigns the icon using the same alignment originally used to align it.
*/
PHOS_GUI_API Vector2 phos_gui_realign_elem_icon(phos_gui_icon *icon, phos_gui_elem *reference_elem);
/**
  Aligns 'target_elem' using the given alignment with 'reference_elem.'
*/
PHOS_GUI_API Vector2 phos_gui_align_elem(phos_gui_elem *target_elem, phos_gui_alignment alignment, phos_gui_elem *reference_elem, phos_gui_opts opts);
/**
  Calculates the position of 'target_elem' if it were aligned with the window.

  @important The given alignment must be one of the PHOS_GUI_ALIGN_INNER... alignments.
*/
PHOS_GUI_API Vector2 phos_gui_align_elem_with_window(phos_gui_elem *target_elem, phos_gui_alignment alignment, phos_gui_opts opts);
/**
  Fills the window's content area with an element.

  @note This automatically sets the alignment on the given element
  to PHOS_GUI_ALIGN_INNER_TOP_LEFT.
*/
PHOS_GUI_API void phos_gui_fill_window_with_elem(phos_gui_elem *elem, phos_gui_opts opts);
/**
  Fills a reference element's bounding box with a target element.
*/
PHOS_GUI_API void phos_gui_fill_elem_with_elem(phos_gui_elem *reference_elem, phos_gui_elem_bounding_box bounds, phos_gui_elem *target_elem, phos_gui_opts opts);

/**
  Makes the given text component fit its owner's bounds.

  This function will modify the text component's font size to
  make it fit its owner's size.

  @note This function is the equivalent of using PHOS_GUI_OPTS_FIT_TEXT.

  @see phos_gui_make_text_fit_rect(phos_gui_text_component*, phos_gui_target_text_string, Rectangle)
*/
PHOS_GUI_API void phos_gui_make_text_fit_elem(phos_gui_text_component *text_component, phos_gui_target_text_string target_str);
/**
  Makes the given text component fit into the given rectangle.

  This function will modify the text component's font size to
  make it fit the rectangle's size.

  @note PHOS_GUI_OPTS_FIT_TEXT does not result in this function being called.
  PHOS_GUI_OPTS_FIT_TEXT results in phos_gui_make_text_fit_elem(...)
  being used.

  @see phos_gui_make_text_fit_elem(phos_gui_text_component*, phos_gui_target_text_string)
*/
PHOS_GUI_API void phos_gui_make_text_fit_rect(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, Rectangle rect);
/**
  Makes the given icon fit the element's bounds.
*/
PHOS_GUI_API void phos_gui_make_icon_fit_elem(phos_gui_icon *icon, phos_gui_elem *elem);
/**
  Makes the given icon fit the rectangle.
*/
PHOS_GUI_API void phos_gui_make_icon_fit_rect(phos_gui_icon *icon, Rectangle rect);

/**
  Initializes an icon.

  @note The default size of icons is equal
  to PHOS_GUI_ICON_SIZE.
*/
PHOS_GUI_API void phos_gui_init_icon(phos_gui_icon *icon, phos_gui_icon_id ID, float x, float y);
/**
  Sets some basic element attributes
  and puts the element in a valid state.

  @note The element uses the current theme of PhosphorusGUI
  when initialized.
*/
PHOS_GUI_API void phos_gui_init_elem(phos_gui_elem *elem, const char *ID, phos_gui_elem_type type, phos_gui_elem_render_mode render_mode, float x, float y, float w, float h);
/**
  Initializes an element and turns it into a button element.

  By default, button elements come with mouse listener components
  and text components. The text component is initialized with the
  'text' string given.

  @important If the text string given is equal to "<no-text>" then
  the button will not have a text component. You can also insert icons
  into text components using the "<icon=ICON_NAME>" format.
*/
PHOS_GUI_API void phos_gui_init_button(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, const char *text);
/**
  Initializes an element and turns it into a text field element.

  By default, text field elements come with mouse listener components,
  text components, and placeholder text components. A scroll
  pane component is added to the container element. Each text
  component is initialized with the respective string given.

  @important If a placeholder string is given, then this function
  will make the placeholder text fit the element.

  @see phos_gui_init_button(phos_gui_elem*, const char*, float, float, float, float, const char*)
*/
PHOS_GUI_API void phos_gui_init_text_field(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, const char *main_text, const char *placeholder_text);
/**
  Initializes an element and turns it into a text area element.

  By default, text area elements come with mouse listener components,
  text components, placeholder text components, as well as scroll
  pane components.

  @note The difference between a text field and text area is that
  text areas are much bigger and usually accept more than one line
  of input from the user. Additionally, this function takes a wrap
  mode determining how the text in the text area wraps. If PHOS_GUI_TEXT_WRAP_NONE
  is given, a scroll pane is added to the text area element. If PHOS_GUI_TEXT_WRAP_CHAR
  or PHOS_GUI_TEXT_WRAP_WORD is given, the text area will not have a scroll pane component.

  @see phos_gui_init_text_field(phos_gui_elem*, phos_gui_elem*, const char*, float, float, float, float, const char*, const char*)
*/
PHOS_GUI_API void phos_gui_init_text_area(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, const char *main_text, const char *placeholder_text, phos_gui_text_wrap_mode wrap_mode);
/**
  Initializes an element and turns it into a drop-down menu.

  By default, drop-down elements come with mouse listener components,
  text components, and drop-down components.

  @note 'container_elem' should point to the container of the drop-down menu.

  @see phos_gui_drop_down_component
  @see phos_gui_init_button(phos_gui_elem*, const char*, float, float, float, float, const char*)
*/
PHOS_GUI_API void phos_gui_init_drop_down(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, phos_gui_elem *container_elem, const char *text);
/**
  Initializes an element and turns it into a single checkbox.

  By default, checkbox elements come with mouse listener components
  and label components.

  Additionally, checkbox elements start out a check mark icon in the center
  of the element. They also come with an event listener that will
  toggle the check mark when clicked (PHOS_GUI_EVENT_MOUSE_CLICK).

  @see phos_gui_checkbox_list_component
  @see phos_gui_init_button(phos_gui_elem*, const char*, float, float, float, float, const char*)
*/
PHOS_GUI_API void phos_gui_init_checkbox(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, const char *label_text);
/**
  Initializes an element and turns it into a checkbox list.

  By default, checkbox list elements come with just
  checkbox list components.

  @note A checkbox list should be a container that only contains checkbox
  elements.

  @see phos_gui_checkbox_list_component
*/
PHOS_GUI_API void phos_gui_init_checkbox_list(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h);
/**
  Initializes an element and turns it into a simple value bar.

  By default, value bar elements come with just
  value bar components.

  @see phos_gui_value_bar_component
*/
PHOS_GUI_API void phos_gui_init_value_bar(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, float min_value, float max_value, float curr_value);
/**
  Initializes an element and turns it into a slider using a
  value bar component.

  By default, slider elements come with value bar
  components and label components.

  @see phos_gui_value_bar_component
  @see phos_gui_init_button(phos_gui_elem*, const char*, float, float, float, float, const char*)
*/
PHOS_GUI_API void phos_gui_init_slider(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, float min_value, float max_value, float curr_value, const char *label_text);

/**
  Generates the background colors on a mouse listener component using brightness factors.

  For example, if the element's primary background color is white, passing
  in some small negative numbers into this function darkens white by that
  percentage and sets that respective color on the mouse listener.
*/
PHOS_GUI_API void phos_gui_gen_bg_colors(phos_gui_mouse_listener_component *mouse_listener, float hover_color_factor, float press_color_factor, float focus_color_factor);
/**
  Generates the outline colors on a mouse listener component using brightness factors.

  For example, if the element's primary outline color is white, passing
  in some small negative numbers into this function darkens white by that
  percentage and sets that respective color on the mouse listener.
*/
PHOS_GUI_API void phos_gui_gen_outline_colors(phos_gui_mouse_listener_component *mouse_listener, float hover_color_factor, float press_color_factor, float focus_color_factor);

/**
  Adds a UI element to the given phos_gui instance.

  This automatically registers the element, and performs
  any other necessary actions to ensure the program works correctly.

  @param elem The element to add to the phos_gui.
  @param gui The phos_gui instance to add an element to.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_elem_to_gui(phos_gui_elem *elem, phos_gui *gui);
/**
  Adds the UI element with the given ID to the given phos_gui instance.

  @see phos_gui_add_elem_to_gui(phos_gui_elem*, phos_gui*)
*/
PHOS_GUI_API int phos_gui_add_elem_to_gui_id(const char *ID, phos_gui *gui);
/**
  Removes a UI element from a phos_gui.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_remove_elem_from_gui(phos_gui_elem *elem, phos_gui *gui);
/**
  Removes the UI element with the given ID from a phos_gui.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_remove_elem_from_gui_id(const char *ID, phos_gui *gui);
/**
  Adds a UI element as a child to another UI element.

  @note This function does not add the parent
  or child to a phos_gui instance. PhosphorusGUI only
  maintains root elements in every phos_gui.

  @note There are some additional and optional settings you can
  pass in using phos_gui_child_opts.

  @important This function expects the child's position
  to be relative to the parent's position. For example, a child with
  a position of (0, 0) indicates it's top-left corner is at the same
  position as the parent's top-left corner.

  @return 1 on success, 0 on failure.

  @see phos_gui_child_opts
*/
PHOS_GUI_API int phos_gui_add_child_to_elem(phos_gui_elem *child, phos_gui_elem *parent, phos_gui_opts child_opts);
/**
  Adds a UI element as a child to another UI element
  using the child's ID.

  @important This function does not add the parent
  or child to a phos_gui instance. PhosphorusGUI only
  maintains root elements in every phos_gui.

  @return 1 on success, 0 on failure.

  @see phos_gui_add_child_to_elem(phos_gui_elem*, phos_gui_elem*, phos_gui_child_opts)
*/
PHOS_GUI_API int phos_gui_add_child_to_elem_id(const char *ID, phos_gui_elem *parent, phos_gui_opts child_opts);
/**
  Removes a child element from a parent element.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_remove_child(phos_gui_elem *parent, phos_gui_elem *child);
/**
  Removes a child element from a parent element
  using the child's ID.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_remove_child_id(phos_gui_elem *parent, const char *ID);
/**
  Formats the child elements inside of a parent element.

  @important The parent element must have a layout component.

  @return 1 on success, 0 on failure.

  @see phos_gui_layout_component
*/
PHOS_GUI_API int phos_gui_format_children(phos_gui_elem *parent, phos_gui_opts opts);
/**
  Adds an icon to an element.

  @note An element cannot contain duplicate icons.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_icon(phos_gui_icon_list_component *icon_list, phos_gui_icon icon);
/**
  Removes an icon from an element.

  @note This function expects the ID of the
  icon to remove.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_remove_icon(phos_gui_icon_list_component *icon_list, phos_gui_icon_id ID);
/**
  Determines if an element contains the given icon,
  and if it does, it returns a pointer to the icon on the element.
  If the element does not have the icon, NULL is returned instead.
*/
PHOS_GUI_API phos_gui_icon *phos_gui_find_elem_icon(phos_gui_elem *elem, phos_gui_icon_id ID);
/**
  Obtains a UI element with a specific ID.
*/
PHOS_GUI_API phos_gui_elem *phos_gui_get_elem(const char *ID);
/**
  Creates a clone of a UI element for reuse.

  When creating a clone of a UI element, you are creating
  a blueprint that can be instantiated at any point, instantly
  duplicating the element. Because of this blueprints must have unique
  IDs, just like elements. Additionally, to reduce any copies, every
  blueprint also must have a unique element pointer.

  @important This function only clones the element given, and no more.
  To clone an element and its children, use phos_gui_clone_full_elem(...).

  @param elem The element to clone. Note that no two blueprints can be created
  using this element pointer.
  @param ID The ID to give to the blueprint. Later, to initialize an instance of
  the blueprint, you use phos_gui_init_clone(target_elem, ID).

  @see phos_gui_clone_full_elem(phos_gui_elem*, const char*)
*/
PHOS_GUI_API void phos_gui_clone_single_elem(phos_gui_elem *elem, const char *ID);
/**
  Creates a clone of a UI element for reuse.

  When creating a clone of a UI element, you are creating
  a blueprint that can be instantiated at any point, instantly
  duplicating the element. Because of this blueprints must have unique
  IDs, just like elements. Additionally, to reduce any copies, every
  blueprint also must have a unique element pointer.

  @important This function clones the element given, and all of
  the element's children. To clone a single element, use
  phos_gui_clone_single_elem(...).

  @param elem The element to clone. Note that no two blueprints can be created
  using this element pointer.
  @param ID The ID to give to the blueprint. Later, to initialize an instance of
  the blueprint, you use phos_gui_init_clone(target_elem, ID).

  @see phos_gui_clone_single_elem(phos_gui_elem*, const char*)
*/
PHOS_GUI_API void phos_gui_clone_full_elem(phos_gui_elem *elem, const char *ID);
/**
  Creates a new instance of a cloned element,
  and inserts the data into 'target_elem.'

  @note This function does not automatically add the clone element to a phos_gui
  instance. However, it does give the clone element an auto-generated ID.

  @see phos_gui_clone_single_elem(phos_gui_elem*, const char*)
  @see phos_gui_clone_full_elem(phos_gui_elem*, const char*)
*/
PHOS_GUI_API void phos_gui_init_clone(phos_gui_elem *target_elem, const char *ID);

/**
  Initializes Raylib for PhosphorusGUI.

  @note You can customize the initial window state
  and config with the 'flags' argument. Refer to Raylib's
  FLAG_... values.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_init_window(const char *title, int width, int height, unsigned int flags);

/**
  Obtains the current mouse position. This function
  takes the current window scale into account.

  To set window scale, use phos_gui_set_window_scale(float, float).

  @note This mouse position does not take the current translation
  offset into account. You can use phos_gui_get_translated_vec2(...)
  to translate the mouse position if necessary.
*/
PHOS_GUI_API Vector2 phos_gui_get_mouse_pos(void);
/**
  Determines if the mouse is currently over a region
  defined by a rectangle.
*/
PHOS_GUI_API bool phos_gui_is_mouse_over_rect(Rectangle r);
/**
  Returns the element that is currently being interacted
  with by the mouse.

  @note PhosphorusGUI uses a method that returns the top-most
  element that interacts with the mouse.
*/
PHOS_GUI_API phos_gui_elem *phos_gui_get_mouse_target(void);

/**
  Adds an event listener to the given phos_gui_elem.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_event_listener(phos_gui_elem *elem, phos_gui_event_listener listener);
/**
  Creates and adds an event listener to the given phos_gui.

  @param elem Points to the target element.
  @param event The event to listen for.
  @param target_button The target button/key in the event.
  @param opts Any additional options you want to use
  in the action function.
  @param action The action the event listener should
  execute when the event occurs.

  @see phos_gui_add_event_listener(phos_gui*, phos_gui_event_listener)
*/
PHOS_GUI_API int phos_gui_new_event_listener(phos_gui_elem *elem, phos_gui_event_type event, int target_button, phos_gui_opts opts, phos_gui_event_listener_action action);
/**
  Removes an event listener from a phos_gui_elem.

  @note Because event listeners do not have IDs or some unique
  way of identifying them, this function finds the
  first event listener with the exact same element and event
  type given and removes that event listener, if found.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_remove_event_listener(phos_gui_elem *elem, phos_gui_event_type event);
/**
  Adds a timer to the given phos_gui.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_timer(phos_gui *gui, phos_gui_timer timer);
/**
  Creates and adds a timer to the given phos_gui.

  @see phos_gui_add_timer(phos_gui*, phos_gui_timer)
*/
PHOS_GUI_API int phos_gui_new_timer(phos_gui *gui, phos_gui_timer_action action, void *args, float target_time, int execution_count);
/**
  Adds an animation to the given phos_gui.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_animation(phos_gui *gui, phos_gui_animation animation);
/**
  Creates and adds an animation to the given phos_gui.

  @see phos_gui_add_animation(phos_gui*, phos_gui_animation)
*/
PHOS_GUI_API int phos_gui_new_animation(phos_gui *gui, phos_gui_elem *elem, float *curr_value, float end_value, float duration, float step, int execution_count, phos_gui_animation_end_value_interpretation end_value_interpretation, phos_gui_opts opts);

/**
  Launches a custom program loop for PhosphorusGUI.

  @note If you need to add extra functionality to
  your program loop, do not call this function.

  The loop looks like this:

  @code
  while(!WindowShouldClose())
  {
	  float dt = GetFrameTime();

	  vl_update(dt);
	  phos_gui_update(dt);

	  BeginDrawing();
	  ClearBackground(PHOS_GUI_COLOR_BLACK);

	  phos_gui_render();

	  EndDrawing();
  }
  @endcode
*/
PHOS_GUI_API void phos_gui_launch(void);
/**
  Updates the current phos_gui's elements.

  If there is no GUI set, the function does send
  a delayed warning message just in case, but other
  than that, nothing happens.
*/
PHOS_GUI_API void phos_gui_update(float dt);
/**
  Renders the current phos_gui's elements.

  If there is no GUI set, the function does send
  a delayed warning message just in case, but other
  than that, nothing happens.

  @note If a custom tint has been set for the screen,
  this function automatically renders it.
*/
PHOS_GUI_API void phos_gui_render(void);
/**
  Renders the given shape at the given position
  and size.

  @note The 'round_rect_corner_radius' and 'outline_thickness'
  fields are used exclusively for the rounded rectangle shape.
  PhosphorusGUI renders and handles rounded rectangles a little
  differently than Raylib, which requires outline thickness
  for this function.
*/
PHOS_GUI_API void phos_gui_fill_shape(phos_gui_shape shape, float x, float y, float w, float h, float outline_thickness, float round_rect_corner_radius, Color color);
/**
  Renders the outline of the given shape at the
  given position and size.

  @note The 'round_rect_corner_radius' field is used
  exclusively for the rounded rectangle shape.
*/
PHOS_GUI_API void phos_gui_outline_shape(phos_gui_shape shape, float x, float y, float w, float h, float outline_thickness, float round_rect_corner_radius, Color color);
/**
  Updates the given element. If the element
  has any children, its children are automatically
  updated.
*/
PHOS_GUI_API void phos_gui_update_elem(phos_gui_elem *elem, float dt);
/**
  Renders the given element. If the element
  has any children, its children are
  automatically rendered.
*/
PHOS_GUI_API void phos_gui_render_elem(phos_gui_elem *elem);
/**
  Renders the given icon.

  @see phos_gui_icon
*/
PHOS_GUI_API void phos_gui_render_icon(phos_gui_icon *icon);
/**
  Measures a text block using PhosphorusGUI's custom
  text engine.
*/
PHOS_GUI_API Vector2 phos_gui_measure_text(Font font, const char *text, float font_size);
/**
  Renders text.

  PhosphorusGUI has a custom engine separate from Raylib.

  The text engine is very similar but it also gives you
  the ability to render icons within the text. To enter
  an icon into a text block, use the "<icon=ICON_NAME>"
  format. The size of the icon will match the font size
  of the text.
*/
PHOS_GUI_API void phos_gui_render_text(Font font, const char *text, Vector2 pos, float font_size, Color color);
/**
  Generates a random color.
*/
PHOS_GUI_API Color phos_gui_random_color(void);
/**
  Obtains the current theme for PhosphorusGUI.
*/
PHOS_GUI_API phos_gui_theme phos_gui_get_theme(void);
/**
  Creates a theme based on a single starting color.

  @see phos_gui_create_theme_accented(Color, Color)
  @see phos_gui_create_theme_full(Color, Color, Color, Color, Color, Color)
*/
PHOS_GUI_API phos_gui_theme phos_gui_create_theme_basic(Color base_color);
/**
  Creates a theme using a starting color as well as an accent
  color. The accent color is used on things like outlines and the
  window's background color.

  @see phos_gui_create_theme_basic(Color)
  @see phos_gui_create_theme_full(Color, Color, Color, Color, Color, Color)
*/
PHOS_GUI_API phos_gui_theme phos_gui_create_theme_accented(Color base_color, Color accent_color);
/**
  Creates a theme using a starting color as well as an accent
  color, then specifically overrides the text, icon, and window background
  colors of the theme.

  @see phos_gui_create_theme_basic(Color)
  @see phos_gui_create_theme_accented(Color, Color)
*/
PHOS_GUI_API phos_gui_theme phos_gui_create_theme_full(Color base_color, Color accent_color, Color decoration_color, Color text_color, Color icon_color, Color window_bg_color);
/**
  Applies a custom theme to all the elements in
  the given phos_gui.

  @note Make sure you call this function after
  all elements have been added to the phos_gui.
  It only applies the theme to the current elements
  in the phos_gui.

  To set the current theme of PhosphorusGUI, use
  phos_gui_set_theme(...) instead.
*/
PHOS_GUI_API void phos_gui_apply_theme_to_gui(phos_gui *gui, phos_gui_theme theme);
/**
  Applies a custom theme to the given element and its children.

  @see phos_gui_apply_theme_to_gui(phos_gui*, phos_gui_theme)
*/
PHOS_GUI_API void phos_gui_apply_theme_to_elem(phos_gui_elem *elem, phos_gui_theme theme);
/**
  Sets the current theme of PhosphorusGUI.

  Unlike phos_gui_apply_theme(...) this function
  sets the global current theme of the program.

  When setting the current theme, all elements
  created with phos_gui_init_elem(...) start
  out with this theme, and you do not have to
  apply it later.
*/
PHOS_GUI_API void phos_gui_set_theme(phos_gui_theme theme);
/**
  Brightens a theme by the given percentage and
  returns the new theme created.

  @note A negative factor results in darker colors,
  whereas a positive factor results in brighter colors.
*/
PHOS_GUI_API phos_gui_theme phos_gui_brighten_theme(phos_gui_theme theme, float factor);
/**
  Changes the contrast in a theme using the given
  percentage and returns the new theme created.

  @note A negative factor results in duller colors,
  whereas a positive factor results in more vibrant colors.
*/
PHOS_GUI_API phos_gui_theme phos_gui_saturate_theme(phos_gui_theme theme, float factor);
/**
  Sets a custom screen tint for the window.

  If a screen tint is applied, PhosphorusGUI
  renders it over the current phos_gui.

  To remove a screen tint, pass in BLANK.

  @note For most tints, the alpha value of the color
  should be less than 255, otherwise, the screen tint
  will cover up everything in the GUI.
*/
PHOS_GUI_API void phos_gui_apply_screen_tint(Color color);
/**
  Obtains the current screen tint or BLANK if no screen
  tint has been applied.
*/
PHOS_GUI_API Color phos_gui_get_screen_tint(void);
/**
  Sets the background color of the window.

  @note If BLANK is passed in, the function defaults to
  WHITE for the window's background color.
*/
PHOS_GUI_API void phos_gui_set_window_bg_color(Color color);
/**
  Obtains the current background color of the window.
*/
PHOS_GUI_API Color phos_gui_get_window_bg_color(void);

/**
  Adds a new clip region to the list of active
  clip regions.

  @note There can be up to 16 active clip regions.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_new_clip(int x, int y, int width, int height);
/**
  Adds a new clip region to the list of active
  clip regions, using a rectangle.

  @see phos_gui_new_clip(int, int, int, int)
*/
PHOS_GUI_API int phos_gui_new_clip_r(Rectangle r);
/**
  Removes the current active clip region. If there are more than
  1 clip region, the previous one is restored.
*/
PHOS_GUI_API void phos_gui_end_clip(void);

/**
  Loads a texture.

  PhosphorusGUI automatically handles and manages all textures loaded this way.

  When phos_gui_shutdown() is called, all the textures loaded through this function
  are automatically freed and unloaded.

  @important If a texture has already been loaded with the given file path,
  the existing texture is returned.
*/
PHOS_GUI_API Texture2D *phos_gui_load_texture(const char *file_path);
/**
  Obtains a texture for a specific icon using an icon ID.

  @see phos_gui_load_texture(const char*)
  @see phos_gui_get_icon_str(const char*)
*/
PHOS_GUI_API Texture2D *phos_gui_get_icon_id(phos_gui_icon_id icon);
/**
  Obtains a texture for a specific icon using an icon string.

  An icon string must follow this format: "<icon=ICON_NAME>."

  For example, to obtain the check mark icon, use
  "<icon=CHECK_MARK>."

  @note You can also obtain the ID of the icon if a valid pointer
  is passed into 'out_icon_id.'

  @see phos_gui_get_icon_id(phos_gui_icon)
*/
PHOS_GUI_API Texture2D *phos_gui_get_icon_str(const char *str, phos_gui_icon_id *out_icon_id);
/**
  Sets up an icon and its texture.

  @note Every icon in PhosphorusGUI comes with a default
  texture. If you want to modify the icon's texture, you
  can directly edit the texture or use this function to use
  a different file entirely.
*/
PHOS_GUI_API void phos_gui_set_icon(phos_gui_icon_id icon, const char *file_path);

/**
  Loads a font.

  PhosphorusGUI automatically handles and manages all fonts loaded this way.

  When phos_gui_shutdown() is called, all the fonts loaded through this function
  are automatically freed and unloaded.

  @important If a font has already been loaded with the given file path,
  the existing font is returned.
*/
PHOS_GUI_API Font *phos_gui_load_font(const char *file_path);
/**
  Sets the default font in PhosphorusGUI. The default
  font is used by the library whenever a null or invalid
  font is encountered.

  @important If you are going to use this function, is is very important
  that you call it as soon as you can after setting up the window, and before
  using any PhosphorusGUI functions.
*/
PHOS_GUI_API void phos_gui_set_default_font(const char *file_path);
/**
  Returns the defualt font or NULL if one was never set.
*/
PHOS_GUI_API Font *phos_gui_get_default_font(void);
