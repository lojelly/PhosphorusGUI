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
  Sets the ID field of the given object.

  @note This macro expects a pointer to the object.
  The object must have a valid char array field called
  'ID.'
*/
#define phos_gui_set_id(object, id) \
	do { \
		strcpy((object) -> ID, id); \
	} while(0)


/**
  The different types of elements.
*/
typedef enum phos_gui_elem_type
{
	/**
	  The default element type.
	  
	  Indicates invalid state.
	*/
	PHOS_GUI_TYPE_INVALID,
	/**
	  The most basic element type.

	  This type indicates the element has no extra
	  functionality, and it should simply be rendered
	  using its set attributes. However, basic elements
	  are not updated like other elements; they do not
	  process mouse or keyboard input.
	*/
	PHOS_GUI_TYPE_BASIC,
	/**
	  The container element type.

	  A container element is a special type of element.
	  A container is used to hold and organize other elements,
	  which are called its children. The container becomes the
	  parent element of all the child elements. Container
	  elements behave like any other type of element.
	*/
	PHOS_GUI_TYPE_CONTAINER,
	/**
	  The button element type.
	*/
	PHOS_GUI_TYPE_BUTTON,
	/**
	  The text field element type.
	*/
	PHOS_GUI_TYPE_TEXT_FIELD
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
	  Indicates the targeted item should be aligned to the inner left
	  of an element. The inner left represents the visual left
	  of the element + left padding.
	*/
	PHOS_GUI_ALIGN_INNER_LEFT,
	/**
	  Indicates the targeted item should be aligned to the inner
	  top of an element. The inner top represents the visual top
	  of the element + top padding.
	*/
	PHOS_GUI_ALIGN_INNER_TOP,
	/**
	  Indicates the targeted item should be aligned to the inner right
	  of an element. The inner right represents the visual right
	  of the element - right padding.
	*/
	PHOS_GUI_ALIGN_INNER_RIGHT,
	/**
	  Indicates the targeted item should be aligned to the inner bottom
	  of an element. The inner bottom represents the visual bottom of
	  the element - bottom padding.
	*/
	PHOS_GUI_ALIGN_INNER_BOTTOM,
	/**
	  Indicates the targeted item should be placed in the center
	  of an element.
	*/
	PHOS_GUI_ALIGN_INNER_CENTER,
	/**
	  Indicates the targeted item should be placed in the top-left
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_INNER_TOP_LEFT,
	/**
	  Indicates the targeted item should be placed in the top-right
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_INNER_TOP_RIGHT,
	/**
	  Indicates the targeted item should be placed in the bottom-left
	  corner of an element.
	*/
	PHOS_GUI_ALIGN_INNER_BOTTOM_LEFT,
	/**
	  Indicates the targeted item should be placed in the bottom-right
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
} phos_gui_alignment;

/**
  The different layout types.
*/
typedef enum phos_gui_layout_type
{
	/**
	  Indicates that all items are organized vertically.

	  When margin collisions occur, this layout resolves
	  them by pushing the colliding objects down.

	  @note This is the default layout type.
	*/
	PHOS_GUI_LAYOUT_VERTICAL,
	/**
	  Indicates that all items are organized horizontally.

	  When margin collisions occur, this layout resolves
	  them by pushing the colliding objects to the side.
	*/
	PHOS_GUI_LAYOUT_HORIZONTAL,
} phos_gui_layout_type;


/**
  The max number of elements within a single
  phos_gui instance.
*/
#define PHOS_GUI_MAX_ELEMS 64

/**
  The max number of child elements a parent element
  can contain.
*/
#define PHOS_GUI_MAX_CHILDREN 48

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
	  The color of the placeholder text ('placeholder_str').
	*/
	Color placeholder_color;

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
	  Whether or not the text field was edited by the user this frame.

	  Hitting a key like the enter key does not mark the text field
	  as edited. Only updating the contents of the text marks
	  it as edited.
	*/
	bool edited;
	/**
	  Whether or not letters are accepted in this text field's input.
	*/
	bool accept_letters;
	/**
	  Whether or not numbers are accepted in this text field's input.
	*/
	bool accept_nums;
	/**
	  Whether or not special characters are accepted in this text field's input.
	*/
	bool accept_specials;
} phos_gui_text_component;

struct phos_gui; // forward declare phos_gui

