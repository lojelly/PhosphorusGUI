#include <math.h>
#include <ctype.h>
#include "dynamic_array_spellbook.h"
#include "dynamic_map_spellbook.h"
#include "vibrant_logs.h"
#include "plutonium_cs.h"
#include "phosphorus_gui.h"
#include "raymath.h"

#define PHOS_GUI_DEBUG_PADDING_COLOR RED
#define PHOS_GUI_DEBUG_MARGIN_COLOR GREEN
#define PHOS_GUI_DEBUG_LINE_THICKNESS 5.0f

#define PHOS_GUI_CURSOR_WIDTH 3
#define PHOS_GUI_KEY_REPEAT_DELAY 0.5f
#define PHOS_GUI_KEY_REPEAT_INTERVAL 0.033f

#define PHOS_GUI_ROUND_RECT_SEGMENTS 64

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

static phos_gui_tex_arr textures;
static phos_gui_font_arr fonts;


// for objects with "auto" ID, ensures unique auto-generated IDs
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

// keep track of current 'goto' elem in the current GUI
static int curr_gui_elem_num = -1;

static float win_scale_x = 1.0f;
static float win_scale_y = 1.0f;


#define phos_gui_assert_obj_ptr(obj, ptr, ...) \
	do { \
		if(!(obj) -> ptr) \
		{ \
			vl_log(VL_ERROR, "Failed to allocate memory!\n"); \
			return __VA_ARGS__; \
		} \
	} while(0)

#define phos_gui_init_arr(arr, ...) \
	do { \
		dynas_init(arr); \
		phos_gui_assert_obj_ptr(arr, data, __VA_ARGS__); \
	} while(0)

#define phos_gui_init_map(map, ...) \
	do { \
		dynmaps_init(map); \
		phos_gui_assert_obj_ptr(map, keys, __VA_ARGS__); \
		phos_gui_assert_obj_ptr(map, values, __VA_ARGS__); \
	} while(0)

int phos_gui_init()
{
	if(init)
	{
		vl_log(VL_ERROR, "Cannot re-initialize PhosphorusGUI!\n");
		return 0;
	}

	// simple registry arrays:
	phos_gui_init_arr(&elem_registry, 0);
	phos_gui_init_arr(&blueprint_registry, 0);
	phos_gui_init_arr(&gui_registry, 0);

	// resources:
	phos_gui_init_arr(&textures, 0);
	phos_gui_init_arr(&fonts, 0);

	backspace_timer.key = KEY_BACKSPACE;
	left_arrow_timer.key = KEY_LEFT;
	right_arrow_timer.key = KEY_RIGHT;

	init = true;
	return 1;
}
void phos_gui_shutdown()
{
	if(!init)
	{
		vl_log(VL_ERROR, "PhosphorusGUI was never initialized, cannot shutdown!\n");
		return;
	}

	// simple registry arrays:
	dynas_free(&elem_registry);
	dynas_free(&blueprint_registry);
	dynas_free(&gui_registry);

	// resources:

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
	if(strcmp(ID, "!auto") == 0)
		sprintf(target, "!auto");
	else if(strcmp(ID, "auto") == 0)
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

	// reset 'goto' elem tracker
	curr_gui_elem_num = -1;
}
void phos_gui_set_gui_by_id(const char *ID)
{
	if(!ID || strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Cannot set phos_gui with an invalid ID!\n");
		return;
	}

	// try to get phos_gui with matching ID
	phos_gui *gui = phos_gui_get_gui(ID);

	if(!gui)
	{
		vl_log(VL_ERROR, "Failed to set the current GUI, no matching phos_gui found!\n");
		return;
	}

	phos_gui_set_gui(gui);
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

	// first move elem
	elem -> pos.x += x;
	elem -> pos.y += y;

	// then move elem's text component
	if(pluto_cs_check_component(elem, PHOS_GUI_COMPONENT_TEXT))
	{
		phos_gui_text_component *txt = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
		txt -> pos.x += x;
		txt -> pos.y += y;
	}

	// then move elem's child elements the same amount of pixels
	for(size_t i = 0; i < elem -> num_children; ++i)
		phos_gui_move_elem(elem -> children[i], x, y);
}

Vector2 phos_gui_get_elem_center(phos_gui_elem *elem)
{
	Vector2 v = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain center of a null UI element!\n");
		return v;
	}

	// start at inner origin
	Rectangle inner_rect = phos_gui_get_inner_elem_rect(elem);
	v = (Vector2) { inner_rect.x, inner_rect.y };

	// add half the elem size
	v.x += inner_rect.width / 2.0f;
	v.y += inner_rect.height / 2.0f;

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
	if(!pluto_cs_check_component(elem, PHOS_GUI_COMPONENT_TEXT))
	{
		vl_log(VL_ERROR, "Cannot obtain text center of a UI element ('%s') with no text component!\n", elem -> ID);
		return v;
	}

	phos_gui_text_component *txt = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);

	if(!txt -> font)
	{
		vl_log(VL_ERROR, "This element ('%s') has a null font, cannot obtain center with text!\n", elem -> ID);
		return v;
	}

	// get center of elem
	v = phos_gui_get_elem_center(elem);

	// measure text
	Vector2 text_size = {0};
	if(strlen(txt -> str) > 0)
		text_size = MeasureTextEx(*txt -> font, txt -> str, txt -> font_size, 0.0f);
	else if(strlen(txt -> placeholder_str) > 0)
		text_size = MeasureTextEx(*txt -> font, txt -> placeholder_str, txt -> font_size, 0.0f);

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

