#pragma once

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


#include <string.h>
#include "raylib.h"


/**
  Writes into a string buffer on an object with a valid
  char buffer (ex: char ID[...]).
*/
#define phos_gui_write_str(dest, src) \
	do { \
		snprintf((dest), sizeof((dest)), (src)); \
	} while(0)

/**
  Quickly exits the program early if an error has occurred.

  @important Only use this macro in the main function. The macro
  also expects all systems to be properly initialized.
*/
#define phos_gui_exit(exit_code) \
	do { \
		phos_gui_shutdown(); \
		pluto_cs_shutdown(); \
		CloseWindow(); \
		return (exit_code); \
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
  The max number of event listeners that a
  single phos_gui can hold.
*/
#define PHOS_GUI_MAX_EVENT_LISTENERS 64

/**
  The max length of an element's ID.
*/
#define PHOS_GUI_MAX_ID_LEN 32

/**
  The max length of a text component's string.
*/
#define PHOS_GUI_MAX_TEXT_LEN 128

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
  The default width/height of a drag bar on a phos_gui_drag_pane_component.
*/
#define PHOS_GUI_DRAG_BAR_DEFAULT_LENGTH 25.0f

/**
  The window's origin.
*/
#define PHOS_GUI_WINDOW_ORIGIN (Vector2) { 0.0f, 0.0f }
/**
  The window's current size.
*/
#define PHOS_GUI_WINDOW_SIZE (Vector2) { GetScreenWidth(), GetScreenHeight() }
/**
  A Vector2 representing the window's origin and size.

  @note This macro uses GetScreenWidth() and GetScreenHeight()
  for the window size.
*/
#define PHOS_GUI_WINDOW_RECT (Rectangle) { 0.0f, 0.0f, GetScreenWidth(), GetScreenHeight() }
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
  PhoshporusGUI's custom dark gray color.
*/
#define PHOS_GUI_DARK_GRAY (Color) { 35, 35, 35, 255 }
/**
  PhosphorusGUI's custom light gray color.
*/
#define PHOS_GUI_LIGHT_GRAY (Color) { 200, 200, 200, 255 }
/**
  PhosphorusGUI's custom gray color.
*/
#define PHOS_GUI_GRAY (Color) { 125, 125, 125, 255 }
/**
  PhosphorusGUI's custom black color.
*/
#define PHOS_GUI_BLACK (Color) { 15, 15, 15, 255 }
/**
  PhosphorusGUI's custom brown color.
*/
#define PHOS_GUI_BROWN (Color) { 100, 50, 0, 255 }
/**
  PhosphorusGUI's custom red color.
*/
#define PHOS_GUI_RED (Color) { 255, 0, 0, 255 }
/**
  PhosphorusGUI's custom neon red color.
*/
#define PHOS_GUI_NEON_RED (Color) { 255, 0, 50, 255 }
/**
  PhosphorusGUI's custom green color.
*/
#define PHOS_GUI_GREEN (Color) { 0, 255, 0, 255 }
/**
  PhosphorusGUI's custom neon green color.
*/
#define PHOS_GUI_NEON_GREEN (Color) { 0, 255, 50, 255 }
/**
  PhosphorusGUI's custom blue color.
*/
#define PHOS_GUI_BLUE (Color) { 0, 0, 255, 255 }
/**
  PhosphorusGUI's custom neon blue color.
*/
#define PHOS_GUI_NEON_BLUE (Color) { 0, 50, 255, 255 }
/**
  PhosphorusGUI's custom yellow color.
*/
#define PHOS_GUI_YELLOW (Color) { 255, 255, 0, 255 }
/**
  PhosphorusGUI's custom neon yellow color.
*/
#define PHOS_GUI_NEON_YELLOW (Color) { 255, 255, 50, 255 }
/**
  PhosphorusGUI's custom violet color.
*/
#define PHOS_GUI_VIOLET (Color) { 125, 0, 255, 255 }
/**
  PhosphorusGUI's custom neon violet color.
*/
#define PHOS_GUI_NEON_VIOLET (Color) { 125, 50, 255, 255 }
/**
  PhosphorusGUI's custom orange color.
*/
#define PHOS_GUI_ORANGE (Color) { 255, 125, 0, 255 }
/**
  PhosphorusGUI's custom neon orange color.
*/
#define PHOS_GUI_NEON_ORANGE (Color) { 255, 75, 0, 255 }
/**
  PhosphorusGUI's custom mint color.
*/
#define PHOS_GUI_MINT (Color) { 0, 255, 125, 255 }
/**
  PhosphorusGUI's custom neon mint color.
*/
#define PHOS_GUI_NEON_MINT (Color) { 100, 255, 125, 255 }
/**
  PhosphorusGUI's custom cyan color.
*/
#define PHOS_GUI_CYAN (Color) { 0, 255, 255, 255 }
/**
  PhosphorusGUI's custom neon cyan color.
*/
#define PHOS_GUI_NEON_CYAN (Color) { 100, 255, 255, 255 }
/**
  PhosphorusGUI's custom pink color.
*/
#define PHOS_GUI_PINK (Color) { 255, 100, 200, 255 }
/**
  PhosphorusGUI's custom neon pink color.
*/
#define PHOS_GUI_NEON_PINK (Color) { 255, 25, 200, 255 }
/**
  PhosphorusGUI's custom magenta color.
*/
#define PHOS_GUI_MAGENTA (Color) { 255, 50, 150, 255 }
/**
  PhosphorusGUI's custom neon magenta color.
*/
#define PHOS_GUI_NEON_MAGENTA (Color) { 255, 0, 100, 255 }

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
  The different shapes of elements.
*/
typedef enum phos_gui_elem_shape
{
	/**
	  The default type of all elements: the rectangle shape.
	*/
	PHOS_GUI_SHAPE_RECT,
	/**
	  The ellipse shape.
	*/
	PHOS_GUI_SHAPE_ELLIPSE,
	/**
	  The rounded-rectangle shape.

	  Change the element's 'corner_radius' attribute
	  to modify the roundness of the rectangle.
	*/
	PHOS_GUI_SHAPE_ROUND_RECT
} phos_gui_elem_shape;

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
	  now expects the element to have a texture set on it,
	  and you only want the texture rendered.
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
	PHOS_GUI_ALIGN_TOP_LEFT,
	/**
	  Indicates the targeted item should be placed in the outer top-right
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_TOP_RIGHT,
	/**
	  Indicates the targeted item should be placed in the outer bottom-left
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_BOTTOM_LEFT,
	/**
	  Indicates the targeted item should be placed in the outer bottom-right
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_BOTTOM_RIGHT,
} phos_gui_alignment;

/**
  Additional and optional settings for phos_gui functions.
*/
typedef enum phos_gui_opts
{
	/**
	  Indicates there are no extra options to apply.
	*/
	PHOS_GUI_OPTS_NONE = 0,
	/**
	  Indicates that the action the function performs on an element
	  should be passed down to all of the elements children.
	*/
	PHOS_GUI_OPTS_PASS_DOWN = 1 << 0,
	/**
	  Indicates that when resizing an element, its text component
	  (if it has one) should be modified to fit the new size of the
	  element.

	  @note This only takes effect when the element becomes too small
	  to contain its text.
	*/
	PHOS_GUI_OPTS_FIT_TEXT = 1 << 1,
	/**
	  Indicates that when resizing an element, its text component
	  (if it has one) should be realigned. This option expects that
	  the text's 'alignment' field to be a valid alignment.
	*/
	PHOS_GUI_OPTS_REALIGN_TEXT = 1 << 2,
	/**
	  Indicates that when moving an element, collisions between the
	  element and other elements should be resolved.
	*/
	PHOS_GUI_OPTS_CHECK_ELEM_COLLISIONS = 1 << 3,
	/**
	  Indicates that when moving an element, collisions between the
	  element and the window's edges should be resolved.
	*/
	PHOS_GUI_OPTS_CHECK_WINDOW_COLLISIONS = 1 << 4,
} phos_gui_opts;

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
	PHOS_GUI_ELEM_BOUNDS_TOTAL
} phos_gui_elem_bounding_box;

