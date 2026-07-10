#pragma once

#ifdef _WIN32
	#ifdef PHOS_GUI_EXPORTS
		#define PHOS_GUI_API __declspec(dllexport)
	#else
		#define PHOS_GUI_API __declspec(dllimport)
	#endif
#else
	#define PHOS_GUI_API
#endif


#include <string.h>
#include "raylib.h"


/**
  A click callback component. Click callbacks provide
  elements with actions upon being clicked.

  @param return_type The return type of the callback function.
  @param name The name of the callback function field inside
  the UI element.
  @param ... The argument types the callback function should expect
  when called.
*/
#define PHOS_GUI_CLICK_CALLBACK(return_type, name, ...) \
	return_type (*name) (__VA_ARGS__)


/**
  The different types of elements.
*/
typedef enum phos_gui_elem_type
{
	/**
	  The button element type.
	*/
	PHOS_GUI_BUTTON,
	/**
	  The text field element type.
	*/
	PHOS_GUI_TEXT_FIELD
} phos_gui_elem_type;

/**
  The different shapes of elements.
*/
typedef enum phos_gui_elem_shape
{
	/**
	  The default type of all elements: the rectangle shape.
	*/
	PHOS_GUI_RECT,
	/**
	  The ellipse shape.
	*/
	PHOS_GUI_ELLIPSE
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
	PHOS_GUI_FILL_OUTLINE,
	/**
	  Results in just the element's shape being filled.
	*/
	PHOS_GUI_FILL,
	/**
	  Results in just the element's outline being rendered.
	*/
	PHOS_GUI_OUTLINE,
} phos_gui_elem_render_mode;

/**
  The different alignments.
*/
typedef enum phos_gui_alignment
{
	/**
	  Indicates the targeted item should be aligned to the left.
	*/
	PHOS_GUI_ALIGN_LEFT
} phos_gui_alignment;


/**
  The max number of elements within a single
  phos_gui instance.
*/
#define PHOS_GUI_MAX_ELEMS 64

/**
  The max length of an element's ID.
*/
#define PHOS_GUI_MAX_ID_LEN 32

/**
  The max length of a text component's string.
*/
#define PHOS_GUI_MAX_TEXT_LEN 128

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
	  For some elements, they may require placeholder
	  text.

	  Use this buffer for placeholder text specifically.

	  Just like the 'str' field, it also allocates (PHOS_GUI_MAX_TEXT_LEN + 1) bytes.
	*/
	char placeholder_str[PHOS_GUI_MAX_TEXT_LEN + 1];

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
	  The position of the text within its parent element.
	*/
	Vector2 pos;

	/**
	  This text component's font size.
	*/
	float font_size;

	/**
	  The color of the text component's main contents ('str').
	*/
	Color color;
	/**
	  The color of the placeholder text ('placeholder_str').
	*/
	Color placeholder_color;

	/**
	  The last key the user typed into this text component.

	  If there was no character typed, it will be equal to KEY_NULL.

	  @note To convert to a char, cast it to (char).
	*/
	int key_typed;

	/**
	  Whether or not the user can edit this text component.
	*/
	bool editable;
	/**
	  Whether or not the text field was edited by the user this frame.
	*/
	bool edited;
	
	// TODO implement acceptance fields (accepts_chars, accepts_nums, accepts_special_chars, etc)
} phos_gui_text_component;

/**
  A phos_gui_elem represents a single UI element
  within a phos_gui.
*/
typedef struct phos_gui_elem
{
	/**
	  This element's text component.
	*/
	phos_gui_text_component text;

	/**
	  This UI element's ID.

	  The ID should be unique.

	  @important If you assign the ID to
	  '<auto-gen>' then the library will
	  automatically generate an ID for
	  the element. If you just so happen
	  to name an element '<auto-gen>' then
	  instead name the element '!<auto-gen>'
	  and PhosphorusGUI will properly name the
	  element '<auto-gen>.'
	*/
	char id[PHOS_GUI_MAX_ID_LEN + 1];

	/**
	  This UI element's background texture.

	  @important If you load the texture yourself,
	  it is up to you to unload it later. However,
	  if you use phos_gui_load_texture(...) instead,
	  PhosphorusGUI will handle it all for you.
	*/
	Texture2D *bg_texture;

	/**
	  The element's position.
	*/
	Vector2 pos;
	/**
	  The element's size.
	*/
	Vector2 size;

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
	  The element's color.

	  This represents the element's
	  default color.
	*/
	Color color;
	/**
	  The element's hover color.

	  This color is used when the mouse
	  is hovered over this element.
	*/
	Color hover_color;
	/**
	  The element's press color.

	  This color is used when the mouse
	  is held down over this element.
	*/
	Color press_color;
	/**
	  The element's outline color.

	  @note If set to { 0, 0, 0, 0 }, it will
	  not be visible.

	  @important If the render mode of the element is set to PHOS_GUI_OUTLINE,
	  do not set this value, as PhosphorusGUI will use the 'color' field of
	  the element instead.
	*/
	Color outline_color;

	/**
	  The element's rotation in degrees.
	*/
	float rotation;
	/**
	  The thickness of the element's outline.

	  @note If set to 0, it will not be visible.
	*/
	float outline_thickness;

	/**
	  Determines if this element has focus.

	  An element gains focus when the user clicks
	  it.

	  An element loses focus when the user clicks
	  somewhere else on screen.
	*/
	bool has_focus;
	/**
	  Determines if the mouse is over this element.
	*/
	bool hovered;
	/**
	  Determines if the mouse is clicked while
	  hovered over this element.
	*/
	bool clicked;
	/**
	  Determines if the mouse is held down while
	  hovered over this element.
	*/
	bool pressed;
} phos_gui_elem;