Rectangle phos_gui_get_text_bounds(const phos_gui_text_component *const text_component, const char *str)
{
	Rectangle r = {0};

	if(!text_component || !text_component -> owner)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for null text component/UI element!\n");
		return r;
	}
	if(!str)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for null string!\n");
		return r;
	}

	// get text component owner
	const phos_gui_elem *const elem = text_component -> owner;

	if(strlen(str) == 0)
	{
		vl_delay_log(VL_WARNING, 5.0f, "Cannot obtain text bounds for empty text on element: '%s'!\n", elem -> ID);
		return r;
	}
	if(!text_component -> font || !IsFontValid(*text_component -> font) || text_component -> font_size <= 0.0f)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for invalid font on element: '%s'!\n", elem -> ID);
		return r;
	}

	// width and height of the text
	Vector2 text_size = MeasureTextEx(*text_component -> font, str, text_component -> font_size, 0.0f);

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
Vector2 phos_gui_get_text_bounds_v(const phos_gui_text_component *const text_component, const char *str)
{
	Rectangle r = phos_gui_get_text_bounds(text_component, str);
	return (Vector2) { r.width, r.height };
}

static void phos_gui_update_text_scrolling(phos_gui_text_component *text)
{
	// first determine whether or not to use text.str or text.placeholder_str
	const char *str = NULL;
	if(strlen(text -> str) > 0)
		str = text -> str;
	else if(strlen(text -> placeholder_str) > 0)
		str = text -> placeholder_str;
	else
	{
		text -> scroll = 0.0f;
		text -> max_scroll = 0.0f;
		vl_delay_log(VL_WARNING, 5.0f, "Failed to update text scrolling. Text component must contain string data to calculate!\n");
		return;
	}

	// get owner of text component
	phos_gui_elem *e = text -> owner;

	// first get visual bounds of text component on screen
	Rectangle vis_bounds = phos_gui_get_visible_elem_rect(e);

	// get bounds of text
	Rectangle text_bounds = phos_gui_get_text_bounds(text, str);

	// calculate the overflow
	float vis_left = vis_bounds.x + e -> left_padding;
	float vis_right = vis_bounds.x + vis_bounds.width - e -> right_padding;
	float vis_width = vis_right - text -> pos.x;
	float overflow = text_bounds.width - vis_width;

	if(overflow > 0.0f)
		text -> max_scroll = overflow;
	else
	{
		text -> max_scroll = 0.0f;
		text -> scroll = 0.0f;
	}

	// only handle caret logic for the editable string:
	if(strlen(text -> str) > 0)
	{
		char buf[PHOS_GUI_MAX_TEXT_LEN + 1];
		memcpy(buf, text -> str, text -> cursor_pos);
		buf[text -> cursor_pos] = '\0';

		float caret_x = MeasureTextEx(*text -> font, buf, text -> font_size, 0.0f).x;
		float caret_screen = text -> pos.x + caret_x - text -> scroll;

		// right-side check (include cursor width because the cursor takes up that many more pixels)
		if(caret_screen + PHOS_GUI_CURSOR_WIDTH > vis_right)
			text -> scroll += (caret_screen + PHOS_GUI_CURSOR_WIDTH) - vis_right;

		// left-side check (don't include cursor width)
		if(caret_screen < vis_left)
			text -> scroll -= vis_left - caret_screen;
	}

	text -> scroll = Clamp(text -> scroll, 0.0f, text -> max_scroll);
}

void phos_gui_init_text(phos_gui_text_component *text, const char *str, float font_size, Color color)
{
	if(!text)
	{
		vl_log(VL_ERROR, "Cannot initialize null text component!\n");
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

	text -> len = strlen(str);
	strcpy(text -> str, str);
	text -> max_len = PHOS_GUI_MAX_TEXT_LEN;
	text -> color = color;
	text -> font_size = font_size;
	text -> editable = true;
	text -> edited = false;
	text -> cursor_pos = text -> len;
	text -> accept_letters = text -> accept_nums = text -> accept_specials = true;
}
void phos_gui_init_placeholder_text(phos_gui_text_component *text, const char *str, Color color)
{
	if(!text)
	{
		vl_log(VL_ERROR, "Cannot initialize placeholder text on null text component!\n");
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

	strcpy(text -> placeholder_str, str);
	text -> placeholder_color = color;
}

void phos_gui_set_elem_pos(phos_gui_elem *elem, float x, float y)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set the position of a null UI element!\n");
		return;
	}

	// find difference between current pos and new pos
	float x_diff = x - elem -> pos.x;
	float y_diff = y - elem -> pos.y;

	// move based on difference
	phos_gui_move_elem(elem, x_diff, y_diff);
}
void phos_gui_set_elem_size(phos_gui_elem *elem, float w, float h)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set the size of a null UI element!\n");
		return;
	}

	elem -> size = (Vector2) { w, h };
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
	if(pluto_cs_check_component(elem, PHOS_GUI_COMPONENT_TEXT))
	{
		phos_gui_text_component *txt = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
		txt -> pos = Vector2Add(txt -> pos, diff);
	}

	// then move elem's child elements the same amount of pixels
	for(size_t i = 0; i < elem -> num_children; ++i)
		phos_gui_move_elem(elem -> children[i], diff.x, diff.y);
}