/**
  The available component types.
*/
typedef enum phos_gui_component_type
{
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
	  Represents the last component ID in PhosphorusGUI.
	  
	  @note PhosphorusGUI does not register a component using this ID.

	  If using custom components in your own program, set the first
	  component ID to this exact value. It will ensure your components
	  have unique IDs.

	  Example:

	  @code
	  enum my_components
	  {
		  C1 = PHOS_GUI_COMPONENT_LAST,
		  C2,
		  C3,
		  ...
	  }
	  @endcode
	*/
	PHOS_GUI_COMPONENT_LAST
} phos_gui_component_type;

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

	  @important When the mouse listener's type is PHOS_GUI_MOUSE_LISTENER_TOGGLED,
	  this color is used to tint the current color of the element.
	*/
	Color bg_hover_color;
	/**
	  The background press color.

	  This color is used when the mouse is held down while hovered over
	  the element.

	  @important When the mouse listener's type is PHOS_GUI_MOUSE_LISTENER_TOGGLED,
	  this color is used to tint the current color of the element.
	*/
	Color bg_press_color;
	/**
	  The background focus color.

	  This color is used when the element currently has focus.

	  @important When the mouse listener's type is PHOS_GUI_MOUSE_LISTENER_TOGGLED,
	  this color represents the toggled-on color.
	*/
	Color bg_focus_color;

	/**
	  The outline hover color.

	  This color is used when the mouse is hovering over the element.

	  @important When the mouse listener's type is PHOS_GUI_MOUSE_LISTENER_TOGGLED,
	  this color is used to tint the current color of the element.
	*/
	Color outline_hover_color;
	/**
	  The outline press color.

	  This color is used when the mouse is held down while hovered over
	  the element.

	  @important When the mouse listener's type is PHOS_GUI_MOUSE_LISTENER_TOGGLED,
	  this color is used to tint the current color of the element.
	*/
	Color outline_press_color;
	/**
	  The outline focus color.

	  This color is used when the element currently has focus.

	  @important When the mouse listener's type is PHOS_GUI_MOUSE_LISTENER_TOGGLED,
	  this color represents the toggled-on color.
	*/
	Color outline_focus_color;

	/**
	  Whether or not the element was clicked this frame.

	  @note This value is only modified when the mouse listener
	  uses the PHOS_GUI_MOUSE_LISTENER_DEFAULT type.
	*/
	bool clicked;
	/**
	  Whether or not the element is being held down.

	  This value is used no matter what the type of the mouse
	  listener is.
	*/
	bool pressed;
	/**
	  Whether or not the element was toggled this frame.

	  @note For a mouse listener to be toggled on/off, set the mouse listeners's
	  type to PHOS_GUI_BUTTON_TOGGLED.
	*/
	bool toggled;
	/**
	  Whether or not the element is toggled on.
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
  A phos_gui_text_component represents a piece of
  text within an element.
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
	  The current number of chars that have been typed into this text component.

	  @note This value should be less than or equal to PHOS_GUI_MAX_TEXT_LEN or
	  the text component's 'max_len' field.
	*/
	size_t len;

	/**
	  The position of the cursor in this text component. This position is
	  used as an index into the 'str' field of the text component.
	*/
	size_t cursor_pos;

	/**
	  The relative position of this text within its parent element.
	*/
	Vector2 offset;

	/**
	  The alignment of the text.
	*/
	phos_gui_alignment alignment;

	/**
	  This text component's font size.
	*/
	float font_size;
	
	/**
	  How much the text has been scrolled horizontally.

	  When the user types enough characters and they
	  reach the right side of the text's visual
	  bounds, the text will start to scroll. This value
	  represents how much it has been scrolled so far.
	*/
	float scroll;
	/**
	  The max amount of pixels to be scrolled.

	  @see scroll
	*/
	float max_scroll;

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

	  @note This shouldn't be true if 'clamp_parent' is also true.
	*/
	bool auto_fit_children;
	/**
	  Indicates whether or not phos_gui_format_children(...) clamps
	  the parent element's size to match the total amount of space
	  its children take up. By default, this is false.

	  @note This shouldn't be true if 'auto_fit_children' is also true.
	*/
	bool clamp_parent;
} phos_gui_layout_component;

