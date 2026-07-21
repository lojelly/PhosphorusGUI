#include <math.h>
#include <ctype.h>
#include "dynamic_array_spellbook.h"
#include "dynamic_map_spellbook.h"
#include "vibrant_logs.h"
#include "plutonium_cs.h"
#include "phosphorus_gui.h"
#include "raymath.h"

#define CURSOR_WIDTH 3
#define KEY_REPEAT_DELAY 0.5f
#define KEY_REPEAT_INTERVAL 0.033f

#define ROUND_RECT_SEGMENTS 64

#define TEXT_PADDING PHOS_GUI_FONT_SIZE_MED

#define INVALID_RECT (Rectangle) { 0, 0, 0, 0 }

// array of element pointers
typedef struct elem_arr
{
	phos_gui_elem **data;
	size_t size, capacity;
} elem_arr;

// custom phos_gui texture (holds file path as well as Texture2D)
typedef struct texture
{
	Texture2D tex;
	const char *file_path;
} texture;

// custom phos_gui font (holds file path as well as Font)
typedef struct font
{
	Font font;
	const char *file_path;
} font;

// array of loaded textures
typedef struct tex_arr
{
	texture *data;
	size_t size, capacity;
} tex_arr;

// array of loaded fonts
typedef struct font_arr
{
	font *data;
	size_t size, capacity;
} font_arr;

// blueprints:
typedef struct blueprint
{
	char ID[PHOS_GUI_MAX_ID_LEN + 1];
	phos_gui_elem *elem;
	bool clone_children;
} blueprint;
typedef struct blueprint_arr
{
	blueprint *data;
	size_t size, capacity;
} blueprint_arr;

// gui registry:
typedef struct gui_arr
{
	phos_gui **data;
	size_t size, capacity;
} gui_arr;

static bool init = false;
static dynas_string_arr all_ids;
static elem_arr elem_registry;
static blueprint_arr blueprint_registry;
static gui_arr gui_registry;

static tex_arr textures;
static font_arr fonts;


// for objects with "auto" ID, ensures unique auto-generated IDs
static size_t elem_auto_id = 0;
static size_t blueprint_auto_id = 0;
static size_t gui_auto_id = 0;

// keyboard input
typedef struct key_timer
{
	// what key is being used?
	int key;
	// time left
	float timer;
	// is the key down?
	bool active;
} key_timer;

static key_timer backspace_timer = {0};
static key_timer left_arrow_timer = {0};
static key_timer right_arrow_timer = {0};

static phos_gui *prev_gui = NULL;
static phos_gui *curr_gui = NULL;
static bool resolve_focus_on_start_elem = false;

// keep track of current 'goto' elem in the current GUI
static int curr_gui_elem_num = -1;

static float win_scale_x = 1.0f;
static float win_scale_y = 1.0f;


#define assert_obj_ptr(obj, ptr, ...) \
	do { \
		if(!(obj)->ptr) \
		{ \
			vl_log(VL_ERROR, "Failed to allocate memory!\n"); \
			return __VA_ARGS__; \
		} \
	} while(0)

#define init_arr(arr, ...) \
	do { \
		dynas_init(arr); \
		assert_obj_ptr(arr, data, __VA_ARGS__); \
	} while(0)

#define init_map(map, ...) \
	do { \
		dynmaps_init(map); \
		assert_obj_ptr(map, keys, __VA_ARGS__); \
		assert_obj_ptr(map, values, __VA_ARGS__); \
	} while(0)

static void init_text_component(void *text_component, void *owner)
{
	if(!text_component || !owner)
		return;

	phos_gui_text_component *text = text_component;
	phos_gui_elem *elem = owner;

	text->owner = elem;

	text->font = NULL;
	text->font_size = PHOS_GUI_FONT_SIZE_DEFAULT;
	text->accept_letters = true;
	text->accept_nums = true;
	text->accept_specials = true;
	text->editable = true;
	text->max_len = PHOS_GUI_MAX_TEXT_LEN;
	text->len = 0;
	text->color = BLACK;
	text->pos = Vector2Zero();
	strcpy(text->str, "");

	vl_log(VL_SUCCESS, "Added text component to '%s'!\n", elem->ID);
}

static void init_placeholder_text_extension(void *placeholder_text_component, void *owner)
{
	if(!placeholder_text_component || !owner)
		return;

	phos_gui_placeholder_text_extension *placeholder_text = placeholder_text_component;
	phos_gui_elem *elem = owner;

	// see if owner has a base text component
	phos_gui_text_component *host_text = NULL;
	if(!(host_text = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT)))
	{
		vl_log(VL_ERROR, "To initialize a placeholder text extension, the element '%s' must also own a text component!\n", elem->ID);
		return;
	}

	placeholder_text->host = host_text;

	// match owners
	if(placeholder_text->host->owner != owner)
	{
		vl_log(VL_ERROR, "Mismatched owner pointer! Cannot initialize placeholder text extension!\n");
		return;
	}

	placeholder_text->color = GRAY;
	strcpy(placeholder_text->str, "");

	vl_log(VL_SUCCESS, "Added placeholder text extension to '%s'!\n", elem->ID);
}

int phos_gui_init()
{
	if(init)
	{
		vl_log(VL_ERROR, "Cannot re-initialize PhosphorusGUI!\n");
		return 0;
	}

	// simple registry arrays:
	init_arr(&all_ids, 0);
	init_arr(&elem_registry, 0);
	init_arr(&blueprint_registry, 0);
	init_arr(&gui_registry, 0);

	// resources:
	init_arr(&textures, 0);
	init_arr(&fonts, 0);

	// keyboard input:
	backspace_timer.key = KEY_BACKSPACE;
	left_arrow_timer.key = KEY_LEFT;
	right_arrow_timer.key = KEY_RIGHT;

	// register PhosphorusGUI component types
	pluto_cs_register(PHOS_GUI_COMPONENT_TEXT, sizeof(phos_gui_text_component), init_text_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT, sizeof(phos_gui_placeholder_text_extension), init_placeholder_text_extension, NULL);

	init = true;
	vl_log(VL_SUCCESS, "Initialized PhosphorusGUI!\n");

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
	dynas_free(&all_ids);
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
	vl_log(VL_SUCCESS, "PhosphorusGUI shut down!\n");
}

static void auto_gen_id(const char *ID, char *target, const char *prefix, size_t *generator)
{
	if(strcmp(ID, "!auto") == 0)
		sprintf(target, "!auto");
	else if(strcmp(ID, "auto") == 0)
		sprintf(target, "%s_#%zu", prefix, (*generator)++);
}
static int search_for_duplicate_id(const char *ID)
{
	int found = -1;
	dynas_find_str(&all_ids, ID, found);
	if(found != -1)
		vl_log(VL_ERROR, "Another object already uses this ID: '%s'!\n", ID);
	return found;
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
		auto_gen_id(new_gui->ID, new_gui->ID, "gui", &gui_auto_id);

		// see if a duplicate ID is found anywhere
		if(search_for_duplicate_id(new_gui->ID) != -1)
			return;

		// register the phos_gui
		dynas_add(&all_ids, new_gui->ID);
		dynas_add(&gui_registry, new_gui);

		vl_log(VL_SUCCESS, "Registered GUI with ID: '%s'!\n", new_gui->ID);
	}

	// reset 'goto' elem tracker
	curr_gui_elem_num = -1;

	// signal that the focus_on_start elem has to be resolved for this gui when updated for the first time
	resolve_focus_on_start_elem = true;
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
		if(strcmp(gui_registry.data[i]->ID, ID) == 0)
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

	Vector2 elem_size = elem->size;

	Vector2 container_center = { origin.x + size.x / 2.0f, origin.y + size.y / 2.0f };
	
	Vector2 elem_centered = { container_center.x - elem_size.x / 2.0f, container_center.y - elem_size.y / 2.0f };

	phos_gui_set_elem_bounds(elem, elem_centered.x, elem_centered.y, elem->size.x, elem->size.y);
}

void phos_gui_move_elem_xy(phos_gui_elem *elem, float x, float y)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot move a null UI element!\n");
		return;
	}

	// first move elem
	elem->pos.x += x;
	elem->pos.y += y;

	// then move elem's text component
	if(pluto_cs_check_component(elem, PHOS_GUI_COMPONENT_TEXT))
	{
		phos_gui_text_component *txt = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
		txt->pos.x += x;
		txt->pos.y += y;
	}

	// then move elem's child elements the same amount of pixels
	for(size_t i = 0; i < elem->num_children; ++i)
		phos_gui_move_elem_xy(elem->children[i], x, y);
}