void phos_gui_init_color_set(phos_gui_color_set *set, Color normal_color, Color hover_color, Color press_color, Color focus_color)
{
	if(!set)
	{
		vl_log(VL_ERROR, "Cannot set colors of null color set!\n");
		return;
	}

	*set = (phos_gui_color_set) {
		.normal_color = normal_color,
		.hover_color = hover_color,
		.press_color = press_color,
		.focus_color = focus_color };
}
PHOS_GUI_API void phos_gui_fill_color_set(phos_gui_color_set *set, Color color)
{
	phos_gui_init_color_set(set, color, color, color, color);
}
void phos_gui_gen_color_set(phos_gui_color_set *set, Color normal_color, float hover_color_factor, float press_color_factor, float focus_color_factor)
{
	if(!set)
	{
		vl_log(VL_ERROR, "Cannot generate colors of null color set!\n");
		return;
	}
	
	*set = (phos_gui_color_set) {
		.normal_color = normal_color,
		.hover_color = ColorBrightness(normal_color, hover_color_factor),
		.press_color = ColorBrightness(normal_color, press_color_factor),
		.focus_color = ColorBrightness(normal_color, focus_color_factor) };
}

void phos_gui_set_text_contents(phos_gui_text_component *text_component, char *target_str, const char *str)
{
	if(!text_component)
	{
		vl_log(VL_ERROR, "Cannot set string contents of a null text_component!\n");
		return;
	}
	if(!target_str || !str)
	{
		vl_log(VL_ERROR, "String is null, cannot set UI element's text contents!\n");
		return;
	}

	strcpy(target_str, str);
	text_component -> cursor_pos = strlen(target_str);
}

Vector2 phos_gui_get_proposed_align_pos(const phos_gui_elem *const reference_elem, phos_gui_alignment alignment, Vector2 target_object_size)
{
	Vector2 v = {0};

	if(!reference_elem)
	{
		vl_log(VL_ERROR, "Cannot align null UI element!\n");
		return v;
	}

	// define rectangle bounds for element's content area (visible area + padding)
	float content_top = reference_elem -> pos.y + reference_elem -> top_padding;
	float content_left = reference_elem -> pos.x + reference_elem -> left_padding;
	float content_bottom = reference_elem -> pos.y + reference_elem -> size.y - reference_elem -> bottom_padding;
	float content_right = reference_elem -> pos.x + reference_elem -> size.x - reference_elem -> right_padding;
	float content_width = content_right - content_left;
	float content_height = content_bottom - content_top;

	// define rectangle bounds for element's logical area (visible_area + margin)
	float elem_top = reference_elem -> pos.y - reference_elem -> top_margin;
	float elem_left = reference_elem -> pos.x - reference_elem -> left_margin;
	float elem_bottom = reference_elem -> pos.y + reference_elem -> size.y + reference_elem -> bottom_margin;
	float elem_right = reference_elem -> pos.x + reference_elem -> size.x + reference_elem -> right_margin;
	float elem_width = elem_right - elem_left;
	float elem_height = elem_bottom - elem_top;

	switch(alignment)
	{
		case PHOS_GUI_ALIGN_INNER_TOP:
			v.x = content_left + (content_width - target_object_size.x) / 2.0f;
			v.y = content_top;
			break;
		case PHOS_GUI_ALIGN_INNER_LEFT:
			v.x = content_left;
			v.y = content_top + (content_height - target_object_size.y) / 2.0f;
			break;
		case PHOS_GUI_ALIGN_INNER_BOTTOM:
			v.x = content_left + (content_width - target_object_size.x)  / 2.0f;
			v.y = content_bottom - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_INNER_RIGHT:
			v.x = content_right - target_object_size.x;
			v.y = content_top + (content_height - target_object_size.y) / 2.0f;
			break;
		case PHOS_GUI_ALIGN_INNER_CENTER:
			v.x = content_left + (content_width - target_object_size.x) / 2.0f;
			v.y = content_top + (content_height - target_object_size.y) / 2.0f;
			break;
		case PHOS_GUI_ALIGN_INNER_TOP_LEFT:
			v.x = content_left;
			v.y = content_top;
			break;
		case PHOS_GUI_ALIGN_INNER_TOP_RIGHT:
			v.x = content_right - target_object_size.x;
			v.y = content_top;
			break;
		case PHOS_GUI_ALIGN_INNER_BOTTOM_LEFT:
			v.x = content_left;
			v.y = content_bottom - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_INNER_BOTTOM_RIGHT:
			v.x = content_right - target_object_size.x;
			v.y = content_bottom - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_LEFT:
			v.x = elem_left - target_object_size.x;
			v.y = elem_top + (elem_height - target_object_size.y) / 2.0f;
			break;
		case PHOS_GUI_ALIGN_TOP:
			v.x = elem_left + (elem_width - target_object_size.x) / 2.0f;
			v.y = elem_top - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_RIGHT:
			v.x = elem_right;
			v.y = elem_top + (elem_height - target_object_size.y) / 2.0f;
			break;
		case PHOS_GUI_ALIGN_BOTTOM:
			v.x = elem_left + (elem_width - target_object_size.x) / 2.0f;
			v.y = elem_bottom;
			break;
		case PHOS_GUI_ALIGN_TOP_LEFT:
			v.x = elem_left - target_object_size.x;
			v.y = elem_top - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_TOP_RIGHT:
			v.x = elem_right;
			v.y = elem_top - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_BOTTOM_LEFT:
			v.x = elem_left - target_object_size.x;
			v.y = elem_bottom;
			break;
		case PHOS_GUI_ALIGN_BOTTOM_RIGHT:
			v.x = elem_right;
			v.y = elem_bottom;
			break;
		default:
			vl_log(VL_ERROR, "Invalid alignment: %d!\n", alignment);
			break;
	}

	return v;
}
Vector2 phos_gui_get_proposed_text_align_pos(const phos_gui_text_component *const reference_text_component, phos_gui_alignment alignment, const char *target_str)
{
	Vector2 v = {0};

	if(!reference_text_component || !reference_text_component -> owner)
	{
		vl_log(VL_ERROR, "Cannot align text on null text component/UI element!\n");
		return v;
	}
	if(!target_str || strlen(target_str) <= 0)
	{
		vl_log(VL_ERROR, "Invalid string in phos_gui_get_proposed_text_align_pos!\n");
		return v;
	}

	Rectangle elem_text_size = phos_gui_get_text_bounds(reference_text_component, target_str);
	v = phos_gui_get_proposed_align_pos(reference_text_component -> owner, alignment, (Vector2) { elem_text_size.width, elem_text_size.height });
	return v;
}
Vector2 phos_gui_align_elem(phos_gui_elem *target_elem, phos_gui_alignment alignment, const phos_gui_elem *const reference_elem)
{
	Vector2 v = {0};

	if(!target_elem)
	{
		vl_log(VL_ERROR, "Cannot align null target element!\n");
		return v;
	}
	if(!reference_elem)
	{
		vl_log(VL_ERROR, "Cannot align target element with null reference element!\n");
		return v;
	}

	v = phos_gui_get_proposed_align_pos(reference_elem, alignment, target_elem -> size);
	phos_gui_set_elem_pos(target_elem, v.x, v.y);

	return v;
}