/**
  A phos_gui_scroll_pane_component provides an element
  with the ability to be scrolled. Scroll panes result
  in the element's children being clipped if not in the
  visible region. The way the pane is scrolled can also
  be customized.
*/
typedef struct phos_gui_scroll_pane_component
{
	/**
	  The amount of pixels that have been scrolled.
	*/
	float scroll;
	/**
	  The max amount of pixels that can be scrolled.
	*/
	float max_scroll;

	/**
	  Determines how many pixels are in one scroll tick.

	  By default, this is equal to 1, for 1 pixel per tick.

	  @note This should remain positive, but making it negative
	  inverts the scrolling direction.
	*/
	float px_per_tick;

	/**
	  The width of the scroll bar.
	  By default, this is equal to 12.0f.
	*/
	float scroll_bar_width;

	/**
	  The color of the scroll bar's background.

	  This is PHOS_GUI_LIGHT_GRAY by default.
	*/
	Color scroll_bar_bg_color;
	/**
	  The color of the scroll thumb.

	  This is PHOS_GUI_GRAY by default.
	*/
	Color scroll_thumb_color;
	/**
	  The color of the scroll thumb when the mouse
	  is currently interacting with it in any way.

	  This color is used when 'has_focus' is true.

	  This is PHOS_GUI_DARK_GRAY by default.
	*/
	Color scroll_thumb_focus_color;

	/**
	  Indicates whether or not the scroll pane's scroll bar
	  should be rendered. By default, this is set to true.
	*/
	bool render_scroll_bar;

	/**
	  Whether or not the user was interacting with the scroll
	  thumb this frame.
	*/
	bool thumb_has_focus;
	/**
	  Whether or not the user is currently holding onto the thumb.
	*/
	bool thumb_grabbed;
} phos_gui_scroll_pane_component;