static Vector2 get_proposed_align_pos(const phos_gui_elem *const reference_elem, phos_gui_alignment alignment, Vector2 target_object_size)
{
	Vector2 v = {0};

	// start at reference_rect origin
	Rectangle whole_rect = phos_gui_get_elem_rect(reference_elem);
	Rectangle content_rect = phos_gui_get_elem_content_area(reference_elem);
	v = phos_gui_get_rect_pos(whole_rect);

	// define bounds
	float outer_left = v.x;
	float outer_top = v.y;
	float outer_right = outer_left + whole_rect.width;
	float outer_bottom = outer_top + whole_rect.height;

	float inner_left = content_rect.x;
	float inner_top = content_rect.y;
	float inner_right = inner_left + content_rect.width;
	float inner_bottom = inner_top + content_rect.height;

	float center_x = v.x + (whole_rect.width - target_object_size.x) / 2.0f;
	float center_y = v.y + (whole_rect.height - target_object_size.y) / 2.0f;

	switch(alignment)
	{
		case PHOS_GUI_ALIGN_INNER_LEFT:
			v.x = inner_left;
			v.y = center_y;
			break;
		case PHOS_GUI_ALIGN_INNER_TOP:
			v.x = center_x;
			v.y = inner_top;
			break;
		case PHOS_GUI_ALIGN_INNER_RIGHT:
			v.x = inner_right - target_object_size.x;
			v.y = center_y;
			break;
		case PHOS_GUI_ALIGN_INNER_BOTTOM:
			v.x = center_x;
			v.y = inner_bottom - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_INNER_CENTER:
			v.x = center_x;
			v.y = center_y;
			break;
		case PHOS_GUI_ALIGN_INNER_TOP_LEFT:
			v.x = inner_left;
			v.y = inner_top;
			break;
		case PHOS_GUI_ALIGN_INNER_TOP_RIGHT:
			v.x = inner_right - target_object_size.x;
			v.y = inner_top;
			break;
		case PHOS_GUI_ALIGN_INNER_BOTTOM_LEFT:
			v.x = inner_left;
			v.y = inner_bottom - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_INNER_BOTTOM_RIGHT:
			v.x = inner_right - target_object_size.x;
			v.y = inner_bottom - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_LEFT:
			v.x = outer_left - target_object_size.x;
			v.y = center_y;
			break;
		case PHOS_GUI_ALIGN_TOP:
			v.x = center_x;
			v.y = outer_top - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_RIGHT:
			v.x = outer_right;
			v.y = center_y;
			break;
		case PHOS_GUI_ALIGN_BOTTOM:
			v.x = center_x;
			v.y = outer_bottom;
			break;
		case PHOS_GUI_ALIGN_TOP_LEFT:
			v.x = outer_left - target_object_size.x;
			v.y = outer_top - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_TOP_RIGHT:
			v.x = outer_right;
			v.y = outer_top - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_BOTTOM_LEFT:
			v.x = outer_left - target_object_size.x;
			v.y = outer_bottom;
			break;
		case PHOS_GUI_ALIGN_BOTTOM_RIGHT:
			v.x = outer_right;
			v.y = outer_bottom;
			break;
		default:
			vl_log(VL_ERROR, "Invalid alignment: %d!\n", alignment);
			break;
	}

	return v;
}

static void resize_single_elem_wh(phos_gui_elem *elem, float w, float h, phos_gui_opts opts)
{
	// first see what new elem size would be
	float new_w = elem->size.x + w;
	float new_h = elem->size.y + h;

	// see if size is now negative
	if(new_w <= 0.0f || new_h <= 0.0f)
	{
		vl_log(VL_ERROR, "Cannot resize this elem ('%s') anymore, its size cannot be <= 0.0f!\n", elem->ID);
		return;
	}

	phos_gui_text_component *elem_tx = NULL;
	Vector2 old_rel_center_pos = {0};

	if(opts & PHOS_GUI_OPTS_FIX_TEXT)
	{
		if(pluto_cs_check_component(elem, PHOS_GUI_COMPONENT_TEXT))
		{
			elem_tx = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);

			Rectangle old_content = phos_gui_get_elem_content_area(elem);

			if(old_content.width <= 0.0f || old_content.height <= 0.0f)
			{
				vl_log(VL_ERROR, "Invalid content size: %.2f, %.2f\n", old_content.width, old_content.height);
				return;
			}

			// get old relative center pos TODO use just main bounds or placeholder as well?
			Vector2 old_text_bounds;
			phos_gui_get_text_bounds_v(elem_tx, &old_text_bounds, NULL);

			Vector2 old_text_center = {
				elem_tx->pos.x + (old_text_bounds.x / 2.0f),
				elem_tx->pos.y + (old_text_bounds.y / 2.0f)
			};

			old_rel_center_pos.x = (old_text_center.x - old_content.x) / old_content.width;
			old_rel_center_pos.y = (old_text_center.y - old_content.y) / old_content.height;
		}
	}

	// apply new size
	elem->size = (Vector2) { new_w, new_h };

	if(elem_tx)
	{
		// resize font
		phos_gui_make_text_fit_elem(elem, elem_tx, elem_tx->str);
		
		// restore text's pos TODO same thing as TODO directly above
		Vector2 text_bounds;
		phos_gui_get_text_bounds_v(elem_tx, &text_bounds, NULL);
		elem_tx->pos = get_proposed_align_pos(elem, elem_tx->alignment, text_bounds);
	}
}
void phos_gui_resize_elem_wh(phos_gui_elem *elem, float w, float h, phos_gui_opts opts)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot resize a null UI element!\n");
		return;
	}

	// resize the given elem
	resize_single_elem_wh(elem, w, h, opts);

	// pass changes down to children
	if(elem->num_children > 0)
	{
		// then shrink/expand the elem's child elements the same amount of pixels (only if PHOS_GUI_PASS_DOWN is set)
		if(opts & PHOS_GUI_OPTS_PASS_DOWN_FIRST)
		{
			opts &= ~PHOS_GUI_OPTS_PASS_DOWN;

			// resize child
			resize_single_elem_wh(elem->children[0], w, h, opts);
		}
		else if(opts & PHOS_GUI_OPTS_PASS_DOWN)
		{
			for(size_t i = 0; i < elem->num_children; ++i)
				// resize child
				phos_gui_resize_elem_wh(elem->children[i], w, h, opts);
		}
	}
}

Vector2 phos_gui_get_elem_center(phos_gui_elem *elem)
{
	Vector2 v = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain center of a null UI element!\n");
		return v;
	}

	// add half the elem size
	v.x += elem->size.x / 2.0f;
	v.y += elem->size.y / 2.0f;

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
		vl_log(VL_ERROR, "Cannot obtain text center of a UI element ('%s') with no text component!\n", elem->ID);
		return v;
	}

	phos_gui_text_component *txt = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);

	if(!txt->font)
	{
		vl_log(VL_ERROR, "This element ('%s') has a null font, cannot obtain center with text!\n", elem->ID);
		return v;
	}

	// get center of elem
	v = phos_gui_get_elem_center(elem);

	// measure text
	Vector2 text_size, placeholder_text_size;
	phos_gui_get_text_bounds_v(txt, &text_size, &placeholder_text_size);

	// minus half the text's bounds
	v.x -= text_size.x / 2.0f;
	v.y -= text_size.y / 2.0f;

	return v;
}

Rectangle phos_gui_get_elem_rect(const phos_gui_elem *const elem)
{
	Rectangle r = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain visible bounds for a null UI element!\n");
		return r;
	}

	r.x = elem->pos.x;
	r.y = elem->pos.y;
	r.width = elem->size.x;
	r.height = elem->size.y;

	return r;
}
Rectangle phos_gui_get_elem_content_area(const phos_gui_elem *const elem)
{
	Rectangle r = phos_gui_get_elem_rect(elem);
	if(!elem)
		return r;

	r.x += elem->outline_thickness + elem->left_padding;
	r.y += elem->outline_thickness + elem->top_padding;
	r.width -= ((elem->outline_thickness * 2.0f) + elem->right_padding + elem->left_padding);
	r.height -= ((elem->outline_thickness * 2.0f) + elem->bottom_padding + elem->top_padding);

	return r;
}

