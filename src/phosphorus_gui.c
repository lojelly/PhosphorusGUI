#include <math.h>
#include <ctype.h>
#include "dynamic_array_spellbook.h"
#include "vibrant_logs.h"
#include "phosphorus_gui.h"
#include "raymath.h"

#define PHOS_GUI_DEBUG_PADDING_COLOR RED
#define PHOS_GUI_DEBUG_MARGIN_COLOR GREEN
#define PHOS_GUI_DEBUG_LINE_THICKNESS 7.5f

#define PHOS_GUI_CURSOR_WIDTH 3
#define PHOS_GUI_KEY_REPEAT_DELAY 0.5f
#define PHOS_GUI_KEY_REPEAT_INTERVAL 0.033f

// array of element pointers
typedef struct phos_gui_elem_arr
{
	phos_gui_elem **data;
	size_t size, capacity;
} phos_gui_elem_arr;

// custom phos_gui texture (holds file path as well as Texture2D)
typedef struct phos_gui_tex
{
	Texture2D tex;
	const char *file_path;
} phos_gui_tex;

// custom phos_gui font (holds file path as well as Font)
typedef struct phos_gui_font
{
	Font font;
	const char *file_path;
} phos_gui_font;

// array of loaded textures
typedef struct phos_gui_tex_arr
{
	phos_gui_tex *data;
	size_t size, capacity;
} phos_gui_tex_arr;

// array of loaded fonts
typedef struct phos_gui_font_arr
{
	phos_gui_font *data;
	size_t size, capacity;
} phos_gui_font_arr;

// event listener
typedef struct phos_gui_event_listener
{
	phos_gui_elem *target;
	phos_gui_event_listener_condition event;
	phos_gui_event_listener_action action;
} phos_gui_event_listener;

// array of event listeners
typedef struct phos_gui_event_listener_arr
{
	phos_gui_event_listener *data;
	size_t size, capacity;
} phos_gui_event_listener_arr;

// blueprints:
typedef struct phos_gui_blueprint
{
	char ID[PHOS_GUI_MAX_ID_LEN + 1];
	phos_gui_elem *elem;
} phos_gui_blueprint;
typedef struct phos_gui_blueprint_arr
{
	phos_gui_blueprint *data;
	size_t size, capacity;
} phos_gui_blueprint_arr;

// gui registry:
typedef struct phos_gui_arr
{
	phos_gui **data;
	size_t size, capacity;
} phos_gui_arr;

static bool init = false;
static phos_gui_elem_arr elem_registry;
static phos_gui_blueprint_arr blueprint_registry;
static phos_gui_arr gui_registry;
static phos_gui_event_listener_arr event_listeners;
static phos_gui_tex_arr textures;
static phos_gui_font_arr fonts;

// for objects with "<auto-gen>" ID, ensures unique auto-generated IDs
static size_t elem_auto_id = 0;
static size_t blueprint_auto_id = 0;
static size_t gui_auto_id = 0;

// keyboard input
typedef struct phos_gui_key_timer
{
	// what key is being used?
	int key;
	// time left
	float timer;
	// is the key down?
	bool active;
} phos_gui_key_timer;

static phos_gui_key_timer backspace_timer = {0};
static phos_gui_key_timer left_arrow_timer = {0};
static phos_gui_key_timer right_arrow_timer = {0};

static bool in_debug_mode = false;

static phos_gui *prev_gui = NULL;
static phos_gui *curr_gui = NULL;


void phos_gui_init()
{
	if(init)
	{
		vl_log(VL_ERROR, "Cannot re-initialize PhosphorusGUI!\n");
		return;
	}

	dynas_init(&elem_registry);
	dynas_init(&blueprint_registry);
	dynas_init(&gui_registry);
	dynas_init(&event_listeners);
	dynas_init(&textures);
	dynas_init(&fonts);

	backspace_timer.key = KEY_BACKSPACE;
	left_arrow_timer.key = KEY_LEFT;
	right_arrow_timer.key = KEY_RIGHT;

	init = true;
}
void phos_gui_shutdown()
{
	if(!init)
	{
		vl_log(VL_ERROR, "PhosphorusGUI was never initialized, cannot shutdown!\n");
		return;
	}

	dynas_free(&elem_registry);
	dynas_free(&blueprint_registry);
	dynas_free(&gui_registry);
	dynas_free(&event_listeners);

	// unload every texture loaded
	for(size_t i = 0; i < textures.size; ++i)
	{
		UnloadTexture(textures.data[i].tex);
		vl_log(VL_SUCCESS, "Unloaded texture: '%s'!\n", textures.data[i].file_path);
	}
	dynas_free(&textures);

	// unload every font loaded
	for(size_t i = 0; i < fonts.size; ++i)
	{
		UnloadFont(fonts.data[i].font);
		vl_log(VL_SUCCESS, "Unloaded font: '%s'!\n", fonts.data[i].file_path);
	}
	dynas_free(&fonts);

	init = false;
}

void phos_gui_toggle_debug_mode()
{
	in_debug_mode = !in_debug_mode;
}

static void phos_gui_auto_gen_id(const char *ID, char *target, const char *prefix, size_t *generator)
{
	if(strcmp(ID, "!<auto-gen>") == 0)
		sprintf(target, "<auto-gen>");
	else if(strcmp(ID, "<auto-gen>") == 0)
		sprintf(target, "%s_#%zu", prefix, (*generator)++);
}