/**
  A phos_gui_color_set represents a collection
  of colors a UI element uses.
*/
typedef struct phos_gui_color_set
{
	/**
	  The normal color.

	  This color is used when rendering the element in its default state.
	*/
	Color normal_color;
	/**
	  The hover color.

	  This color is used when the mouse is hovering over the element.
	*/
	Color hover_color;
	/**
	  The press color.

	  This color is used when the mouse is held down while hovered over
	  the element.
	*/
	Color press_color;
	/**
	  The focus color.

	  This color is used when the element currently has focus.
	*/
	Color focus_color;
} phos_gui_color_set;

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
	  This element's text component.
	*/
	phos_gui_text_component text;

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
	  The element's primary color set.
	*/
	phos_gui_color_set primary_colors;
	/**
	  The element's outline color set.
	*/
	phos_gui_color_set outline_colors;

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
	  The element's rotation in degrees.
	*/
	float rotation;
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
	  Paddig on the left side of the element.

	  @note Make sure this value remains >= 0.
	*/
	float left_padding;
	/**
	  Padding on the top side of the element.

	  @note Make sure this value remains >= 0.
	*/
	float top_padding;
	/**
	  Padding on the right side of the element.
	  
	  @note Make sure this value remains >= 0.
	*/
	float right_padding;
	/**
	  Padding on the bottom side of the element.

	  @note Make sure this value remains >= 0.
	*/
	float bottom_padding;

	/**
	  The left margin value.

	  @note Make sure this value remains >= 0.
	*/
	float left_margin;
	/**
	  The top margin value.

	  @note Make sure this value remains >= 0.
	*/
	float top_margin;
	/**
	  The right margin value.

	  @note Make sure this value remains >= 0.
	*/
	float right_margin;
	/**
	  The bottom margin value.

	  @note Make sure this value remains >= 0.
	*/
	float bottom_margin;

	/**
	  Determines if this element currently has focus.

	  An element gains focus when the user clicks
	  it.

	  An element loses focus when the user clicks
	  somewhere else on screen.
	*/
	bool has_focus;
	/**
	  Determines if the element gained focus this frame.

	  @see has_focus
	*/
	bool gained_focus;
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
	  The layout of this GUI.
	*/
	phos_gui_layout_type layout_type;
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
  Toggles debug mode of the program.

  Debug mode renders some extra information
  about elements, and the program itself.
  
  @note The padding will be rendered as a red
  rectangle, and the margin will be
  rendered as a green rectangle.
*/
PHOS_GUI_API void phos_gui_toggle_debug_mode(void);

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
  instance. It means you can later use phos_gui_get_gui(const char *ID)
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
PHOS_GUI_API void phos_gui_move_elem(phos_gui_elem *elem, float x, float y);

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
  Returns the inner bounds of an element.
  
  The inner bounds of an element represents the element's contents bounds,
  as well as the element's padding.
*/
PHOS_GUI_API Rectangle phos_gui_get_inner_elem_rect(const phos_gui_elem *const elem);
/**
  Returns the outer bounds of an element.

  The outer bounds of an element represents the element's visual bounds,
  as well as the element's margin.
*/
PHOS_GUI_API Rectangle phos_gui_get_outer_elem_rect(const phos_gui_elem *const elem);
/**
  Returns the visible bounds of an element.

  These bounds are the same as the 'pos' and 'size'
  vector fields in the element. Padding is not included
  here.
*/
PHOS_GUI_API Rectangle phos_gui_get_visible_elem_rect(const phos_gui_elem *const elem);

/**
  Returns the bounds of an element's text component.

  @note Because an element contains primary text ('str') and
  placeholder text ('placeholder_str'), you must also pass
  the target string to use.
*/
PHOS_GUI_API Rectangle phos_gui_get_text_bounds(const phos_gui_elem *const elem, const char *str);

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
  Quickly sets the position of an element.
*/
PHOS_GUI_API void phos_gui_set_elem_pos(phos_gui_elem *elem, float x, float y);
/**
  Quickly sets the bounds of an element (its position and size).
*/
PHOS_GUI_API void phos_gui_set_elem_bounds(phos_gui_elem *elem, float x, float y, float w, float h);

/**
  Quickly sets up the specified color set.
*/
PHOS_GUI_API void phos_gui_init_color_set(phos_gui_color_set *set, Color normal_color, Color hover_color, Color press_color, Color focus_color);
/**
  Sets each color in a color set to the one color given here.
*/
PHOS_GUI_API void phos_gui_fill_color_set(phos_gui_color_set *set, Color color);
/**
  Quickly generates an color set.

  This function uses the ColorBrightness(...) function
  to quickly generate the hover and press color of
  a color set based on the default color given here.
  All you have to provide is the brightness
  factors for each respective color.

  @note This function sets the 'normal_color' field in the
  given color set to the 'normal_color' given.
*/
PHOS_GUI_API void phos_gui_gen_color_set(phos_gui_color_set *set, Color normal_color, float hover_color_factor, float press_color_factor, float focus_color_factor);