void phos_gui_get_text_bounds(const phos_gui_text_component *const text_component, Rectangle *out_main_bounds, Rectangle *out_placeholder_bounds)
{
	if(!out_main_bounds && !out_placeholder_bounds)
		return;

	Rectangle main_bounds = {0};
	Rectangle placeholder_bounds = {0};

	// init each requested rectangle in invalid state
	if(out_main_bounds)
		*out_main_bounds = main_bounds;
	if(out_placeholder_bounds)
		*out_placeholder_bounds = placeholder_bounds;

	if(!text_component || !text_component->owner)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for null text component/UI element!\n");
		return;
	}

	// get text component owner
	const phos_gui_elem *const elem = text_component->owner;
	const char *main_str = text_component->str;

	// ensure font is valid
	if(!text_component->font || !IsFontValid(*text_component->font))
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for null/invalid font on element: '%s'!\n", elem->ID);
		return;
	}

	// get placeholder info if necessary
	phos_gui_placeholder_text_extension *placeholder_text = NULL;
	const char *placeholder_str = NULL;
	// if user requests placeholder text bounds
	if(out_placeholder_bounds)
	{
		// if it has the component, obtain its data
		if(pluto_cs_check_component(elem, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT))
		{
			placeholder_text = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT);
			placeholder_str = placeholder_text->str;
		}
		// otherwise, send a warning because the placeholder bounds can't be requested
		else
			vl_delay_log(VL_WARNING, 5.0f, "Cannot request placeholder bounds for element '%s' because it does not have the PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT component!\n", elem->ID);
	}

	// handle all main text data here:
	if(main_str && out_main_bounds)
	{
		if(strlen(main_str) == 0)
		{
			vl_delay_log(VL_WARNING, 5.0f, "Cannot obtain text bounds for empty text on element: '%s'!\n", elem->ID);
			return;
		}

		// width and height of the text
		Vector2 text_size = MeasureTextEx(*text_component->font, main_str, text_component->font_size, 0.0f);

		main_bounds.width = text_size.x;
		main_bounds.height = text_size.y;
		main_bounds.x = text_component->pos.x;
		main_bounds.y = text_component->pos.y;

		*out_main_bounds = main_bounds;
	}

	// handle all placeholder data here:
	if(placeholder_text && placeholder_str && out_placeholder_bounds)
	{
		if(strlen(placeholder_str) == 0)
		{
			vl_delay_log(VL_WARNING, 5.0f, "Cannot obtain text bounds for empty placeholder text on element: '%s'!\n", elem->ID);
			return;
		}

		// width and height of the text
		Vector2 text_size = MeasureTextEx(*text_component->font, placeholder_str, text_component->font_size, 0.0f);

		placeholder_bounds.width = text_size.x;
		placeholder_bounds.height = text_size.y;
		placeholder_bounds.x = text_component->pos.x;
		placeholder_bounds.y = text_component->pos.y;

		*out_placeholder_bounds = placeholder_bounds;
	}
}
void phos_gui_get_text_bounds_v(const phos_gui_text_component *const text_component, Vector2 *out_main_bounds, Vector2 *out_placeholder_bounds)
{
	Rectangle main_r, placeholder_r;
	phos_gui_get_text_bounds(text_component, &main_r, &placeholder_r);

	if(out_main_bounds)
		*out_main_bounds = phos_gui_get_rect_size(main_r);
	if(out_placeholder_bounds)
		*out_placeholder_bounds = phos_gui_get_rect_size(placeholder_r);
}

static void update_text_scrolling(phos_gui_text_component *text)
{
	if(strlen(text->str) == 0)
	{
		text->scroll = 0.0f;
		text->max_scroll = 0.0f;
		vl_delay_log(VL_WARNING, 5.0f, "Failed to update text scrolling. Text component must contain string data to calculate!\n");
		return;
	}

	// get owner of text component
	phos_gui_elem *e = text->owner;

	// first get visual bounds of text component on screen
	//Rectangle vis_bounds = phos_gui_get_elem_rect(e);
	Rectangle vis_bounds = phos_gui_get_elem_content_area(e);

	// get bounds of text
	Rectangle text_bounds;
	phos_gui_get_text_bounds(text, &text_bounds, NULL);

	// calculate the overflow
	float vis_left = vis_bounds.x;
	float vis_right = vis_bounds.x + vis_bounds.width;
	float vis_width = vis_right - text->pos.x;
	float overflow = text_bounds.width - vis_width;

	if(overflow > 0.0f)
		text->max_scroll = overflow;
	else
	{
		text->max_scroll = 0.0f;
		text->scroll = 0.0f;
	}

	// only handle caret logic for the editable string:
	if(strlen(text->str) > 0)
	{
		char buf[PHOS_GUI_MAX_TEXT_LEN + 1];
		memcpy(buf, text->str, text->cursor_pos);
		buf[text->cursor_pos] = '\0';

		float caret_x = MeasureTextEx(*text->font, buf, text->font_size, 0.0f).x;
		float caret_screen = text->pos.x + caret_x - text->scroll;

		// right-side check (include cursor width because the cursor takes up that many more pixels)
		if(caret_screen + CURSOR_WIDTH > vis_right)
			text->scroll += (caret_screen + CURSOR_WIDTH) - vis_right;

		// left-side check (don't include cursor width)
		if(caret_screen < vis_left)
			text->scroll -= vis_left - caret_screen;
	}

	text->scroll = Clamp(text->scroll, 0.0f, text->max_scroll);
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

	text->len = strlen(str);
	strcpy(text->str, str);
	text->max_len = PHOS_GUI_MAX_TEXT_LEN;
	text->color = color;
	text->font_size = font_size;
	text->editable = true;
	text->edited = false;
	text->cursor_pos = text->len;
	text->accept_letters = text->accept_nums = text->accept_specials = true;
}
void phos_gui_init_placeholder_text(phos_gui_placeholder_text_extension *placeholder_text, const char *str, Color color)
{
	if(!placeholder_text)
	{
		vl_log(VL_ERROR, "Cannot initialize null placeholder text!\n");
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

	strcpy(placeholder_text->str, str);
	placeholder_text->color = color;
}

Vector2 phos_gui_get_rect_pos(Rectangle r)
{
	return (Vector2) { r.x, r.y };
}
Vector2 phos_gui_get_rect_size(Rectangle r)
{
	return (Vector2) { r.width, r.height };
}

bool phos_gui_is_rect_valid(Rectangle r)
{
	return r.x != 0.0f && r.y != 0.0f && r.width != 0.0f && r.height != 0.0f;
}

void phos_gui_set_elem_pos(phos_gui_elem *elem, float x, float y)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set the position of a null UI element!\n");
		return;
	}

	// find difference between current pos and new pos
	float x_diff = x - elem->pos.x;
	float y_diff = y - elem->pos.y;

	// move based on difference
	phos_gui_move_elem_xy(elem, x_diff, y_diff);
}
void phos_gui_set_elem_size(phos_gui_elem *elem, float w, float h)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set the size of a null UI element!\n");
		return;
	}

	elem->size = (Vector2) { w, h };
}
void phos_gui_set_elem_bounds(phos_gui_elem *elem, float x, float y, float w, float h)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set bounds of null UI element!\n");
		return;
	}

	// save current elem pos
	Vector2 elem_pos = elem->pos;

	// save new elem bounds
	elem->pos = (Vector2) { x, y };
	elem->size = (Vector2) { w, h };

	// calculate difference between elem positions
	Vector2 diff = Vector2Subtract(elem->pos, elem_pos);

	// move text based on 'diff'
	if(pluto_cs_check_component(elem, PHOS_GUI_COMPONENT_TEXT))
	{
		phos_gui_text_component *txt = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
		txt->pos = Vector2Add(txt->pos, diff);
	}

	// then move elem's child elements the same amount of pixels
	for(size_t i = 0; i < elem->num_children; ++i)
		phos_gui_move_elem_xy(elem->children[i], diff.x, diff.y);
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
void phos_gui_fill_color_set(phos_gui_color_set *set, Color color)
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

