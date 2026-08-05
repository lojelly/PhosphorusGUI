#include <float.h>
#include <math.h>
#include <ctype.h>
#include "dynamic_array_spellbook.h"
#include "dynamic_map_spellbook.h"
#include "raylib.h"
#include "vibrant_logs.h"
#include "plutonium_cs.h"
#include "phosphorus_gui.h"
#include "raymath.h"
#include "rlgl.h"

#define CURSOR_WIDTH 3.0f
#define KEY_REPEAT_DELAY 0.5f
#define KEY_REPEAT_INTERVAL 0.033f

#define ROUND_RECT_SEGMENTS 32

#define TEXT_PADDING 8.0f

#define MAX_CLIPS 16

#define MIN_SCROLL_BAR_HEIGHT 25.0f
#define SCROLL_THUMB_DRAG_FACTOR 0.25f

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
static float win_size_w = 0.0f;
static float win_size_h = 0.0f;

static Font *default_font = NULL;

// clip regions:
static size_t num_clips = 0;
static Rectangle clips[MAX_CLIPS];

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

static void init_text_component(void *text_component)
{
	if(!text_component)
		return;

	phos_gui_text_component *text = text_component;

	text->font = default_font;
	text->font_size = PHOS_GUI_FONT_SIZE_DEFAULT;
	text->accept_letters = true;
	text->accept_nums = true;
	text->accept_specials = true;
	text->editable = true;
	text->max_len = PHOS_GUI_MAX_TEXT_LEN;
	text->len = 0;
	text->cursor_pos = 0;
	text->color = PHOS_GUI_BLACK;
	text->offset = Vector2Zero();
	snprintf(text->str, sizeof(text->str), "");
}

static void init_placeholder_text_extension(void *placeholder_text_component)
{
	if(!placeholder_text_component)
		return;

	phos_gui_placeholder_text_extension *placeholder_text = placeholder_text_component;
	phos_gui_elem *elem = pluto_cs_get_owner(placeholder_text_component);

	// see if owner has a base text component
	phos_gui_text_component *host_text = NULL;
	if(!(host_text = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT)))
	{
		vl_log(VL_ERROR, "To initialize a placeholder text extension, the element '%s' must also own a text component!\n", elem->ID);
		return;
	}

	placeholder_text->host = host_text;

	// match owners
	if(pluto_cs_get_owner(placeholder_text->host) != elem)
	{
		vl_log(VL_ERROR, "Mismatched owner pointer! Cannot initialize placeholder text extension!\n");
		return;
	}

	placeholder_text->color = PHOS_GUI_GRAY;
	snprintf(placeholder_text->str, sizeof(placeholder_text->str), "");
}

static void init_layout_component(void *layout_component)
{
	if(!layout_component)
		return;

	phos_gui_layout_component *layout = layout_component;

	layout->rows = 0;
	layout->cols = 0;
	layout->spacing_x = 0.0f;
	layout->spacing_y = 0.0f;
	layout->total_content_width = 0.0f;
	layout->total_content_height = 0.0f;
	layout->flow = PHOS_GUI_LAYOUT_ROW_MAJOR;
	layout->auto_fit_children = true;
	layout->clamp_parent = false;
}

static void init_scroll_pane_component(void *scroll_pane_component)
{
	if(!scroll_pane_component)
		return;

	phos_gui_scroll_pane_component *scroll_pane = scroll_pane_component;

	// get owner of scroll pane
	phos_gui_elem *owner = pluto_cs_get_owner(scroll_pane);
	if(!owner)
	{
		vl_log(VL_ERROR, "Scroll pane has no owner!\n");
		return;
	}

	// by default, scroll panes result in the elem's content rect being clipped
	owner->clip_content_rect = true;

	scroll_pane->scroll = 0.0f;
	scroll_pane->max_scroll = 0.0f;
	scroll_pane->px_per_tick = 1.0f;
	scroll_pane->scroll_bar_width = 12.0f;
	scroll_pane->scroll_bar_bg_color = PHOS_GUI_LIGHT_GRAY;
	scroll_pane->scroll_thumb_color = PHOS_GUI_DARK_GRAY;
	scroll_pane->scroll_thumb_focus_color = PHOS_GUI_GRAY;
	scroll_pane->render_scroll_bar = true;
	scroll_pane->thumb_has_focus = false;
	scroll_pane->thumb_grabbed = false;
}

static void init_drag_pane_component(void *drag_pane_component)
{
	if(!drag_pane_component)
		return;

	phos_gui_drag_pane_component *drag_pane = drag_pane_component;

	// get owner and owner's width
	phos_gui_elem *owner = pluto_cs_get_owner(drag_pane);
	if(!owner)
	{
		vl_log(VL_ERROR, "Drag pane has no owner!\n");
		return;
	}
	float owner_width = phos_gui_get_rect_size(phos_gui_get_elem_rect(owner, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE)).x;

	// drag bar matches usable content width of owner, and a height of 25 pixels
	drag_pane->drag_bar_size = (Vector2) { owner_width, 25 };
	drag_pane->drag_bar_pos = PHOS_GUI_ALIGN_INNER_TOP;
	drag_pane->drag_bar_color = PHOS_GUI_LIGHT_GRAY;
	drag_pane->collision_opts = PHOS_GUI_OPTS_NONE;
	drag_pane->use_drag_bar = false;
	drag_pane->grabbed = false;
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
	pluto_cs_register(PHOS_GUI_COMPONENT_LAYOUT, sizeof(phos_gui_layout_component), init_layout_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_SCROLL_PANE, sizeof(phos_gui_scroll_pane_component), init_scroll_pane_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_DRAG_PANE, sizeof(phos_gui_drag_pane_component), init_drag_pane_component, NULL);

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

static void auto_gen_id(const char *ID, char *target, size_t target_size, const char *prefix, size_t *generator)
{
	if(strcmp(ID, "!auto") == 0)
		snprintf(target, target_size, "!auto");
	else if(strcmp(ID, "auto") == 0)
		snprintf(target, target_size, "%s_#%zu", prefix, (*generator)++);
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

	// only register non-NULL pointers:
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
		auto_gen_id(new_gui->ID, new_gui->ID, sizeof(new_gui->ID), "gui", &gui_auto_id);

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
		vl_log(VL_ERROR, "Cannot center a NULL element!\n");
		return;
	}

	Vector2 elem_size = elem->size;

	Vector2 container_center = { origin.x + size.x / 2.0f, origin.y + size.y / 2.0f };
	
	Vector2 elem_centered = { container_center.x - elem_size.x / 2.0f, container_center.y - elem_size.y / 2.0f };

	phos_gui_set_elem_bounds(elem, elem_centered.x, elem_centered.y, elem->size.x, elem->size.y, PHOS_GUI_OPTS_REALIGN_TEXT);
}

static bool check_elem_collision(phos_gui_elem *elem1, phos_gui_elem *elem2, phos_gui_elem_bounding_box bounds)
{
	// get elem rects
	Rectangle r1 = phos_gui_get_elem_rect(elem1, bounds);
	Rectangle r2 = phos_gui_get_elem_rect(elem2, bounds);

	// check collision between them
	return CheckCollisionRecs(r1, r2);
}
void phos_gui_move_elem_xy(phos_gui_elem *elem, float x, float y, phos_gui_opts opts)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot move a NULL element!\n");
		return;
	}

	// get curr elem pos
	Vector2 old_pos = elem->pos;

	// then move elem
	elem->pos.x += x;
	elem->pos.y += y;

	// see if element falls within any kind of clip region
	bool in_clip_region = false;
	phos_gui_elem *parent = elem->parent;
	while(parent)
	{
		in_clip_region = parent->clip_content_rect;
		if(in_clip_region)
			break;
		parent = parent->parent;
	}

	// check for collisions:

	bool collision = false;

	// window collisions first
	if(!in_clip_region && opts & PHOS_GUI_OPTS_CHECK_WINDOW_COLLISIONS)
	{
		// first, check for window edge collisions
		if(elem->pos.x <= 0.0f || elem->pos.y <= 0.0f ||
				elem->pos.x + elem->size.x >= GetRenderWidth() || elem->pos.y + elem->size.y >= GetRenderHeight())
		{
			collision = true;
		}
	}

	// then elem collisions
	if(!in_clip_region & opts & PHOS_GUI_OPTS_CHECK_ELEM_COLLISIONS)
	{
		if(elem->gui) // TODO warning/error against no gui set?
		{
			for(size_t i = 0; i < elem->gui->num_elems; ++i)
			{
				// elem at i
				phos_gui_elem *e = elem->gui->elems[i];

				// ensure elem does not collide with self
				if(elem != e && check_elem_collision(elem, e, PHOS_GUI_ELEM_BOUNDS_REAL))
				{
					collision = true;
					break;
				}
			}
		}
	}

	// if any collision occurred, reset elem pos to old pos:
	if(collision)
		elem->pos = old_pos;

	// check for PHOS_GUI_OPTS_PASS_DOWN and if applied, check collisions on the child elements as well
	if(!collision && opts & PHOS_GUI_OPTS_PASS_DOWN)
		for(size_t i = 0; i < elem->num_children; ++i)
			phos_gui_move_elem_xy(elem->children[i], x, y, opts);
}