/**
  An alias for a function pointer used by PhosphorusGUI
  in event listeners.

  The function should return nothing, and it takes in
  the target element of the event listener.
*/
typedef void (*phos_gui_event_listener_action) (phos_gui_elem*);

/**
  A phos_gui stores up to 64 UI elements.
*/
typedef struct phos_gui
{
	/**
	  The elements inside the GUI.
	*/
	phos_gui_elem *elems[PHOS_GUI_MAX_ELEMS];
	/**
	  The current amount of elements inside this GUI.
	*/
	size_t num_elems;
} phos_gui;


/**
  The window's origin.
*/
#define PHOS_GUI_WIN_ORIGIN (Vector2) { 0.0f, 0.0f }
/**
  The window's current size.
*/
#define PHOS_GUI_WIN_SIZE (Vector2) { GetScreenWidth(), GetScreenHeight() }


/**
  Initializes the PhosphorusGUI library.

  @important Do not forget to call phos_gui_shutdown() before
  the end of the program and before CloseWindow()!
*/
PHOS_GUI_API void phos_gui_init(void);
/**
  Frees all resources used by the PhosphorusGUI library.
*/
PHOS_GUI_API void phos_gui_shutdown(void);

/**
  Centers a UI element inside the given bounds.

  @param elem The UI element to center.
  @param origin The origin of the container the element is being centered in.
  @param bounds The size of the container the element is being centered in.
*/
PHOS_GUI_API void phos_gui_center_elem(phos_gui_elem *elem, Vector2 origin, Vector2 size);

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
  Returns the bounds of an element's text component.

  @note Because an element contains primary text ('str') and
  placeholder text ('placeholder_str'), you must also pass
  the target string to use.
*/
PHOS_GUI_API Rectangle phos_gui_get_text_bounds(phos_gui_elem *elem, const char *str);

/**
  Initializes an element's text component.

  @note This function initializes the main
  text of the element ('str'). To initialize placeholder
  text, use phos_gui_init_placeholder_text(...).
*/
PHOS_GUI_API void phos_gui_init_text(phos_gui_elem *elem, const char *str, float font_size, Color color);
/**
  Initializes an element's text component.

  @note This function initializes the placeholder
  text of the text component.
*/
PHOS_GUI_API void phos_gui_init_placeholder_text(phos_gui_elem *elem, const char *str, Color color);

/**
  Quickly sets the bounds of an element (its position and size).
*/
PHOS_GUI_API void phos_gui_set_elem_bounds(phos_gui_elem *elem, float x, float y, float w, float h);

/**
  Quickly sets up the outline of an element (the color and line thickness).
*/
PHOS_GUI_API void phos_gui_set_elem_outline(phos_gui_elem *elem, Color color, float thickness);

/**
  Quickly sets up an element's colors.
*/
PHOS_GUI_API void phos_gui_set_elem_colors(phos_gui_elem *elem, Color color, Color hover_color, Color press_color);
/**
  Quickly generates an element's colors.

  This function uses the ColorBrightness(...) function
  to quickly generate the hover and press color of
  an element based on the default color given here.
  All you have to provide is the brightness
  factors for each respective color.

  @note This function will set the 'color' field of the element
  to the default color given.
*/
PHOS_GUI_API void phos_gui_gen_elem_colors(phos_gui_elem *elem, Color default_color, float hover_color_factor, float press_color_factor);

/**
  Sets the contents of the given element's text component.
*/
PHOS_GUI_API void phos_gui_set_text_contents(phos_gui_elem *elem, const char *str);

/**
  Aligns a targted item based on an alignment + padding and returns
  the new position of the targeted item.
*/
PHOS_GUI_API Vector2 phos_gui_align(phos_gui_elem *elem, phos_gui_alignment alignment, float pad_x, float pad_y);

/**
  Adds an event listener to an element.

  PhosphorusGUI automatically manages all event
  listeners added through this function.

  @param elem The element to add a listener to.
  @param eval The boolean value, or the event. This value
  is checked by PhosphorusGUI to update and execute the event listener.
  @param action The phos_gui_event_listener_action to execute when the
  boolean value given is true.
*/
PHOS_GUI_API void phos_gui_add_event_listener(phos_gui_elem *elem, bool *event, phos_gui_event_listener_action action);

/**
  Registers a UI element using the element's ID.

  This function does check for duplicate IDs, as well
  as duplicate pointers.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_register_elem(phos_gui_elem *elem);
/**
  Adds a UI element to the given phos_gui instance.

  This automatically registers the element, and performs
  any other necessary actions to ensure the program works correctly.

  @param gui The phos_gui instance to add an element to.
  @param elem The element to add to the phos_gui.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_elem(phos_gui *gui, phos_gui_elem *elem);
/**
  Obtains a UI element with a specific ID.
*/
PHOS_GUI_API phos_gui_elem *phos_gui_get_elem(const char *ID);

/**
  Updates a phos_gui's elements.
*/
PHOS_GUI_API void phos_gui_update(phos_gui *gui);
/**
  Renders a phos_gui's elements.
*/
PHOS_GUI_API void phos_gui_render(const phos_gui *const gui);

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