static char *get_targeted_str(phos_gui_text_component *text, phos_gui_target_text_string target_str)
{
	switch(target_str)
	{
		case PHOS_GUI_TARGET_MAIN_TEXT:
			return text->str;
			break;
		case PHOS_GUI_TARGET_PLACEHOLDER_TEXT:
			// see if text component can be linked to a placeholder text extension:
			if(!text->owner)
			{
				vl_log(VL_ERROR, "Cannot use PHOS_GUI_TARGET_PLACEHOLDER_TEXT, the text component's owner is NULL!\n");
				return NULL;
			}

			phos_gui_placeholder_text_extension *placeholder_text = NULL;
			if(!(placeholder_text = pluto_cs_get_component(text->owner, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT)))
			{
				vl_log(VL_ERROR, "Cannot use PHOS_GUI_TARGET_PLACEHOLDER_TEXT, the element '%s' does not own a phos_gui_placeholder_text_extension!\n", text->owner->ID);
				return NULL;
			}

			return placeholder_text->str;
			break;
		default:
			vl_log(VL_ERROR, "Invalid target string: %d!\n", target_str);
			return NULL;
	}
}
void phos_gui_set_text_contents(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, const char *new_contents)
{
	if(!text_component)
	{
		vl_log(VL_ERROR, "Cannot set string contents of a null text_component!\n");
		return;
	}
	if(!new_contents)
	{
		vl_log(VL_ERROR, "String is null, cannot set UI element's text contents!\n");
		return;
	}

	char *dest = get_targeted_str(text_component, target_str);
	if(!dest)
		return;
	strcpy(dest, new_contents);
	text_component->cursor_pos = text_component->len = strlen(dest);
}

Vector2 phos_gui_align_elem_text(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, phos_gui_alignment alignment)
{
	Vector2 v = {0};

	if(!text_component || !text_component->owner)
	{
		vl_log(VL_ERROR, "Cannot align text on null text component/UI element!\n");
		return v;
	}

	// ensure alignment is a valid one
	if(alignment < PHOS_GUI_ALIGN_INNER_LEFT || alignment > PHOS_GUI_ALIGN_INNER_BOTTOM_RIGHT)
	{
		vl_log(VL_ERROR, "Cannot align an element's text component with an alignment of %d! The alignment must be a PHOS_GUI_ALIGN_INNER... alignment! Defaulting to PHOS_GUI_ALIGN_INNER_CENTER!\n", alignment);
		alignment = PHOS_GUI_ALIGN_INNER_CENTER;
	}

	// obtain placeholder text data
	phos_gui_placeholder_text_extension *placeholder_text = NULL;
	if(pluto_cs_check_component(text_component->owner, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT))
		placeholder_text = pluto_cs_get_component(text_component->owner, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT);

	Rectangle elem_text_size;
	switch(target_str)
	{
		case PHOS_GUI_TARGET_MAIN_TEXT:
			phos_gui_get_text_bounds(text_component, &elem_text_size, NULL);
			break;
		case PHOS_GUI_TARGET_PLACEHOLDER_TEXT:
			phos_gui_get_text_bounds(text_component, NULL, &elem_text_size);
			break;
		default:
			vl_log(VL_ERROR, "Invalid target string: %d!\n", target_str);
			return v;
	}

	v = get_proposed_align_pos(text_component->owner, alignment, (Vector2) { elem_text_size.width, elem_text_size.height });
	text_component->pos = v;
	text_component->alignment = alignment;
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

	target_elem->alignment = alignment;
	v = get_proposed_align_pos(reference_elem, target_elem->alignment, phos_gui_get_rect_size(phos_gui_get_elem_rect(target_elem)));
	phos_gui_set_elem_pos(target_elem, v.x, v.y);

	return v;
}

static bool phos_gui_elem_in_bounds(phos_gui_elem *elem, Vector2 origin, Vector2 size)
{
	// get elem rect
	Rectangle r = phos_gui_get_elem_rect(elem);

	// see if either rect is out of bounds (origin + size)
	if(r.x < origin.x || r.x + r.width > origin.x + size.x ||
			r.y < origin.y || r.y + r.height > origin.y + size.y)
	{
		vl_log(VL_ERROR, "Element '%s' is out of bounds (%.2f, %.2f, %.2f, %.2f)!\n", elem->ID, origin.x, origin.y, size.x, size.y);
		return false;
	}

	return true;
}
static bool phos_gui_check_elem_collision(phos_gui_elem *elem1, phos_gui_elem *elem2)
{
	// get elem rects
	Rectangle r1 = phos_gui_get_elem_rect(elem1);
	Rectangle r2 = phos_gui_get_elem_rect(elem2);

	// check collision between each elem
	if(CheckCollisionRecs(r1, r2))
	{
		vl_log(VL_ERROR, "Elements '%s' and '%s' are colliding!\n", elem1->ID, elem2->ID);
		return true;
	}

	return false;
}
Vector2 phos_gui_align_elem_with_window(phos_gui_elem *target_elem, phos_gui_alignment alignment)
{
	Vector2 v = {0};

	if(!target_elem)
	{
		vl_log(VL_ERROR, "Cannot align null target element!\n");
		return v;
	}
	if(!curr_gui)
	{
		vl_log(VL_ERROR, "To align an element with the window, a phos_gui must be set! Use phos_gui_set_gui(...)!\n");
		return v;
	}

	// ensure elem is in bounds
	if(!phos_gui_elem_in_bounds(target_elem, PHOS_GUI_WIN_ORIGIN, PHOS_GUI_WIN_SIZE))
	{
		vl_log(VL_ERROR, "Failed to align '%s' with window, it must be in the window's bounds!\n", target_elem->ID);
		return v;
	}

	// ensure elem is not colliding with any other elems in the curr gui
	for(size_t i = 0; i < curr_gui->num_elems; ++i)
	{
		// get elem at i
		phos_gui_elem *e = curr_gui->elems[i];

		// do not collide with self
		if(e == target_elem)
			continue;

		if(phos_gui_check_elem_collision(e, target_elem))
		{
			vl_log(VL_ERROR, "Failed to align '%s' with the window, it is colliding with another element!\n", target_elem->ID);
			return v;
		}
	}

	// if alignment is not an INNER alignment, cannot continue
	if(alignment < PHOS_GUI_ALIGN_INNER_LEFT || alignment > PHOS_GUI_ALIGN_INNER_BOTTOM_RIGHT)
	{
		vl_log(VL_ERROR, "Invalid alignment: %d! When aligning an element with the window, the alignment must be a PHOS_GUI_ALIGN_INNER... alignment! Defaulting to PHOS_GUI_ALIGN_INNER_TOP_LEFT!\n", alignment);
		alignment = PHOS_GUI_ALIGN_TOP_LEFT;
	}

	target_elem->alignment = alignment;
	
	// create temp elem representing the window
	phos_gui_elem temp = {0};
	phos_gui_set_elem_bounds(&temp, 0, 0, GetScreenWidth(), GetScreenHeight());
	v = get_proposed_align_pos(&temp, target_elem->alignment, temp.size);
	phos_gui_set_elem_pos(target_elem, v.x, v.y);

	return v;
}
void phos_gui_fill_window_with_elem(phos_gui_elem *elem)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot fill window with a null UI element!\n");
		return;
	}

	elem->alignment = PHOS_GUI_ALIGN_INNER_TOP_LEFT;
	phos_gui_set_elem_bounds(elem, 0.0f, 0.0f, GetScreenWidth(), GetScreenHeight());
}