static Vector2 get_proposed_align_pos(Vector2 target_object_size, phos_gui_alignment alignment, const phos_gui_elem *const reference_elem)
{
	// start at reference_rect origin
	Rectangle whole_rect = phos_gui_get_elem_rect(reference_elem, PHOS_GUI_ELEM_BOUNDS_TOTAL);
	Rectangle content_rect = phos_gui_get_elem_rect(reference_elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);

	// keep track of position of the whole space rect
	Vector2 v = phos_gui_get_rect_pos(whole_rect);

	// define bounds
	float outer_left = v.x;
	float outer_top = v.y;
	float outer_right = outer_left + whole_rect.width;
	float outer_bottom = outer_top + whole_rect.height;

	float inner_left = content_rect.x;
	float inner_top = content_rect.y;
	float inner_right = inner_left + content_rect.width;
	float inner_bottom = inner_top + content_rect.height;

	float inner_center_x = inner_left + (content_rect.width - target_object_size.x) / 2.0f;
	float inner_center_y = inner_top + (content_rect.height - target_object_size.y) / 2.0f;

	switch(alignment)
	{
		case PHOS_GUI_ALIGN_INNER_LEFT:
			v.x = inner_left;
			v.y = inner_center_y;
			break;
		case PHOS_GUI_ALIGN_INNER_TOP:
			v.x = inner_center_x;
			v.y = inner_top;
			break;
		case PHOS_GUI_ALIGN_INNER_RIGHT:
			v.x = inner_right - target_object_size.x;
			v.y = inner_center_y;
			break;
		case PHOS_GUI_ALIGN_INNER_BOTTOM:
			v.x = inner_center_x;
			v.y = inner_bottom - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_INNER_CENTER:
			v.x = inner_center_x;
			v.y = inner_center_y;
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
			v.y = inner_center_y;
			break;
		case PHOS_GUI_ALIGN_TOP:
			v.x = inner_center_x;
			v.y = outer_top - target_object_size.y;
			break;
		case PHOS_GUI_ALIGN_RIGHT:
			v.x = outer_right;
			v.y = inner_center_y;
			break;
		case PHOS_GUI_ALIGN_BOTTOM:
			v.x = inner_center_x;
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
		vl_log(VL_ERROR, "Cannot shrink this element ('%s') anymore, its size cannot be <= 0.0f!\n", elem->ID);
		return;
	}

	// apply new size
	elem->size = (Vector2) { new_w, new_h };

	phos_gui_text_component *elem_tx = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
	if(elem_tx)
	{
		// based on given options, modify text component:
		if(opts & PHOS_GUI_OPTS_FIT_TEXT)
			phos_gui_make_text_fit_elem(elem_tx, PHOS_GUI_TARGET_AUTO_TEXT);
		if(opts & PHOS_GUI_OPTS_REALIGN_TEXT)
			phos_gui_align_elem_text(elem_tx, PHOS_GUI_TARGET_AUTO_TEXT, elem_tx->alignment);
	}
}
void phos_gui_resize_elem_wh(phos_gui_elem *elem, float w, float h, phos_gui_opts opts)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot resize a NULL element!\n");
		return;
	}

	// resize the given elem
	resize_single_elem_wh(elem, w, h, opts);

	// pass changes down to children
	if(elem->num_children > 0)
	{
		// then shrink/expand the elem's child elements the same amount of pixels (only if PHOS_GUI_PASS_DOWN is set)
		if(opts & PHOS_GUI_OPTS_PASS_DOWN)
		{
			for(size_t i = 0; i < elem->num_children; ++i)
				phos_gui_resize_elem_wh(elem->children[i], w, h, opts);
		}
	}
}

Vector2 phos_gui_get_elem_center(phos_gui_elem *elem)
{
	Vector2 v = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain center of a NULL element!\n");
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
		vl_log(VL_ERROR, "Cannot obtain center of a NULL element!\n");
		return v;
	}
	if(!pluto_cs_check_component(elem, PHOS_GUI_COMPONENT_TEXT))
	{
		vl_log(VL_ERROR, "Cannot obtain text center of an element ('%s') with no text component!\n", elem->ID);
		return v;
	}

	phos_gui_text_component *txt = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);

	if(!txt->font)
	{
		vl_log(VL_ERROR, "This element ('%s') has a NULL font, cannot obtain center with text!\n", elem->ID);
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

Rectangle phos_gui_get_elem_rect(const phos_gui_elem *const elem, phos_gui_elem_bounding_box bounds)
{
	Rectangle r = {0};

	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot obtain visible bounds for a NULL element!\n");
		return r;
	}

	// start out with the 'real' bounds of the element
	r.x = elem->pos.x;
	r.y = elem->pos.y;
	r.width = elem->size.x;
	r.height = elem->size.y;

	switch(bounds)
	{
		// for an invalid bounds selection, return empty rectangle
		case PHOS_GUI_ELEM_BOUNDS_NONE:
			return (Rectangle) { 0, 0, 0, 0 };

		// if obtaining real bounds, just return the rect as is
		case PHOS_GUI_ELEM_BOUNDS_REAL:
			return r;
		case PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL:
			r.x += elem->outline_thickness + elem->left_padding;
			r.y += elem->outline_thickness + elem->top_padding;
			r.width -= ((elem->outline_thickness * 2.0f) + elem->right_padding + elem->left_padding);
			r.height -= ((elem->outline_thickness * 2.0f) + elem->bottom_padding + elem->top_padding);
			break;
		case PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE:
			// copy same bounds from total content area:
			r.x += elem->outline_thickness + elem->left_padding;
			r.y += elem->outline_thickness + elem->top_padding;
			r.width -= ((elem->outline_thickness * 2.0f) + elem->right_padding + elem->left_padding);
			r.height -= ((elem->outline_thickness * 2.0f) + elem->bottom_padding + elem->top_padding);

			// check for scroll pane
			phos_gui_scroll_pane_component *scroll_pane = NULL;
			if((scroll_pane = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_SCROLL_PANE)))
				r.width -= scroll_pane->scroll_bar_width;

			// check for drag pane and drag bar
			phos_gui_drag_pane_component *drag_pane = NULL;
			if((drag_pane = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_DRAG_PANE)))
			{
				// only modify usable content area if drag pane uses a drag bar
				if(drag_pane->use_drag_bar)
				{
					// resize and shift content rect based on drag bar pos:

					Vector2 drag_bar_pos = {0};
					switch(drag_pane->drag_bar_pos)
					{
						case PHOS_GUI_ALIGN_INNER_LEFT:
							r.x += drag_pane->drag_bar_size.x;
							r.width -= drag_pane->drag_bar_size.x;
							break;
						case PHOS_GUI_ALIGN_INNER_TOP:
							r.y += drag_pane->drag_bar_size.y;
							r.height -= drag_pane->drag_bar_size.y;
							break;
						case PHOS_GUI_ALIGN_INNER_RIGHT:
							r.width -= drag_pane->drag_bar_size.x;
							break;
						case PHOS_GUI_ALIGN_INNER_BOTTOM:
							r.height -= drag_pane->drag_bar_size.y;
							break;
						default:
							vl_delay_log(VL_ERROR, 5.0f, "Invalid drag bar position: %d! It must be either PHOS_GUI_ALIGN_INNER_LEFT, PHOS_GUI_ALIGN_INNER_TOP, PHOS_GUI_ALIGN_INNER_RIGHT, or PHOS_GUI_ALIGN_INNER_BOTTOM!\n", drag_pane->drag_bar_pos);
							break;
					}
				}
			}
			break;
		case PHOS_GUI_ELEM_BOUNDS_TOTAL:
			r.x -= elem->left_margin;
			r.y -= elem->top_margin;
			r.width = elem->size.x + elem->left_margin + elem->right_margin;
			r.height = elem->size.y + elem->top_margin + elem->bottom_margin;
			break;
		default:
			vl_log(VL_ERROR, "Invalid element bounding box requested: %d!\n", bounds);
			break;
	}

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

	if(!text_component)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for NULL text component/element!\n");
		return;
	}

	// get text component owner
	const phos_gui_elem *const elem = pluto_cs_get_owner(text_component);
	if(!elem)
	{
		vl_delay_log(VL_ERROR, 1.0f, "The text component's owner is invalid, cannot obtain text bounds for the text component!\n");
		return;
	}
	const char *main_str = text_component->str;

	// ensure font is valid
	if(!text_component->font || !IsFontValid(*text_component->font))
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot obtain text bounds for NULL/invalid font on element: '%s'!\n", elem->ID);
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
		main_bounds.x = text_component->offset.x;
		main_bounds.y = text_component->offset.y;

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
		placeholder_bounds.x = text_component->offset.x;
		placeholder_bounds.y = text_component->offset.y;

		*out_placeholder_bounds = placeholder_bounds;
	}
}
void phos_gui_get_text_bounds_v(const phos_gui_text_component *const text_component, Vector2 *out_main_bounds, Vector2 *out_placeholder_bounds)
{
	Rectangle main_r, placeholder_r;

	if(out_main_bounds)
		phos_gui_get_text_bounds(text_component, &main_r, NULL);
	if(out_placeholder_bounds)
		phos_gui_get_text_bounds(text_component, NULL, &placeholder_r);

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
	phos_gui_elem *e = pluto_cs_get_owner(text);
	if(!e)
	{
		text->scroll = 0.0f;
		text->max_scroll = 0.0f;
		vl_delay_log(VL_WARNING, 5.0f, "Failed to update text scrolling. Text component's owner is invalid!\n");
		return;
	}

	// text always resides in free content space:
	Rectangle real_bounds = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

	// get bounds of text
	Rectangle text_bounds;
	phos_gui_get_text_bounds(text, &text_bounds, NULL);

	// calculate the overflow
	float vis_left = real_bounds.x;
	float vis_right = real_bounds.x + real_bounds.width;
	float vis_width = vis_right - (e->pos.x + text->offset.x);
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
		float caret_screen = e->pos.x + text->offset.x + caret_x - text->scroll;

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
		vl_log(VL_ERROR, "Cannot initialize NULL text component!\n");
		return;
	}
	if(!str)
	{
		vl_log(VL_ERROR, "Cannot initialize text component with NULL string!\n");
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
	snprintf(text->str, sizeof(text->str), "%s", str);
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
		vl_log(VL_ERROR, "Cannot initialize NULL placeholder text!\n");
		return;
	}
	if(!str)
	{
		vl_log(VL_ERROR, "Cannot initialize placeholder text with NULL string!\n");
		return;
	}
	if(strlen(str) >= PHOS_GUI_MAX_TEXT_LEN)
	{
		vl_log(VL_ERROR, "String is too long! The max is PHOS_GUI_MAX_TEXT_LEN!\n");
		return;
	}

	snprintf(placeholder_text->str, sizeof(placeholder_text->str), "%s", str);
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