void phos_gui_set_gui(phos_gui *new_gui)
{
	prev_gui = curr_gui;
	curr_gui = new_gui;

	// only register non-null pointers:
	if(new_gui)
	{
		// see if a duplicate is found
		for(size_t i = 0; i < gui_registry.size; ++i)
		{
			phos_gui *saved_gui = gui_registry.data[i];

			// if a match is found, the GUI is already registered:
			if(saved_gui == new_gui)
				return;
		}

		// auto-gen ID if necessary
		phos_gui_auto_gen_id(new_gui -> ID, new_gui -> ID, "gui", &gui_auto_id);

		// register the phos_gui
		dynas_add(&gui_registry, new_gui);

		vl_log(VL_SUCCESS, "Registered GUI with ID: '%s'!\n", new_gui -> ID);
	}
}
phos_gui *phos_gui_get_curr_gui()
{
	return curr_gui;
}
phos_gui *phos_gui_get_prev_gui()
{
	return prev_gui;
}
phos_gui *phos_gui_get_gui(const char *ID)
{
	if(!ID || strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Cannot search for a phos_gui with an invalid ID!\n");
		return NULL;
	}

	// try to find the matching GUI
	for(size_t i = 0; i < gui_registry.size; ++i)
	{
		if(strcmp(gui_registry.data[i] -> ID, ID) == 0)
			return gui_registry.data[i];
	}

	vl_log(VL_ERROR, "No phos_gui found with the ID: '%s'!\n", ID);
	return NULL;
}

void phos_gui_center_elem(phos_gui_elem *elem, Vector2 origin, Vector2 size)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot center a null UI element!\n");
		return;
	}

	Vector2 elem_size = elem -> size;

	Vector2 container_center = { origin.x + size.x / 2.0f, origin.y + size.y / 2.0f };
	
	Vector2 elem_centered = { container_center.x - elem_size.x / 2.0f, container_center.y - elem_size.y / 2.0f };

	phos_gui_set_elem_bounds(elem, elem_centered.x, elem_centered.y, elem -> size.x, elem -> size.y);
}

void phos_gui_move_elem(phos_gui_elem *elem, float x, float y)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot move a null UI element!\n");
		return;
	}

	elem -> pos.x += x;
	elem -> text.pos.x += x;
	elem -> pos.y += y;
	elem -> text.pos.y += y;
}

Vector2 phos_gui_get_elem_center(phos_gui_elem *elem)
{
	Vector2 v = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain center of a null UI element!\n");
		return v;
	}

	// start at origin
	v = elem -> pos;

	// add half the elem size
	v.x += elem -> size.x / 2.0f;
	v.y += elem -> size.y / 2.0f;

	return v;
}
Vector2 phos_gui_get_elem_center_with_text(phos_gui_elem *elem)
{
	Vector2 v = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain center of a null UI element!\n");
		return v;
	}
	if(!elem -> text.font)
	{
		vl_log(VL_ERROR, "This element ('%s') has a null font, cannot obtain center with text!\n", elem -> ID);
		return v;
	}

	// get center of elem
	v = phos_gui_get_elem_center(elem);

	// measure text
	Vector2 text_size = MeasureTextEx(*elem -> text.font, elem -> text.str, elem -> text.font_size, 0.0f);

	// minus half the text's bounds
	v.x -= text_size.x / 2.0f;
	v.y -= text_size.y / 2.0f;

	return v;
}

Rectangle phos_gui_get_inner_elem_rect(const phos_gui_elem *const elem)
{
	Rectangle r = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain inner rect for a null UI element!\n");
		return r;
	}

	// iner rect is visible rect + padding
	r = phos_gui_get_visible_elem_rect(elem);

	r.x += elem -> left_padding;
	r.y += elem -> top_padding;
	r.width = r.width - elem -> left_padding - elem -> right_padding;
	r.height = r.height - elem -> top_padding - elem -> bottom_padding;

	return r;
}
Rectangle phos_gui_get_outer_elem_rect(const phos_gui_elem *const elem)
{
	Rectangle r = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain outer rect for a null UI element!\n");
		return r;
	}

	// outer rect is visible rect - margin
	r = phos_gui_get_visible_elem_rect(elem);

	r.x -= elem -> left_margin;
	r.y -= elem -> top_margin;
	r.width = r.width + elem -> left_margin + elem -> right_margin;
	r.height = r.height + elem -> top_margin + elem -> bottom_margin;

	return r;
}
Rectangle phos_gui_get_visible_elem_rect(const phos_gui_elem *const elem)
{
	Rectangle r = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain visible bounds for a null UI element!\n");
		return r;
	}

	r.x = elem -> pos.x;
	r.y = elem -> pos.y;
	r.width = elem -> size.x;
	r.height = elem -> size.y;

	return r;
}

Rectangle phos_gui_get_text_bounds(const phos_gui_elem *const elem, const char *str)
{
	Rectangle r = {0};

	if(!elem)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for null UI element!\n");
		return r;
	}
	if(!str)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for null string!\n");
		return r;
	}
	if(strlen(str) == 0)
	{
		vl_delay_log(VL_WARNING, 5.0f, "Cannot obtain text bounds for empty text on element: '%s'!\n", elem -> ID);
		return r;
	}
	if(!elem -> text.font || !IsFontValid(*elem -> text.font) || elem -> text.font_size <= 0.0f)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for invalid font on element: '%s'!\n", elem -> ID);
		return r;
	}

	// width and height of the text
	Vector2 text_size = MeasureTextEx(*elem -> text.font, str, elem -> text.font_size, 0.0f);

	if(text_size.x == 0 || text_size.y == 0)
	{
		vl_delay_log(VL_ERROR, 2.5f, "Invalid text bounds; width and height must exceed 0!\n");
		return r;
	}

	r.width = text_size.x;
	r.height = text_size.y;

	r.x = elem -> pos.x + elem -> left_padding;
	r.y = elem -> pos.y + elem -> top_padding;

	return r;
}