static float get_largest_possible_font_size(const phos_gui_elem *const elem, const phos_gui_text_component *const text_component, const char *text_component_target_str)
{
	// get content area
	Vector2 size = phos_gui_get_rect_size(phos_gui_get_elem_content_area(elem));

	// first off, start off with a very large font size
	float font_size = PHOS_GUI_FONT_SIZE_XHUGE;

	size.x -= TEXT_PADDING * 2.0f;
	size.y -= TEXT_PADDING * 2.0f;

	// measure initial text bounds
	Vector2 text_bounds = MeasureTextEx(*text_component->font, text_component_target_str, font_size, 0.0f);

	// while the text takes up more space than the inner bounds, font size automatically has to shrink
	while(font_size >= 1.0f && (text_bounds.x >= size.x || text_bounds.y >= size.y))
	{
		// go to next font size
		font_size -= 2.0f;

		// re-measure text using current font size
		text_bounds = MeasureTextEx(*text_component->font, text_component_target_str, font_size, 0.0f);
	}

	vl_log(VL_INFO, "Largest possible font size for '%s' is %.2f.\n", elem->ID, font_size);

	return font_size;
}
void phos_gui_clamp_elem_to_text(phos_gui_elem *elem, const phos_gui_text_component *const text_component, const char *text_component_target_str)
{
	if(!elem || !text_component || !text_component->font || !text_component_target_str || strlen(text_component_target_str) <= 0)
	{
		vl_log(VL_ERROR, "To clamp the given element to the text component, 'elem,' 'text_component,' and 'text_component_target_str' can't be NULL/invalid!\n");
		return;
	}

	// measure text bounds
	Vector2 text_bounds = MeasureTextEx(*text_component->font, text_component_target_str, text_component->font_size, 0.0f);

	// match elem bounds to text bounds
	phos_gui_set_elem_size(elem, text_bounds.x, text_bounds.y);
}

void phos_gui_make_text_fit_elem(const phos_gui_elem *const elem, phos_gui_text_component *text_component, const char *text_component_target_str)
{
	if(!elem || !text_component || !text_component->font || !text_component_target_str || strlen(text_component_target_str) <= 0)
	{
		vl_log(VL_ERROR, "To make the given text component fit the element, 'elem,' 'text_component,' and 'text_component_target_str' can't be NULL/invalid!\n");
		return;
	}

	// use largest possible font size
	text_component->font_size = get_largest_possible_font_size(elem, text_component, text_component_target_str);
}
void phos_gui_make_elem_fit_text(phos_gui_elem *elem, const phos_gui_text_component *const text_component, const char *text_component_target_str)
{
	if(!elem || !text_component || !text_component_target_str || strlen(text_component_target_str) <= 0)
	{
		vl_log(VL_ERROR, "To make the given element fit the text component, 'elem,' 'text_component,' and 'text_component_target_str' can't be NULL/invalid!\n");
		return;
	}

	// get content area of elem
	Rectangle bounds = phos_gui_get_elem_content_area(elem);

	// measure text bounds
	Vector2 text_bounds = MeasureTextEx(*text_component->font, text_component_target_str, text_component->font_size, 0.0f);

	// expand element if necessary
	if(text_bounds.x > bounds.width)
	{
		// find diff in widths
		float diff_w = text_bounds.x - bounds.height;

		// expand by that much
		phos_gui_resize_elem_wh(elem, diff_w, 0.0f, PHOS_GUI_OPTS_NONE);
	}
	if(text_bounds.y > bounds.height)
	{
		// find diff in heights
		float diff_h = text_bounds.y - bounds.height;

		// expand by that much
		phos_gui_resize_elem_wh(elem, 0.0f, diff_h, PHOS_GUI_OPTS_NONE);
	}

	// if element has a parent, make sure it can contain the text too
	if(elem->parent)
		phos_gui_make_elem_fit_text(elem->parent, text_component, text_component_target_str);
}

void phos_gui_init_elem(phos_gui_elem *elem, phos_gui_elem_type type, phos_gui_elem_render_mode render_mode, float x, float y, float w, float h)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot setup null UI element!\n");
		return;
	}

	elem->type = type;
	elem->render_mode = render_mode;
	elem->pos = (Vector2) { x, y };
	elem->size = (Vector2) { w, h };
	elem->left_padding = elem->top_padding = elem->right_padding = elem->bottom_padding = 0.0f;
}

static int register_elem(phos_gui_elem *elem)
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
	if(strlen(elem->ID) == 0)
	{
		vl_log(VL_ERROR, "Cannot register element with empty ID!\n");
		return 0;
	}

	// auto-generate if necessary
	auto_gen_id(elem->ID, elem->ID, "elem", &elem_auto_id);

	// assert no duplicate IDs
	if(search_for_duplicate_id(elem->ID) != -1)
		return 0;

	// find duplicate pointers and IDs:
	for(size_t i = 0; i < elem_registry.size; ++i)
	{
		phos_gui_elem *e = elem_registry.data[i];

		if(e == elem)
		{
			vl_log(VL_ERROR, "Duplicate UI element pointer found while registering element: %p!\n", e);
			return 0;
		}
		if(strcmp(e->ID, elem->ID) == 0)
		{
			vl_log(VL_ERROR, "Duplicate ID found: '%s'!\n", e->ID);
			return 0;
		}
	}

	// no duplicate, the elem can be registered
	dynas_add(&all_ids, elem->ID);
	dynas_add(&elem_registry, elem);

	vl_log(VL_SUCCESS, "Registered element with ID: '%s'!\n", elem->ID);

	return 1;
}