void phos_gui_set_elem_pos(phos_gui_elem *elem, float x, float y, phos_gui_opts opts)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set the position of a NULL element!\n");
		return;
	}

	// find difference between current pos and new pos
	float x_diff = x - elem->pos.x;
	float y_diff = y - elem->pos.y;

	// move based on difference
	phos_gui_move_elem_xy(elem, x_diff, y_diff, opts);
}
void phos_gui_set_elem_size(phos_gui_elem *elem, float w, float h, phos_gui_opts opts)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set the size of a NULL element!\n");
		return;
	}

	// find difference between current size and new size
	float x_diff = w - elem->size.x;
	float y_diff = h - elem->size.y;

	// resize elem and fix text
	phos_gui_resize_elem_wh(elem, x_diff, y_diff, opts);
}
void phos_gui_set_elem_bounds(phos_gui_elem *elem, float x, float y, float w, float h, phos_gui_opts opts)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set bounds of a NULL element!\n");
		return;
	}

	phos_gui_set_elem_pos(elem, x, y, opts);
	phos_gui_set_elem_size(elem, w, h, opts);
}

void phos_gui_init_color_set(phos_gui_color_set *set, Color normal_color, Color hover_color, Color press_color, Color focus_color)
{
	if(!set)
	{
		vl_log(VL_ERROR, "Cannot set colors of NULL color set!\n");
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
		vl_log(VL_ERROR, "Cannot generate colors of NULL color set!\n");
		return;
	}
	
	*set = (phos_gui_color_set) {
		.normal_color = normal_color,
		.hover_color = ColorBrightness(normal_color, hover_color_factor),
		.press_color = ColorBrightness(normal_color, press_color_factor),
		.focus_color = ColorBrightness(normal_color, focus_color_factor) };
}

void phos_gui_set_text_contents(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, const char *new_contents, phos_gui_opts opts)
{
	if(!text_component)
	{
		vl_log(VL_ERROR, "Cannot set string contents of a NULL text_component!\n");
		return;
	}
	if(!new_contents)
	{
		vl_log(VL_ERROR, "String is NULL, cannot set an element's text contents!\n");
		return;
	}

	// find out which string pointer is being targeted
	char *dest = NULL;
	switch(target_str)
	{
		case PHOS_GUI_TARGET_MAIN_TEXT:
			dest = text_component->str;
			break;
		case PHOS_GUI_TARGET_PLACEHOLDER_TEXT:
			// see if text component can be linked to a placeholder text extension:
			phos_gui_elem *owner = pluto_cs_get_owner(text_component);
			if(!owner)
			{
				vl_log(VL_ERROR, "Cannot use PHOS_GUI_TARGET_PLACEHOLDER_TEXT, the text component's owner is NULL!\n");
				return;
			}

			phos_gui_placeholder_text_extension *placeholder_text = NULL;
			if(!(placeholder_text = pluto_cs_get_component(owner, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT)))
			{
				vl_log(VL_ERROR, "Cannot use PHOS_GUI_TARGET_PLACEHOLDER_TEXT, the element '%s' does not own a phos_gui_placeholder_text_extension component!\n", owner->ID);
				return;
			}

			dest = placeholder_text->str;
			break;
		default:
			vl_log(VL_ERROR, "Invalid target string: %d!\n", target_str);
			return;
	}

	// use hardcoded size from string buffers:
	snprintf(dest, PHOS_GUI_MAX_TEXT_LEN + 1, "%s", new_contents);
	text_component->cursor_pos = text_component->len = strlen(dest);

	// check opts
	if(opts & PHOS_GUI_OPTS_FIT_TEXT)
		phos_gui_make_text_fit_elem(text_component, target_str);
	if(opts & PHOS_GUI_OPTS_REALIGN_TEXT)
		phos_gui_align_elem_text(text_component, target_str, text_component->alignment);
}

/*
   IMPORTANT: even though the owner of the text component could be obtained, and the 'elem' argument could be taken out,
   other functions rely on copies of text components
*/
static Vector2 resolve_elem_text_bounds(const phos_gui_text_component *const text_component, phos_gui_target_text_string target_str)
{
	// get owner of text component
	const phos_gui_elem *const elem = pluto_cs_get_owner(text_component);

	// get placeholder text data
	phos_gui_placeholder_text_extension *placeholder_text = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT);
	
	// measure text bounds
	Vector2 text_bounds = {0};
	switch(target_str)
	{
		case PHOS_GUI_TARGET_MAIN_TEXT:
			text_bounds = MeasureTextEx(*text_component->font, text_component->str, text_component->font_size, 0.0f);
			break;
		case PHOS_GUI_TARGET_PLACEHOLDER_TEXT:
			if(placeholder_text)
				text_bounds = MeasureTextEx(*text_component->font, placeholder_text->str, text_component->font_size, 0.0f);
			else
				vl_log(VL_ERROR, "Cannot obtain placeholder text because the element '%s' does not own a phos_gui_placeholder_text_extension component!\n", elem->ID);
			break;
		case PHOS_GUI_TARGET_AUTO_TEXT:
			size_t main_len = strlen(text_component->str);
			size_t placeholder_len = placeholder_text ? strlen(placeholder_text->str) : 0;
			if(main_len > placeholder_len)
				text_bounds = MeasureTextEx(*text_component->font, text_component->str, text_component->font_size, 0.0f);
			else if(placeholder_text)
				text_bounds = MeasureTextEx(*text_component->font, placeholder_text->str, text_component->font_size, 0.0f);
			else
				vl_log(VL_ERROR, "PHOS_GUI_TARGET_AUTO_TEXT failed!\n");
			break;
		default:
			vl_log(VL_ERROR, "Invalid target string: %d!\n", target_str);
			break;
	}

	return text_bounds;
}

Vector2 phos_gui_align_elem_text(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, phos_gui_alignment alignment)
{
	Vector2 v = {0};

	if(!text_component)
	{
		vl_log(VL_ERROR, "Cannot align text on a NULL text component!\n");
		return v;
	}
	if(!text_component->font)
	{
		vl_log(VL_ERROR, "To align a text component, its font must be set first!\n");
		return v;
	}

	// get owner of text component
	phos_gui_elem *owner = pluto_cs_get_owner(text_component);
	if(!owner)
	{
		vl_log(VL_ERROR, "Cannot align text on the given text component because its owner is NULL!\n");
		return v;
	}

	// ensure alignment is a valid one
	if(alignment < PHOS_GUI_ALIGN_INNER_LEFT || alignment > PHOS_GUI_ALIGN_INNER_BOTTOM_RIGHT)
	{
		vl_log(VL_ERROR, "Cannot align an element's text component with an alignment of %d! The alignment must be a PHOS_GUI_ALIGN_INNER... alignment! Defaulting to PHOS_GUI_ALIGN_INNER_CENTER!\n", alignment);
		alignment = PHOS_GUI_ALIGN_INNER_CENTER;
	}

	Vector2 text_bounds = resolve_elem_text_bounds(text_component, target_str);

	v = get_proposed_align_pos(text_bounds, alignment, owner);
	text_component->offset = Vector2Subtract(v, owner->pos);
	text_component->alignment = alignment;

	return v;
}
Vector2 phos_gui_align_elem(phos_gui_elem *target_elem, phos_gui_alignment alignment, const phos_gui_elem *const reference_elem, phos_gui_opts opts)
{
	Vector2 v = {0};

	if(!target_elem)
	{
		vl_log(VL_ERROR, "Cannot align NULL target element!\n");
		return v;
	}
	if(!reference_elem)
	{
		vl_log(VL_ERROR, "Cannot align target element with NULL reference element!\n");
		return v;
	}

	target_elem->alignment = alignment;
	// use the entire target_elem rect when aligning
	v = get_proposed_align_pos(phos_gui_get_rect_size(phos_gui_get_elem_rect(target_elem, PHOS_GUI_ELEM_BOUNDS_TOTAL)), target_elem->alignment, reference_elem);
	phos_gui_set_elem_pos(target_elem, v.x, v.y, opts);

	return v;
}