static void phos_gui_update_text_scrolling(phos_gui_elem *e)
{
	// first determine whether or not to use text.str or text.placeholder_str
	const char *str = NULL;
	if(strlen(e -> text.str) > 0)
		str = e -> text.str;
	else if(strlen(e -> text.placeholder_str) > 0)
		str = e -> text.placeholder_str;
	else
	{
		e -> text.scroll = 0.0f;
		e -> text.max_scroll = 0.0f;
		vl_delay_log(VL_WARNING, 5.0f, "Failed to update text scrolling. Text component must contain string data to calculate!\n");
		return;
	}

	// first get visual bounds of text component on screen
	Rectangle vis_bounds = phos_gui_get_visible_elem_rect(e);

	// get bounds of text
	Rectangle text_bounds = phos_gui_get_text_bounds(e, str);

	// calculate the overflow
	float vis_left = vis_bounds.x + e -> left_padding;
	float vis_right = vis_bounds.x + vis_bounds.width - e -> right_padding;
	float vis_width = vis_right - e -> text.pos.x;
	float overflow = text_bounds.width - vis_width;

	if(overflow > 0.0f)
		e -> text.max_scroll = overflow;
	else
	{
		e -> text.max_scroll = 0.0f;
		e -> text.scroll = 0.0f;
	}

	// only handle caret logic for the editable string:
	if(strlen(e -> text.str) > 0)
	{
		char buf[PHOS_GUI_MAX_TEXT_LEN + 1];
		memcpy(buf, e -> text.str, e -> text.cursor_pos);
		buf[e -> text.cursor_pos] = '\0';

		float caret_x = MeasureTextEx(*e -> text.font, buf, e -> text.font_size, 0.0f).x;
		float caret_screen = e -> text.pos.x + caret_x - e -> text.scroll;

		// right-side check (include cursor width because the cursor takes up that many more pixels)
		if(caret_screen + PHOS_GUI_CURSOR_WIDTH > vis_right)
			e -> text.scroll += (caret_screen + PHOS_GUI_CURSOR_WIDTH) - vis_right;

		// left-side check (don't include cursor width)
		if(caret_screen < vis_left)
			e -> text.scroll -= vis_left - caret_screen;
	}

	e -> text.scroll = Clamp(e -> text.scroll, 0.0f, e -> text.max_scroll);
}

void phos_gui_init_text(phos_gui_elem *elem, const char *str, float font_size, Color color)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot initialize text component on null UI element!\n");
		return;
	}
	if(!str)
	{
		vl_log(VL_ERROR, "Cannot initialize text component with null string!\n");
		return;
	}
	if(strlen(str) >= PHOS_GUI_MAX_TEXT_LEN)
	{
		vl_log(VL_ERROR, "String is too long! The max is PHOS_GUI_MAX_TEXT_LEN!\n");
		return;
	}
	if(font_size <= 0.0f)
	{
		vl_log(VL_ERROR, "Text cannot have a font size of 0 or less!\n");
		return;
	}

	elem -> text.len = strlen(str);
	strcpy(elem -> text.str, str);
	elem -> text.max_len = PHOS_GUI_MAX_TEXT_LEN;
	elem -> text.color = color;
	elem -> text.font_size = font_size;
	elem -> text.editable = true;
	elem -> text.edited = false;
	elem -> text.cursor_pos = elem -> text.len;
	elem -> text.accept_letters = elem -> text.accept_nums = elem -> text.accept_specials = true;
}
void phos_gui_init_placeholder_text(phos_gui_elem *elem, const char *str, Color color)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot initialize placeholder text on null UI element!\n");
		return;
	}
	if(!str)
	{
		vl_log(VL_ERROR, "Cannot initialize placeholder text with null string!\n");
		return;
	}
	if(strlen(str) >= PHOS_GUI_MAX_TEXT_LEN)
	{
		vl_log(VL_ERROR, "String is too long! The max is PHOS_GUI_MAX_TEXT_LEN!\n");
		return;
	}

	strcpy(elem -> text.placeholder_str, str);
	elem -> text.placeholder_color = color;
}

void phos_gui_set_elem_bounds(phos_gui_elem *elem, float x, float y, float w, float h)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set bounds of null UI element!\n");
		return;
	}

	// save current elem pos
	Vector2 elem_pos = elem -> pos;

	// save new elem bounds
	elem -> pos = (Vector2) { x, y };
	elem -> size = (Vector2) { w, h };

	// calculate difference between elem positions
	Vector2 diff = Vector2Subtract(elem -> pos, elem_pos);

	// move text based on 'diff'
	elem -> text.pos = Vector2Add(elem -> text.pos, diff);
}

void phos_gui_set_elem_outline(phos_gui_elem *elem, Color color, float thickness)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set outline of null UI element!\n");
		return;
	}

	if(thickness <= 0)
	{
		vl_log(VL_WARNING, "Element outline disabled: '%s'.\n", elem -> ID);
	}

	// determine which color field to use based on render mode
	if(elem -> render_mode == PHOS_GUI_OUTLINE)
		elem -> color = color;
	else
		elem -> outline_color = color;
	elem -> outline_thickness = thickness;
}