int phos_gui_add_elem(phos_gui *gui, phos_gui_elem *elem)
{
	if(!gui || !elem)
	{
		vl_log(VL_ERROR, "Failed to add UI element. Make sure 'gui' and 'elem' are not NULL!\n");
		return 0;
	}

	// try to register 'elem'
	if(register_elem(elem) == 0)
	{
		vl_log(VL_ERROR, "Failed to register the given UI element!\n");
		return 0;
	}

	// add 'elem' to 'curr_elems' in the given phos_gui
	if(gui->num_elems >= PHOS_GUI_MAX_ELEMS)
	{
		vl_log(VL_WARNING, "No more elements can be added to this phos_gui: %p\n", gui);
		return 0;
	}
	gui->elems[gui->num_elems++] = elem;

	// ensure elem remembers its GUI
	elem->gui = gui;

	vl_log(VL_SUCCESS, "Added element '%s' to GUI '%s'!\n", elem->ID, gui->ID);

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
	strcpy(elem->ID, ID);

	// add normally
	return phos_gui_add_elem(gui, elem);
}
int phos_gui_add_all_elems(phos_gui *gui, phos_gui_elem *elem)
{
	if(!gui || !elem)
	{
		vl_log(VL_ERROR, "Failed to add all UI elements. Make sure 'gui' and 'elem' are not NULL!\n");
		return 0;
	}

	// add the elem first
	phos_gui_add_elem(gui, elem);

	// add children next
	for(size_t i = 0; i < elem->num_children; ++i)
		phos_gui_add_elem(gui, elem->children[i]);

	return 1;
}
int phos_gui_remove_elem_from_gui(phos_gui *gui, phos_gui_elem *elem)
{
	if(!gui)
	{
		vl_log(VL_ERROR, "Cannot remove a UI element from a null GUI!\n");
		return 0;
	}
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot remove a null UI element from a GUI!\n");
		return 0;
	}

	// find elem in the GUI array
	for(size_t i = 0; i < gui->num_elems; ++i)
	{
		// get elem at i
		phos_gui_elem *e = gui->elems[i];

		// compare pointers and IDs
		if(e == elem && strcmp(e->ID, elem->ID) == 0)
		{
			// a matching elem was found:

			// first, see if it's the last element in the array
			if(i >= PHOS_GUI_MAX_ELEMS)
			{
				// just decrement num_elems
				gui->num_elems--;

				vl_log(VL_SUCCESS, "Removed element '%s' from GUI '%s'!\n", e->ID, gui->ID);
				return 1;
			}

			// otherwise, shift all elements to the left and decrement num_elems:
			memmove(gui->elems[i], gui->elems[i + 1], (gui->num_elems - i) - 1);
			gui->num_elems--;

			vl_log(VL_SUCCESS, "Removed element '%s' from GUI '%s'!\n", e->ID, gui->ID);
			return 1;
		}
	}

	// no match found
	vl_log(VL_ERROR, "Failed to remove this element ('%s') from the given phos_gui ('%s')!\n", elem->ID, gui->ID);
	return 0;
}
int phos_gui_remove_elem_from_gui_id(phos_gui *gui, const char *ID)
{
	return phos_gui_remove_elem_from_gui(gui, phos_gui_get_elem(ID));
}
int phos_gui_add_child(phos_gui_elem *parent, phos_gui_elem *child)
{
	if(!parent || !child)
	{
		vl_log(VL_ERROR, "Failed to add element to parent! Make sure 'parent' and 'elem' are not NULL!\n");
		return 0;
	}

	// before anything, make sure parent can add another child
	if(parent->num_children >= PHOS_GUI_MAX_CHILDREN)
	{
		vl_log(VL_ERROR, "This parent cannot contain any more child elements: '%s'!\n", parent->ID);
		return 0;
	}

	// see if elem is in parent bounds
	if(!phos_gui_elem_in_bounds(child, parent->pos, parent->size))
	{
		vl_log(VL_ERROR, "Element '%s' is not in the given parent's ('%s') bounds!\n", child->ID, parent->ID);
		return 0;
	}

	// see if element would cover another element
	for(size_t i = 0; i < parent->num_children; ++i)
	{
		// get elem at i
		phos_gui_elem *ch = parent->children[i];

		// keep in bounds
		if(!phos_gui_elem_in_bounds(ch, parent->pos, parent->size))
		{
			vl_log(VL_ERROR, "Child element '%s' is not in the given parent's ('%s') bounds!\n", ch->ID, parent->ID);
			return 0;
		}

		/* size-bounds are the content area of the parent
		   make sure elem's do not self-collide */
		if(child == ch)
			continue;

		if(phos_gui_check_elem_collision(child, ch))
			return 0;
	}

	// add elem to parent
	parent->children[parent->num_children++] = child;

	// the element's parent becomes the parent
	child->parent = parent;

	vl_log(VL_SUCCESS, "Element '%s' added to parent element '%s'!\n", child->ID, parent->ID);

	return 1;
}
int phos_gui_add_child_id(phos_gui_elem *parent, const char *ID)
{
	return phos_gui_add_child(parent, phos_gui_get_elem(ID));
}
int phos_gui_remove_child(phos_gui_elem *parent, phos_gui_elem *child)
{
	if(!parent)
	{
		vl_log(VL_ERROR, "Cannot remove a UI element from a null container!\n");
		return 0;
	}
	if(!child)
	{
		vl_log(VL_ERROR,"Cannot remove a null UI element from a container!\n");
		return 0;
	}

	// find elem in container's children array
	for(size_t i = 0; i < parent->num_children; ++i)
	{
		// get elem at i
		phos_gui_elem *ch = parent->children[i];

		// compare pointers and IDs
		if(child == ch && strcmp(child->ID, child->ID) == 0)
		{
			// matching elem was found:

			// check for last element in the array
			if(i >= PHOS_GUI_MAX_CHILDREN)
			{
				// just decrement num_children
				parent->num_children--;

				vl_log(VL_SUCCESS, "Removed element '%s' from container '%s'!\n", child->ID, parent->ID);
				return 1;
			}

			// otherwise, shift all elements to the left and decrement num_children:
			memmove(parent->children[i], parent->children[i + 1], (parent->num_children - i) - 1);
			parent->num_children--;

			vl_log(VL_SUCCESS, "Removed element '%s' from parent '%s'!\n", child->ID, parent->ID);
			return 1;
		}
	}

	// no match found
	vl_log(VL_ERROR, "Failed to remove this child ('%s') from the given parent ('%s')!\n", child->ID, parent->ID);
	return 0;
}
int phos_gui_remove_child_id(phos_gui_elem *parent, const char *ID)
{
	return phos_gui_remove_child(parent, phos_gui_get_elem(ID));
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
			if(strcmp(e->ID, ID) == 0)
				return e;
	}

	vl_log(VL_ERROR, "No UI element found with the ID: '%s'\n", ID);
	return NULL;
}
static int create_blueprint(phos_gui_elem *elem, const char *ID, blueprint *bp)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot clone null UI element!\n");
		return 0;
	}
	if(!ID || strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Invalid blueprint ID!\n");
		return 0;
	}

	blueprint new_bp = {0};

	// auto-generate ID if necessary
	auto_gen_id(ID, new_bp.ID, "blueprint", &blueprint_auto_id);

	// assert no duplicate IDs
	if(search_for_duplicate_id(ID) != -1)
	{
		vl_log(VL_ERROR, "Failed to create blueprint for element: '%s'!\n", elem->ID);
		return 0;
	}

	// see if a duplicate blueprint exists
	for(size_t i = 0; i < blueprint_registry.size; ++i)
	{
		blueprint *bp = &blueprint_registry.data[i];

		if(bp->elem == elem)
		{
			vl_log(VL_ERROR, "Duplicate UI element pointer found while creating blueprint: %p!\n", elem);
			return 0;
		}
		if(strcmp(bp->ID, ID) == 0)
		{
			vl_log(VL_ERROR, "Duplicate blueprint ID found: '%s'!\n", bp->ID);
			return 0;
		}
	}

	strcpy(new_bp.ID, ID);
	new_bp.elem = elem;

	// no duplicate, blueprint can be created and saved
	dynas_add(&blueprint_registry, new_bp);
	dynas_add(&all_ids, blueprint_registry.data[blueprint_registry.size - 1].ID);

	vl_log(VL_SUCCESS, "Registered blueprint with ID: '%s'!\n", ID);

	if(bp)
		*bp = new_bp;

	return 1;
}
void phos_gui_clone_single_elem(phos_gui_elem *elem, const char *ID)
{
	blueprint blueprint = {0};

	if(create_blueprint(elem, ID, &blueprint) == 1)
		blueprint.clone_children = false;
}
void phos_gui_clone_full_elem(phos_gui_elem *elem, const char *ID)
{
	blueprint blueprint = {0};

	if(create_blueprint(elem, ID, &blueprint) == 1)
		blueprint.clone_children = true;
}
void phos_gui_init_clone(phos_gui_elem *target_elem, const char *ID)
{
	if(!ID || strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Cannot create new instance of a blueprint with invalid ID!\n");
		return;
	}

	// obtain blueprint
	blueprint *bp = NULL;
	for(size_t i = 0; i < blueprint_registry.size; ++i)
	{
		blueprint *saved_bp = &blueprint_registry.data[i];

		// find matching blueprint
		if(strcmp(saved_bp->ID, ID) == 0)
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

	// start creating new instance (with an auto-generated ID)
	*target_elem = *bp->elem; // 'num_children' value is copied into 'target_elem'
	auto_gen_id("auto", target_elem->ID, "elem", &elem_auto_id);

	// assert no duplicate IDs
	if(search_for_duplicate_id(target_elem->ID) != -1)
	{
		vl_log(VL_ERROR, "Failed to initialize a cloned element! A duplicate ID was found: '%s'!\n", target_elem->ID);
		return;
	}

	// clone children from bp->elem onto target_elem
	if(bp->clone_children)
	{
		for(size_t i = 0; i < bp->elem->num_children; ++i)
		{
			// child at i
			phos_gui_elem *child = bp->elem->children[i];

			// clone the child
			phos_gui_elem child_clone = *child;
			strcpy(child_clone.ID, "auto");
			auto_gen_id("auto", child_clone.ID, "elem", &elem_auto_id);

			// clone elements from child onto child_clone
			if(pluto_cs_clone_all_components(child, &child_clone) == 0)
				vl_log(VL_ERROR, "Failed to clone child components!\n");

			// add child to target_elem
			phos_gui_add_child(target_elem, &child_clone);
		}
	}
	else
		// when the children should not be cloned, reset the element's child count to 0
		target_elem->num_children = 0;

	// clone elements from bp->elem onto target_elem
	if(pluto_cs_clone_all_components(bp->elem, target_elem) == 0)
		vl_log(VL_ERROR, "Failed to initialize a cloned element! Cloning components failed! Source element: '%s', target element: '%s'", bp->elem->ID, target_elem->ID);
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

static void move_cursor_left(phos_gui_text_component *t)
{
	if(t->cursor_pos > 0)
	{
		t->cursor_pos--;
		update_text_scrolling(t);
	}
}
static void move_cursor_right(phos_gui_text_component *t)
{
	if(t->cursor_pos < t->len)
	{
		t->cursor_pos++;
		update_text_scrolling(t);
	}
}
static void backspace(phos_gui_text_component *t)
{
	if(t->len > 0)
	{
		t->str[--t->len] = '\0';
		t->cursor_pos--;
		t->edited = true;
	}
}
static void update_key_timer(phos_gui_text_component *t, float dt, key_timer *kt, void (*action) (phos_gui_text_component*))
{
	if(IsKeyDown(kt->key))
	{
		if(!kt->active)
		{
			action(t);
			kt->timer = KEY_REPEAT_DELAY;
		}
		else
		{
			kt->timer -= dt;

			if(kt->timer <= 0.0f)
			{
				action(t);
				kt->timer = KEY_REPEAT_INTERVAL;
			}
		}

		kt->active = true;
	}
	else
		kt->active = false;
}

static void goto_next_elem()
{
	if(!curr_gui)
	{
		vl_log(VL_WARNING, "No current GUI, cannot go to next element!\n");
		return;
	}

	get_next_elem:
	
	// curr elem loses focus
	if(curr_gui_elem_num >= 0)
		curr_gui->elems[curr_gui_elem_num]->has_focus = false;

	// go to next elem or loop back
	curr_gui_elem_num++;
	if(curr_gui_elem_num >= curr_gui->num_elems)
		curr_gui_elem_num = 0;

	// get new elem
	phos_gui_elem *elem = curr_gui->elems[curr_gui_elem_num];

	// if the elem is unreachable or disabled, skip it and repeat process
	if(elem->unreachable || elem->disabled)
		goto get_next_elem;

	// mark that elem as focused (if a valid elem type)
	if(elem->type != PHOS_GUI_TYPE_INVALID &&
			elem->type != PHOS_GUI_TYPE_BASIC &&
			!elem->disabled)
	{
		elem->has_focus = true;
	}
}
static void goto_prev_elem()
{
	if(!curr_gui)
	{
		vl_log(VL_WARNING, "No current GUI, cannot go to previous element!\n");
		return;
	}

	get_prev_elem:

	// curr elem loses focus
	if(curr_gui_elem_num >= 0)
		curr_gui->elems[curr_gui_elem_num]->has_focus = false;

	// loop back, or go to previous elem
	if(curr_gui_elem_num <= 0)
		curr_gui_elem_num = curr_gui->num_elems - 1;
	else
		curr_gui_elem_num--;

	// get new elem
	phos_gui_elem *elem = curr_gui->elems[curr_gui_elem_num];

	// if the elem is unreachable or disabled, skip it and repeat process
	if(elem->unreachable || elem->disabled)
		goto get_prev_elem;

	// mark that elem as focused (if a valid elem type)
	if(elem->type != PHOS_GUI_TYPE_INVALID &&
			elem->type != PHOS_GUI_TYPE_BASIC)
	{
		elem->has_focus = true;
	}
}
static bool travel_elems()
{
	// goto prev or next elem
	bool tab_pressed = IsKeyPressed(KEY_TAB);
	if(IsKeyDown(KEY_LEFT_SHIFT) && tab_pressed)
	{
		goto_prev_elem();
		return true;
	}
	else if(tab_pressed)
	{
		goto_next_elem();
		return true;
	}

	return false;
}

static void update_elem(phos_gui_elem *e, float dt)
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
	bool enter_clicked = IsKeyPressed(KEY_ENTER);
	bool enter_down = IsKeyDown(KEY_ENTER);

	bool no_focus = !e->has_focus;

	// see if mouse over element:
	float elem_x = e->pos.x;
	float elem_y = e->pos.y;
	float elem_w = e->size.x;
	float elem_h = e->size.y;
	if(mouse_pos.x > elem_x && mouse_pos.x < elem_x + elem_w && mouse_pos.y > elem_y && mouse_pos.y < elem_y + elem_h)
	{
		e->hovered = true;

		if(mouse_clicked)
		{
			e->pressed = false;
			e->clicked = true;
			e->has_focus = true;
		}
		else if(mouse_down)
		{
			e->pressed = true;
			e->clicked = false;
			e->has_focus = true;
		}
		else
		{
			e->clicked = false;
			e->pressed = false;
		}
	}
	else if(mouse_clicked || mouse_down)
	{
		e->has_focus = false;
		e->hovered = false;
		e->clicked = false;
		e->pressed = false;
	}
	else
	{
		// if no mouse input detected, check to see if user reached the elem and is using keyboard input instead
		if(e == curr_gui->elems[curr_gui_elem_num])
		{
			if(enter_clicked)
			{
				e->pressed = true;
				e->clicked = true;
			}
			else if(enter_down)
			{
				e->pressed = true;
				e->clicked = false;
			}
			else
			{
				// when no mouse input or keyboard input detected, always reset input state
				e->clicked = false;
				e->pressed = false;
			}
		}
		else // when no mouse input and the element is not reachable, reset input state
		{
			e->clicked = false;
			e->pressed = false;
		}
	}

	// if elem now has focus, it gained focus
	if(no_focus && e->has_focus)
		e->gained_focus = true;
	else
		e->gained_focus = false;

	// check type of element:
	if(e->type == PHOS_GUI_TYPE_TEXT_FIELD)
	{
		// get text component
		if(!pluto_cs_check_component(e, PHOS_GUI_COMPONENT_TEXT))
			vl_delay_log(VL_WARNING, 5.0f, "Element '%s' is a text field, but is missing a text component!\n", e->ID);
		else
		{
			phos_gui_text_component *text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_TEXT);

			// reset 'edited' field
			text->edited = false;

			// only type into text field if it has focus
			if(e->has_focus)
			{
				// collect key
				int k = GetKeyPressed();
				text->key_typed = k;

				// collect every char typed:
				char c = GetCharPressed();

				while(c > 0)
				{
					// get type of c
					bool letter = isalpha(c);
					bool num = isdigit(c);
					bool special = !letter & !num;

					// see if text field accepts this type of char
					if(letter && !text->accept_letters)
					{
						c = GetCharPressed();
						continue;
					}
					if(num && !text->accept_nums)
					{
						c = GetCharPressed();
						continue;
					}
					// let ' ' through the special char check
					if(special && !text->accept_specials && c != ' ')
					{
						c = GetCharPressed();
						continue;
					}

					// assign char typed
					text->char_typed = c;

					// insert char into string at cursor pos (if possible)
					if(text->len + 1 <= text->max_len && text->len + 1 < PHOS_GUI_MAX_TEXT_LEN)
					{
						// first, move all chars at cursor pos one slot over to the right
						memmove(text->str + text->cursor_pos + 1, text->str + text->cursor_pos, text->len - text->cursor_pos + 1);

						// insert char and move to next cursor pos
						text->str[text->cursor_pos++] = c;

						// increase string length by one
						text->len++;

						text->edited = true;
					}

					// get next char pressed
					c = GetCharPressed();
				}

				update_key_timer(text, dt, &backspace_timer, backspace);
				update_key_timer(text, dt, &left_arrow_timer, move_cursor_left);
				update_key_timer(text, dt, &right_arrow_timer, move_cursor_right);
			}
			else
				text->edited = false;

			// update text scrolling if edited
			if(text->edited)
				update_text_scrolling(text);
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

	// 'tab' and 'shift+tab' to travel between elems
	travel_elems();

	// resolve focus_on_start elem if necessary
	if(resolve_focus_on_start_elem)
	{
		for(size_t i = 0; i < curr_gui->num_elems; ++i)
		{
			// get elem at i
			phos_gui_elem *elem = curr_gui->elems[i];

			if(elem->focus_on_start)
			{
				// verify valid elem type
				if(elem->type == PHOS_GUI_TYPE_INVALID || elem->type == PHOS_GUI_TYPE_BASIC || elem->disabled)
					vl_log(VL_ERROR, "Cannot focus this element '%s' on start because it has an invalid type!\n", elem->ID);
				else
				{
					// use same index
					curr_gui_elem_num = i;
					// give elem focus
					elem->has_focus = true;
					break;
				}
			}
		}

		// signal that the focus_on_start elem was resolved
		resolve_focus_on_start_elem = false;
	}

	// update elems:
	for(size_t i = 0; i < curr_gui->num_elems; ++i)
	{
		// get elem at i
		phos_gui_elem *elem = curr_gui->elems[i];

		// skip disabled elemsn
		if(elem->disabled)
			continue;
		// cannot update invalid elements
		else if(elem->type == PHOS_GUI_TYPE_INVALID)
		{
			vl_delay_log(VL_ERROR, 1.0f, "Element has invalid type: '%s'!\n", elem->ID);
			continue;
		}
		// and skip basic elems
		else if(elem->type == PHOS_GUI_TYPE_BASIC)
			continue;

		// update the element
		update_elem(elem, dt);

		// if elem gained focus, make this elem the current one
		if(elem->gained_focus)
			curr_gui_elem_num = i;
	}

	// if no elems to update, warn user
	if(curr_gui->num_elems == 0)
		vl_delay_log(VL_WARNING, 10.0f, "The current phos_gui ('%s') has no elements! Skipping updating and rendering!\n", curr_gui->ID);
}

static void render_ellipse_outline(Vector2 pos, float rx, float ry, float line_thickness, Color color)
{
	for(int i = 0; i < line_thickness + 1; ++i)
		DrawEllipseLines(pos.x + rx, pos.y + ry, rx - i * 0.7f, ry - i * 0.7f, color);
}

static Color resolve_elem_color(const phos_gui_elem *const e, const phos_gui_color_set *const colors)
{
	// if elem's primary colors are given and the elem is disabled, always use the disabled_color
	if(e->disabled && colors == &e->primary_colors)
		return e->disabled_color;
	// if elem's outline colors are given and the elem is disabled, always use the outline's normal color
	if(e->disabled && colors == &e->outline_colors)
		return e->outline_colors.normal_color;

	// get the normal color of elem
	Color color = colors->normal_color;

	// 1. check to see if mouse button held down over element
	if(e->pressed)
		color = colors->press_color;
	// 2. check to see if they only have the mouse over the element
	else if(e->hovered)
		color = colors->hover_color;
	// 3. check to see if the elem has focus
	else if(e->has_focus)
		color = colors->focus_color;

	return color;
}

static void render_elem(const phos_gui_elem *const e)
{
	// get color of elem
	Color primary_color = resolve_elem_color(e, &e->primary_colors);

	// create elem rects:
	Rectangle vis_bounds = phos_gui_get_elem_rect(e);
	Rectangle content_bounds = phos_gui_get_elem_content_area(e);

	// create elem ellipse info:
	float e_rx = e->size.x / 2.0f;
	float e_ry = e->size.y / 2.0f;

	// draw elem bg if it is valid
	if(e->texture && IsTextureValid(*e->texture))
	{
		Rectangle src = { 0, 0, e->texture->width, e->texture->height };
		DrawTexturePro(*e->texture, src, vis_bounds, PHOS_GUI_WIN_ORIGIN, 0.0f, primary_color);
	}
	// else just draw base shape (if set)
	else if(e->render_mode == PHOS_GUI_RENDER_FILL_OUTLINE || e->render_mode == PHOS_GUI_RENDER_FILL)
	{
		switch(e->shape)
		{
			case PHOS_GUI_SHAPE_RECT:
				DrawRectanglePro(vis_bounds, PHOS_GUI_WIN_ORIGIN, 0.0f, primary_color);
				break;
			case PHOS_GUI_SHAPE_ELLIPSE:
				DrawEllipse(vis_bounds.x + e_rx, vis_bounds.y + e_ry, e_rx, e_ry, primary_color);
				break;
			case PHOS_GUI_SHAPE_ROUND_RECT:
				DrawRectangleRounded(vis_bounds, e->corner_radius, ROUND_RECT_SEGMENTS, primary_color);
				break;
			default:
				vl_log(VL_ERROR, "Invalid element shape: %d!\n", e->shape);
				break;
		}
	}

	// render text component of element (if valid):
	if(pluto_cs_check_component(e, PHOS_GUI_COMPONENT_TEXT))
	{
		phos_gui_text_component *text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_TEXT);

		// get placeholder data as well
		phos_gui_placeholder_text_extension *placeholder_text = NULL;
		if(pluto_cs_check_component(e, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT))
			placeholder_text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT);

		if(text->font && IsFontValid(*text->font))
		{
			if(text->font_size <= 0.0f || ColorIsEqual(text->color, BLANK))
				vl_delay_log(VL_WARNING, 1.0f, "This element's ('%s') text component will not render correctly due to invalid font size, or the color's alpha is 0!\n", e->ID);
			else
			{
				/* begin scissor mode to cut off text that has been scrolled off (USE PADDED REGION, NOT VISUAL)
				   note: add CURSOR_WIDTH when rendering a text field to scissor rect so the cursor is not cut off at the right side */
				int width = e->type == PHOS_GUI_TYPE_TEXT_FIELD ? content_bounds.width + CURSOR_WIDTH : content_bounds.width;
				BeginScissorMode(content_bounds.x, content_bounds.y, width, content_bounds.height);

				switch(e->type)
				{
					// for basic elems and buttons, just render text with the set attributes
					case PHOS_GUI_TYPE_BASIC:
					case PHOS_GUI_TYPE_BUTTON:
						DrawTextEx(*text->font, text->str, text->pos, text->font_size, 0.0f, text->color);
						break;
					case PHOS_GUI_TYPE_TEXT_FIELD:
						// calculate where to draw the text
						Vector2 draw_pos = text->pos;
						draw_pos.x -= text->scroll;

						// determine if text field's main text, or placeholder text should be rendered
						if(placeholder_text && strlen(text->str) == 0 && strlen(placeholder_text->str) > 0)
							DrawTextEx(*text->font, placeholder_text->str, draw_pos, text->font_size, 0.0f, placeholder_text->color);
						else
							DrawTextEx(*text->font, text->str, draw_pos, text->font_size, 0.0f, text->color);

						// render cursor (only if placeholder text is not being rendered and text field has focus)
						if(strlen(text->str) > 0 && e->has_focus)
						{
							char buf[PHOS_GUI_MAX_TEXT_LEN + 1];
							memcpy(buf, text->str, text->cursor_pos);
							buf[text->cursor_pos] = '\0';

							float caret_x = MeasureTextEx(*text->font, buf, text->font_size, 0.0f).x;
							float cx = draw_pos.x + caret_x;
							DrawRectangle(cx, draw_pos.y, CURSOR_WIDTH, text->font_size, text->color);
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
			vl_delay_log(VL_WARNING, 5.0f, "Cannot render text component because it has no font set! Element: '%s'\n", e->ID);
	}

	// render outline (if set)
	if(e->render_mode == PHOS_GUI_RENDER_FILL_OUTLINE || e->render_mode == PHOS_GUI_RENDER_OUTLINE)
	{
		// if thickness is 0 or less, warn
		if(e->outline_thickness <= 0.0f)
			vl_delay_log(VL_WARNING, 5.0f, "Element's ('%s') outline thickness is invalid: %f\n", e->ID, e->outline_thickness);
		else
		{
			// get outline color
			Color outline_color = resolve_elem_color(e, &e->outline_colors);

			switch(e->shape)
			{
				case PHOS_GUI_SHAPE_RECT:
					DrawRectangleLinesEx(vis_bounds, e->outline_thickness, outline_color);
					break;
				case PHOS_GUI_SHAPE_ELLIPSE:
					render_ellipse_outline(e->pos, e_rx, e_ry, e->outline_thickness, outline_color);
					break;
				case PHOS_GUI_SHAPE_ROUND_RECT:
					DrawRectangleRoundedLinesEx(vis_bounds, e->corner_radius, ROUND_RECT_SEGMENTS, e->outline_thickness, outline_color);
					break;
				default:
					vl_log(VL_ERROR, "Invalid element shape: %d!\n", e->shape);
					break;
			}
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

	for(size_t i = 0; i < curr_gui->num_elems; ++i)
	{
		// get elem
		phos_gui_elem *elem = curr_gui->elems[i];

		if(elem->type == PHOS_GUI_TYPE_INVALID)
		{
			vl_delay_log(VL_ERROR, 1.0f, "Element has invalid type: '%s'!\n", elem->ID);
			continue;
		}

		render_elem(elem);

		// warn about empty containers
		if(elem->type == PHOS_GUI_TYPE_CONTAINER && elem->num_children == 0)
			vl_delay_log(VL_WARNING, 10.0f, "Element is a parent but has no children: '%s'!\n", elem->ID);
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

	texture pg_tex = { .tex = tex, .file_path = file_path };

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

	Font f = LoadFontEx(file_path, PHOS_GUI_FONT_SIZE_GIGANTIC, NULL, 0);

	if(!IsFontValid(f))
	{
		vl_log(VL_ERROR, "Failed to load font: '%s'!\n", file_path);
		return NULL;
	}

	SetTextureFilter(f.texture, TEXTURE_FILTER_POINT);

	font pg_font = { .font = f, .file_path = file_path };

	dynas_add(&fonts, pg_font);

	vl_log(VL_SUCCESS, "Loaded font: '%s'!\n", file_path);

	return &fonts.data[fonts.size - 1].font;
}