static bool elem_in_bounds(const phos_gui_elem *const elem, Vector2 origin, Vector2 size)
{
	// get elem rect
	Rectangle r = phos_gui_get_elem_rect(elem, PHOS_GUI_ELEM_BOUNDS_REAL);

	// see if either rect is out of bounds (origin + size)
	if(r.x < origin.x || r.x + r.width > origin.x + size.x ||
			r.y < origin.y || r.y + r.height > origin.y + size.y)
	{
		vl_log(VL_ERROR, "Element '%s' is out of bounds (%.2f, %.2f, %.2f, %.2f)!\n", elem->ID, origin.x, origin.y, size.x, size.y);
		return false;
	}

	return true;
}
Vector2 phos_gui_align_elem_with_window(phos_gui_elem *target_elem, phos_gui_alignment alignment, phos_gui_opts opts)
{
	Vector2 v = {0};

	if(!target_elem)
	{
		vl_log(VL_ERROR, "Cannot align NULL target element!\n");
		return v;
	}
	if(!curr_gui)
	{
		vl_log(VL_ERROR, "To align an element with the window, a phos_gui must be set! Use phos_gui_set_gui(...)!\n");
		return v;
	}

	// ensure elem is in bounds
	if(!elem_in_bounds(target_elem, PHOS_GUI_WINDOW_ORIGIN, PHOS_GUI_WINDOW_SIZE))
	{
		vl_log(VL_ERROR, "Failed to align '%s' with window, it must be in the window's bounds!\n", target_elem->ID);
		return v;
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
	phos_gui_set_elem_bounds(&temp, 0, 0, GetRenderWidth(), GetRenderHeight(), opts);
	v = get_proposed_align_pos(temp.size, target_elem->alignment, &temp);
	phos_gui_set_elem_pos(target_elem, v.x, v.y, opts);

	return v;
}
void phos_gui_fill_window_with_elem(phos_gui_elem *elem, phos_gui_opts opts)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot fill window with a NULL element!\n");
		return;
	}

	elem->alignment = PHOS_GUI_ALIGN_INNER_TOP_LEFT;
	phos_gui_set_elem_bounds(elem, 0.0f, 0.0f, GetRenderWidth(), GetRenderHeight(), opts);
}

static void use_largest_possible_font_size(phos_gui_text_component *text_component, phos_gui_target_text_string target_str)
{
	const phos_gui_elem *const elem = pluto_cs_get_owner(text_component);

	// get content area
	Vector2 size = phos_gui_get_rect_size(phos_gui_get_elem_rect(elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE));

	// create virtual padding around text
	size.x -= TEXT_PADDING * 2.0f;
	size.y -= TEXT_PADDING * 2.0f;

	// start out at largest font size
	text_component->font_size = PHOS_GUI_FONT_SIZE_LARGEST;

	// measure initial text bounds
	Vector2 text_bounds = resolve_elem_text_bounds(text_component, target_str);

	// while the text takes up more space than the inner bounds, font size automatically has to shrink
	while(text_component->font_size > 1.0f && (text_bounds.x >= size.x || text_bounds.y >= size.y))
	{
		// go to next font size
		text_component->font_size -= 1.0f;

		// re-measure text using current font size on the text component copy
		text_bounds = resolve_elem_text_bounds(text_component, target_str);
	}

	vl_log(VL_INFO, "Largest possible font size for '%s' is %.2f.\n", elem->ID, text_component->font_size);
}
void phos_gui_clamp_elem_to_text(const phos_gui_text_component *const text_component, phos_gui_target_text_string target_str, phos_gui_opts opts)
{
	if(!text_component || !text_component->font)
	{
		vl_log(VL_ERROR, "To clamp the element to the text, the text component can't be NULL/invalid!\n");
		return;
	}

	// get owner of text component
	phos_gui_elem *elem = pluto_cs_get_owner(text_component);
	if(!elem)
	{
		vl_log(VL_ERROR, "The text component's owner is NULL, cannot clamp!\n");
		return;
	}

	// get resolved text bounds
	Vector2 text_bounds = resolve_elem_text_bounds(text_component, target_str);

	// match elem bounds to text bounds
	phos_gui_set_elem_size(elem, text_bounds.x, text_bounds.y, opts);
}

void phos_gui_make_text_fit_elem(phos_gui_text_component *text_component, phos_gui_target_text_string target_str)
{
	if(!text_component)
	{
		vl_log(VL_ERROR, "To make the given text component fit the element, the text component cannot be NULL!\n");
		return;
	}
	if(!text_component->font)
	{
		vl_log(VL_ERROR, "To make the given text component fit the element, its font must be set first!\n");
		return;
	}

	// get text component's owner
	const phos_gui_elem *const elem = pluto_cs_get_owner(text_component);
	if(!elem)
	{
		vl_log(VL_ERROR, "To make the given text component fit its owner, the text component must have a valid owner element!\n");
		return;
	}

	// if text already fits element, do not resize text
	Vector2 text_bounds = resolve_elem_text_bounds(text_component, target_str);

	// use largest possible font size
	use_largest_possible_font_size(text_component, target_str);
}
void phos_gui_make_elem_fit_text(const phos_gui_text_component *const text_component, phos_gui_target_text_string target_str)
{
	if(!text_component)
	{
		vl_log(VL_ERROR, "To make the given element fit the text component, the text component cannot be NULL!\n");
		return;
	}
	if(!text_component->font)
	{
		vl_log(VL_ERROR, "To make the given element fit the text component, its font must be set first!\n");
		return;
	}

	// get text component's owner
	phos_gui_elem *elem = pluto_cs_get_owner(text_component);
	if(!elem)
	{
		vl_log(VL_ERROR, "To make the given element fit the text component, the text component must have a valid owner element!\n");
		return;
	}

	// get content area of elem
	Rectangle bounds = phos_gui_get_elem_rect(elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

	// measure text bounds
	Vector2 text_bounds = resolve_elem_text_bounds(text_component, target_str);

	// expand element if necessary
	if(text_bounds.x > bounds.width)
	{
		// find diff in widths
		float diff_w = text_bounds.x - bounds.height;

		// expand by that much
		phos_gui_resize_elem_wh(elem, diff_w, 0.0f, PHOS_GUI_OPTS_REALIGN_TEXT);
	}
	if(text_bounds.y > bounds.height)
	{
		// find diff in heights
		float diff_h = text_bounds.y - bounds.height;

		// expand by that much
		phos_gui_resize_elem_wh(elem, 0.0f, diff_h, PHOS_GUI_OPTS_REALIGN_TEXT);
	}

	// if element has a parent, make sure it can contain the text too
	/*if(elem->parent)
		phos_gui_make_elem_fit_text(elem->parent, text_component, target_str);*/
}

void phos_gui_init_elem(phos_gui_elem *elem, phos_gui_elem_type type, phos_gui_elem_render_mode render_mode, float x, float y, float w, float h)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot setup a NULL element!\n");
		return;
	}

	// TODO finish default values for every field in 'phos_gui_elem'

	elem->type = type;
	elem->render_mode = render_mode;
	elem->pos = (Vector2) { x, y };
	elem->size = (Vector2) { w, h };
	elem->left_padding = elem->top_padding = elem->right_padding = elem->bottom_padding = 0.0f;
	elem->alignment = PHOS_GUI_ALIGN_INNER_TOP_LEFT;
	elem->focus_on_start = false;
	elem->unreachable = false;
	elem->disabled = false;
	elem->clip_content_rect = false;
}