void phos_gui_set_elem_colors(phos_gui_elem *elem, Color color, Color hover_color, Color press_color)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set colors of null UI element!\n");
		return;
	}

	elem -> color = color;
	elem -> hover_color = hover_color;
	elem -> press_color = press_color;
}
void phos_gui_gen_elem_colors(phos_gui_elem *elem, Color default_color, float hover_color_factor, float press_color_factor)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot generate colors of null UI element!\n");
		return;
	}
	
	elem -> color = default_color;
	elem -> hover_color = ColorBrightness(elem -> color, hover_color_factor);
	elem -> press_color = ColorBrightness(elem -> color, press_color_factor);
}

void phos_gui_set_text_contents(phos_gui_elem *elem, char *target_str, const char *str)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set string contents of a null UI element!\n");
		return;
	}
	if(!target_str || !str)
	{
		vl_log(VL_ERROR, "String is null, cannot set UI element's text contents!\n");
		return;
	}

	strcpy(target_str, str);
	elem -> text.cursor_pos = strlen(target_str);
}

Vector2 phos_gui_align(phos_gui_elem *elem, phos_gui_alignment alignment, Vector2 object_size)
{
	Vector2 v = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot align null UI element!\n");
		return v;
	}

	// define rectangle bounds for element using padding
	float content_top = elem -> pos.y + elem -> top_padding;
	float content_left = elem -> pos.x + elem -> left_padding;
	float content_bottom = elem -> pos.y + elem -> size.y - elem -> bottom_padding;
	float content_right = elem -> pos.x + elem -> size.x - elem -> right_padding;
	float content_width = content_right - content_left;
	float content_height = content_bottom - content_top;

	switch(alignment)
	{
		case PHOS_GUI_ALIGN_INNER_TOP:
			v.x = content_left + (content_width - object_size.x) / 2.0f;
			v.y = content_top;
			break;
		case PHOS_GUI_ALIGN_INNER_LEFT:
			v.x = content_left;
			v.y = content_top + (content_height - object_size.y) / 2.0f;
			break;
		case PHOS_GUI_ALIGN_INNER_BOTTOM:
			v.x = content_left + (content_width - object_size.x)  / 2.0f;
			v.y = content_bottom - object_size.y;
			break;
		case PHOS_GUI_ALIGN_INNER_RIGHT:
			v.x = content_right - object_size.x;
			v.y = content_top + (content_height - object_size.y) / 2.0f;
			break;
		case PHOS_GUI_ALIGN_INNER_CENTER:
			v.x = content_left + (content_width - object_size.x) / 2.0f;
			v.y = content_top + (content_height - object_size.y) / 2.0f;
			break;
		case PHOS_GUI_ALIGN_ABOVE:
			// TODO
			break;
	}

	return v;
}
Vector2 phos_gui_align_text(phos_gui_elem *elem, phos_gui_alignment alignment, const char *target_str)
{
	Vector2 v = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot align text on null UI element!\n");
		return v;
	}

	Rectangle elem_text_size = phos_gui_get_text_bounds(elem, target_str);
	v = phos_gui_align(elem, alignment, (Vector2) { elem_text_size.width, elem_text_size.height });
	return v;
}

void phos_gui_add_event_listener(phos_gui_elem *elem, phos_gui_event_listener_condition event, phos_gui_event_listener_action action)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot add event listener to null UI element!\n");
		return;
	}
	if(!event)
	{
		vl_log(VL_ERROR, "Cannot listen for null event!\n");
		return;
	}
	if(!action)
	{
		vl_log(VL_ERROR, "Cannot execute null phos_gui_event_listener_action!\n");
		return;
	}

	phos_gui_event_listener el = { .target = elem, .event = event, .action = action };
	dynas_add(&event_listeners, el);
}

void phos_gui_set_elem_padding(phos_gui_elem *elem, float left, float top, float right, float bottom)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set padding on a null UI element!\n");
		return;
	}

	elem -> left_padding = left;
	elem -> top_padding = top;
	elem -> right_padding = right;
	elem -> bottom_padding = bottom;
}
void phos_gui_set_elem_margin(phos_gui_elem *elem, float left, float top, float right, float bottom)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set margin on a null UI element!\n");
		return;
	}

	elem -> left_margin = left;
	elem -> top_margin = top;
	elem -> right_margin = right;
	elem -> bottom_margin = bottom;
}