void phos_gui_use_largest_possible_font_size(phos_gui_elem *elem, phos_gui_text_component *text_component, const char *text_component_target_str)
{
	if(!elem || !text_component || !text_component_target_str || strlen(text_component_target_str) <= 0)
	{
		vl_log(VL_ERROR, "To get the largest possible font size, 'elem,' 'text_component,' and 'text_component_target_str' can't be NULL/invalid!\n");
		return;
	}

	// get inner bounds of elem
	Rectangle inner_bounds = phos_gui_get_inner_elem_rect(elem);

	// first off, start off with a very large font size
	float font_size = PHOS_GUI_FONT_SIZE_MAX;

	// measure initial text bounds
	Vector2 text_bounds = MeasureTextEx(*text_component -> font, text_component_target_str, font_size, 0.0f);

	// while the text takes up more space than the inner bounds, font size automatically has to shrink
	while(font_size > 1.0f && (text_bounds.x > inner_bounds.width || text_bounds.y > inner_bounds.height))
	{
		// go to next font size
		font_size -= 1.0f;

		// re-measure text using current font size
		text_bounds = MeasureTextEx(*text_component -> font, text_component_target_str, font_size, 0.0f);
	}

	// when the text fits, keep those attributes
	text_component -> font_size = font_size;

	vl_log(VL_INFO, "Largest possible font size for '%s' is %f.\n", elem -> ID, font_size);
}
void phos_gui_clamp_elem_to_text(phos_gui_elem *elem, phos_gui_text_component *text_component, const char *text_component_target_str)
{
	if(!elem || !text_component || !text_component_target_str || strlen(text_component_target_str) <= 0)
	{
		vl_log(VL_ERROR, "To clamp the given element to the text component, 'elem,' 'text_component,' and 'text_component_target_str' can't be NULL/invalid!\n");
		return;
	}

	// get inner bounds of elem
	Rectangle inner_bounds = phos_gui_get_inner_elem_rect(elem);

	// measure text bounds
	Vector2 text_bounds = MeasureTextEx(*text_component -> font, text_component_target_str, text_component -> font_size, 0.0f);

	// match elem bounds to text bounds
	phos_gui_set_elem_size(elem, text_bounds.x, text_bounds.y);
}

static void phos_gui_resolve_margin_positions(phos_gui_elem *e1, phos_gui_elem *e2, phos_gui_layout_type layout_type, float overlap_x, float overlap_y)
{
	// push the 2nd item apart using overlap values
	switch(layout_type)
	{
		case PHOS_GUI_LAYOUT_VERTICAL:
			phos_gui_move_elem(e1, 0, overlap_y / 2.0f);
			phos_gui_move_elem(e2, 0, overlap_y / 2.0f);
			break;
		case PHOS_GUI_LAYOUT_HORIZONTAL:
			phos_gui_move_elem(e1, overlap_x / 2.0f, 0);
			phos_gui_move_elem(e2, overlap_x / 2.0f, 0);
			break;
		default:
			vl_log(VL_ERROR, "Invalid layout type: %d!\n", layout_type);
			break;
	}
}
static bool phos_gui_margins_collide(phos_gui_elem *e1, phos_gui_elem *e2, float *out_overlap_x, float *out_overlap_y)
{
	// create margin rects
	Rectangle margin1 = phos_gui_get_outer_elem_rect(e1);
	Rectangle margin2 = phos_gui_get_outer_elem_rect(e2);

	// check collision
	bool collision = CheckCollisionRecs(margin1, margin2);
	float overlap_x = 0.0f;
	float overlap_y = 0.0f;

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

		overlap_x = fminf(margin1_right, margin2_right) - fmaxf(margin1.x, margin2.x);
		overlap_y = fminf(margin1_bottom, margin2_bottom) - fmaxf(margin1.y, margin2.y);
	}

	if(out_overlap_x)
		*out_overlap_x = overlap_x;
	if(out_overlap_y)
		*out_overlap_y = overlap_y;

	return collision;
}