static int register_elem(phos_gui_elem *elem)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot register a NULL element!\n");
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
	auto_gen_id(elem->ID, elem->ID, sizeof(elem->ID), "elem", &elem_auto_id);

	// assert no duplicate IDs
	if(search_for_duplicate_id(elem->ID) != -1)
		return 0;

	// find duplicate pointers and IDs:
	for(size_t i = 0; i < elem_registry.size; ++i)
	{
		phos_gui_elem *e = elem_registry.data[i];

		if(e == elem)
		{
			vl_log(VL_ERROR, "Duplicate element pointer found while registering element: %p!\n", e);
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

int phos_gui_add_elem_to_gui(phos_gui *gui, phos_gui_elem *elem)
{
	if(!gui || !elem)
	{
		vl_log(VL_ERROR, "Failed to add the element to the given phos_gui. Make sure 'gui' and 'elem' are not NULL!\n");
		return 0;
	}

	// try to register 'elem'
	if(register_elem(elem) == 0)
	{
		vl_log(VL_ERROR, "Failed to register the given element '%s'!\n", elem->ID);
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
int phos_gui_add_elem_to_gui_id(phos_gui *gui, phos_gui_elem *elem, const char *ID)
{
	if(!gui || !elem || !ID)
	{
		vl_log(VL_ERROR, "Failed to add the element to the given phos_gui. Make sure 'gui', 'elem' and 'ID' are not NULL!\n");
		return 0;
	}
	if(strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Element ID cannot be empty!\n");
		return 0;
	}

	// copy ID into element's ID
	snprintf(elem->ID, sizeof(elem->ID), "%s", ID);

	// add normally
	return phos_gui_add_elem_to_gui(gui, elem);
}
int phos_gui_add_all_elems_to_gui(phos_gui *gui, phos_gui_elem *elem)
{
	if(!gui || !elem)
	{
		vl_log(VL_ERROR, "Failed to add all elements to the given phos_gui. Make sure 'gui' and 'elem' are not NULL!\n");
		return 0;
	}

	// add the elem first
	phos_gui_add_elem_to_gui(gui, elem);

	// add children next
	for(size_t i = 0; i < elem->num_children; ++i)
		phos_gui_add_elem_to_gui(gui, elem->children[i]);

	return 1;
}
int phos_gui_remove_elem_from_gui(phos_gui *gui, phos_gui_elem *elem)
{
	if(!gui)
	{
		vl_log(VL_ERROR, "Cannot remove an element from a NULL GUI!\n");
		return 0;
	}
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot remove a NULL element from a GUI!\n");
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

	// parent cannot contain itself as a child
	if(parent == child)
	{
		vl_log(VL_ERROR, "A parent element '%s' cannot contain itself as a child element!\n", parent->ID);
		return 0;
	}

	// add elem to parent
	parent->children[parent->num_children++] = child;

	// the element's parent becomes the parent
	child->parent = parent;

	// auto-gen child ID if necessary
	auto_gen_id(child->ID, child->ID, sizeof(child->ID), "elem", &elem_auto_id);

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
		vl_log(VL_ERROR, "Cannot remove an element from a NULL container!\n");
		return 0;
	}
	if(!child)
	{
		vl_log(VL_ERROR,"Cannot remove a NULL element from a container!\n");
		return 0;
	}

	// find elem in container's children array
	for(size_t i = 0; i < parent->num_children; ++i)
	{
		// get elem at i
		phos_gui_elem *ch = parent->children[i];

		// compare pointers
		if(child == ch)
		{
			// matching elem was found:

			// otherwise, shift all elements to the left and decrement num_children:
			memmove(&parent->children[i], &parent->children[i + 1], (parent->num_children - i - 1) * sizeof(phos_gui_elem*));
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
int phos_gui_format_children(phos_gui_elem *parent, phos_gui_opts opts)
{
	if(!parent)
	{
		vl_log(VL_ERROR, "Cannot format a NULL parent element!\n");
		return 0;
	}

	// parent must have layout component
	phos_gui_layout_component *layout = pluto_cs_get_component(parent, PHOS_GUI_COMPONENT_LAYOUT);
	if(!layout)
	{
		vl_log(VL_ERROR, "The given element ('%s') does not have a layout component, cannot format!\n", parent->ID);
		return 0;
	}

	// obtain parent rects
	Rectangle parent_content_area = phos_gui_get_elem_rect(parent, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);
	float parent_x = parent_content_area.x;
	float parent_y = parent_content_area.y;

	// ensure layout can fit elements
	size_t max_children = layout->rows * layout->cols;
	while(parent->num_children > max_children)
	{
		phos_gui_elem *child = parent->children[parent->num_children - 1];
		phos_gui_remove_child(parent, child);

		vl_log(VL_ERROR, "The grid layout on the parent element given ('%s') cannot fit any more elements. Add more rows or columns to fit more elements!\n", parent->ID);
		vl_print(VL_ERROR, "			V   Removing child '%s' from parent '%s'!\n", child->ID, parent->ID);
	}

	/*
	   figure out the size of each element:

	   size of each grid cell is the size of the parent
	   divided by number of rows and cols, with spacing
	   taken into account properly.
	*/
	const float total_spacing_x = layout->spacing_x * (layout->cols - 1);
	const float total_spacing_y = layout->spacing_y * (layout->rows - 1);
	// size of each cell in the layout:
	const float cell_w = (parent_content_area.width - total_spacing_x) / layout->cols; // cell_w relies on total width of layout
	float cell_h = 0.0f;

	if(layout->auto_fit_children)
		cell_h = (parent_content_area.height - total_spacing_y) / layout->rows;

	// store total content area for layout:
	float total_content_width = 0.0f;
	float total_content_height = 0.0f;

	size_t row = 0, col = 0;
	for(size_t i = 0; i < parent->num_children; ++i)
	{
		// child at i
		phos_gui_elem *child = parent->children[i];

		// child width is the total cell width minus child's side margins
		const float child_w = cell_w - child->left_margin - child->right_margin;
		float child_h, cell_h;

		if(layout->auto_fit_children)
		{
			// force child to fit into cells
			cell_h = (parent_content_area.height - total_spacing_y) / layout->rows;
			child_h = cell_h - child->top_margin - child->bottom_margin;

			phos_gui_set_elem_size(child, child_w, child_h, opts);
		}
		else
		{
			// retain child height
			child_h = child->size.y;
			cell_h = child_h + child->top_margin + child->bottom_margin;

			// but if necessary, force child to fit horizontally (ex. used when scroll bars are rendered)
			if(child->size.x > child_w)
				phos_gui_set_elem_size(child, child_w, child_h, opts);
		} 

		// move elem to the grid slot (take child's margin into account)
		float cell_x = parent_x + ((cell_w + layout->spacing_x) * col);
		float cell_y = parent_y + ((cell_h + layout->spacing_y) * row);
		float child_x = cell_x + child->left_margin;
		float child_y = cell_y + child->top_margin;
		phos_gui_set_elem_pos(child, child_x, child_y, opts);

		// total content width becomes the right edge of this child
		float child_right = (child_x - parent_x) + child_w + child->right_margin;;
		if(child_right > total_content_width)
			total_content_width = child_right;

		// total content height becomes the bottom edge of this child
		float child_bottom = (child_y - parent_y) + child_h + child->bottom_margin;
		if(child_bottom > total_content_height)
			total_content_height = child_bottom;

		// go to next grid slot based on the layout's flow:
		switch(layout->flow)
		{
			case PHOS_GUI_LAYOUT_ROW_MAJOR:
				// fill row, then go to next row
				col++;
				if(col >= layout->cols)
				{
					col = 0;
					row++;
				}
				break;
			case PHOS_GUI_LAYOUT_COLUMN_MAJOR:
				// fill column, then go to next column
				row++;
				if(row >= layout->rows)
				{
					row = 0;
					col++;
				}
				break;
			default:
				vl_log(VL_ERROR, "Invalid layout flow: %d!\n", layout->flow);
				break;
		}
	}

	// set 'total_content_width' and 'total_content_height' on the layout
	layout->total_content_width = total_content_width;
	layout->total_content_height = total_content_height;

	// should the parent be clamped?
	if(layout->clamp_parent)
	{
		Rectangle outer = phos_gui_get_elem_rect(parent, PHOS_GUI_ELEM_BOUNDS_REAL);
		Rectangle inner = phos_gui_get_elem_rect(parent, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

		float extra_width = outer.width - inner.width;
		float extra_height = outer.height - inner.height;

		phos_gui_set_elem_size(parent, layout->total_content_width + extra_width, layout->total_content_height + extra_height, PHOS_GUI_OPTS_REALIGN_TEXT);
	}

	return 1;
}
phos_gui_elem *phos_gui_get_elem(const char *ID)
{
	if(!ID || strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Cannot obtain an element with NULL ID!\n");
		return NULL;
	}
	if(!init)
	{
		vl_log(VL_ERROR, "Cannot obtain an element, phos_gui_init() was never called!\n");
		return NULL;
	}

	for(size_t i = 0; i < elem_registry.size; ++i)
	{
		phos_gui_elem *e = elem_registry.data[i];

		if(e)
			if(strcmp(e->ID, ID) == 0)
				return e;
	}

	vl_log(VL_ERROR, "No element found with the ID: '%s'\n", ID);
	return NULL;
}
static int create_blueprint(phos_gui_elem *elem, const char *ID, blueprint *bp)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot clone a NULL element!\n");
		return 0;
	}
	if(!ID || strlen(ID) == 0)
	{
		vl_log(VL_ERROR, "Invalid blueprint ID!\n");
		return 0;
	}

	blueprint new_bp = {0};

	// auto-generate ID if necessary
	auto_gen_id(ID, new_bp.ID, sizeof(new_bp.ID), "blueprint", &blueprint_auto_id);

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
			vl_log(VL_ERROR, "Duplicate element pointer found while creating blueprint: %p!\n", elem);
			return 0;
		}
		if(strcmp(bp->ID, ID) == 0)
		{
			vl_log(VL_ERROR, "Duplicate blueprint ID found: '%s'!\n", bp->ID);
			return 0;
		}
	}

	snprintf(new_bp.ID, sizeof(new_bp.ID), "%s", ID);
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
	auto_gen_id("auto", target_elem->ID, sizeof(target_elem->ID), "elem", &elem_auto_id);

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
			phos_gui_elem *child_clone = target_elem->children[i];
			snprintf(child_clone->ID, sizeof(child_clone->ID), "auto");
			auto_gen_id("auto", child_clone->ID, sizeof(child_clone->ID), "elem", &elem_auto_id);

			// clone elements from child onto child_clone
			if(pluto_cs_clone_all_components(child, child_clone) == 0)
				vl_log(VL_ERROR, "Failed to clone child components!\n");

			// add child to target_elem
			phos_gui_add_child(target_elem, child_clone);
		}
	}
	else
		// when the children should not be cloned, reset the element's child count to 0
		target_elem->num_children = 0;

	// clone elements from bp->elem onto target_elem
	if(pluto_cs_clone_all_components(bp->elem, target_elem) == 0)
		vl_log(VL_ERROR, "Failed to initialize a cloned element! Cloning components failed! Source element: '%s', target element: '%s'", bp->elem->ID, target_elem->ID);
}

int phos_gui_create_button(phos_gui_elem *elem)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot turn the given element into a button!\n");
		return 0;
	}

	phos_gui_init_elem(elem, PHOS_GUI_TYPE_BUTTON, PHOS_GUI_RENDER_FILL_OUTLINE, 0, 0, 200, 100);
	phos_gui_gen_color_set(&elem->primary_colors, WHITE, -0.1f, -0.2f, 0.0f);
	phos_gui_fill_color_set(&elem->outline_colors, PHOS_GUI_BLACK);
	elem->outline_thickness = 5.0f;

	// add text component to button
	phos_gui_text_component *text = pluto_cs_add_component(elem, PHOS_GUI_COMPONENT_TEXT);
	if(!text)
	{
		vl_log(VL_ERROR, "Failed to add a text component to the button element '%s'!\n", elem->ID);
		return 0;
	}
	phos_gui_set_text_contents(text, PHOS_GUI_TARGET_MAIN_TEXT, "Text", PHOS_GUI_OPTS_NONE);

	// if text has valid font, align text to center of elem
	if(text->font && IsFontValid(*text->font))
		phos_gui_align_elem_text(text, PHOS_GUI_TARGET_MAIN_TEXT, PHOS_GUI_ALIGN_INNER_CENTER);

	return 1;
}