static int phos_gui_register_elem(phos_gui_elem *elem)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot register null UI element!\n");
		return 0;
	}
	if(!init)
	{
		vl_log(VL_ERROR, "Cannot register element, phos_gui_init() was never called!\n");
		return 0;
	}
	if(strlen(elem -> ID) == 0)
	{
		vl_log(VL_ERROR, "Cannot register element with empty ID!\n");
		return 0;
	}

	// auto-generate if necessary
	phos_gui_auto_gen_id(elem -> ID, elem -> ID, "elem", &elem_auto_id);

	// find duplicate pointers and IDs:
	for(size_t i = 0; i < elem_registry.size; ++i)
	{
		phos_gui_elem *e = elem_registry.data[i];

		if(e == elem)
		{
			vl_log(VL_ERROR, "Duplicate UI element pointer found while registering element: %p!\n", e);
			return 0;
		}
		if(strcmp(e -> ID, elem -> ID) == 0)
		{
			vl_log(VL_ERROR, "Duplicate ID found: '%s'!\n", e -> ID);
			return 0;
		}
	}

	// no duplicate, the elem can be registered
	dynas_add(&elem_registry, elem);

	vl_log(VL_SUCCESS, "Registered element with ID: '%s'!\n", elem -> ID);

	return 1;
}
static void phos_gui_resolve_margin_collisions(phos_gui *gui)
{
	// iterate over all elements in the gui
	for(size_t i = 0; i < gui -> num_elems - 1; ++i)
	{
		// re-iterate over all other elements
		for(size_t j = 1; j < gui -> num_elems; ++j)
		{
			// get elems:
			phos_gui_elem *e1 = gui -> elems[i];
			phos_gui_elem *e2 = gui -> elems[j];

			// create margin rects
			Rectangle margin1 = phos_gui_get_outer_elem_rect(e1);
			Rectangle margin2 = phos_gui_get_outer_elem_rect(e2);

			// check collision
			bool collision = CheckCollisionRecs(margin1, margin2);

			// resolve collision
			if(collision)
			{
				// calculate widths and heights of margins
				float margin1_right = margin1.x + margin1.width;
				float margin1_bottom = margin1.y + margin1.height;

				float margin2_right = margin2.x + margin2.width;
				float margin2_bottom = margin2.y + margin2.height;

				/* to calculate overlap:

				20					   40

				   --------------------
				   -			      -
				   -				  -
				   -		30		  -         50
				   -		+++++++++ - ++++++++
				   -		+		  -        +
				   --------------------		   +
							+				   +
							+				   +
							++++++++++++++++++++



					r1 = [20, 40]
					r2 = [30, 50]

					the two left edges, 20 and 30, are compared, and 30 is greater, so it is chosen as a starting point.

					the two right edges, 40 and 50, are compared, and 40 is smaller, so it is chosen as the ending point.

					now just do 40 - 30 to obtain the length of the horizontal overlap: 10.

					apply the same logic to the y-axes, and you can obtain vertical overlap.
				*/

				float overlap_x = fminf(margin1_right, margin2_right) - fmaxf(margin1.x, margin2.x);
				float overlap_y = fminf(margin1_bottom, margin2_bottom) - fmaxf(margin1.y, margin2.y);

				// push the items apart using overlap values
				switch(gui -> layout_type)
				{
					case PHOS_GUI_VERTICAL_LAYOUT:
						if(e1 -> pos.y < e2 -> pos.y)
							phos_gui_move_elem(e2, 0, overlap_y);
						else if(e2 -> pos.y < e1 -> pos.y)
							phos_gui_move_elem(e1, 0, overlap_y);
						break;
					case PHOS_GUI_HORIZONTAL_LAYOUT:
						if(e1 -> pos.x < e2 -> pos.x)
							phos_gui_move_elem(e2, overlap_x, 0);
						else if(e2 -> pos.x < e1 -> pos.x)
							phos_gui_move_elem(e1, overlap_x, 0);
						break;
				}
			}
		}
	}
}
int phos_gui_add_elem(phos_gui *gui, phos_gui_elem *elem)
{
	if(!gui || !elem)
	{
		vl_log(VL_ERROR, "Failed to add UI element. Make sure 'gui' and 'elem' are not NULL!\n");
		return 0;
	}

	// try to register 'elem'
	if(phos_gui_register_elem(elem) == 0)
	{
		vl_log(VL_ERROR, "Failed to register the given UI element!\n");
		return 0;
	}

	// add 'elem' to 'curr_elems' in the given phos_gui
	if(gui -> num_elems >= PHOS_GUI_MAX_ELEMS)
	{
		vl_log(VL_WARNING, "No more elements can be added to this phos_gui: %p\n", gui);
		return 0;
	}
	gui -> elems[gui -> num_elems++] = elem;

	// resolve any collisions after adding element
	phos_gui_resolve_margin_collisions(gui);

	// ensure elem remembers its GUI
	elem -> gui = gui;

	return 1;
}
int phos_gui_add_elem_id(phos_gui *gui, phos_gui_elem *elem, const char *ID)
{
	if(!gui || !elem || !ID)
	{
		vl_log(VL_ERROR, "Failed to add UI element. Make sure 'gui', 'elem' and 'ID' are not NULL!\n");
		return 0;
	}
	if(strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Element ID cannot be empty!\n");
		return 0;
	}

	// copy ID into element's ID
	strcpy(elem -> ID, ID);

	// add normally
	return phos_gui_add_elem(gui, elem);
}
phos_gui_elem *phos_gui_get_elem(const char *ID)
{
	if(!ID || strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Cannot obtain UI element with null ID!\n");
		return NULL;
	}
	if(!init)
	{
		vl_log(VL_ERROR, "Cannot obtain UI element, phos_gui_init() was never called!\n");
		return NULL;
	}

	for(size_t i = 0; i < elem_registry.size; ++i)
	{
		phos_gui_elem *e = elem_registry.data[i];

		if(e)
			if(strcmp(e -> ID, ID) == 0)
				return e;
	}

	vl_log(VL_ERROR, "No UI element found with the ID: '%s'\n", ID);
	return NULL;
}
void phos_gui_clone_elem(phos_gui_elem *elem, const char *ID)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot clone null UI element!\n");
		return;
	}
	if(!ID || strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Invalid blueprint ID!\n");
		return;
	}

	phos_gui_blueprint blueprint = {0};

	// auto-generate ID if necessary
	phos_gui_auto_gen_id(ID, blueprint.ID, "blueprint", &blueprint_auto_id);

	// see if a duplicate blueprint exists
	for(size_t i = 0; i < blueprint_registry.size; ++i)
	{
		phos_gui_blueprint *bp = &blueprint_registry.data[i];

		if(bp -> elem == elem)
		{
			vl_log(VL_ERROR, "Duplicate UI element pointer found while creating blueprint: %p!\n", elem);
			return;
		}
		if(strcmp(bp -> ID, ID) == 0)
		{
			vl_log(VL_ERROR, "Duplicate blueprint ID found: '%s'!\n", bp -> ID);
			return;
		}
	}

	strcpy(blueprint.ID, ID);
	blueprint.elem = elem;

	// no duplicate, blueprint can be created and saved
	dynas_add(&blueprint_registry, blueprint);

	vl_log(VL_SUCCESS, "Registered blueprint with ID: '%s'!\n", ID);
}
void phos_gui_init_clone(phos_gui_elem *target_elem, const char *ID)
{
	if(!ID || strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Cannot create new instance of a blueprint with invalid ID!\n");
		return;
	}

	// obtain blueprint
	phos_gui_blueprint *bp = NULL;
	for(size_t i = 0; i < blueprint_registry.size; ++i)
	{
		phos_gui_blueprint *saved_bp = &blueprint_registry.data[i];

		// find matching blueprint
		if(strcmp(saved_bp -> ID, ID) == 0)
		{
			bp = saved_bp;
			break;
		}
	}

	// if no matching blueprint:
	if(!bp)
	{
		vl_log(VL_ERROR, "No blueprint found with the ID: '%s'!\n", ID);
		return;
	}

	// start creating new instance
	*target_elem = *bp -> elem;
	strcpy(target_elem -> ID, "<auto-gen>");

	// add and register the elem
	phos_gui_add_elem(bp -> elem -> gui, target_elem);
}