PHOS_GUI_API void phos_gui_make_elem_fit_text(phos_gui_elem *elem, phos_gui_text_component *text_component, const char *text_component_target_str)
{
	if(!elem || !text_component || !text_component_target_str || strlen(text_component_target_str) <= 0)
	{
		vl_log(VL_ERROR, "To make the given element fit the text component, 'elem,' 'text_component,' and 'text_component_target_str' can't be NULL/invalid!\n");
		return;
	}

	// first, make sure every single parent for this element can fit the text
	phos_gui_elem *parent = elem -> parent;
	// while there is a valid parent elem
	while(parent)
	{
		// re-call phos_gui_make_elem_fit_text(...) but on the parent
		phos_gui_make_elem_fit_text(parent, text_component, text_component_target_str);

		// get the parent's parent and continue looping until a NULL parent is reached
		parent = parent -> parent;
	}

	// get inner bounds of elem
	Rectangle inner_bounds = phos_gui_get_inner_elem_rect(elem);

	// if the elem has a parent, obtain its rect as well
	Rectangle parent_bounds = {0};
	if(elem -> parent)
		parent_bounds = phos_gui_get_inner_elem_rect(elem -> parent);

	// measure text bounds
	Vector2 text_bounds = MeasureTextEx(*text_component -> font, text_component_target_str, text_component -> font_size, 0.0f);

	// expand element if necessary
	if(text_bounds.x > inner_bounds.width)
	{
		// find diff in widths
		float diff_w = text_bounds.x - inner_bounds.height;

		vl_log(VL_DEBUG, "initial diff_w = %f\n", diff_w);
		
		// expand by that much
		elem -> size.x += diff_w;
	}
	if(text_bounds.y > inner_bounds.height)
	{
		// find diff in heights
		float diff_h = text_bounds.y - inner_bounds.height;

		vl_log(VL_DEBUG, "initial diff_h = %f\n", diff_h);

		// expand by that much
		elem -> size.y += diff_h;
	}
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

void phos_gui_setup_elem(phos_gui_elem *elem, phos_gui_elem_type type, phos_gui_elem_render_mode render_mode, float x, float y, float w, float h)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot setup null UI element!\n");
		return;
	}

	elem -> type = type;
	elem -> render_mode = render_mode;
	elem -> pos = (Vector2) { x, y };
	elem -> size = (Vector2) { w, h };
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
	/* if no elements to resolve, skip.

	   the reason is because the first for loop declares 'gui -> num_elems - 1', but
	   'num_elems' is a size_t, so if it is equal to 0, subtracting 1 loops it back to SIZE_MAX
	   instead, causing undefined behavior.
	*/
	if(gui -> num_elems == 0)
		return;

	// iterate over all elements in the gui
	for(size_t i = 0; i < gui -> num_elems - 1; ++i)
	{
		// re-iterate over all other elements
		for(size_t j = 1; j < gui -> num_elems; ++j)
		{
			// get elems:
			phos_gui_elem *e1 = gui -> elems[i];
			phos_gui_elem *e2 = gui -> elems[j];

			// if these elems have a parent-child relationship, ignore their margins
			if(e1 -> parent == e2 || e2 -> parent == e1)
				continue;

			// check collision between margins, then resolve
			float overlap_x, overlap_y;
			if(phos_gui_margins_collide(e1, e2, &overlap_x, &overlap_y))
				phos_gui_resolve_margin_positions(e1, e2, gui -> layout_type, overlap_x, overlap_y);
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
int phos_gui_add_elem_to_container(phos_gui_elem *elem, phos_gui_elem *container)
{
	if(!container || !elem)
	{
		vl_log(VL_ERROR, "Failed to add element to container! Make sure 'container' and 'elem' are not NULL!\n");
		return 0;
	}

	// try to add elem to same phos_gui
	int success = phos_gui_add_elem(container -> gui, elem);
	if(success == 0)
	{
		vl_log(VL_ERROR, "Failed to add element to container!\n");
		return 0;
	}

	// add elem to container
	if(container -> num_children >= PHOS_GUI_MAX_CHILDREN)
	{
		vl_log(VL_ERROR, "This container cannot contain any more child elements: '%s'!\n", container -> ID);
		return 0;
	}
	container -> children[container -> num_children++] = elem;

	// the element's parent becomes the container
	elem -> parent = container;

	return 1;
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
	strcpy(target_elem -> ID, "auto");

	// add and register the elem
	phos_gui_add_elem(bp -> elem -> gui, target_elem);
}

void phos_gui_set_win_scale(float x, float y)
{
	win_scale_x = x;
	win_scale_y = y;
}

Vector2 phos_gui_get_mouse_pos()
{
	Vector2 mouse_pos = GetMousePosition();

	mouse_pos.x /= win_scale_x;
	mouse_pos.y /= win_scale_y;

	return mouse_pos;
}

static void phos_gui_move_cursor_left(phos_gui_text_component *t)
{
	if(t -> cursor_pos > 0)
	{
		t -> cursor_pos--;
		phos_gui_update_text_scrolling(t);
	}
}
static void phos_gui_move_cursor_right(phos_gui_text_component *t)
{
	if(t -> cursor_pos < t -> len)
	{
		t -> cursor_pos++;
		phos_gui_update_text_scrolling(t);
	}
}
static void phos_gui_backspace(phos_gui_text_component *t)
{
	if(t -> len > 0)
	{
		t -> str[--t -> len] = '\0';
		t -> cursor_pos--;
		t -> edited = true;
	}
}
static void phos_gui_update_key_timer(phos_gui_text_component *t, float dt, phos_gui_key_timer *kt, void (*action) (phos_gui_text_component*))
{
	if(IsKeyDown(kt -> key))
	{
		if(!kt -> active)
		{
			action(t);
			kt -> timer = PHOS_GUI_KEY_REPEAT_DELAY;
		}
		else
		{
			kt -> timer -= dt;

			if(kt -> timer <= 0.0f)
			{
				action(t);
				kt -> timer = PHOS_GUI_KEY_REPEAT_INTERVAL;
			}
		}

		kt -> active = true;
	}
	else
		kt -> active = false;
}

static void phos_gui_goto_next_elem()
{
	if(!curr_gui)
	{
		vl_log(VL_WARNING, "No current GUI, cannot go to next element!\n");
		return;
	}

	// curr elem loses focus
	if(curr_gui_elem_num >= 0)
		curr_gui -> elems[curr_gui_elem_num] -> has_focus = false;

	// go to next elem or loop back
	curr_gui_elem_num++;
	if(curr_gui_elem_num >= curr_gui -> num_elems)
		curr_gui_elem_num = 0;

	// mark that elem as focused
	curr_gui -> elems[curr_gui_elem_num] -> has_focus = true;
}
static void phos_gui_goto_prev_elem()
{
	if(!curr_gui)
	{
		vl_log(VL_WARNING, "No current GUI, cannot go to previous element!\n");
		return;
	}

	// curr elem loses focus
	if(curr_gui_elem_num >= 0)
		curr_gui -> elems[curr_gui_elem_num] -> has_focus = false;

	// loop back, or go to previous elem
	if(curr_gui_elem_num <= 0)
		curr_gui_elem_num = curr_gui -> num_elems - 1;
	else
		curr_gui_elem_num--;

	// get new elem
	phos_gui_elem *elem = curr_gui -> elems[curr_gui_elem_num];

	// mark that elem as focused (if a valid elem type)
	if(elem -> type != PHOS_GUI_TYPE_INVALID &&
			elem -> type != PHOS_GUI_TYPE_BASIC)
	{
		elem -> has_focus = true;
	}
}
static bool phos_gui_travel_elems()
{
	// goto prev or next elem
	bool tab_pressed = IsKeyPressed(KEY_TAB);
	if(IsKeyDown(KEY_LEFT_SHIFT) && tab_pressed)
	{
		phos_gui_goto_prev_elem();
		return true;
	}
	else if(tab_pressed)
	{
		phos_gui_goto_next_elem();
		return true;
	}

	return false;
}

static void phos_gui_update_elem(phos_gui_elem *e, float dt)
{
	/* no checks to see what kind of element is given,
	   as it is expected for the calling function to check
	   for the type. (ex phos_gui_update(...) calls this
	   function, but it only calls it on valid elements.
	*/

	// get mouse information
	Vector2 mouse_pos = phos_gui_get_mouse_pos();

	bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
	bool mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

	bool no_focus = !e -> has_focus;

	// see if mouse over element:
	
	float elem_x = e -> pos.x;
	float elem_y = e -> pos.y;
	float elem_w = e -> size.x;
	float elem_h = e -> size.y;
	if(mouse_pos.x > elem_x && mouse_pos.x < elem_x + elem_w && mouse_pos.y > elem_y && mouse_pos.y < elem_y + elem_h)
	{
		e -> hovered = true;

		if(mouse_down)
		{
			e -> pressed = true;
			e -> clicked = false;
			e -> has_focus = true;
		}
		else if(mouse_clicked)
		{
			e -> clicked = true;
			e -> pressed = true;
			e -> has_focus = true;
		}
		else
		{
			e -> clicked = false;
			e -> pressed = false;
		}
	}
	else if(mouse_clicked || mouse_down)
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

	// if elem now has focus, it gained focus
	if(no_focus && e -> has_focus)
		e -> gained_focus = true;
	else
		e -> gained_focus = false;

	// check type of element:
	if(e -> type == PHOS_GUI_TYPE_TEXT_FIELD)
	{
		// get text component
		if(!pluto_cs_check_component(e, PHOS_GUI_COMPONENT_TEXT))
			vl_delay_log(VL_WARNING, 5.0f, "Element '%s' is a text field, but is missing a text component!\n", e -> ID);
		else
		{
			phos_gui_text_component *text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_TEXT);

			// reset 'edited' field
			text -> edited = false;

			// only type into text field if it has focus
			if(e -> has_focus)
			{
				// collect key
				int k = GetKeyPressed();
				text -> key_typed = k;

				// collect every char typed:
				char c = GetCharPressed();

				while(c > 0)
				{
					// get type of c
					bool letter = isalpha(c);
					bool num = isdigit(c);
					bool special = !letter & !num;

					// see if text field accepts this type of char
					if(letter && !text -> accept_letters)
					{
						c = GetCharPressed();
						continue;
					}
					if(num && !text -> accept_nums)
					{
						c = GetCharPressed();
						continue;
					}
					// let ' ' through the special char check
					if(special && !text -> accept_specials && c != ' ')
					{
						c = GetCharPressed();
						continue;
					}

					// assign char typed
					text -> char_typed = c;

					// insert char into string at cursor pos (if possible)
					if(text -> len + 1 <= text -> max_len && text -> len + 1 < PHOS_GUI_MAX_TEXT_LEN)
					{
						// first, move all chars at cursor pos one slot over to the right
						memmove(text -> str + text -> cursor_pos + 1, text -> str + text -> cursor_pos, text -> len - text -> cursor_pos + 1);

						// insert char and move to next cursor pos
						text -> str[text -> cursor_pos++] = c;

						// increase string length by one
						text -> len++;

						text -> edited = true;
					}

					// get next char pressed
					c = GetCharPressed();
				}

				phos_gui_update_key_timer(text, dt, &backspace_timer, phos_gui_backspace);
				phos_gui_update_key_timer(text, dt, &left_arrow_timer, phos_gui_move_cursor_left);
				phos_gui_update_key_timer(text, dt, &right_arrow_timer, phos_gui_move_cursor_right);
			}
			else
				text -> edited = false;

			// update text scrolling if edited
			if(text -> edited)
				phos_gui_update_text_scrolling(text);
		}
	}
}

void phos_gui_update(float dt)
{
	if(!curr_gui)
	{
		vl_delay_log(VL_WARNING, 10.0f, "No phos_gui set, skipping updating.\n");
		return;
	}

	// if any margin collisions, resolve them immediately
	phos_gui_resolve_margin_collisions(curr_gui);

	// 'tab' and 'shift+tab' to travel between elems
	phos_gui_travel_elems();

	// update elems:
	for(size_t i = 0; i < curr_gui -> num_elems; ++i)
	{
		// get elem at i
		phos_gui_elem *elem = curr_gui -> elems[i];

		// cannot update invalid elements
		if(elem -> type == PHOS_GUI_TYPE_INVALID)
		{
			vl_delay_log(VL_ERROR, 1.0f, "Element has invalid type: '%s'!\n", elem -> ID);
			continue;
		}

		// and skip basic elems
		else if(elem -> type == PHOS_GUI_TYPE_BASIC)
			continue;

		// update the element
		phos_gui_update_elem(elem, dt);

		// if elem gained focus, make this elem the current one
		if(elem -> gained_focus)
			curr_gui_elem_num = i;
	}

	// if no elems to update, warn user
	if(curr_gui -> num_elems == 0)
		vl_delay_log(VL_WARNING, 10.0f, "The current phos_gui ('%s') has no elements! Skipping updating and rendering!\n", curr_gui -> ID);
}

static void phos_gui_render_ellipse_outline(Vector2 pos, float rx, float ry, float line_thickness, Color color)
{
	for(int i = 0; i < line_thickness + 1; ++i)
		DrawEllipseLines(pos.x + rx, pos.y + ry, rx - i * 0.7f, ry - i * 0.7f, color);
}

static Color phos_gui_resolve_elem_color(const phos_gui_elem *const e, const phos_gui_color_set *const colors)
{
	// get the normal color of elem
	Color color = colors -> normal_color;

	// 1. check to see if mouse button held down over element
	if(e -> pressed)
		color = colors -> press_color;
	// 2. check to see if they only have the mouse over the element
	else if(e -> hovered)
		color = colors -> hover_color;
	// 3. check to see if the elem has focus
	else if(e -> has_focus)
		color = colors -> focus_color;

	return color;
}

static void phos_gui_render_elem(const phos_gui_elem *const e)
{
	// get color of elem
	Color primary_color = phos_gui_resolve_elem_color(e, &e -> primary_colors);

	// create elem rects:
	Rectangle vis_bounds = phos_gui_get_visible_elem_rect(e);
	Rectangle inner_bounds = phos_gui_get_inner_elem_rect(e);

	// create elem ellipse info:
	float e_rx = e -> size.x / 2.0f;
	float e_ry = e -> size.y / 2.0f;

	// draw elem bg if it is valid
	if(e -> texture && IsTextureValid(*e -> texture))
	{
		Rectangle src = { 0, 0, e -> texture -> width, e -> texture -> height };
		DrawTexturePro(*e -> texture, src, vis_bounds, PHOS_GUI_WIN_ORIGIN, e -> rotation, primary_color);
	}
	// else just draw base shape (if set)
	else if(e -> render_mode == PHOS_GUI_RENDER_FILL_OUTLINE || e -> render_mode == PHOS_GUI_RENDER_FILL)
	{
		switch(e -> shape)
		{
			case PHOS_GUI_SHAPE_RECT:
				DrawRectanglePro(vis_bounds, PHOS_GUI_WIN_ORIGIN, e -> rotation, primary_color);
				break;
			case PHOS_GUI_SHAPE_ELLIPSE:
				DrawEllipse(vis_bounds.x + e_rx, vis_bounds.y + e_ry, e_rx, e_ry, primary_color);
				break;
			case PHOS_GUI_SHAPE_ROUND_RECT:
				DrawRectangleRounded(vis_bounds, e -> corner_radius, PHOS_GUI_ROUND_RECT_SEGMENTS, primary_color);
				break;
			default:
				vl_log(VL_ERROR, "Invalid element shape: %d!\n", e -> shape);
				break;
		}
	}

	// render text component of element (if valid):
	if(pluto_cs_check_component(e, PHOS_GUI_COMPONENT_TEXT))
	{
		phos_gui_text_component *text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_TEXT);

		if(text -> font && IsFontValid(*text -> font))
		{
			if(text -> font_size <= 0.0f || ColorIsEqual(text -> color, BLANK))
				vl_delay_log(VL_WARNING, 1.0f, "This element's ('%s') text component will not render correctly due to invalid font size, or the color's alpha is 0!\n", e -> ID);
			else
			{
				/* begin scissor mode to cut off text that has been scrolled off (USE PADDED REGION, NOT VISUAL)
				   note: add PHOS_GUI_CURSOR_WIDTH when rendering a text field to scissor rect so the cursor is not cut off at the right side */
				int width = e -> type == PHOS_GUI_TYPE_TEXT_FIELD ? inner_bounds.width + PHOS_GUI_CURSOR_WIDTH : inner_bounds.width;
				BeginScissorMode(inner_bounds.x, inner_bounds.y, width, inner_bounds.height);

				switch(e -> type)
				{
					// for basic elems and buttons, just render text with the set attributes
					case PHOS_GUI_TYPE_BASIC:
					case PHOS_GUI_TYPE_BUTTON:
						DrawTextEx(*text -> font, text -> str, text -> pos, text -> font_size, 0.0f, text -> color);
						break;
					case PHOS_GUI_TYPE_TEXT_FIELD:
						// calculate where to draw the text
						Vector2 draw_pos = text -> pos;
						draw_pos.x -= text -> scroll;

						// determine if text field's main text, or placeholder text should be rendered
						if(strlen(text -> str) == 0 && strlen(text -> placeholder_str) > 0)
							DrawTextEx(*text -> font, text -> placeholder_str, draw_pos, text -> font_size, 0.0f, text -> placeholder_color);
						else
							DrawTextEx(*text -> font, text -> str, draw_pos, text -> font_size, 0.0f, text -> color);

						// render cursor (only if placeholder text is not being rendered and text field has focus)
						if(strlen(text -> str) > 0 && e -> has_focus)
						{
							char buf[PHOS_GUI_MAX_TEXT_LEN + 1];
							memcpy(buf, text -> str, text -> cursor_pos);
							buf[text -> cursor_pos] = '\0';

							float caret_x = MeasureTextEx(*text -> font, buf, text -> font_size, 0.0f).x;
							float cx = draw_pos.x + caret_x;
							DrawRectangle(cx, draw_pos.y, PHOS_GUI_CURSOR_WIDTH, text -> font_size, text -> color);
						}
						break;
					default:
						// for INVALID_ELEM_TYPE or other types, just skip text component
						break;
				}

				EndScissorMode();
			}
		}
		else
			vl_delay_log(VL_WARNING, 5.0f, "Cannot render text component because it has no font set! Element: '%s'\n", e -> ID);
	}

	// render outline (if set)
	if(e -> render_mode == PHOS_GUI_RENDER_FILL_OUTLINE || e -> render_mode == PHOS_GUI_RENDER_OUTLINE)
	{
		// if thickness is 0 or less, warn
		if(e -> outline_thickness <= 0.0f)
			vl_delay_log(VL_WARNING, 5.0f, "Element's ('%s') outline thickness is invalid: %f\n", e -> ID, e -> outline_thickness);
		else
		{
			// get outline color
			Color outline_color = phos_gui_resolve_elem_color(e, &e -> outline_colors);

			switch(e -> shape)
			{
				case PHOS_GUI_SHAPE_RECT:
					DrawRectangleLinesEx(vis_bounds, e -> outline_thickness, outline_color);
					break;
				case PHOS_GUI_SHAPE_ELLIPSE:
					phos_gui_render_ellipse_outline(e -> pos, e_rx, e_ry, e -> outline_thickness, outline_color);
					break;
				case PHOS_GUI_SHAPE_ROUND_RECT:
					DrawRectangleRoundedLinesEx(vis_bounds, e -> corner_radius, PHOS_GUI_ROUND_RECT_SEGMENTS, e -> outline_thickness, outline_color);
					break;
				default:
					vl_log(VL_ERROR, "Invalid element shape: %d!\n", e -> shape);
					break;
			}
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
			case PHOS_GUI_SHAPE_RECT:
				DrawRectangleLinesEx(padding_bounds, PHOS_GUI_DEBUG_LINE_THICKNESS, PHOS_GUI_DEBUG_PADDING_COLOR);
				DrawRectangleLinesEx(margin_bounds, PHOS_GUI_DEBUG_LINE_THICKNESS, PHOS_GUI_DEBUG_MARGIN_COLOR);
				break;
			case PHOS_GUI_SHAPE_ELLIPSE:
				phos_gui_render_ellipse_outline(padding_pos, padding_bounds.width / 2.0f, padding_bounds.height / 2.0f, PHOS_GUI_DEBUG_LINE_THICKNESS, PHOS_GUI_DEBUG_PADDING_COLOR);
				phos_gui_render_ellipse_outline(margin_pos, margin_bounds.width / 2.0f, margin_bounds.height / 2.0f, PHOS_GUI_DEBUG_LINE_THICKNESS, PHOS_GUI_DEBUG_MARGIN_COLOR);
				break;
			case PHOS_GUI_SHAPE_ROUND_RECT:
				DrawRectangleRoundedLinesEx(padding_bounds, e -> corner_radius, PHOS_GUI_ROUND_RECT_SEGMENTS, e -> outline_thickness, PHOS_GUI_DEBUG_PADDING_COLOR);
				DrawRectangleRoundedLinesEx(margin_bounds, e -> corner_radius, PHOS_GUI_ROUND_RECT_SEGMENTS, e -> outline_thickness, PHOS_GUI_DEBUG_MARGIN_COLOR);
				break;
			default:
				vl_log(VL_ERROR, "Invalid element shape: %d!\n", e -> shape);
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
	{
		// get elem
		phos_gui_elem *elem = curr_gui -> elems[i];

		if(elem -> type == PHOS_GUI_TYPE_INVALID)
		{
			vl_delay_log(VL_ERROR, 1.0f, "Element has invalid type: '%s'!\n", elem -> ID);
			continue;
		}

		phos_gui_render_elem(elem);

		// warn about empty containers
		if(elem -> type == PHOS_GUI_TYPE_CONTAINER && elem -> num_children == 0)
			vl_delay_log(VL_WARNING, 10.0f, "Element is a container but has no children: '%s'!\n", elem -> ID);
	}
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

	SetTextureFilter(tex, TEXTURE_FILTER_POINT);

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

	Font font = LoadFontEx(file_path, PHOS_GUI_FONT_SIZE_MAX, NULL, 0);

	if(!IsFontValid(font))
	{
		vl_log(VL_ERROR, "Failed to load font: '%s'!\n", file_path);
		return NULL;
	}

	SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);

	phos_gui_font pg_font = { .font = font, .file_path = file_path };

	dynas_add(&fonts, pg_font);

	vl_log(VL_SUCCESS, "Loaded font: '%s'!\n", file_path);

	return &fonts.data[fonts.size - 1].font;
}