/**
  Sets the contents of the given element's text component.

  @param elem The element to modify.
  @param target_str The specific string buffer to set on the element ('str' or 'placeholder_str').
  @param str The string that should occupy the target string given.
*/
PHOS_GUI_API void phos_gui_set_text_contents(phos_gui_elem *elem, char *target_str, const char *str);

/**
  Calculates the position of an object if it were aligned with the given reference element
  using the given alignment. The returned position is not a relative position, it
  is absolute.

  @param reference_elem The element to align with.
  @param alignment The alignment to use.
  @param target_object_size The size of whatever is being aligned with 'reference_elem'.
  Can be another object, element, text, etc.
*/
PHOS_GUI_API Vector2 phos_gui_get_proposed_align_pos(const phos_gui_elem *const reference_elem, phos_gui_alignment alignment, Vector2 target_object_size);
/**
  Calculates the position of the text component of an element based on an alignment.

  @param reference_elem The element to align with.
  @param alignment The alignment to use.
  @param target_str The string buffer on the element to use when aligning. For example, to use placeholder text,
  pass in 'reference_elem -> text.placeholder_str'.
*/
PHOS_GUI_API Vector2 phos_gui_get_proposed_text_align_pos(const phos_gui_elem *const reference_elem, phos_gui_alignment alignment, const char *target_str);
/**
  Calculates the position of 'target_elem' if it were aligned with 'reference_elem'
  using the given alignment.

  @param target_elem The element to move and align.
  @param alignment The alignment to use.
  @param reference_elem The element 'target_elem' is being aligned with.
*/
PHOS_GUI_API Vector2 phos_gui_align_elem(phos_gui_elem *target_elem, phos_gui_alignment alignment, const phos_gui_elem *const reference_elem);

/**
  Sets the padding on an element.
*/
PHOS_GUI_API void phos_gui_set_elem_padding(phos_gui_elem *elem, float left, float top, float right, float bottom);
/**
  Sets the margin on an element.
*/
PHOS_GUI_API void phos_gui_set_elem_margin(phos_gui_elem *elem, float left, float top, float right, float bottom);

/**
  Sets some basic element attributes.
*/
PHOS_GUI_API void phos_gui_setup_elem(phos_gui_elem *elem, phos_gui_elem_type type, phos_gui_elem_render_mode render_mode, float x, float y, float w, float h);

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
  Adds a UI element to the given phos_gui instance,
  as well as gives the element an ID.

  @see phos_gui_add_elem(phos_gui*, phos_gui_elem*)

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_elem_id(phos_gui *gui, phos_gui_elem *elem, const char *ID);
/**
  Adds a UI element to a container element.

  @note This function expects the container to already
  be added to a phos_gui instance. The function automatically
  adds 'elem' to the container as well as the phos_gui the container
  is already in.

  @return 1 on success, 0 on failure.
*/
PHOS_GUI_API int phos_gui_add_elem_to_container(phos_gui_elem *elem, phos_gui_elem *container);
/**
  Obtains a UI element with a specific ID.
*/
PHOS_GUI_API phos_gui_elem *phos_gui_get_elem(const char *ID);
/**
  Creates a clone of a UI element for reuse.

  When creating a clone of a UI element, you are creating
  a blueprint that can be instantiated at any point, instantly
  cloning the element. Because of this blueprints must have unique
  IDs, just like elements. Additionally, to reduce any copies, every
  blueprint also must have a unique element pointer.

  @param elem The element to clone. Note that no two blueprints can be created
  using this element pointer.
  @param ID The ID to give to the blueprint. Later, to instantiate the blueprint,
  you use phos_gui_new_instance(ID).
*/
PHOS_GUI_API void phos_gui_clone_elem(phos_gui_elem *elem, const char *ID);
/**
  Creates a new instance of a cloned element,
  and inserts the data into 'target_elem.'

  @note The target element will have an auto-generated ID.
  Additionally, this function is going to automatically add the target
  element to the same phos_gui the original element belongs to.
  You can undo it by using phos_gui_remove_elem(...).

  @see phos_gui_clone_elem(phos_gui_elem*, const char*)
  @see phos_gui_remove_elem(phos_gui*, phos_gui_elem*)
*/
PHOS_GUI_API void phos_gui_init_clone(phos_gui_elem *target_elem, const char *ID);

/**
  Sets the global window scale in PhosphorusGUI.

  If your window is using a custom aspect ratio or
  scaling method, the mouse information PhosphorusGUI
  reads may be incorrect. This function will let
  PhosphorusGUI know more about the window and your
  program.

  @note This function does not affect the window.
*/
PHOS_GUI_API void phos_gui_set_win_scale(float x, float y);

/**
  Obtains the current mouse position. This function
  takes the current window scale into affect.

  To set window scale, use phos_gui_set_win_scale(float, float).
*/
PHOS_GUI_API Vector2 phos_gui_get_mouse_pos(void);

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