static void phos_gui_move_cursor_left(phos_gui_elem *e)
{
	if(e -> text.cursor_pos > 0)
	{
		e -> text.cursor_pos--;
		phos_gui_update_text_scrolling(e);
	}
}
static void phos_gui_move_cursor_right(phos_gui_elem *e)
{
	if(e -> text.cursor_pos < e -> text.len)
	{
		e -> text.cursor_pos++;
		phos_gui_update_text_scrolling(e);
	}
}
static void phos_gui_backspace(phos_gui_elem *e)
{
	if(e -> text.len > 0)
	{
		e -> text.str[--e -> text.len] = '\0';
		e -> text.cursor_pos--;
		e -> text.edited = true;
	}
}
static void phos_gui_update_key_timer(phos_gui_elem *e, float dt, phos_gui_key_timer *kt, void (*action) (phos_gui_elem*))
{
	if(IsKeyDown(kt -> key))
	{
		if(!kt -> active)
		{
			action(e);
			kt -> timer = PHOS_GUI_KEY_REPEAT_DELAY;
		}
		else
		{
			kt -> timer -= dt;

			if(kt -> timer <= 0.0f)
			{
				action(e);
				kt -> timer = PHOS_GUI_KEY_REPEAT_INTERVAL;
			}
		}

		kt -> active = true;
	}
	else
		kt -> active = false;
}

static void phos_gui_update_elem(phos_gui_elem *e, float dt)
{
	// get mouse information
	Vector2 mouse_pos = GetMousePosition();

	bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
	bool mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

	// see if mouse over element:
	if(mouse_pos.x > e -> pos.x && mouse_pos.x < e -> pos.x + e -> size.x && mouse_pos.y > e -> pos.y && mouse_pos.y < e -> pos.y + e -> size.y)
	{
		e -> hovered = true;

		if(mouse_clicked)
		{
			e -> clicked = true;
			e -> pressed = true;
			e -> has_focus = true;
		}
		else if(mouse_down)
		{
			e -> pressed = true;
			e -> clicked = false;
			e -> has_focus = true;
		}
		else
		{
			e -> clicked = false;
			e -> pressed = false;
		}
	}
	else if(mouse_clicked)
	{
		e -> has_focus = false;
		e -> hovered = false;
		e -> clicked = false;
		e -> pressed = false;
	}
	else
	{
		e -> hovered = false;
		e -> clicked = false;
		e -> pressed = false;
	}

	// check type of element:
	if(e -> type == PHOS_GUI_TEXT_FIELD)
	{
		// reset 'edited' field
		e -> text.edited = false;

		// only type into text field if it has focus
		if(e -> has_focus)
		{
			// collect key
			int k = GetKeyPressed();
			e -> text.key_typed = k;

			// collect every char typed:
			char c = GetCharPressed();
			
			while(c > 0)
			{
				// get type of c
				bool letter = isalpha(c);
				bool num = isdigit(c);
				bool special = !letter & !num;

				// see if text field accepts this type of char
				if(letter && !e -> text.accept_letters)
				{
					c = GetCharPressed();
					continue;
				}
				if(num && !e -> text.accept_nums)
				{
					c = GetCharPressed();
					continue;
				}
				// let ' ' through the special char check
				if(special && !e -> text.accept_specials && c != ' ')
				{
					c = GetCharPressed();
					continue;
				}

				// assign char typed
				e -> text.char_typed = c;

				// insert char into string at cursor pos (if possible)
				if(e -> text.len + 1 <= e -> text.max_len && e -> text.len + 1 < PHOS_GUI_MAX_TEXT_LEN)
				{
					// first, move all chars at cursor pos one slot over to the right
					memmove(e -> text.str + e -> text.cursor_pos + 1, e -> text.str + e -> text.cursor_pos, e -> text.len - e -> text.cursor_pos + 1);

					// insert char and move to next cursor pos
					e -> text.str[e -> text.cursor_pos++] = c;

					// increase string length by one
					e -> text.len++;

					e -> text.edited = true;
				}

				// get next char pressed
				c = GetCharPressed();
			}

			phos_gui_update_key_timer(e, dt, &backspace_timer, phos_gui_backspace);
			phos_gui_update_key_timer(e, dt, &left_arrow_timer, phos_gui_move_cursor_left);
			phos_gui_update_key_timer(e, dt, &right_arrow_timer, phos_gui_move_cursor_right);
		}
		else
			e -> text.edited = false;

		// update text scrolling if edited
		if(e -> text.edited)
			phos_gui_update_text_scrolling(e);
	}
}
// TODO create phos_gui_update function for specific window scaling so GetMousePosition() always works!!
void phos_gui_update(float dt)
{
	if(!curr_gui)
	{
		vl_delay_log(VL_WARNING, 10.0f, "No phos_gui set, skipping updating.\n");
		return;
	}

	// if any margin collisions, resolve them immediately
	phos_gui_resolve_margin_collisions(curr_gui);

	// update elems:
	for(size_t i = 0; i < curr_gui -> num_elems; ++i)
		phos_gui_update_elem(curr_gui -> elems[i], dt);

	// update event listeners
	for(size_t i = 0; i < event_listeners.size; ++i)
	{
		// eval condition function and check to see if the target elem has focus
		bool exec = event_listeners.data[i].event(event_listeners.data[i].target) && event_listeners.data[i].target -> has_focus;
		if(exec)
			event_listeners.data[i].action(event_listeners.data[i].target);
	}
}