/**
  A phos_gui_drag_pane_component provides an element with the ability
  to be dragged around the screen using the mouse.
*/
typedef struct phos_gui_drag_pane_component
{
	/**
	  If the drag pane uses a drag bar, this determines
	  the size of the drag bar. By default, the drag
	  bar's width is equal to the width of the
	  drag pane's owner, and the drag bar's height is
	  equal to PHOS_GUI_DRAG_BAR_DEFAULT_LENGTH, or 25.
	*/
	Vector2 drag_bar_size;
	/**
	  If the drag pane uses a drag bar, this determines
	  where the drag bar is located on the element.

	  By default, the drag bar is placed at the top
	  of the element (PHOS_GUI_ALIGN_INNER_TOP).

	  @important This alignment should always be either
	  PHOS_GUI_ALIGN_INNER_LEFT, PHOS_GUI_ALIGN_INNER_TOP,
	  PHOS_GUI_ALIGN_INNER_RIGHT, or PHOS_GUI_ALIGN_INNER_BOTTOM!
	*/
	phos_gui_alignment drag_bar_pos;
	/**
	  If the drag pane uses a drag bar, this color
	  determines the color of the drag bar. The drag
	  bar color is PHOS_GUI_LIGHT_GRAY by default.
	*/
	Color drag_bar_color;

	/**
	  Provides this drag pane with collisions based
	  on what options you give it.

	  For example, to give the drag pane collisions with
	  other elements, set this to PHOS_GUI_OPTS_CHECK_ELEM_COLLISIONS.
	  To give it collisions with the window's edges, add the
	  PHOS_GUI_OPTS_CHECK_WINDOW_COLLISIONS option using the '|'
	  operator. For both collision options, use PHOS_GUI_CHECK_ELEM_COLLISIONS
	  | PHOS_GUI_CHECK_WINDOW_COLLISIONS. You can also add other options
	  such as PHOS_GUI_OPTS_PASS_DOWN to add collisions for children.

	  By default, this is set to PHOS_GUI_OPTS_NONE which means the drag
	  pane will not have collisions enabled.
	*/
	phos_gui_opts collision_opts;

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
  A phos_gui_elem represents a single UI element
  within a phos_gui.
*/
typedef struct phos_gui_elem
{
	/**
	  This element's children.
	*/
	struct phos_gui_elem *children[PHOS_GUI_MAX_CHILDREN];

	/**
	  This UI element's ID.

	  The ID should be unique.

	  @important If you assign the ID to
	  'auto' then PhosphorusGUI will
	  automatically generate an ID for
	  the element. If you want the element to
	  actually have the ID 'auto,' then set the
	  ID to '!auto' and it will be properly set.
	*/
	char ID[PHOS_GUI_MAX_ID_LEN + 1];

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
	  This UI element's background texture.

	  @note When an element has a valid, non-null
	  texture, that texture is rendered instead
	  of the element's shape. Additionally,
	  when using a texture, the render mode of the
	  element is expected to be PHOS_GUI_TEXTURE.

	  @important If you load the texture yourself,
	  it is up to you to unload it later. However,
	  if you use phos_gui_load_texture(...) instead,
	  PhosphorusGUI will handle it all for you.
	*/
	Texture2D *texture;

	/**
	  The element's position.
	*/
	Vector2 pos;
	/**
	  The element's size.
	*/
	Vector2 size;

	/**
	  This element's number of children.
	*/
	size_t num_children;

	/**
	  The type of this element.
	*/
	phos_gui_elem_type type;
	/**
	  The shape of this element.
	*/
	phos_gui_elem_shape shape;
	/**
	  How this element should be rendered.

	  @important If this is set to PHOS_GUI_OUTLINE,
	  PhosphorusGUI will not use the element's outline color field when
	  rendering it. It will instead use the element's 'color' field (while
	  also still taking mouse hover and press effects into account).
	*/
	phos_gui_elem_render_mode render_mode;
	/**
	  This element's alignment.

	  When the element is added to a parent element or
	  aligned with another element, this alignment
	  tells PhosphorusGUI how to align this element
	  with the reference element.
	*/
	phos_gui_alignment alignment;

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
	  Indicates whether or not this element, when rendered,
	  has an active clip region around its free content rectangle
	  (PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE).

	  Some actions automatically set this to true, such as the
	  phos_gui_scroll_pane_component being added to an element.

	  @important You can also modify it but it is not recommended.
	  If you do set it to false, PhoshporusGUI interprets it as an override.
	*/
	bool clip_content_rect;
} phos_gui_elem;

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
*/
typedef void (*phos_gui_event_listener_action) (phos_gui_elem *elem, phos_gui_opts opts);

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
	phos_gui_elem *elem;

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
	  If listening for a mouse event, this determines
	  the mouse button to listen for.
	*/
	MouseButton mouse_btn;
	/**
	  If listening for a key event, this determines
	  the key to listen for.
	*/
	KeyboardKey key;
} phos_gui_event_listener;