int phos_gui_init_window(const char *title, int width, int height)
{
	if(!init)
	{
		vl_log(VL_ERROR, "Cannot init window because PhosphorusGUI was never initialized!\n");
		return 0;
	}

	InitWindow(width, height, title);

	int curr_monitor = GetCurrentMonitor();
	SetTargetFPS(GetMonitorRefreshRate(curr_monitor));

	SetExitKey(KEY_NULL);
	phos_gui_set_window_size(width, height);

	return 1;
}
void phos_gui_set_window_scale(float x, float y)
{
	win_scale_x = x;
	win_scale_y = y;
}
void phos_gui_set_window_size(float w, float h)
{
	win_size_w = w;
	win_size_h = h;
}

Vector2 phos_gui_get_mouse_pos()
{
	// raw mouse pos
	Vector2 mouse_pos = GetMousePosition();

	// window scale
	mouse_pos.x /= win_scale_x;
	mouse_pos.y /= win_scale_y;

	return mouse_pos;
}
bool phos_gui_is_mouse_over_rect(Rectangle r)
{
	// get mouse pos
	Vector2 mouse_pos = phos_gui_get_mouse_pos();

	// is mouse pos within the rectangle?
	return CheckCollisionPointRec(mouse_pos, r);
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

static bool is_elem_unreachable(const phos_gui_elem *const e)
{
	return e->unreachable ||
		   e->disabled ||
		   e->type == PHOS_GUI_TYPE_INVALID ||
		   e->type == PHOS_GUI_TYPE_BASIC;
}
static void goto_next_elem()
{
	if(!curr_gui)
	{
		vl_log(VL_WARNING, "No current GUI, cannot go to next element!\n");
		return;
	}

	if(curr_gui->num_elems == 0)
	{
		vl_log(VL_WARNING, "The current GUI is empty, cannot travel elements!\n");
		return;
	}

	// see if the gui only contains unreachable elements which would create an infinite loop below:
	bool all_unreachable = true;
	for(size_t i = 0; i < curr_gui->num_elems; ++i)
	{
		if(!is_elem_unreachable(curr_gui->elems[i]))
		{
			all_unreachable = false;
			break;
		}
	}

	if(all_unreachable)
	{
		vl_log(VL_WARNING, "The current GUI only contains unreachable elements, cannot travel elements!\n");
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

	// if the elem is unreachable or disabled or has an invalid type, skip it and repeat process
	if(is_elem_unreachable(elem))
		goto get_next_elem;

	// mark that elem as focused 
	elem->has_focus = true;
}
static void goto_prev_elem()
{
	if(!curr_gui)
	{
		vl_log(VL_WARNING, "No current GUI, cannot go to previous element!\n");
		return;
	}

	// see if the gui only contains unreachable elements which would create an infinite loop below:
	bool all_unreachable = true;
	for(size_t i = 0; i < curr_gui->num_elems; ++i)
	{
		if(!is_elem_unreachable(curr_gui->elems[i]))
		{
			all_unreachable = false;
			break;
		}
	}

	if(all_unreachable)
	{
		vl_log(VL_WARNING, "The current GUI only contains unreachable elements, cannot travel elements!\n");
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

	// if the elem is unreachable or disabled or has an invalid type, skip it and repeat process
	if(is_elem_unreachable(elem))
		goto get_prev_elem;

	// mark that elem as focused
	elem->has_focus = true;
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
static float get_max_scroll(const phos_gui_elem *const e)
{
	float min_y = FLT_MAX;
	float max_y = -FLT_MAX;

	for(size_t i = 0; i < e->num_children; ++i)
	{
		const phos_gui_elem *const child = e->children[i];

		float top = child->pos.y;
		float bottom = child->pos.y + child->size.y + child->bottom_margin;

		if(top < min_y)
			min_y = top;
		if(bottom > max_y)
			max_y = bottom;
	}

	float content_height = max_y - min_y;

	float viewport_height = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE).height;

	float max_scroll = content_height - viewport_height;

	return max_scroll > 0 ? max_scroll : 0.0f;
}
static float get_elem_total_content_height(const phos_gui_elem *const e)
{
	float tch = 0.0f;

	// default to layout component's total content height
	phos_gui_layout_component *layout = NULL;
	if((layout = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_LAYOUT)))
		return layout->total_content_height;

	// get whole content rect of elem
	Rectangle whole_content = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);

	// with no layout, default to elem free content rect's height
	Rectangle free_content = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);
	tch = free_content.height;

	// manually find total content height now:
	for(size_t i = 0; i < e->num_children; ++i)
	{
		const phos_gui_elem *const child = e->children[i];

		// get total rect of child
		Rectangle child_rect = phos_gui_get_elem_rect(child, PHOS_GUI_ELEM_BOUNDS_TOTAL);

		float child_bottom = child_rect.y + child_rect.height;

		// either choose 'child_bottom' or keep current 'tch' value
		tch = fmax(tch, child_bottom - whole_content.y);
	}

	return tch;
}
static void get_scroll_bar_rects(phos_gui_scroll_pane_component *scroll_pane, Rectangle *out_bar, Rectangle *out_thumb)
{
	if(!out_bar && !out_thumb)
		return;

	// get owner of scroll pane
	const phos_gui_elem *const elem = pluto_cs_get_owner(scroll_pane);
	if(!elem)
		return;

	// get elem rects
	Rectangle whole_content_bounds = phos_gui_get_elem_rect(elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);
	Rectangle usable_content_bounds = phos_gui_get_elem_rect(elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

	// whole scroll bar
	float scroll_bar_x = whole_content_bounds.x + whole_content_bounds.width - scroll_pane->scroll_bar_width;
	float scroll_bar_y = usable_content_bounds.y;

	// get elem's total content height
	float total_content_height = get_elem_total_content_height(elem);

	// scroll thumb
	float scroll_thumb_height = (usable_content_bounds.height / total_content_height) * usable_content_bounds.height;
	scroll_thumb_height = fmax(scroll_thumb_height, MIN_SCROLL_BAR_HEIGHT);
	float scroll_thumb_interval = usable_content_bounds.height - scroll_thumb_height;

	float scroll_thumb_y = usable_content_bounds.y;
	if(total_content_height > usable_content_bounds.height)
	{
		float max_scroll = total_content_height - usable_content_bounds.height;
		float scroll_percent = scroll_pane->scroll / max_scroll;
		scroll_thumb_y = usable_content_bounds.y + (scroll_pane->scroll / max_scroll) * scroll_thumb_interval;
	}
	else
		scroll_thumb_height = usable_content_bounds.height;

	Rectangle thumb = {0};
	thumb.x = scroll_bar_x;
	thumb.y = scroll_thumb_y;
	thumb.width = scroll_pane->scroll_bar_width;
	thumb.height = scroll_thumb_height;

	Rectangle bar = {0};
	bar.x = scroll_bar_x;
	bar.y = scroll_bar_y;
	bar.width = scroll_pane->scroll_bar_width;
	bar.height = usable_content_bounds.height;

	if(out_bar)
		*out_bar = bar;
	if(out_thumb)
		*out_thumb = thumb;
}
static void get_drag_bar_rect(phos_gui_drag_pane_component *drag_pane, Rectangle *out_rect)
{
	if(!out_rect)
		return;

	// get owner of drag pane
	phos_gui_elem *owner = pluto_cs_get_owner(drag_pane);
	if(!owner)
	{
		vl_log(VL_ERROR, "Drag pane has no owner!\n");
		return;
	}

	Vector2 drag_bar_pos = get_proposed_align_pos(drag_pane->drag_bar_size, drag_pane->drag_bar_pos, owner);

	*out_rect = (Rectangle) { drag_bar_pos.x, drag_bar_pos.y, drag_pane->drag_bar_size.x, drag_pane->drag_bar_size.y };
}
static void update_elem(phos_gui_elem *e, float dt)
{
	// skip disabled elems
	if(e->disabled)
		return;
	// cannot update invalid elements
	else if(e->type == PHOS_GUI_TYPE_INVALID)
	{
		vl_delay_log(VL_ERROR, 5.0f, "Cannot update element with invalid type: '%s'!\n", e->ID);
		return;
	}

	// obtain all input state here:
	Vector2 mouse_pos = phos_gui_get_mouse_pos();
	Vector2 mouse_delta = GetMouseDelta();
	float mouse_wheel_move = GetMouseWheelMove();
	bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
	bool mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

	// if mouse is not in window, manually clear out mouse delta to stop all dragging logic from occurring
	if(mouse_pos.x <= 0.0f || mouse_pos.y <= 0.0f || mouse_pos.x >= win_size_w || mouse_pos.y >= win_size_h)
		mouse_delta = Vector2Zero();

	// get all rects for the elem:
	Rectangle real_bounds = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_REAL);
	Rectangle whole_content_bounds = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);
	Rectangle usable_content_bounds = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

	// if the element is not basic, update it
	if(e->type != PHOS_GUI_TYPE_BASIC)
	{
		// keep track of whether or not elem gained focus
		bool no_focus = !e->has_focus;

		// reset mouse state
		e->hovered = false;
		e->pressed = false;
		e->clicked = false;

		// see if mouse over element:
		bool mouse_over_elem = CheckCollisionPointRec(mouse_pos, real_bounds);

		if(mouse_over_elem)
		{
			// see if mouse is over the element's clip region
			bool mouse_over_clip_region = true;

			// walk element-parent tree and check all clip regions:
			phos_gui_elem *parent = e->parent;
			while(parent)
			{
				if(pluto_cs_check_component(parent, PHOS_GUI_COMPONENT_SCROLL_PANE))
				{
					Rectangle clip_bounds = phos_gui_get_elem_rect(parent, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);

					if(!CheckCollisionPointRec(mouse_pos, clip_bounds))
					{
						mouse_over_clip_region = false;
						break;
					}
				}

				// go to next parent in the tree
				parent = parent->parent;
			}

			// if mouse is over elem and all clip regions, handle elem-mouse logic:
			if(mouse_over_clip_region)
			{
				e->hovered = true;

				if(mouse_clicked)
				{
					e->clicked = true;
					e->has_focus = true;
				}
				else if(mouse_down)
				{
					e->pressed = true;
					e->has_focus = true;
				}
			}
			// else if user clicks OFF of the element
			else if(mouse_clicked || mouse_down)
			{
				// ensure elem loses focus
				e->has_focus = false;

				// if curr_gui_elem_num points to this elem, reset it
				if(curr_gui->elems[curr_gui_elem_num] == e)
					curr_gui_elem_num = -1;
			}
			else
			{
				// if no mouse input detected, check to see if user reached the elem and is using keyboard input instead
				if(e == curr_gui->elems[curr_gui_elem_num])
				{
					if(IsKeyPressed(KEY_ENTER))
					{
						e->pressed = true;
						e->clicked = true;
					}
					else if(IsKeyDown(KEY_ENTER))
						e->pressed = true;
				}
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

		// if elem has focus and ESC pressed, lose focus
		if(e->has_focus && IsKeyPressed(KEY_ESCAPE))
		{
			e->has_focus = false;
			e->gained_focus = false;

			// if curr_gui_elem_num points to this elem, reset it
			if(curr_gui->elems[curr_gui_elem_num] == e)
				curr_gui_elem_num = -1;
		}
	}

	// then no matter what, update input panes:
	phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_SCROLL_PANE);
	phos_gui_drag_pane_component *drag_pane = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_DRAG_PANE);

	// is user currently scrolling the scroll pane?
	bool scrolling_scroll_pane = scroll_pane && scroll_pane->thumb_grabbed;
	// is user currently dragging the drag pane?
	bool dragging_drag_pane = drag_pane && drag_pane->grabbed;

	if(scroll_pane)
	{
		// calculate max scroll
		scroll_pane->max_scroll = get_max_scroll(e);

		// store current scroll value
		float prev_scroll = scroll_pane->scroll;

		// define rect around scroll thumb:
		Rectangle thumb;
		get_scroll_bar_rects(scroll_pane, NULL, &thumb);

		// number of ticks (pixels of movement)
		float ticks = 0.0f;

		// reset mouse state on scroll thumb
		scroll_pane->thumb_has_focus = false;

		// see if mouse is over the thumb
		bool mouse_over_thumb = phos_gui_is_mouse_over_rect(thumb);

		// see if user is hovered over the thumb and not currently dragging pane
		if(mouse_over_thumb && !dragging_drag_pane)
			// give scroll thumb focus
			scroll_pane->thumb_has_focus = true;

		// see if user is holding down the thumb, and user not currently dragging the drag pane
		if(!dragging_drag_pane && ((mouse_down && mouse_over_thumb) ||
				(mouse_down && scroll_pane->thumb_grabbed)))
		{
			// if user moving the mouse vertically:
			if(mouse_delta.y != 0.0f)
				// ticks becomes the opposite mouse delta y value (and multiply by SCROLL_THUMB_DRAG_FACTOR to slow down move speed)
				ticks = -mouse_delta.y * SCROLL_THUMB_DRAG_FACTOR;

			// thumb is grabbed
			scroll_pane->thumb_grabbed = true;
		}
		else // when user not using mouse buttons
		{
			// ticks becomes mouse wheel movement instead
			ticks = mouse_wheel_move;

			// thumb no longer grabbed
			scroll_pane->thumb_grabbed = false;
		}

		// use ticks * px_per_tick to add to total scroll offset
		scroll_pane->scroll -= ticks * scroll_pane->px_per_tick;

		// clamp scroll amount
		if(scroll_pane->scroll < 0.0f)
			scroll_pane->scroll = 0.0f;
		else if(scroll_pane->scroll > scroll_pane->max_scroll)
			scroll_pane->scroll = scroll_pane->max_scroll;

		float delta = scroll_pane->scroll - prev_scroll;

		// translate children based on scroll pane translation (only if user scrolled)
		if(delta != 0.0f)
			for(size_t i = 0; i < e->num_children; ++i)
				phos_gui_move_elem_xy(e->children[i], 0.0f, -delta, PHOS_GUI_OPTS_NONE);
	}

	// check for drag pane component
	if(drag_pane)
	{
		// only handle drag pane logic if left mouse button down and user not currently scrolling the scroll pane
		if(!scrolling_scroll_pane && mouse_down)
		{
			// get drag bar rect
			Rectangle drag_bar_rect;
			get_drag_bar_rect(drag_pane, &drag_bar_rect);

			// determine how to interpret mouse state
			if(drag_pane->use_drag_bar)
			{
				// check for mouse over drag bar
				bool mouse_over_bar = phos_gui_is_mouse_over_rect(drag_bar_rect);

				if(mouse_over_bar || drag_pane->grabbed)
				{
					// if mouse down and over the bar, it becomes grabbed
					drag_pane->grabbed = true;

					// add to elem pos based on mouse delta
					phos_gui_move_elem_xy(e, mouse_delta.x, mouse_delta.y, PHOS_GUI_OPTS_REALIGN_TEXT | PHOS_GUI_OPTS_PASS_DOWN | drag_pane->collision_opts);
				}
			}
			else
			{
				// check for mouse over free content rect
				bool mouse_over_elem = phos_gui_is_mouse_over_rect(usable_content_bounds);

				if(mouse_over_elem || drag_pane->grabbed)
				{
					// if mouse down and over the elem, it becomes grabbed
					drag_pane->grabbed = true;

					// add to elem pos based on mouse delta
					phos_gui_move_elem_xy(e, mouse_delta.x, mouse_delta.y, PHOS_GUI_OPTS_REALIGN_TEXT | PHOS_GUI_OPTS_PASS_DOWN | drag_pane->collision_opts);
				}
			}
		}
		else
			// when mouse not down, the drag pane is no longer grabbed
			drag_pane->grabbed = false;
	}

	// update child elements
	for(size_t i = 0; i < e->num_children; ++i)
		update_elem(e->children[i], dt);
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

		// update the element and its children
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