static void phos_gui_render_ellipse_outline(Vector2 pos, float rx, float ry, float line_thickness, Color color)
{
	for(int i = 0; i < line_thickness + 1; ++i)
		DrawEllipseLines(pos.x + rx, pos.y + ry, rx - i * 0.7f, ry - i * 0.7f, color);
}

static void phos_gui_render_elem(const phos_gui_elem *const e)
{
	// get color of elem
	Color e_color = e -> color;
	
	if(e -> pressed)
		e_color = e -> press_color;
	else if(e -> hovered)
		e_color = e -> hover_color;

	// create elem rects:
	Rectangle vis_bounds = phos_gui_get_visible_elem_rect(e);
	Rectangle inner_bounds = phos_gui_get_inner_elem_rect(e);

	// create elem ellipse info:
	float e_rx = e -> size.x / 2.0f;
	float e_ry = e -> size.y / 2.0f;

	// draw elem bg if it is valid
	if(e -> bg_texture && IsTextureValid(*e -> bg_texture))
	{
		// warn if the render mode is not PHOS_GUI_TEXTURE
		if(e -> render_mode != PHOS_GUI_TEXTURE)
			vl_delay_log(VL_WARNING, 3.0f, "This element ('%s') does not have the PHOS_GUI_TEXTURE render mode, skipping rendering texture.\n", e -> ID);
		else
		{
			Rectangle src = { 0, 0, e -> bg_texture -> width, e -> bg_texture -> height };
			DrawTexturePro(*e -> bg_texture, src, vis_bounds, PHOS_GUI_WIN_ORIGIN, e -> rotation, e_color);
		}
	}
	// else just draw base shape (if set)
	else if(e -> render_mode == PHOS_GUI_FILL_OUTLINE || e -> render_mode == PHOS_GUI_FILL)
	{
		switch(e -> shape)
		{
			case PHOS_GUI_RECT:
				DrawRectanglePro(vis_bounds, PHOS_GUI_WIN_ORIGIN, e -> rotation, e_color);
				break;
			case PHOS_GUI_ELLIPSE:
				DrawEllipse(vis_bounds.x + e_rx, vis_bounds.y + e_ry, e_rx, e_ry, e_color);
				break;
		}
	}

	// render text component of element (if valid):
	if(e -> text.font && IsFontValid(*e -> text.font))
	{
		if(e -> text.font_size <= 0.0f || ColorIsEqual(e -> text.color, BLANK))
			vl_delay_log(VL_WARNING, 1.0f, "This element's ('%s') text component will not render correctly due to invalid font size, or the color's alpha is 0!\n", e -> ID);
		else
		{
			/* begin scissor mode to cut off text that has been scrolled off (USE PADDED REGION, NOT VISUAL)
			note: add PHOS_GUI_CURSOR_WIDTH to scissor rect so the cursor is not cut off at the right side */
			BeginScissorMode(inner_bounds.x, inner_bounds.y, inner_bounds.width + PHOS_GUI_CURSOR_WIDTH, inner_bounds.height);

			switch(e -> type)
			{
				case PHOS_GUI_BUTTON:
					DrawTextEx(*e -> text.font, e -> text.str, e -> text.pos, e -> text.font_size, 0.0f, e -> text.color);
					break;
				case PHOS_GUI_TEXT_FIELD:
					// calculate where to draw the text
					Vector2 draw_pos = e -> text.pos;
					draw_pos.x -= e -> text.scroll;

					// determine if text field's main text, or placeholder text should be rendered
					if(strlen(e -> text.str) == 0 && strlen(e -> text.placeholder_str) > 0)
						DrawTextEx(*e -> text.font, e -> text.placeholder_str, draw_pos, e -> text.font_size, 0.0f, e -> text.placeholder_color);
					else
						DrawTextEx(*e -> text.font, e -> text.str, draw_pos, e -> text.font_size, 0.0f, e -> text.color);

					// render cursor (only if placeholder text is not being rendered and text field has focus)
					if(strlen(e -> text.str) > 0 && e -> has_focus)
					{
						char buf[PHOS_GUI_MAX_TEXT_LEN + 1];
						memcpy(buf, e -> text.str, e -> text.cursor_pos);
						buf[e -> text.cursor_pos] = '\0';

						float caret_x = MeasureTextEx(*e -> text.font, buf, e -> text.font_size, 0.0f).x;
						float cx = draw_pos.x + caret_x;
						DrawRectangle(cx, draw_pos.y, PHOS_GUI_CURSOR_WIDTH, e -> text.font_size, e -> text.color);
					}
					break;
			}

			EndScissorMode();
		}
	}

	// render outline (if set)
	if(e -> render_mode == PHOS_GUI_FILL_OUTLINE || e -> render_mode == PHOS_GUI_OUTLINE)
	{
		// get outline color
		Color outline_color = e -> outline_color;

		// if the elem is just being outlined, switch its outline color for 'e_color' so mouse state affects the elem visually
		if(e -> render_mode == PHOS_GUI_OUTLINE)
			outline_color = e_color;

		switch(e -> shape)
		{
			case PHOS_GUI_RECT:
				DrawRectangleLinesEx(vis_bounds, e -> outline_thickness, outline_color);
				break;
			case PHOS_GUI_ELLIPSE:
				phos_gui_render_ellipse_outline(e -> pos, e_rx, e_ry, e -> outline_thickness, outline_color);
				break;
		}
	}

	// if debug mode enabled, draw outlines around element margin and padding
	if(in_debug_mode)
	{
		// get elem bounds
		Rectangle padding_bounds = phos_gui_get_inner_elem_rect(e);
		Rectangle margin_bounds = phos_gui_get_outer_elem_rect(e);

		// convert bounds to positions
		Vector2 padding_pos = { padding_bounds.x, padding_bounds.y };
		Vector2 margin_pos = { margin_bounds.x, margin_bounds.y };

		switch(e -> shape)
		{
			case PHOS_GUI_RECT:
				DrawRectangleLinesEx(padding_bounds, PHOS_GUI_DEBUG_LINE_THICKNESS, PHOS_GUI_DEBUG_PADDING_COLOR);
				DrawRectangleLinesEx(margin_bounds, PHOS_GUI_DEBUG_LINE_THICKNESS, PHOS_GUI_DEBUG_MARGIN_COLOR);
				break;
			case PHOS_GUI_ELLIPSE:
				phos_gui_render_ellipse_outline(padding_pos, padding_bounds.width / 2.0f, padding_bounds.height / 2.0f, PHOS_GUI_DEBUG_LINE_THICKNESS, PHOS_GUI_DEBUG_PADDING_COLOR);
				phos_gui_render_ellipse_outline(margin_pos, margin_bounds.width / 2.0f, margin_bounds.height / 2.0f, PHOS_GUI_DEBUG_LINE_THICKNESS, PHOS_GUI_DEBUG_MARGIN_COLOR);
				break;
		}
	}
}
void phos_gui_render()
{
	if(!curr_gui)
	{
		vl_delay_log(VL_WARNING, 10.0f, "No phos_gui set, skipping rendering.\n");
		return;
	}

	for(size_t i = 0; i < curr_gui -> num_elems; ++i)
		phos_gui_render_elem(curr_gui -> elems[i]);
}