/**
  A phos_gui is used to store and organize UI elements.

  It represents the scene, or the context, in which the
  elements live in. This makes a phos_gui the top-level
  container.
*/
typedef struct phos_gui
{
	/**
	  The event listeners added to this GUI.
	*/
	phos_gui_event_listener listeners[PHOS_GUI_MAX_EVENT_LISTENERS];

	/**
	  The elements inside the GUI.
	*/
	phos_gui_elem *elems[PHOS_GUI_MAX_ELEMS];

	/**
	  This GUI's ID.

	  The ID should be unique.

	  @important If you assign the ID to
	  'auto' then PhosphorusGUI will
	  automatically generate an ID for
	  the element. If you want the element to
	  actually have the ID 'auto,' then set the
	  ID to '!auto' and it will be properly set. 
	*/
	char ID[PHOS_GUI_MAX_ID_LEN + 1];

	/**
	  The current amount of elements inside this GUI.
	*/
	size_t num_elems;
	/**
	  The current amount of event listeners added to this GUI.
	*/
	size_t num_listeners;
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
PHOS_GUI_API void phos_gui_move_elem_xy(phos_gui_elem *elem, float x, float y, phos_gui_opts opts);
/**
  Resizes an element by w pixels horizontally and h pixels vertically.

  For example, if 'w' is 200 and 'h' is -100, the element's
  width grows by 200 pixels and its height shrinks by 100 pixels.

  @param elem The element to resize.
  @param w The amount of pixels the element's width should change.
  @param h The amount of pixels the element's height should change.
  @param opts Any additional optional arguments you want to pass.
  To add additional arguments, use OPT1 | OPT2 | OPT3... and so on.
  For this function, you can use PHOS_GUI_OPTS_PASS_DOWN to have the
  element's children inherit the resizing. Use PHOS_GUI_OPTS_PASS_DOWN_FIRST
  to only pass the resizing down to the very first child of the element. Using
  both PHOS_GUI_OPTS_PASS_DOWN and PHOS_GUI_OPTS_PASS_DOWN_FIRST behaves the
  same as just using PHOS_GUI_OPTS_PASS_DOWN_FIRST.
*/
PHOS_GUI_API void phos_gui_resize_elem_wh(phos_gui_elem *elem, float w, float h, phos_gui_opts opts);

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
  Returns the requested bounds of an element.

  @see phos_gui_elem_bounding_box
*/
PHOS_GUI_API Rectangle phos_gui_get_elem_rect(const phos_gui_elem *const elem, phos_gui_elem_bounding_box bounds);

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
  Sets the contents of the given element's text component.

  @note If the options given include PHOS_GUI_OPTS_REALIGN_TEXT,
  make sure the text component's alignment has already been set.

  @param text_component The text component to modify.
  @param target_str The specific string buffer to set on the text component.
  See phos_gui_target_text_string.
  @param str The string that should occupy the target string given.
  @param opts Additional options.

  @see phos_gui_target_text_string
*/
PHOS_GUI_API void phos_gui_set_text_contents(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, const char *new_contents, phos_gui_opts opts);

/**
  Calculates the position of the text component of an element based on an alignment.

  @note This function uses the text component's owner as the reference
  element to align with.

  @important The given alignment must be one of the PHOS_GUI_ALIGN_INNER... alignments.

  @param text_component The text component to align.
  @param target_str The string buffer on the text component to use when aligning.
  See phos_gui_target_text_string.
  @param alignment The alignment to use.

  @see phos_gui_align_elem(phos_gui_elem*, phos_gui_alignment, const phos_gui_elem *const)
  @see phos_gui_target_text_string
*/
PHOS_GUI_API Vector2 phos_gui_align_elem_text(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, phos_gui_alignment alignment);
/**
  Calculates the position of 'target_elem' if it were aligned with 'reference_elem'
  using the given alignment, and then uses the calculated position to properly
  move 'target_elem.'

  @param target_elem The element to move and align.
  @param alignment The alignment to use. The element's 'alignment' field is automatically assigned to the value given.
  @param reference_elem The element 'target_elem' is being aligned with.
*/
PHOS_GUI_API Vector2 phos_gui_align_elem(phos_gui_elem *target_elem, phos_gui_alignment alignment, const phos_gui_elem *const reference_elem, phos_gui_opts opts);
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
  Makes the owner of the text component fit the text's bounds.

  @note This makes the element's visible bounds exactly equal to the text's bounds.
  In most cases, this will shrink the element significantly. To make the element
  fit a text component but retain its original size, use phos_gui_make_elem_fit_text(...).
*/
PHOS_GUI_API void phos_gui_clamp_elem_to_text(const phos_gui_text_component *const text_component, phos_gui_target_text_string target_str, phos_gui_opts opts);
/**
  Makes the given text component fit its owner's bounds.

  This function will modify the text component's font size to
  make it fit its owner's size.

  @see phos_gui_make_elem_fit_text(phos_gui_text_component*, phos_gui_target_text_string)
*/
PHOS_GUI_API void phos_gui_make_text_fit_elem(phos_gui_text_component *text_component, phos_gui_target_text_string target_str);
/**
  Makes the owner of the given text component fit the
  the text component's bounds.

  @note This function walks up the parent tree of the given element
  and makes each parent also fit the text component. This is because
  if only the child was affected, it may cause size-collisions.

  @see phos_gui_make_text_fit_elem(phos_gui_text_component*, phos_gui_target_text_string)
*/
PHOS_GUI_API void phos_gui_make_elem_fit_text(const phos_gui_text_component *const text_component, phos_gui_target_text_string target_str);

/**
  Sets some basic element attributes
  and puts the element in a valid state.
*/
PHOS_GUI_API void phos_gui_init_elem(phos_gui_elem *elem, phos_gui_elem_type type, phos_gui_elem_render_mode render_mode, float x, float y, float w, float h);

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

  @note This function only adds the given element to the given phos_gui.
  To add the element and its children, use phos_gui_add_all_elems(...)

  @param gui The phos_gui instance to add an element to.
  @param elem The element to add to the phos_gui.

  @see phos_gui_add_all_elems(phos_gui*, phos_gui_elem*)

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_elem_to_gui(phos_gui *gui, phos_gui_elem *elem);
/**
  Adds a UI element to the given phos_gui instance,
  as well as gives the element an ID.

  @see phos_gui_add_elem(phos_gui*, phos_gui_elem*)

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_elem_to_gui_id(phos_gui *gui, phos_gui_elem *elem, const char *ID);
/**
  Adds a UI element along with all of its children
  to a phos_gui instance.

  @see phos_gui_add_elem(phos_gui*, phos_gui_elem*)

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_all_elems_to_gui(phos_gui *gui, phos_gui_elem *elem);
/**
  Removes a UI element from a phos_gui.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_remove_elem_from_gui(phos_gui *gui, phos_gui_elem *elem);
/**
  Removes a UI element from a phos_gui using an ID.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_remove_elem_from_gui_id(phos_gui *gui, const char *ID);
/**
  Adds a UI element as a child to another UI element.

  @important This function does not add the parent
  or child to a phos_gui instance. PhosphorusGUI only
  maintains root elements in every phos_gui.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_child(phos_gui_elem *parent, phos_gui_elem *child);
/**
  Adds a UI element as a child to another UI element
  using the child's ID.

  @important This function does not add the parent
  or child to a phos_gui instance. PhosphorusGUI only
  maintains root elements in every phos_gui.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_child_id(phos_gui_elem *parent, const char *ID);
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

  @note If the parent element also has a scroll pane component, and
  its 'render_scroll_bar' field is true, this function automatically
  adds spacing so that the scroll bar is not colliding with any of the
  child elememnts in the layout.

  @return 1 on success, 0 on failure.

  @see phos_gui_layout_component
*/
PHOS_GUI_API int phos_gui_format_children(phos_gui_elem *parent, phos_gui_opts opts);
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

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_init_window(const char *title, int width, int height);
/**
  Sets the global window scale in PhosphorusGUI.

  If your window is using a custom aspect ratio or
  scaling method, the mouse information PhosphorusGUI
  reads may be incorrect. This function will let
  PhosphorusGUI know more about the window and your
  program.

  @note This function does not affect the window.
*/
PHOS_GUI_API void phos_gui_set_window_scale(float x, float y);
/**
  Sets the global window size in PhosphoursGUI.

  This function only lets PhosphorusGUI know
  about your window's size so it can use
  correct calculations throughout the program.

  @note If you are using a render texture that creates a virtual window size,
  pass in the size of the render texture instead.

  @important You must call this function after initializing Raylib! Note that
  phos_gui_init_window(...), automatically does this for you.

  @see phos_gui_init_window(const char*, int, int)
*/
PHOS_GUI_API void phos_gui_set_window_size(float w, float h);

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
  Adds an event listener to the current phos_gui.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_event_listener(phos_gui *gui, phos_gui_event_listener listener);

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
	  ClearBackground(PHOS_GUI_BLACK);

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
*/
PHOS_GUI_API void phos_gui_render(void);

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