static void render_children(const phos_gui_elem *const e, phos_gui_elem_bounding_box bounds);
static void render_elem(const phos_gui_elem *const e)
{
	// cannot render invalid elements
	if(e->type == PHOS_GUI_TYPE_INVALID)
	{
		vl_delay_log(VL_ERROR, 5.0f, "Cannot render element with invalid type: '%s'!\n", e->ID);
		return;
	}
	// skip elements with no render mode
	else if(e->render_mode == PHOS_GUI_RENDER_BLANK)
		return;

	// get color of elem
	const Color primary_color = resolve_elem_color(e, &e->primary_colors);

	// create elem rects:
	const Rectangle real_bounds = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_REAL);
	const Rectangle whole_content_bounds = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);
	const Rectangle usable_content_bounds = phos_gui_get_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

	// if empty size, cannot render
	if(real_bounds.width <= 0 || real_bounds.height <= 0)
	{
		vl_delay_log(VL_ERROR, 5.0f, "Cannot render element '%s' with negative visual bounds: %.2f, %.2f!\n", e->ID, real_bounds.width, real_bounds.height);
		return;
	}
	if(whole_content_bounds.width <= 0 || whole_content_bounds.height <= 0)
	{
		vl_delay_log(VL_ERROR, 5.0f, "Cannot render element '%s' with negative content bounds: %.2f, %.2f!\n", e->ID, whole_content_bounds.width, whole_content_bounds.height);
		return;
	}
	if(usable_content_bounds.width <= 0 || usable_content_bounds.height <= 0)
	{
		vl_delay_log(VL_ERROR, 5.0f, "Cannot render element '%s' with negative usable content bounds: %.2f, %.2f!\n", e->ID, usable_content_bounds.width, usable_content_bounds.height);
		return;
	}

	// create elem ellipse info:
	const float e_rx = real_bounds.width / 2.0f;
	const float e_ry = real_bounds.height / 2.0f;

	// draw elem bg if it is valid
	if(e->texture && IsTextureValid(*e->texture))
	{
		Rectangle src = { 0, 0, e->texture->width, e->texture->height };
		DrawTexturePro(*e->texture, src, real_bounds, PHOS_GUI_WINDOW_ORIGIN, 0.0f, primary_color);
	}
	// else just draw base shape (if set)
	else if(e->render_mode == PHOS_GUI_RENDER_FILL_OUTLINE || e->render_mode == PHOS_GUI_RENDER_FILL)
	{
		switch(e->shape)
		{
			case PHOS_GUI_SHAPE_RECT:
				DrawRectanglePro(real_bounds, PHOS_GUI_WINDOW_ORIGIN, 0.0f, primary_color);
				break;
			case PHOS_GUI_SHAPE_ELLIPSE:
				DrawEllipse(real_bounds.x + e_rx, real_bounds.y + e_ry, e_rx, e_ry, primary_color);
				break;
			case PHOS_GUI_SHAPE_ROUND_RECT:
			{
				Rectangle r = real_bounds;
				float t = e->outline_thickness;

				r.x += t;
				r.y += t;
				r.width -= t * 2.0f;
				r.height -= t * 2.0f;

				DrawRectangleRounded(r, e->corner_radius, ROUND_RECT_SEGMENTS, primary_color);
				break;
			}
			default:
				vl_log(VL_ERROR, "Invalid element shape: %d!\n", e->shape);
				break;
		}
	}

	// render text component of element (if valid):
	if(pluto_cs_check_component(e, PHOS_GUI_COMPONENT_TEXT))
	{
		// get text component
		const phos_gui_text_component *const text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_TEXT);

		// get placeholder data as well (will be NULL if no placeholder text extension found)
		const phos_gui_placeholder_text_extension *const placeholder_text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT);

		const Vector2 text_pos = Vector2Add(phos_gui_get_rect_pos(usable_content_bounds), text->offset);

		if(text->font && IsFontValid(*text->font))
		{
			if(text->font_size <= 0.0f || ColorIsEqual(text->color, BLANK))
				vl_delay_log(VL_WARNING, 1.0f, "This element's ('%s') text component will not render correctly due to invalid font size, or the color's alpha is 0!\n", e->ID);
			else
			{
				/*
				   begin scissor mode to cut off text that has been scrolled off (USE PADDED REGION, NOT VISUAL)
				   note: add CURSOR_WIDTH when rendering a text field to scissor rect so the cursor is not cut off at the right side
				*/
				int clip_width = e->type == PHOS_GUI_TYPE_TEXT_FIELD ? usable_content_bounds.width + CURSOR_WIDTH : usable_content_bounds.width;
				if(phos_gui_new_clip(usable_content_bounds.x, usable_content_bounds.y, clip_width, usable_content_bounds.height))
				{
					switch(e->type)
					{
						// for basic elems and buttons, just render text with the set attributes
						case PHOS_GUI_TYPE_BASIC:
						case PHOS_GUI_TYPE_BUTTON:
							DrawTextEx(*text->font, text->str, text_pos, text->font_size, 0.0f, text->color);
							break;
						case PHOS_GUI_TYPE_TEXT_FIELD:
						{
							// calculate where to draw the text
							Vector2 draw_pos = text_pos;
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
						}
						default:
							break;
					}

					phos_gui_end_clip();
				}
			}
		}
		else
			vl_delay_log(VL_WARNING, 5.0f, "Cannot render text component on element '%s' because it does not have a valid font!!\n", e->ID);
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
					DrawRectangleLinesEx(real_bounds, e->outline_thickness, outline_color);
					break;
				case PHOS_GUI_SHAPE_ELLIPSE:
					render_ellipse_outline(phos_gui_get_rect_pos(real_bounds), e_rx, e_ry, e->outline_thickness, outline_color);
					break;
				case PHOS_GUI_SHAPE_ROUND_RECT:
				{
					Rectangle r = real_bounds;
					float t = e->outline_thickness;
					
					r.x += t;
					r.y += t;
					r.width -= t * 2.0f;
					r.height -= t * 2.0f;

					DrawRectangleRoundedLinesEx(r, e->corner_radius, ROUND_RECT_SEGMENTS, t, outline_color);
					break;
				}
				default:
					vl_log(VL_ERROR, "Invalid element shape: %d!\n", e->shape);
					break;
			}
		}
	}

	// should there be a clip rect around the element's children?
	phos_gui_elem_bounding_box child_clip_bounds = PHOS_GUI_ELEM_BOUNDS_NONE;

	// render drag bar if necessary (use_drag_bar is true)
	phos_gui_drag_pane_component *drag_pane = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_DRAG_PANE);
	if(drag_pane && drag_pane->use_drag_bar)
	{
		// get drag bar bounds
		Rectangle drag_bar_rect;
		get_drag_bar_rect(drag_pane, &drag_bar_rect);

		// render drag bar
		DrawRectangleRec(drag_bar_rect, drag_pane->drag_bar_color);
	}

	// render scroll bar if necessary (render_scroll_bar is true)
	phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_SCROLL_PANE);
	if(scroll_pane && scroll_pane->render_scroll_bar)
	{
		// when using a scroll pane, clip around free content rect
		child_clip_bounds = PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE;

		// obtain bar and thumb rects
		Rectangle bar, thumb;
		get_scroll_bar_rects(scroll_pane, &bar, &thumb);

		// render whole scroll bar
		DrawRectangleRec(bar, scroll_pane->scroll_bar_bg_color);

		// choose color of thumb based on mouse state and render thumb
		Color thumb_color = scroll_pane->thumb_has_focus || scroll_pane->thumb_grabbed ? scroll_pane->scroll_thumb_focus_color : scroll_pane->scroll_thumb_color;
		DrawRectangleRec(thumb, thumb_color);
	}

	render_children(e, child_clip_bounds);
}
static void render_children(const phos_gui_elem *const e, phos_gui_elem_bounding_box bounds)
{
	// start a new clip around children based on bounding box given
	bool clip = false;
	if(bounds != PHOS_GUI_ELEM_BOUNDS_NONE)
		// 'clip_content_rect' must be true, and the clip rect must have been created
		clip = e->clip_content_rect && phos_gui_new_clip_r(phos_gui_get_elem_rect(e, bounds));

	for(size_t i = 0; i < e->num_children; ++i)
		render_elem(e->children[i]);

	// end clip if necessary
	if(clip)
		phos_gui_end_clip();
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

		// render the element and its children
		render_elem(elem);
	}
}