Texture2D *phos_gui_load_texture(const char *file_path)
{
	if(!file_path)
	{
		vl_log(VL_ERROR, "Cannot load texture using null file path!\n");
		return NULL;
	}

	// see if an existing texture has been loaded with the given file path
	for(size_t i = 0; i < textures.size; ++i)
		if(strcmp(textures.data[i].file_path, file_path) == 0)
			return &textures.data[i].tex;
	
	Texture2D tex = LoadTexture(file_path);

	if(!IsTextureValid(tex))
	{
		vl_log(VL_ERROR, "Failed to load texture: '%s'!\n", file_path);
		return NULL;
	}

	phos_gui_tex pg_tex = { .tex = tex, .file_path = file_path };

	dynas_add(&textures, pg_tex);

	vl_log(VL_SUCCESS, "Loaded texture: '%s'!\n", file_path);

	return &textures.data[textures.size - 1].tex;
}

Font *phos_gui_load_font(const char *file_path)
{
	if(!file_path)
	{
		vl_log(VL_ERROR, "Cannot load font using null file path!\n");
		return NULL;
	}

	// see if an existing font has been loaded with the given file path
	for(size_t i = 0; i < fonts.size; ++i)
		if(strcmp(fonts.data[i].file_path, file_path) == 0)
			return &fonts.data[i].font;

	Font font = LoadFont(file_path);

	if(!IsFontValid(font))
	{
		vl_log(VL_ERROR, "Failed to load font: '%s'!\n", file_path);
		return NULL;
	}

	phos_gui_font pg_font = { .font = font, .file_path = file_path };

	dynas_add(&fonts, pg_font);

	vl_log(VL_SUCCESS, "Loaded font: '%s'!\n", file_path);

	return &fonts.data[fonts.size - 1].font;
}