int phos_gui_new_clip(int x, int y, int width, int height)
{
	if(num_clips >= MAX_CLIPS)
	{
		vl_delay_log(VL_WARNING, 2.5f, "Cannot begin a new clip region, max clips reached!\n");
		return 0;
	}

	rlDrawRenderBatchActive();

	if(num_clips == 0)
		rlEnableScissorTest();

	Rectangle clip = { x, y, width, height };

	if(num_clips > 0)
		clip = GetCollisionRec(clips[num_clips - 1], clip);

	if(clip.width == 0 || clip.height == 0)
		return 0;

	rlScissor(clip.x, GetRenderHeight() - (clip.y + clip.height), clip.width, clip.height);
	clips[num_clips++] = clip;

	return 1;
}
int phos_gui_new_clip_r(Rectangle r)
{
	return phos_gui_new_clip(r.x, r.y, r.width, r.height);
}
void phos_gui_end_clip()
{
	if(num_clips == 0)
	{
		vl_delay_log(VL_WARNING, 2.5f, "No clip to end!\n");
		return;
	}

	rlDrawRenderBatchActive();

	num_clips--;

	if(num_clips > 0)
	{
		Rectangle *curr	= &clips[num_clips - 1];
		rlScissor(curr->x, GetRenderHeight() - (curr->y + curr->height), curr->width, curr->height);
	}
	else
		rlDisableScissorTest();
}

Texture2D *phos_gui_load_texture(const char *file_path)
{
	if(!file_path)
	{
		vl_log(VL_ERROR, "Cannot load texture using NULL file path!\n");
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
		vl_log(VL_ERROR, "Cannot load font using NULL file path!\n");
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
void phos_gui_set_default_font(const char *file_path)
{
	if(!file_path)
	{
		vl_log(VL_ERROR, "Cannot create defualt font using NULL file path!\n");
		return;
	}
	default_font = phos_gui_load_font(file_path);
	if(!default_font)
		vl_log(VL_ERROR, "Failed to create default font!\n");
}
Font *phos_gui_get_default_font()
{
	return default_font;
}
