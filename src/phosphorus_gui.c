#include <time.h>
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

#define CURSOR_WIDTH 5.0f
#define KEY_REPEAT_DELAY 0.5f
#define KEY_REPEAT_INTERVAL 0.033f

#define ROUND_RECT_SEGMENTS 32

#define TEXT_PADDING 2.0f

#define MAX_CLIPS 16

#define MIN_SCROLL_THUMB_LENGTH 25.0f

#define DEFAULT_SCROLL_BAR (phos_gui_scroll_bar) { .bg_color = PHOS_GUI_COLOR_LIGHT_GRAY, .thumb_color = PHOS_GUI_COLOR_GRAY, .thumb_focus_color = PHOS_GUI_COLOR_DARK_GRAY, .thumb_shape = PHOS_GUI_SHAPE_RECT, .thumb_corner_radius = 0.0f, .span = 15.0f, .thumb_grab_offset = 0.0f, .thumb_has_focus = false, .thumb_grabbed = false, .rendered = true, .active = true }

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

// icon registry:
typedef struct icon_map
{
	phos_gui_icon *keys;
	const char **values;
	size_t size, capacity;
} icon_map;

// core info and registries
static bool init = false;
static dynas_string_arr all_ids;
static elem_arr elem_registry;
static blueprint_arr blueprint_registry;
static gui_arr gui_registry;

// resources
static tex_arr textures;
static icon_map icons;
static font_arr fonts;

// for objects with ID="auto":
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
static key_timer del_timer = {0};
static key_timer left_arrow_timer = {0};
static key_timer up_arrow_timer = {0};
static key_timer right_arrow_timer = {0};
static key_timer down_arrow_timer = {0};
static key_timer enter_timer = {0};
static key_timer tab_timer = {0};

static phos_gui *prev_gui = NULL;
static phos_gui *curr_gui = NULL;

// travel logic:

// the element the user has traveled to (could be a parent or child element)
static phos_gui_elem *curr_travel_elem = NULL;
static bool resolve_focus_on_start_elem = false;

// window info:
static float win_scale_x = 1.0f;
static float win_scale_y = 1.0f;

static Font *default_font = NULL;

// clip regions:
static size_t num_clips = 0;
static Rectangle clips[MAX_CLIPS];

// current mouse target
static phos_gui_elem *mouse_target = NULL;

static Color screen_tint = BLANK;
static Color window_bg_color = WHITE;
static phos_gui_theme default_theme = {0};

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
#define arr_add(arr, item, ...) \
	do { \
		dynas_add(arr, item); \
		assert_obj_ptr(arr, data, __VA_ARGS__); \
	} while(0)

#define init_map(map, ...) \
	do { \
		dynmaps_init(map); \
		assert_obj_ptr(map, keys, __VA_ARGS__); \
		assert_obj_ptr(map, values, __VA_ARGS__); \
	} while(0)
#define map_add(map, key, value, ...) \
	do { \
		dynmaps_set(map, key, value); \
		assert_obj_ptr(map, keys, __VA_ARGS__); \
		assert_obj_ptr(map, values, __VA_ARGS__); \
	} while(0)

static void init_shadow_component(void *shadow_component)
{
	if(!shadow_component)
		return;

	phos_gui_shadow_component *shadow = shadow_component;

	phos_gui_elem *owner = pluto_cs_get_owner(shadow);
	if(!owner)
	{
		vl_log(VL_ERROR, "Shadow component has no owner!\n");
		return;
	}

	shadow->edges = PHOS_GUI_SHADOW_BOTTOM_RIGHT;
	shadow->length = 10.0f;
	shadow->initial_color = PHOS_GUI_COLOR_DARK_GRAY;
	shadow->fade_color = PHOS_GUI_COLOR_LIGHT_GRAY;

	// re-apply default theme to elem
	phos_gui_apply_theme_to_elem(owner, phos_gui_get_default_theme());
}
static void init_texture_component(void *texture_component)
{
	if(!texture_component)
		return;

	phos_gui_texture_component *texture = texture_component;

	phos_gui_elem *owner = pluto_cs_get_owner(texture);
	if(!owner)
	{
		vl_log(VL_ERROR, "Texture component has no owner!\n");
		return;
	}

	texture->src = NULL;

	// re-apply default theme to elem
	phos_gui_apply_theme_to_elem(owner, phos_gui_get_default_theme());
}
static void init_mouse_listener_component(void *mouse_listener_component)
{
	if(!mouse_listener_component)
		return;

	phos_gui_mouse_listener_component *listener = mouse_listener_component;

	// get owner of mouse listener
	phos_gui_elem *owner = pluto_cs_get_owner(listener);
	if(!owner)
	{
		vl_log(VL_ERROR, "Mouse listener has no owner!\n");
		return;
	}

	listener->type = PHOS_GUI_MOUSE_LISTENER_DEFAULT;

	// generate colors for the listener:
	phos_gui_gen_bg_colors(listener, -0.2f, -0.3f, 0.0f);
	phos_gui_gen_outline_colors(listener, 0.0f, 0.0f, 0.0f);

	listener->clicked = false;
	listener->pressed = false;
	listener->toggled = false;
	listener->toggled_on = false;
	listener->hovered = false;
	listener->has_focus = false;
	listener->gained_focus = false;
	listener->focus_on_start = false;
}
static void force_calculate_elem_rects(phos_gui_elem *elem);
static void init_text_component(void *text_component)
{
	if(!text_component)
		return;

	phos_gui_text_component *text = text_component;

	// get owner of text component
	phos_gui_elem *owner = pluto_cs_get_owner(text);
	if(!owner)
	{
		vl_log(VL_ERROR, "Text has no owner!\n");
		return;
	}

	text->font = default_font;
	text->max_len = PHOS_GUI_MAX_TEXT_LEN;
	text->curr_line_len = 0;
	text->len = 0;
	text->num_lines = 0;
	text->cursor_pos = 0;
	text->spaces_per_tab = 4;
	text->offset = Vector2Zero();
	text->font_size = PHOS_GUI_FONT_SIZE_DEFAULT;
	text->wrap_mode = PHOS_GUI_TEXT_WRAP_NONE;
	text->alignment = PHOS_GUI_ALIGN_INNER_CENTER;
	text->edit_opts = PHOS_GUI_OPTS_NONE;
	text->color = PHOS_GUI_COLOR_BLACK;
	text->key_typed = KEY_NULL;
	text->char_typed = '\0';
	text->editable = false;
	text->edited = false;
	text->accept_letters = true;
	text->accept_nums = true;
	text->accept_specials = true;
	text->enter_inserts_new_line = false;
	text->auto_indent = false;
	snprintf(text->str, sizeof(text->str), "");

	// enforce no additional text line spacing
	SetTextLineSpacing(0);

	// re-apply default theme to element
	phos_gui_apply_theme_to_elem(owner, phos_gui_get_default_theme());
}
static void init_placeholder_text_extension(void *placeholder_text_component)
{
	if(!placeholder_text_component)
		return;

	phos_gui_placeholder_text_extension *placeholder_text = placeholder_text_component;
	phos_gui_elem *elem = pluto_cs_get_owner(placeholder_text_component);
	if(!elem)
	{
		vl_log(VL_ERROR, "No owner on placeholder text component!\n");
		return;
	}

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

	snprintf(placeholder_text->str, sizeof(placeholder_text->str), "");

	// re-apply default theme to element
	phos_gui_apply_theme_to_elem(elem, phos_gui_get_default_theme());
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

static void calculate_elem_rects(phos_gui_elem *e);
static Rectangle get_calculated_elem_rect(phos_gui_elem *elem, phos_gui_elem_bounding_box bounds)
{
	Rectangle r = {0};

	// if no calculation for all rects necessary, perform exact calculation necessary
	switch(bounds)
	{
		// skip switch statement and return invalid rect below
		case PHOS_GUI_ELEM_BOUNDS_NONE:
			return r;

		// if obtaining real bounds, just return the elem's real bounds rect, as it never needs calculating
		case PHOS_GUI_ELEM_BOUNDS_REAL:
			r = elem->bounds;
			break;

		case PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL:
			// if should cache, recalculate rect:
			if(elem->content_total_bounds.should_calculate)
			{
				elem->content_total_bounds.rect = elem->bounds;
				elem->content_total_bounds.rect.x += elem->left_padding;
				elem->content_total_bounds.rect.y += elem->top_padding;
				elem->content_total_bounds.rect.width -= (elem->right_padding + elem->left_padding);
				elem->content_total_bounds.rect.height -= (elem->bottom_padding + elem->top_padding);

				// include outline thickness when necessary
				if(elem->render_mode == PHOS_GUI_RENDER_OUTLINE || elem->render_mode == PHOS_GUI_RENDER_FILL_OUTLINE)
				{
					elem->content_total_bounds.rect.x += elem->outline_thickness;
					elem->content_total_bounds.rect.y += elem->outline_thickness;
					elem->content_total_bounds.rect.width -= (elem->outline_thickness * 2.0f);
					elem->content_total_bounds.rect.height -= (elem->outline_thickness * 2.0f);
				}

				// mark it as cached
				elem->content_total_bounds.should_calculate = false;
			}
			r = elem->content_total_bounds.rect;
			break;
		case PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE:
			// if should cache, recalculate rect:
			if(elem->content_free_bounds.should_calculate)
			{
				elem->content_free_bounds.rect = elem->content_total_bounds.rect;

				// check for scroll pane on this elem or its parent
				phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_SCROLL_PANE);
				if((scroll_pane = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_SCROLL_PANE)))
				{
					if(scroll_pane->v_bar.active && scroll_pane->v_bar.rendered)
						elem->content_free_bounds.rect.width -= scroll_pane->v_bar.span;
					if(scroll_pane->h_bar.active && scroll_pane->h_bar.rendered)
						elem->content_free_bounds.rect.height -= scroll_pane->h_bar.span;
				}
				else
				{
					// get upper-most scroll pane on elem tree:
					phos_gui_elem *parent = elem->parent;
					while(parent && parent->parent)
						parent = parent->parent;

					if(parent)
						scroll_pane = pluto_cs_get_component(parent, PHOS_GUI_COMPONENT_SCROLL_PANE);

					if(scroll_pane)
					{
						if(scroll_pane->v_bar.active && scroll_pane->v_bar.rendered)
							elem->content_free_bounds.rect.width -= scroll_pane->v_bar.span;
						if(scroll_pane->h_bar.active && scroll_pane->h_bar.rendered)
							elem->content_free_bounds.rect.height -= scroll_pane->h_bar.span;
					}
				}

				// check for drag pane and drag bar
				phos_gui_drag_pane_component *drag_pane = NULL;
				if((drag_pane = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_DRAG_PANE)))
				{
					// only modify usable content area if drag pane uses a drag bar
					if(drag_pane->use_drag_bar)
					{
						// resize and shift content rect based on drag bar pos and orientation:
						switch(drag_pane->drag_bar_orientation)
						{
							case PHOS_GUI_DRAG_BAR_HORIZONTAL_TOP:
								elem->content_free_bounds.rect.y += drag_pane->span;
								elem->content_free_bounds.rect.height -= drag_pane->span;
								break;
							case PHOS_GUI_DRAG_BAR_VERTICAL_LEFT:
								elem->content_free_bounds.rect.x += drag_pane->span;
								elem->content_free_bounds.rect.width -= drag_pane->span;
								break;
							case PHOS_GUI_DRAG_BAR_VERTICAL_RIGHT:
								elem->content_free_bounds.rect.width -= drag_pane->span;
								break;
							case PHOS_GUI_DRAG_BAR_HORIZONTAL_BOTTOM:
								elem->content_free_bounds.rect.height -= drag_pane->span;
								break;
							default:
								vl_log(VL_ERROR, "Invalid drag bar orientation: %d!\n", drag_pane->drag_bar_orientation);
								break;
						}
					}
				}

				// mark it as cached
				elem->content_free_bounds.should_calculate = false;
			}
			r = elem->content_free_bounds.rect;
			break;
		case PHOS_GUI_ELEM_BOUNDS_TOTAL:
			// if should cache, recalculate rect:
			if(elem->total_bounds.should_calculate)
			{
				elem->total_bounds.rect = elem->bounds;
				elem->total_bounds.rect.x -= elem->left_margin;
				elem->total_bounds.rect.y -= elem->top_margin;
				elem->total_bounds.rect.width = elem->bounds.width + elem->left_margin + elem->right_margin;
				elem->total_bounds.rect.height = elem->bounds.height + elem->top_margin + elem->bottom_margin;

				// mark it as cached
				elem->total_bounds.should_calculate = false;
			}
			r = elem->total_bounds.rect;
			break;
		default:
			vl_log(VL_ERROR, "Invalid element bounding box requested: %d!\n", bounds);
			return r; // invalid rect
	}

	return r;
}
static void calculate_elem_rects(phos_gui_elem *e)
{
	e->total_bounds.rect = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_TOTAL);
	e->content_total_bounds.rect = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);
	e->content_free_bounds.rect = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);
}
static void prepare_elem_rects_for_caching(phos_gui_elem *elem)
{
	elem->total_bounds.should_calculate = true;
	elem->content_total_bounds.should_calculate = true;
	elem->content_free_bounds.should_calculate = true;
}
static void force_calculate_elem_rects(phos_gui_elem *e)
{
	prepare_elem_rects_for_caching(e);
	calculate_elem_rects(e);
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

	// enable clip region for the element
	owner->clipped = true;

	scroll_pane->v_bar = DEFAULT_SCROLL_BAR;
	scroll_pane->h_bar = DEFAULT_SCROLL_BAR;
	scroll_pane->px_per_tick = 10.0f;
	scroll_pane->scroll_x = 0.0f;
	scroll_pane->max_scroll_x = 0.0f;
	scroll_pane->scroll_y = 0.0f;
	scroll_pane->max_scroll_y = 0.0f;
	scroll_pane->use_mouse_wheel_input = true;
	scroll_pane->v_bar.active = true;
	scroll_pane->h_bar.active = true;

	// re-calculate elem rects instantly
	force_calculate_elem_rects(owner);

	// re-apply default theme to elem
	phos_gui_apply_theme_to_elem(owner, phos_gui_get_default_theme());
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

	// get content rect
	force_calculate_elem_rects(owner);
	Rectangle owner_total_content = get_calculated_elem_rect(owner, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);

	drag_pane->drag_bar_orientation = PHOS_GUI_DRAG_BAR_HORIZONTAL_TOP;
	drag_pane->drag_bar_color = PHOS_GUI_COLOR_LIGHT_GRAY;
	drag_pane->drag_opts = PHOS_GUI_OPTS_NONE;
	drag_pane->span = 35.0f;
	drag_pane->use_drag_bar = false;
	drag_pane->grabbed = false;

	// re-calculate elem rects instantly
	force_calculate_elem_rects(owner);

	// re-apply default theme to elem
	phos_gui_apply_theme_to_elem(owner, phos_gui_get_default_theme());
}
static void init_drop_down_component(void *drop_down_component)
{
	if(!drop_down_component)
		return;

	phos_gui_drop_down_component *drop_down = drop_down_component;

	// get owner
	phos_gui_elem *owner = pluto_cs_get_owner(drop_down);
	if(!owner)
	{
		vl_log(VL_ERROR, "Drop down has no owner!\n");
		return;
	}

	drop_down->container = NULL;
	drop_down->selection = NULL;
	drop_down->expanded = false;
}
static void init_value_bar_component(void *value_bar_component)
{
	if(!value_bar_component)
		return;

	phos_gui_value_bar_component *value_bar = value_bar_component;

	// get owner
	phos_gui_elem *owner = pluto_cs_get_owner(value_bar);
	if(!owner)
	{
		vl_log(VL_ERROR, "Value bar has no owner!\n");
		return;
	}

	value_bar->min_value = 0.0f;
	value_bar->max_value = 100.0f;
	value_bar->curr_value = 0.0f;
	value_bar->slider_knob_shape = PHOS_GUI_SHAPE_RECT;
	value_bar->slider_knob_corner_radius = 0.0f;
	value_bar->slider_knob_span = phos_gui_get_rect_size(get_calculated_elem_rect(owner, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE)).y * 3.0f;
	value_bar->slider_knob_grab_offset = 0.0f;
	value_bar->slider_knob_color = WHITE;
	value_bar->slider_knob_focus_color = PHOS_GUI_COLOR_GRAY;
	value_bar->progress_color = PHOS_GUI_COLOR_GREEN;
	value_bar->editable = false;
	value_bar->slider_knob_grabbed = false;
	value_bar->slider_knob_released = false;
	value_bar->slider_knob_has_focus = false;

	// re-apply default theme to elem
	phos_gui_apply_theme_to_elem(owner, phos_gui_get_default_theme());
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
	init_map(&icons, 0);
	init_arr(&fonts, 0);

	// keyboard input:
	backspace_timer.key = KEY_BACKSPACE;
	del_timer.key = KEY_DELETE;
	left_arrow_timer.key = KEY_LEFT;
	up_arrow_timer.key = KEY_UP;
	right_arrow_timer.key = KEY_RIGHT;
	down_arrow_timer.key = KEY_DOWN;
	enter_timer.key = KEY_ENTER;
	tab_timer.key = KEY_TAB;

	// register PhosphorusGUI component types
	pluto_cs_register(PHOS_GUI_COMPONENT_SHADOW, sizeof(phos_gui_shadow_component), init_shadow_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_TEXTURE, sizeof(phos_gui_texture_component), init_texture_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_MOUSE_LISTENER, sizeof(phos_gui_mouse_listener_component), init_mouse_listener_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_TEXT, sizeof(phos_gui_text_component), init_text_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT, sizeof(phos_gui_placeholder_text_extension), init_placeholder_text_extension, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_LAYOUT, sizeof(phos_gui_layout_component), init_layout_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_SCROLL_PANE, sizeof(phos_gui_scroll_pane_component), init_scroll_pane_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_DRAG_PANE, sizeof(phos_gui_drag_pane_component), init_drag_pane_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_DROP_DOWN, sizeof(phos_gui_drop_down_component), init_drop_down_component, NULL);
	pluto_cs_register(PHOS_GUI_COMPONENT_VALUE_BAR, sizeof(phos_gui_value_bar_component), init_value_bar_component, NULL);

	// set default theme
	default_theme = PHOS_GUI_THEME_MONOTONE;

	// fill icon map
	phos_gui_set_icon(PHOS_GUI_ICON_DOWN_ARROW, "icons/down_arrow.png");

	// change seed for rand()
	srand((unsigned int) time(NULL));

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

	// after unloading all textures, free icons map
	dynmaps_free(&icons);

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
void phos_gui_exit(int exit_code)
{
	if(exit_code != 0)
		vl_log(VL_ERROR, "Exiting with exit code: %d!\n", exit_code);
	else
		vl_log(VL_INFO, "Exiting with exit code: %d!\n", exit_code);

	pluto_cs_shutdown();
	phos_gui_shutdown();
	CloseWindow();
	exit(exit_code);
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
		arr_add(&all_ids, new_gui->ID);
		arr_add(&gui_registry, new_gui);

		vl_log(VL_SUCCESS, "Registered GUI with ID: '%s'!\n", new_gui->ID);
	}

	// reset 'goto' elem tracker
	curr_travel_elem = NULL;

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

	Vector2 elem_size = phos_gui_get_rect_size(elem->bounds);

	Vector2 container_center = { origin.x + size.x / 2.0f, origin.y + size.y / 2.0f };
	
	Vector2 elem_centered = { container_center.x - elem_size.x / 2.0f, container_center.y - elem_size.y / 2.0f };

	phos_gui_set_elem_bounds(elem, elem_centered.x, elem_centered.y, elem->bounds.width, elem->bounds.height, PHOS_GUI_OPTS_NONE);
}

void phos_gui_move_elem_xy(phos_gui_elem *elem, float x, float y, phos_gui_opts opts)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot move a NULL element!\n");
		return;
	}

	// if moving 0 pixels, exit early
	if(x == 0.0f && y == 0.0f)
		return;

	// get curr elem pos
	Vector2 old_pos = phos_gui_get_rect_pos(elem->bounds);

	// then move elem
	elem->bounds.x += x;
	elem->bounds.y += y;

	// see if element falls within any kind of clip region
	bool in_clip_region = false;
	phos_gui_elem *parent = elem->parent;
	while(parent)
	{
		// a clip region is present if the child or parent are being clipped
		in_clip_region = parent->clipped || elem->clipped;
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
		if(elem->bounds.x <= 0.0f || elem->bounds.y <= 0.0f ||
				elem->bounds.x + elem->bounds.width >= GetRenderWidth() || elem->bounds.y + elem->bounds.height >= GetRenderHeight())
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

				// ensure elem does not collide with self, then check for collision between elems
				if(elem != e && CheckCollisionRecs(elem->bounds, e->bounds))
				{
					collision = true;
					break;
				}
			}
		}
	}

	// if any collision occurred, reset elem pos to old pos:
	if(collision)
	{
		elem->bounds.x = old_pos.x;
		elem->bounds.y = old_pos.y;
	}
	// if no collision occurred on the parent, its children can move
	else
		for(size_t i = 0; i < elem->num_children; ++i)
			phos_gui_move_elem_xy(elem->children[i], x, y, opts);

	// calculate all rects of elem in update loop
	prepare_elem_rects_for_caching(elem);
}

static Vector2 get_proposed_align_pos(Vector2 target_object_size, phos_gui_alignment alignment, phos_gui_elem *reference_elem)
{
	// start at reference_rect origin
	Rectangle whole_rect = get_calculated_elem_rect(reference_elem, PHOS_GUI_ELEM_BOUNDS_TOTAL);
	Rectangle whole_content_rect = get_calculated_elem_rect(reference_elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);

	// keep track of position of the whole space rect
	Vector2 v = phos_gui_get_rect_pos(whole_rect);

	// define bounds
	float outer_left = v.x;
	float outer_top = v.y;
	float outer_right = outer_left + whole_rect.width;
	float outer_bottom = outer_top + whole_rect.height;

	float inner_left = whole_content_rect.x;
	float inner_top = whole_content_rect.y;
	float inner_right = inner_left + whole_content_rect.width;
	float inner_bottom = inner_top + whole_content_rect.height;

	float inner_center_x = inner_left + (whole_content_rect.width - target_object_size.x) / 2.0f;
	float inner_center_y = inner_top + (whole_content_rect.height - target_object_size.y) / 2.0f;

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
	// if resizing by 0 pixels, exit early
	if(w == 0.0f && h == 0.0f)
		return;

	// first see what new elem size would be
	float new_w = elem->bounds.width + w;
	float new_h = elem->bounds.height + h;

	// see if size is now negative
	if(new_w <= 0.0f || new_h <= 0.0f)
	{
		vl_log(VL_ERROR, "Cannot shrink this element ('%s') anymore, its size cannot be <= 0.0f!\n", elem->ID);
		return;
	}

	// apply new size
	elem->bounds.width = new_w;
	elem->bounds.height = new_h;

	// force new bounds update
	force_calculate_elem_rects(elem);

	phos_gui_text_component *elem_tx = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
	if(elem_tx)
	{
		// based on given options, modify text component:

		// fit text first
		if(opts & PHOS_GUI_OPTS_FIT_TEXT)
			phos_gui_make_text_fit_elem(elem_tx, PHOS_GUI_TARGET_AUTO_TEXT);

		// then realign text no matter what
		phos_gui_realign_elem_text(elem_tx);
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
		for(size_t i = 0; i < elem->num_children; ++i)
		{
			phos_gui_elem *child = elem->children[i];

			// resize child same amount of pixels as parent
			phos_gui_resize_elem_wh(child, w, h, opts);

			// but always realign text
			phos_gui_text_component *child_text = pluto_cs_get_component(child, PHOS_GUI_COMPONENT_TEXT);
			if(child_text)
				phos_gui_realign_elem_text(child_text);
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
	v.x += elem->bounds.width / 2.0f;
	v.y += elem->bounds.height / 2.0f;

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
Rectangle phos_gui_get_elem_rect(phos_gui_elem *elem, phos_gui_elem_bounding_box bounds)
{
	return get_calculated_elem_rect(elem, bounds);
}
void phos_gui_reload_elem(phos_gui_elem *elem)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot reload a NULL element!\n");
		return;
	}

	// finally, update elem rectangles
	force_calculate_elem_rects(elem);
}
void phos_gui_reload_gui(phos_gui *gui)
{
	// reload all parent and child elems
	for(size_t i = 0; i < gui->num_elems; ++i)
	{
		phos_gui_elem *elem = gui->elems[i];

		for(size_t j = 0; j < elem->num_children; ++j)
		{
			phos_gui_elem *child = elem->children[j];

			phos_gui_reload_elem(child);
		}

		phos_gui_reload_elem(elem);
	}
}

static Vector2 get_text_draw_pos(const phos_gui_text_component *const text, const phos_gui_scroll_pane_component *const scroll_pane)
{
	// get initial pos of text and owner:
	phos_gui_elem *owner = pluto_cs_get_owner(text);
	if(!owner)
		return Vector2Zero();

	// add text offset
	Vector2 text_elem_pos = phos_gui_get_rect_pos(get_calculated_elem_rect(owner, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE));
	Vector2 text_pos = Vector2Add(text_elem_pos, text->offset);

	// calculate where to draw the text based on scrolling (only if scroll pane is not null)
	if(scroll_pane)
	{
		if(scroll_pane->h_bar.active)
			text_pos.x -= scroll_pane->scroll_x;
		if(scroll_pane->v_bar.active)
			text_pos.y -= scroll_pane->scroll_y;
	}

	return text_pos;
}
void phos_gui_get_text_bounds(const phos_gui_text_component *const text_component, Rectangle *out_main_bounds, Rectangle *out_placeholder_bounds)
{
	if(!out_main_bounds && !out_placeholder_bounds)
		return;

	// get owner of text component
	const phos_gui_elem *const elem = pluto_cs_get_owner(text_component);
	if(!elem)
	{
		vl_delay_log(VL_ERROR, 2.0f, "Text component is missing an owner! Cannot obtain bounds.\n");
		return;
	}

	phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_SCROLL_PANE);

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

	// point to the destination string, starting out with main string on the text component
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

		// get text draw pos:
		Vector2 text_pos = get_text_draw_pos(text_component, scroll_pane);
		main_bounds.x = text_pos.x;
		main_bounds.y = text_pos.y;

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

		// get text draw pos:
		Vector2 text_pos = get_text_draw_pos(text_component, scroll_pane);
		placeholder_bounds.x = text_pos.x;
		placeholder_bounds.y = text_pos.y;

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

static Vector2 get_cursor_draw_pos(const phos_gui_text_component *const text, const phos_gui_scroll_pane_component *const scroll_pane)
{
	const Vector2 text_pos = get_text_draw_pos(text, scroll_pane);

	char buf[PHOS_GUI_MAX_TEXT_LEN + 1];
	memcpy(buf, text->str, text->cursor_pos);
	buf[text->cursor_pos] = '\0';

	const char *curr_line = strrchr(buf, '\n');

	// skip '\n' character
	if(curr_line)
		curr_line++;
	else
		// use entire buffer
		curr_line = buf;

	Vector2 caret_size = MeasureTextEx(*text->font, curr_line, text->font_size, 0.0f);
	float line_height = text->font_size;

	Vector2 cursor_pos = { text_pos.x + caret_size.x, text_pos.y };
	if(curr_line != buf)
	{
		size_t line_count = 1;

		for(const char *p = buf; p < curr_line; ++p)
		{
			if(*p == '\n')
				line_count++;
		}

		cursor_pos.y += line_height * (line_count - 1);
	}

	return cursor_pos;
}
static void update_text_scrolling(phos_gui_text_component *text)
{
	// get owner of text component
	phos_gui_elem *text_elem = pluto_cs_get_owner(text);
	if(!text_elem)
	{
		vl_delay_log(VL_WARNING, 5.0f, "Failed to update text scrolling. Text component's owner is invalid!\n");
		return;
	}

	// if the elem does not own a scroll pane, then the text cannot be scrolled
	phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(text_elem, PHOS_GUI_COMPONENT_SCROLL_PANE);
	if(!scroll_pane)
		return;

	// if no characters in text, reset all scrolling
	if(text->len == 0)
	{
		scroll_pane->max_scroll_x = scroll_pane->max_scroll_y = scroll_pane->scroll_x = scroll_pane->scroll_y = 0.0f;
		return;
	}

	// text always resides in the element's free content space:
	Rectangle free_content_bounds = get_calculated_elem_rect(text_elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

	// get bounds of text
	Rectangle text_bounds;
	phos_gui_get_text_bounds(text, &text_bounds, NULL);

	// calculate the overflow on each axis
	float vis_left = free_content_bounds.x;
	float vis_right = free_content_bounds.x + free_content_bounds.width - (CURSOR_WIDTH * 2.0f);
	float vis_width = vis_right - (free_content_bounds.x + text->offset.x);
	float overflow_w = text_bounds.width - vis_width;

	float vis_top = free_content_bounds.y;
	float vis_bottom = free_content_bounds.y + free_content_bounds.height;
	float vis_height = vis_bottom - (free_content_bounds.y + text->offset.y);
	float overflow_h = text_bounds.height - vis_height;

	if(overflow_w > 0.0f)
		scroll_pane->max_scroll_x = overflow_w;
	else
	{
		scroll_pane->max_scroll_x = 0.0f;
		scroll_pane->scroll_x = 0.0f;
	}

	if(overflow_h > 0.0f)
		scroll_pane->max_scroll_y = overflow_h;
	else
	{
		scroll_pane->max_scroll_y = 0.0f;
		scroll_pane->scroll_y = 0.0f;
	}

	// caret logic:

	// copy typed string into temp buffer
	char buf[PHOS_GUI_MAX_TEXT_LEN + 1];
	memcpy(buf, text->str, text->cursor_pos);
	buf[text->cursor_pos] = '\0';

	Vector2 cursor_pos = get_cursor_draw_pos(text, scroll_pane);

	// right-side check (include cursor width because the cursor takes up that many more pixels)
	if(cursor_pos.x + (CURSOR_WIDTH * 2.0f) >= vis_right)
		scroll_pane->scroll_x += (cursor_pos.x + (CURSOR_WIDTH * 2.0f)) - vis_right;

	// left-side check (don't include cursor width)
	if(cursor_pos.x <= vis_left)
		scroll_pane->scroll_x -= vis_left - cursor_pos.x;

	// bottom-side check
	if(cursor_pos.y + text->font_size >= vis_bottom)
		scroll_pane->scroll_y += (cursor_pos.y + text->font_size) - vis_bottom;

	// top-side check
	if(cursor_pos.y <= vis_top)
		scroll_pane->scroll_y -= vis_top - cursor_pos.y;

	scroll_pane->scroll_x = Clamp(scroll_pane->scroll_x, 0.0f, scroll_pane->max_scroll_x);
	scroll_pane->scroll_y = Clamp(scroll_pane->scroll_y, 0.0f, scroll_pane->max_scroll_y);
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

	snprintf(text->str, sizeof(text->str), "%s", str);
	text->len = strlen(str);
	text->max_len = PHOS_GUI_MAX_TEXT_LEN;
	text->color = color;
	text->font_size = font_size;
	text->editable = false;
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
	float x_diff = x - elem->bounds.x;
	float y_diff = y - elem->bounds.y;

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
	float x_diff = w - elem->bounds.width;
	float y_diff = h - elem->bounds.height;

	// resize elem and fix text
	phos_gui_resize_elem_wh(elem, x_diff, y_diff, opts);
}
void phos_gui_set_elem_bounds(phos_gui_elem *elem, float x, float y, float w, float h, phos_gui_opts opts)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set bounds on a NULL element!\n");
		return;
	}

	phos_gui_set_elem_pos(elem, x, y, opts);
	phos_gui_set_elem_size(elem, w, h, opts);
}
void phos_gui_set_elem_bounds_r(phos_gui_elem *elem, Rectangle r, phos_gui_opts opts)
{
	phos_gui_set_elem_bounds(elem, r.x, r.y, r.width, r.height, opts);
}
void phos_gui_set_elem_paddings(phos_gui_elem *elem, float left, float top, float right, float bottom)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set padding on a NULL element!\n");
		return;
	}

	elem->left_padding = left;
	elem->top_padding = top;
	elem->right_padding = right;
	elem->bottom_padding = bottom;

	// force calculation of new rects
	force_calculate_elem_rects(elem);
}
void phos_gui_set_elem_padding(phos_gui_elem *elem, float padding)
{
	phos_gui_set_elem_paddings(elem, padding, padding, padding, padding);
}
void phos_gui_add_elem_paddings(phos_gui_elem *elem, float left, float top, float right, float bottom)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot add to padding on a NULL element!\n");
		return;
	}

	phos_gui_set_elem_paddings(elem, elem->left_padding + left, elem->top_padding + top, elem->right_padding + right, elem->bottom_padding + bottom);
}
void phos_gui_add_elem_padding(phos_gui_elem *elem, float padding)
{
	phos_gui_add_elem_paddings(elem, padding, padding, padding, padding);
}
void phos_gui_set_elem_margins(phos_gui_elem *elem, float left, float top, float right, float bottom)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set margin on a NULL element!\n");
		return;
	}

	elem->left_margin = left;
	elem->top_margin = top;
	elem->right_margin = right;
	elem->bottom_margin = bottom;

	// force calculation of new rects
	force_calculate_elem_rects(elem);
}
void phos_gui_set_elem_margin(phos_gui_elem *elem, float margin)
{
	phos_gui_set_elem_margins(elem, margin, margin, margin, margin);
}
void phos_gui_add_elem_margins(phos_gui_elem *elem, float left, float top, float right, float bottom)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot add to margin on a NULL element!\n");
		return;
	}

	phos_gui_set_elem_margins(elem, elem->left_margin + left, elem->top_margin + top, elem->right_margin + right, elem->bottom_margin + bottom);
}
void phos_gui_add_elem_margin(phos_gui_elem *elem, float margin)
{
	phos_gui_add_elem_margins(elem, margin, margin, margin, margin);
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

	// get owner of text component
	phos_gui_elem *owner = pluto_cs_get_owner(text_component);
	if(!owner)
	{
		vl_log(VL_ERROR, "Cannot use PHOS_GUI_TARGET_PLACEHOLDER_TEXT, the text component's owner is NULL!\n");
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

	// only modify cursor pos if targeting main string
	if(dest == text_component->str)
	{
		text_component->cursor_pos = text_component->len = strlen(dest);

		// curr line len is length of last string before a '\n' is encountered
		size_t last_line_len = 0;
		for(size_t i = text_component->len; i > 0 && dest[i - 1] != '\n'; --i)
			last_line_len++;
		text_component->curr_line_len = last_line_len;

		// num lines becomes number of '\n's found
		text_component->num_lines = text_component->len > 0 ? 1 : 0; // reset line count first
		for(size_t i = 0; i < text_component->len; ++i)
			if(text_component->str[i] == '\n')
				text_component->num_lines++;
	}

	// check opts
	if(opts & PHOS_GUI_OPTS_FIT_TEXT)
		phos_gui_make_text_fit_elem(text_component, target_str);
	// realign text no matter what
	phos_gui_realign_elem_text(text_component);
}

/*
   IMPORTANT: even though the owner of the text component could be obtained, and the 'elem' argument could be taken out,
   other functions rely on copies of text components
*/
static Vector2 resolve_elem_text_bounds(const phos_gui_text_component *const text_component, phos_gui_target_text_string target_str)
{
	// get owner of text component
	const phos_gui_elem *const elem = pluto_cs_get_owner(text_component);
	if(!elem)
		return Vector2Zero();

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
			else if(placeholder_text && placeholder_len > main_len)
				text_bounds = MeasureTextEx(*text_component->font, placeholder_text->str, text_component->font_size, 0.0f);
			else
				vl_log(VL_WARNING, "PHOS_GUI_TARGET_AUTO_TEXT failed for element: '%s'!\n", elem->ID);
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

	// position text component relative to owner
	text_component->offset = Vector2Subtract(v, phos_gui_get_rect_pos(get_calculated_elem_rect(owner, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE)));
	text_component->alignment = alignment;

	return v;
}
Vector2 phos_gui_realign_elem_text(phos_gui_text_component *text_component)
{
	if(!text_component)
	{
		vl_delay_log(VL_ERROR, 2.0f, "Cannot realign NULL text component!\n");
		return Vector2Zero();
	}
	
	return phos_gui_align_elem_text(text_component, PHOS_GUI_TARGET_AUTO_TEXT, text_component->alignment);
}
Vector2 phos_gui_align_elem(phos_gui_elem *target_elem, phos_gui_alignment alignment, phos_gui_elem *reference_elem, phos_gui_opts opts)
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
	v = get_proposed_align_pos(phos_gui_get_rect_size(get_calculated_elem_rect(target_elem, PHOS_GUI_ELEM_BOUNDS_TOTAL)), target_elem->alignment, reference_elem);
	phos_gui_set_elem_pos(target_elem, v.x, v.y, opts);

	return v;
}

static bool elem_in_bounds(phos_gui_elem *elem, Vector2 origin, Vector2 size)
{
	// get elem rect
	Rectangle r = get_calculated_elem_rect(elem, PHOS_GUI_ELEM_BOUNDS_TOTAL);

	// see if either rect is out of bounds (origin + size)
	if(r.x < origin.x || r.x + r.width > origin.x + size.x ||
			r.y < origin.y || r.y + r.height > origin.y + size.y)
	{
		vl_log(VL_ERROR, "Element '%s' is out of the bounds: [%.2f, %.2f, %.2f, %.2f]!\n", elem->ID, origin.x, origin.y, size.x, size.y);
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
		alignment = PHOS_GUI_ALIGN_INNER_TOP_LEFT;
	}

	target_elem->alignment = alignment;

	// create temp elem representing the window
	phos_gui_elem temp = {0};
	phos_gui_set_elem_bounds(&temp, 0, 0, GetRenderWidth(), GetRenderHeight(), opts);
	force_calculate_elem_rects(&temp); // force temp elem to have synced bounds
	v = get_proposed_align_pos(phos_gui_get_rect_size(temp.bounds), target_elem->alignment, &temp);
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
void phos_gui_fill_elem_with_elem(phos_gui_elem *reference_elem, phos_gui_elem_bounding_box bounds, phos_gui_elem *target_elem, phos_gui_opts opts)
{
	if(!target_elem || !reference_elem)
	{
		vl_log(VL_ERROR, "To fill an element with another element, both elements must be valid pointers!\n");
		return;
	}

	// match bounds on element to the given bounding box
	force_calculate_elem_rects(reference_elem); // but first the reference elem's bounds should be finalized
	phos_gui_set_elem_bounds_r(target_elem, get_calculated_elem_rect(reference_elem, bounds), opts);
}

static void use_largest_possible_font_size(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, Rectangle rect)
{
	// get content area
	Vector2 size = phos_gui_get_rect_size(rect);

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
	phos_gui_elem *elem = pluto_cs_get_owner(text_component);
	if(!elem)
	{
		vl_log(VL_ERROR, "To make the given text component fit its owner, the text component must have a valid owner element!\n");
		return;
	}

	// use element's free content bounds rect
	use_largest_possible_font_size(text_component, target_str, get_calculated_elem_rect(elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE));
}
void phos_gui_make_text_fit_rect(phos_gui_text_component *text_component, phos_gui_target_text_string target_str, Rectangle rect)
{
	if(!text_component)
	{
		vl_log(VL_ERROR, "To make the given text component fit the rectangle, the text component cannot be NULL!\n");
		return;
	}
	if(!text_component->font)
	{
		vl_log(VL_ERROR, "To make the given text component fit the rectangle, its font must be set first!\n");
		return;
	}

	// use the rectangle the user gave
	use_largest_possible_font_size(text_component, target_str, rect);
}

void phos_gui_init_elem(phos_gui_elem *elem, const char *ID, phos_gui_elem_type type, phos_gui_elem_render_mode render_mode, float x, float y, float w, float h)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot initialize a NULL element!\n");
		return;
	}

	// write elem ID first
	phos_gui_write_str(elem->ID, "%s", ID);

	Rectangle bounds = elem->bounds = (Rectangle) { x, y, w, h };
	elem->total_bounds.rect = bounds;
	elem->content_total_bounds.rect = bounds;
	elem->content_free_bounds.rect = bounds;
	elem->gui = NULL;
	elem->parent = NULL;
	elem->num_children = 0;
	elem->type = type;
	elem->shape = PHOS_GUI_SHAPE_RECT;
	elem->render_mode = render_mode;
	elem->alignment = PHOS_GUI_ALIGN_INNER_CENTER;
	elem->child_opts = PHOS_GUI_OPTS_NONE;
	elem->input_test_bounds = PHOS_GUI_ELEM_BOUNDS_REAL;
	elem->bg_color = WHITE;
	elem->outline_color = PHOS_GUI_COLOR_BLACK;
	elem->disabled_color = PHOS_GUI_COLOR_LIGHT_GRAY;
	elem->outline_thickness = 1.0f;
	elem->corner_radius = 0.0f;
	elem->left_padding = elem->top_padding = elem->right_padding = elem->bottom_padding = 0.0f;
	elem->left_margin = elem->top_margin = elem->right_margin = elem->bottom_margin = 0.0f;
	elem->disabled = false;
	elem->auto_render = true;
	elem->clipped = false;

	// apply default theme to element
	phos_gui_apply_theme_to_elem(elem, phos_gui_get_default_theme());

	prepare_elem_rects_for_caching(elem);
}
void phos_gui_init_button(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, const char *text)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot initialize a NULL element!\n");
		return;
	}

	// init element's basic attributes first
	phos_gui_init_elem(elem, ID, PHOS_GUI_TYPE_INTERACTIVE, PHOS_GUI_RENDER_FILL_OUTLINE, x, y, w, h);

	// create text component
	phos_gui_text_component *text_component = pluto_cs_add_component(elem, PHOS_GUI_COMPONENT_TEXT);
	if(!text_component)
		phos_gui_exit(EXIT_FAILURE);
	phos_gui_set_text_contents(text_component, PHOS_GUI_TARGET_MAIN_TEXT, text, PHOS_GUI_OPTS_FIT_TEXT);

	// create mouse listener component
	phos_gui_mouse_listener_component *mouse_listener = pluto_cs_add_component(elem, PHOS_GUI_COMPONENT_MOUSE_LISTENER);
	if(!mouse_listener)
		phos_gui_exit(EXIT_FAILURE);
}
void phos_gui_init_text_field(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, const char *main_text, const char *placeholder_text)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot initialize a NULL element!\n");
		return;
	}

	// first, init elem as a button
	phos_gui_init_button(elem, ID, x, y, w, h, main_text);

	// now obtain text component and modify it so that element becomes a text field
	phos_gui_text_component *text = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
	text->editable = true;

	// add placeholder text component
	phos_gui_placeholder_text_extension *placeholder_text_component = pluto_cs_add_component(elem, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT);
	if(!placeholder_text_component)
		phos_gui_exit(EXIT_FAILURE);
	phos_gui_set_text_contents(text, PHOS_GUI_TARGET_PLACEHOLDER_TEXT, placeholder_text, PHOS_GUI_OPTS_NONE);

	// make placeholder text fit elem if necessary
	if(strlen(placeholder_text) > 0)
		phos_gui_make_text_fit_elem(text, PHOS_GUI_TARGET_PLACEHOLDER_TEXT);

	// add scroll pane component TO THE TEXT CONTAINER so text scrolls as user types
	phos_gui_scroll_pane_component *scroll_pane = pluto_cs_add_component(elem, PHOS_GUI_COMPONENT_SCROLL_PANE);
	if(!scroll_pane)
		phos_gui_exit(EXIT_FAILURE);
	// text only scrolls horizontally by default:
	scroll_pane->v_bar.active = false;
	// neither scroll bar is rendered
	scroll_pane->v_bar.rendered = false;
	scroll_pane->h_bar.rendered = false;
	// mouse wheel cannot scroll the text
	scroll_pane->use_mouse_wheel_input = false;

	// change alignment and realign
	text->alignment = PHOS_GUI_ALIGN_INNER_LEFT;
	phos_gui_realign_elem_text(text);
}
void phos_gui_init_text_area(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, const char *main_text, const char *placeholder_text, phos_gui_text_wrap_mode wrap_mode)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot initialize a NULL element!\n");
		return;
	}

	// first, init elem as text field
	phos_gui_init_text_field(elem, ID, x, y, w, h, main_text, placeholder_text);

	// check which wrap mode the user gave:
	if(wrap_mode == PHOS_GUI_TEXT_WRAP_NONE)
	{
		// keep scroll pane component:

		// modify scroll pane to work for multiple lines
		phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_SCROLL_PANE);
		scroll_pane->h_bar.active = true;
		scroll_pane->h_bar.rendered = true;
		scroll_pane->v_bar.active = true;
		scroll_pane->v_bar.rendered = true;
	}
	else
	{
		// no need for scroll pane component since text wraps
		pluto_cs_remove_component(elem, PHOS_GUI_COMPONENT_SCROLL_PANE);
	}

	// set wrap mode of text component
	phos_gui_text_component *text = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
	text->wrap_mode = wrap_mode;
	text->enter_inserts_new_line = true;

	// start text at top left of text area and go back to default font size
	text->alignment = PHOS_GUI_ALIGN_INNER_TOP_LEFT;
	text->font_size = PHOS_GUI_FONT_SIZE_MED;
	phos_gui_realign_elem_text(text);
}
void phos_gui_init_drop_down(phos_gui_elem *elem, const char *ID, float x, float y, float w, float h, phos_gui_elem *container_elem, const char *text)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot initialize a NULL element!\n");
		return;
	}
	if(!container_elem)
	{
		vl_log(VL_ERROR, "Cannot initialize a drop down with a NULL container element!\n");
		return;
	}

	// first init elem as a button
	phos_gui_init_button(elem, ID, x, y, w, h, text);

	// add drop down component
	phos_gui_drop_down_component *drop_down = pluto_cs_add_component(elem, PHOS_GUI_COMPONENT_DROP_DOWN);
	if(!drop_down)
		phos_gui_exit(EXIT_FAILURE);
	// container element becomes a child to elem
	phos_gui_add_child_to_elem(container_elem, elem, PHOS_GUI_OPTS_NONE);
	// and point to the container elem in the drop down
	drop_down->container = container_elem;

	// re-align text to inner left
	phos_gui_text_component *text_component = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
	text_component->alignment = PHOS_GUI_ALIGN_INNER_LEFT;
	phos_gui_realign_elem_text(text_component);

	// move container to elem pos
	phos_gui_set_elem_pos(container_elem, elem->bounds.x, elem->bounds.y + elem->bounds.height, PHOS_GUI_OPTS_NONE);
}

void phos_gui_gen_bg_colors(phos_gui_mouse_listener_component *mouse_listener, float hover_color_factor, float press_color_factor, float focus_color_factor)
{
	if(!mouse_listener)
	{
		vl_log(VL_ERROR, "Cannot generate colors on a null mouse listener component!\n");
		return;
	}

	phos_gui_elem *owner = pluto_cs_get_owner(mouse_listener);
	if(!owner)
	{
		vl_log(VL_ERROR, "Generating colors requires the mouse listener to have a valid owner!\n");
		return;
	}

	mouse_listener->bg_hover_color = ColorBrightness(owner->bg_color, hover_color_factor);
	mouse_listener->bg_press_color = ColorBrightness(owner->bg_color, press_color_factor);
	mouse_listener->bg_focus_color = ColorBrightness(owner->bg_color, focus_color_factor);
}
void phos_gui_gen_outline_colors(phos_gui_mouse_listener_component *mouse_listener, float hover_color_factor, float press_color_factor, float focus_color_factor)
{
	if(!mouse_listener)
	{
		vl_log(VL_ERROR, "Cannot generate colors on a null mouse listener component!\n");
		return;
	}

	phos_gui_elem *owner = pluto_cs_get_owner(mouse_listener);
	if(!owner)
	{
		vl_log(VL_ERROR, "Generating colors requires the mouse listener to have a valid owner!\n");
		return;
	}

	mouse_listener->outline_hover_color = ColorBrightness(owner->outline_color, hover_color_factor);
	mouse_listener->outline_press_color = ColorBrightness(owner->outline_color, press_color_factor);
	mouse_listener->outline_focus_color = ColorBrightness(owner->outline_color, focus_color_factor);
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
	arr_add(&all_ids, elem->ID, 0);
	arr_add(&elem_registry, elem, 0);

	vl_log(VL_SUCCESS, "Registered element with ID: '%s'!\n", elem->ID);

	return 1;
}

int phos_gui_add_elem_to_gui(phos_gui_elem *elem, phos_gui *gui)
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

	// force calculate rectangles around elem
	force_calculate_elem_rects(elem);
	prepare_elem_rects_for_caching(elem);

	vl_log(VL_SUCCESS, "Added element '%s' to GUI '%s'!\n", elem->ID, gui->ID);

	return 1;
}
int phos_gui_add_elem_to_gui_id(const char *ID, phos_gui* gui)
{
	return phos_gui_add_elem_to_gui(phos_gui_get_elem(ID), gui);
}
int phos_gui_remove_elem_from_gui(phos_gui_elem *elem, phos_gui *gui)
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
			memmove(gui->elems[i], gui->elems[i + 1], (gui->num_elems - i - 1) * sizeof(phos_gui_elem*));
			gui->num_elems--;

			vl_log(VL_SUCCESS, "Removed element '%s' from GUI '%s'!\n", e->ID, gui->ID);
			return 1;
		}
	}

	// no match found
	vl_log(VL_ERROR, "Failed to remove this element ('%s') from the given phos_gui ('%s')!\n", elem->ID, gui->ID);
	return 0;
}
int phos_gui_remove_elem_from_gui_id(const char *ID, phos_gui *gui)
{
	return phos_gui_remove_elem_from_gui(phos_gui_get_elem(ID), gui);
}
int phos_gui_add_child_to_elem(phos_gui_elem *child, phos_gui_elem *parent, phos_gui_opts child_opts)
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

	// handle child opts:
	if(child_opts & PHOS_GUI_OPTS_CHILD_MAXIMIZED)
	{
		phos_gui_resize_elem_wh(child, parent->bounds.width, parent->bounds.height, child_opts);

		// remove maximized option
		child_opts &= ~PHOS_GUI_OPTS_CHILD_MAXIMIZED;
	}
	if(child_opts & PHOS_GUI_OPTS_CHILD_HAS_RELATIVE_POS)
	{
		// calculate child's real pos
		float child_x = child->bounds.x + parent->bounds.x;
		float child_y = child->bounds.y + parent->bounds.y;
		phos_gui_set_elem_pos(child, child_x, child_y, child_opts);

		// remove relative pos option
		child_opts &= ~PHOS_GUI_OPTS_CHILD_HAS_RELATIVE_POS;
	}

	// save child opts
	child->child_opts = child_opts;

	// force calculate rectangles around child
	force_calculate_elem_rects(child);
	prepare_elem_rects_for_caching(child);

	// apply theme to child
	phos_gui_apply_theme_to_elem(child, phos_gui_get_default_theme());

	vl_log(VL_SUCCESS, "Element '%s' added to parent element '%s'!\n", child->ID, parent->ID);

	return 1;
}
int phos_gui_add_child_to_elem_id(const char *ID, phos_gui_elem *parent, phos_gui_opts child_opts)
{
	return phos_gui_add_child_to_elem(phos_gui_get_elem(ID), parent, child_opts);
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
			memmove(parent->children + i, parent->children + i + 1, (parent->num_children - i - 1) * sizeof(phos_gui_elem*));
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

	// obtain parent rects (force calculation first)
	force_calculate_elem_rects(parent);
	const Rectangle parent_content_area = get_calculated_elem_rect(parent, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);
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
		float child_w, child_h, cell_h;

		if(layout->auto_fit_children)
		{
			// force child to fit into cells
			cell_h = (parent_content_area.height - total_spacing_y) / layout->rows;
			child_h = cell_h - child->top_margin - child->bottom_margin;
			child_w = cell_w - child->left_margin - child->right_margin;

			phos_gui_set_elem_size(child, child_w, child_h, opts);
		}
		else
		{
			// retain child height
			child_h = child->bounds.height;
			cell_h = child_h + child->top_margin + child->bottom_margin;
			child_w = child->bounds.width + child->left_margin + child->right_margin;

			// but if necessary, force child to fit horizontally (ex. used when scroll bars are rendered)
			// TODO is this still needed?
			/*if(child->size.x > child_w)

				phos_gui_set_elem_size(child, child_w, child_h, opts);*/
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
		Rectangle outer = parent->bounds;

		float extra_width = outer.width - parent_content_area.width;
		float extra_height = outer.height - parent_content_area.height;

		phos_gui_set_elem_size(parent, layout->total_content_width + extra_width, layout->total_content_height + extra_height, PHOS_GUI_OPTS_NONE);
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
	arr_add(&blueprint_registry, new_bp, 0);
	arr_add(&all_ids, blueprint_registry.data[blueprint_registry.size - 1].ID, 0);

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
			const phos_gui_elem *const child = bp->elem->children[i];

			// clone the child
			phos_gui_elem *child_clone = target_elem->children[i];
			phos_gui_write_str(child_clone->ID, "auto");
			auto_gen_id("auto", child_clone->ID, sizeof(child_clone->ID), "elem", &elem_auto_id);

			// clone elements from child onto child_clone
			if(pluto_cs_clone_all_components(child, child_clone) == 0)
				vl_log(VL_ERROR, "Failed to clone child components!\n");

			// add child to target_elem (use same child options from cloned elem)
			phos_gui_add_child_to_elem(child_clone, target_elem, child->child_opts);
		}
	}
	else
		// when the children should not be cloned, reset the element's child count to 0
		target_elem->num_children = 0;

	// clone elements from bp->elem onto target_elem
	if(pluto_cs_clone_all_components(bp->elem, target_elem) == 0)
		vl_log(VL_ERROR, "Failed to initialize a cloned element! Cloning components failed! Source element: '%s', target element: '%s'\n", bp->elem->ID, target_elem->ID);
}

int phos_gui_init_window(const char *title, int width, int height, unsigned int flags)
{
	if(!init)
	{
		vl_log(VL_ERROR, "Cannot init window because PhosphorusGUI was never initialized!\n");
		return 0;
	}

	// init window with the given flags
	SetConfigFlags(flags);
	InitWindow(width, height, title);

	int curr_monitor = GetCurrentMonitor();
	SetTargetFPS(GetMonitorRefreshRate(curr_monitor));

	SetExitKey(KEY_NULL);

	return 1;
}
void phos_gui_set_window_scale(float x, float y)
{
	win_scale_x = x;
	win_scale_y = y;
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
phos_gui_elem *phos_gui_get_mouse_target()
{
	return mouse_target;
}

int phos_gui_add_event_listener(phos_gui *gui, phos_gui_event_listener listener)
{
	if(!gui)
	{
		vl_log(VL_ERROR, "To add an event listener, the phos_gui cannot be NULL!\n");
		return 0;
	}

	// add the listener to the gui
	if(gui->num_listeners >= PHOS_GUI_MAX_EVENT_LISTENERS)
	{
		vl_log(VL_WARNING, "No more event listeners can be added to this phos_gui: '%s'!\n", gui->ID);
		return 0;
	}
	gui->listeners[gui->num_listeners++] = listener;

	return 1;
}
int phos_gui_new_event_listener(phos_gui *gui, phos_gui_elem *target_elem, phos_gui_event_type event, int target_button, phos_gui_opts opts, phos_gui_event_listener_action action)
{
	phos_gui_event_listener listener = {0};
	listener.elem = target_elem;
	listener.event = event;
	listener.target_btn = target_button;
	listener.opts = opts;
	listener.action = action;

	return phos_gui_add_event_listener(gui, listener);
}
int phos_gui_add_timer(phos_gui *gui, phos_gui_timer timer)
{
	if(!gui)
	{
		vl_log(VL_ERROR, "To add a timer, the phos_gui cannot be NULL!\n");
		return 0;
	}

	// add the timer to the gui
	if(gui->num_timers >= PHOS_GUI_MAX_TIMERS)
	{
		vl_log(VL_WARNING, "No more timers can be added to this phos_gui: '%s'!\n", gui->ID);
		return 0;
	}
	gui->timers[gui->num_timers++] = timer;

	return 1;
}
int phos_gui_new_timer(phos_gui *gui, phos_gui_timer_action action, void *args, float target_time, int execution_count)
{
	phos_gui_timer timer = {0};
	timer.target_time = target_time;
	timer.execution_count = execution_count;
	timer.action = action;
	timer.args = args;

	return phos_gui_add_timer(gui, timer);
}

static void backspace(phos_gui_text_component *t)
{
	if(t->len > 0 && t->cursor_pos > 0)
	{
		// see if user deleted a new line char
		char curr_c = t->str[t->cursor_pos - 1];
		if(curr_c == '\n')
		{
			size_t num_deletes = 1;

			// when wrapping by char, both the '\n' and previous char have to be deleted
			if(t->wrap_mode == PHOS_GUI_TEXT_WRAP_CHAR)
				num_deletes = 2;

			// move all characters after cursor two to the left (one for the character being deleted, as well as the '\n' char)
			memmove(t->str + t->cursor_pos - num_deletes, t->str + t->cursor_pos, t->len - t->cursor_pos + 1);
			t->num_lines--;
			t->len -= num_deletes;

			// find length of new current line
			t->curr_line_len = 0;
			for(int i = t->len - 1; i >= 0 && t->str[i] != '\n'; --i)
				t->curr_line_len++;

			t->cursor_pos -= num_deletes;
		}
		else
		{
			memmove(t->str + t->cursor_pos - 1, t->str + t->cursor_pos, t->len - t->cursor_pos + 1);
			t->len--;
			t->curr_line_len--;
			t->cursor_pos--;
		}

		t->edited = true;

		// fit and realign text based on edit_opts
		if(t->edit_opts & PHOS_GUI_OPTS_FIT_TEXT)
			phos_gui_make_text_fit_elem(t, PHOS_GUI_TARGET_AUTO_TEXT);
		if(t->edit_opts & PHOS_GUI_OPTS_REALIGN_TEXT)
			phos_gui_realign_elem_text(t);
	}
}
static void delete(phos_gui_text_component *t)
{
	if(t->len > 0 && t->cursor_pos < t->len)
	{
		// see if user deleted a new line char
		char curr_c = t->str[t->cursor_pos - 1];
		if(curr_c == '\n')
		{
			// move all characters after the cursor back one
			memmove(t->str + t->cursor_pos, t->str + t->cursor_pos + 1, t->len - t->cursor_pos);
			t->num_lines--;
			t->len--;

			// find length of new current line
			t->curr_line_len = 0;
			for(int i = t->len - 1; i >= 0 && t->str[i] != '\n'; --i)
				t->curr_line_len++;
		}
		else
		{
			// move all characters after the cursor back one
			memmove(t->str + t->cursor_pos, t->str + t->cursor_pos + 1, t->len - t->cursor_pos);
			t->len--;
			t->curr_line_len--;
		}

		t->edited = true;

		// fit and realign text based on edit_opts
		if(t->edit_opts & PHOS_GUI_OPTS_FIT_TEXT)
			phos_gui_make_text_fit_elem(t, PHOS_GUI_TARGET_AUTO_TEXT);
		if(t->edit_opts & PHOS_GUI_OPTS_REALIGN_TEXT)
			phos_gui_realign_elem_text(t);
	}
}
static void move_cursor_left(phos_gui_text_component *t)
{
	if(t->cursor_pos > 0)
	{
		t->cursor_pos--;
		update_text_scrolling(t);
	}
}
static void move_cursor_up(phos_gui_text_component *t)
{
	if(t->cursor_pos > 0 && t->num_lines > 1)
	{
		size_t line_start = t->cursor_pos;

		while(line_start > 0 && t->str[line_start - 1] != '\n')
			line_start--;

		size_t curr_col = t->cursor_pos - line_start;

		if(line_start == 0)
			return;

		size_t prev_line_end = line_start - 1;
		size_t prev_line_start = prev_line_end;

		while(prev_line_start > 0 && t->str[prev_line_start - 1] != '\n')
			prev_line_start--;

		size_t prev_line_len = prev_line_end - prev_line_start;

		if(curr_col > prev_line_len)
			curr_col = prev_line_len;

		t->cursor_pos = prev_line_start + curr_col;

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
static void move_cursor_down(phos_gui_text_component *t)
{
	if(t->cursor_pos < t->len && t->num_lines > 1)
	{
		size_t line_start = t->cursor_pos;

		while(line_start > 0 && t->str[line_start - 1] != '\n')
			line_start--;

		size_t curr_col = t->cursor_pos - line_start;

		size_t line_end = t->cursor_pos;

		while(line_end < t->len && t->str[line_end] != '\n')
			line_end++;

		if(line_end >= t->len)
			return;

		size_t next_line_start = line_end + 1;
		size_t next_line_end = next_line_start;

		while(next_line_end < t->len && t->str[next_line_end] != '\n')
			next_line_end++;

		size_t next_line_len = next_line_end - next_line_start;

		if(curr_col > next_line_len)
			curr_col = next_line_len;

		t->cursor_pos = next_line_start + curr_col;

		update_text_scrolling(t);
	}
}
static void insert_char_text(phos_gui_text_component *text, char c, phos_gui_scroll_pane_component *scroll_pane);
static void wrap_text(phos_gui_text_component *text, phos_gui_scroll_pane_component *scroll_pane)
{
	switch(text->wrap_mode)
	{
		case PHOS_GUI_TEXT_WRAP_CHAR:
			// just go to new line
			insert_char_text(text, '\n', scroll_pane);
			break;
		case PHOS_GUI_TEXT_WRAP_WORD:
			// walk back to the last ' ' character, then insert new line there:

			size_t cursor_pos = text->cursor_pos;

			while(cursor_pos > 0)
			{
				cursor_pos--;

				if(text->str[cursor_pos] == ' ')
				{
					text->str[cursor_pos] = '\n';
					text->num_lines++;
					text->curr_line_len = 0;
					break;
				}
			}
			break;
		default:
			break;
	}
}
static void insert_char_text(phos_gui_text_component *text, char c, phos_gui_scroll_pane_component *scroll_pane)
{
	// insert char into string at cursor pos (if possible)
	if(text->len + 1 <= text->max_len && strlen(text->str) + 1 < PHOS_GUI_MAX_TEXT_LEN)
	{
		// first, move all chars at cursor pos one slot over to the right
		memmove(text->str + text->cursor_pos + 1, text->str + text->cursor_pos, text->len - text->cursor_pos + 1);

		// insert char and move to next cursor pos
		text->str[text->cursor_pos++] = c;

		// increase string length by one
		text->len++;
		text->curr_line_len++;

		// signal that the text was edited by the user
		text->edited = true;

		// fit and realign text based on edit_opts
		if(text->edit_opts & PHOS_GUI_OPTS_FIT_TEXT)
			phos_gui_make_text_fit_elem(text, PHOS_GUI_TARGET_AUTO_TEXT);
		if(text->edit_opts & PHOS_GUI_OPTS_REALIGN_TEXT)
			phos_gui_realign_elem_text(text);
	}

	// if the char inserted is '\n', reset scroll x on text component
	if(c == '\n')
	{
		// only update scroll pane if it's not null
		if(scroll_pane)
			scroll_pane->scroll_x = 0.0f;
		text->curr_line_len = 0;
		text->num_lines++;
	}

	// check to see if the current line needs to wrap to the next line
	char curr_line[PHOS_GUI_MAX_TEXT_LEN];

	size_t line_start = text->cursor_pos;

	while(line_start > 0 && text->str[line_start - 1] != '\n')
		line_start--;
	size_t line_len = text->cursor_pos - line_start;

	// copy text into curr_line
	memcpy(curr_line, text->str + line_start, line_len);
	curr_line[line_len] = '\0';

	// measure line
	Vector2 line_bounds = MeasureTextEx(*text->font, curr_line, text->font_size, 0.0f);

	// compare text bounds to content free bounds
	phos_gui_elem *text_elem = pluto_cs_get_owner(text);
	if(!text_elem)
	{
		vl_log(VL_ERROR, "Cannot properly modify text component without an owner!\n");
		return;
	}
	if(line_bounds.x >= text_elem->content_free_bounds.rect.width)
		wrap_text(text, scroll_pane);
}
static void new_line(phos_gui_text_component *t)
{
	// get scroll pane off owner
	phos_gui_elem *owner = pluto_cs_get_owner(t);
	if(!owner)
	{
		vl_log(VL_ERROR, "Unable to update text component properly, it requires an owner element!\n");
		return;
	}

	phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(owner, PHOS_GUI_COMPONENT_SCROLL_PANE);

	// if text inserts new lines on enter and user hit ENTER key, add line to text
	if(t->enter_inserts_new_line)
	{
		// number of auto-indent spaces
		size_t indent_spaces = 0;

		// handle auto-indentation (requires at least one line to work)
		if(t->auto_indent && t->num_lines > 0)
		{
			// get cursor pos
			size_t cursor_pos = t->cursor_pos;

			// walk back until a '\n' is encountered
			while(cursor_pos > 0 && t->str[cursor_pos - 1] != '\n')
				cursor_pos--;

			/*
			   now cursor pos is at start of previous line, so walk forward until a non whitespace char is found.
			   that number of spaces is equal to the number of spaces to indent for the next line
			*/
			while(cursor_pos < t->len && t->str[cursor_pos] == ' ')
			{
				cursor_pos++;
				indent_spaces++;
			}
		}

		// go to new line
		insert_char_text(t, '\n', scroll_pane);

		// if indent_spaces is greater than 0, indent that many spaces
		if(indent_spaces > 0)
			for(size_t i = 0; i < indent_spaces; ++i)
				insert_char_text(t, ' ', scroll_pane);
	}
}
static void indent(phos_gui_text_component *t)
{
	// get scroll pane off owner
	phos_gui_elem *owner = pluto_cs_get_owner(t);
	if(!owner)
	{
		vl_log(VL_ERROR, "Unable to update text component properly, it requires an owner element!\n");
		return;
	}

	phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(owner, PHOS_GUI_COMPONENT_SCROLL_PANE);

	// enter specified amount of spaces for a tab character
	for(size_t i = 0; i < t->spaces_per_tab; ++i)
		insert_char_text(t, ' ', scroll_pane);
}
static void update_key_timer(phos_gui_text_component *t, float dt, key_timer *kt, void (*action) (phos_gui_text_component*))
{
	if(IsKeyDown(kt->key))
	{
		if(!kt->active)
		{
			action(t);
			t->key_typed = kt->key;
			kt->timer = KEY_REPEAT_DELAY;
		}
		else
		{
			kt->timer -= dt;

			if(kt->timer <= 0.0f)
			{
				action(t);
				t->key_typed = kt->key;
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
	// if elem has no mouse listener component is cannot gain focus:
	const phos_gui_mouse_listener_component *const mouse_listener = e ? pluto_cs_get_component(e, PHOS_GUI_COMPONENT_MOUSE_LISTENER) : NULL;
	return !e ||
		   !mouse_listener ||
		   e->disabled ||
		   e->type == PHOS_GUI_TYPE_INVALID ||
		   e->type == PHOS_GUI_TYPE_BLANK;
}
static bool are_all_elems_unreachable(phos_gui_elem **elems, size_t max_len)
{
	bool all_unreachable = true;
	for(size_t i = 0; i < max_len; ++i)
	{
		// if elem is reachable, then they are not all unreachable
		if(!is_elem_unreachable(elems[i]))
		{
			all_unreachable = false;
			break;
		}
	}

	return all_unreachable;
}
static phos_gui_elem *next_sibling(phos_gui *gui, phos_gui_elem *elem)
{
	if(!elem)
		return NULL;

	// root element:
	if(!elem->parent)
	{
		for(size_t i = 0; i < gui->num_elems - 1; ++i)
		{
			if(gui->elems[i] == elem)
				return gui->elems[i + 1];
		}

		return NULL;
	}

	// child element:
	phos_gui_elem *parent = elem->parent;

	for(size_t i = 0; i < parent->num_children - 1; ++i)
	{
		if(parent->children[i] == elem)
			return parent->children[i + 1];
	}

	return NULL;
}
static phos_gui_elem *tree_next_elem(phos_gui *gui, phos_gui_elem *curr)
{
	if(!curr)
		return gui->elems[0];

	if(curr->num_children > 0)
		return curr->children[0];

	while(curr)
	{
		phos_gui_elem *sibling = next_sibling(gui, curr);

		if(sibling)
			return sibling;

		curr = curr->parent;
	}

	return gui->elems[0];
}
static phos_gui_elem *prev_sibling(phos_gui *gui, phos_gui_elem *elem)
{
	if(!elem)
		return NULL;

	if(!elem->parent)
	{
		for(size_t i = 1; i < gui->num_elems; ++i)
		{
			if(gui->elems[i] == elem)
				return gui->elems[i - 1];
		}

		return NULL;
	}

	phos_gui_elem *parent = elem->parent;

	for(size_t i = 1; i < parent->num_children; ++i)
	{
		if(parent->children[i] == elem)
			return parent->children[i - 1];
	}

	return NULL;
}
// obtain the last deepest child on the element
static phos_gui_elem *get_final_child(phos_gui_elem *elem)
{
	while(elem->num_children > 0)
		elem = elem->children[elem->num_children - 1];

	return elem;
}
static phos_gui_elem *tree_prev_elem(phos_gui *gui, phos_gui_elem *curr)
{
	if(!curr)
		return get_final_child(gui->elems[gui->num_elems - 1]);

	phos_gui_elem *sibling = prev_sibling(gui, curr);

	if(sibling)
		return get_final_child(sibling);

	if(curr->parent)
		return curr->parent;

	return get_final_child(gui->elems[gui->num_elems - 1]);
}
static phos_gui_elem *next_elem(phos_gui *gui, phos_gui_elem *curr)
{
	if(!gui || gui->num_elems == 0)
		return NULL;

	do
	{
		curr = tree_next_elem(gui, curr);
	} while(curr && is_elem_unreachable(curr));

	return curr;
}
static phos_gui_elem *prev_elem(phos_gui *gui, phos_gui_elem *curr)
{
	if(!gui || gui->num_elems == 0)
		return NULL;

	do
	{
		curr = tree_prev_elem(gui, curr);
	} while(curr && is_elem_unreachable(curr));

	return curr;
}
static void go_to_next_elem()
{
	if(!curr_gui)
	{
		vl_log(VL_WARNING, "No current GUI, cannot go to the next element!\n");
		return;
	}

	// get mouse listener component on curr_travel_elem
	phos_gui_mouse_listener_component *mouse_listener = curr_travel_elem ? pluto_cs_get_component(curr_travel_elem, PHOS_GUI_COMPONENT_MOUSE_LISTENER) : NULL;

	// current elem loses focus
	if(curr_travel_elem && mouse_listener)
		mouse_listener->has_focus = false;

	// get next element
	curr_travel_elem = next_elem(curr_gui, curr_travel_elem);

	// if a valid elem was traveled to, it gains focus
	if(curr_travel_elem && mouse_listener)
		mouse_listener->has_focus = true;
}
static void go_to_prev_elem()
{
	if(!curr_gui)
	{
		vl_log(VL_WARNING, "No current GUI, cannot go to the previous element!\n");
		return;
	}

	// get mouse listener component on curr_travel_elem
	phos_gui_mouse_listener_component *mouse_listener = curr_travel_elem ? pluto_cs_get_component(curr_travel_elem, PHOS_GUI_COMPONENT_MOUSE_LISTENER) : NULL;

	// current elem loses focus
	if(curr_travel_elem && mouse_listener)
		mouse_listener->has_focus = false;

	// get prev element
	curr_travel_elem = prev_elem(curr_gui, curr_travel_elem);

	// if a valid elem was traveled to, it gains focus
	if(curr_travel_elem && mouse_listener)
		mouse_listener->has_focus = true;
}
// return true if traveling succeeded, false is no traveling occurred:
static bool travel_elems()
{
	// go to prev or next elem based on keys pressed
	bool tab_pressed = IsKeyPressed(KEY_TAB);
	if(IsKeyDown(KEY_LEFT_SHIFT) && tab_pressed)
	{
		go_to_prev_elem();
		return true;
	}
	else if(tab_pressed)
	{
		go_to_next_elem();
		return true;
	}

	return false;
}

static float get_max_scroll_x(const phos_gui_elem *const e)
{
	float min_x = e->content_free_bounds.rect.x;
	float max_x = min_x + e->content_free_bounds.rect.width;

	for(size_t i = 0; i < e->num_children; ++i)
	{
		const phos_gui_elem *const child = e->children[i];

		float left = child->total_bounds.rect.x;
		float right = left + child->total_bounds.rect.width;

		if(left < min_x)
			min_x = left;
		if(right > max_x)
			max_x = right;
	}

	float content_width = max_x - min_x;
	float viewport_width = e->content_free_bounds.rect.width;
	float max_scroll = content_width - viewport_width;

	return max_scroll > 0 ? max_scroll : 0.0f;
}
static float get_max_scroll_y(const phos_gui_elem *const e)
{
	float min_y = e->content_free_bounds.rect.y;
	float max_y = min_y + e->content_free_bounds.rect.height;

	for(size_t i = 0; i < e->num_children; ++i)
	{
		const phos_gui_elem *const child = e->children[i];

		float top = child->total_bounds.rect.y;
		float bottom = top + child->total_bounds.rect.height;

		if(top < min_y)
			min_y = top;
		if(bottom > max_y)
			max_y = bottom;
	}

	float content_height = max_y - min_y;
	float viewport_height = e->content_free_bounds.rect.height;
	float max_scroll = content_height - viewport_height;

	return max_scroll > 0 ? max_scroll : 0.0f;
}
static float get_elem_total_content_width(phos_gui_elem *e)
{
	float tcw = 0.0f;

	// default to layout component's total content width
	phos_gui_layout_component *layout = NULL;
	if((layout = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_LAYOUT)))
		return layout->total_content_width;

	// get whole rect of elem
	Rectangle whole_content = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);

	// with no layout, default to free content rect's width
	Rectangle free_content = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);
	tcw = free_content.width;

	// manually find total content width now:
	for(size_t i = 0; i < e->num_children; ++i)
	{
		phos_gui_elem *child = e->children[i];

		// get total rect of child
		Rectangle child_rect = get_calculated_elem_rect(child, PHOS_GUI_ELEM_BOUNDS_TOTAL);

		float child_right = child_rect.x + child_rect.width;

		// either choose 'child_right' or keep current 'tcw' value
		tcw = fmax(tcw, child_right - whole_content.x);
	}

	return tcw;
}
static float get_elem_total_content_height(phos_gui_elem *e)
{
	float tch = 0.0f;

	// default to layout component's total content height
	phos_gui_layout_component *layout = NULL;
	if((layout = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_LAYOUT)))
		return layout->total_content_height;

	// see if elem rects should be calculated
	// get whole content rect of elem
	Rectangle whole_content = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);

	// with no layout, default to elem free content rect's height
	Rectangle free_content = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);
	tch = free_content.height;

	// manually find total content height now:
	for(size_t i = 0; i < e->num_children; ++i)
	{
		phos_gui_elem *child = e->children[i];

		// get total rect of child
		Rectangle child_rect = get_calculated_elem_rect(child, PHOS_GUI_ELEM_BOUNDS_TOTAL);

		float child_bottom = child_rect.y + child_rect.height;

		// either choose 'child_bottom' or keep current 'tch' value
		tch = fmax(tch, child_bottom - whole_content.y);
	}

	return tch;
}
static void get_slider_knob_rect(const phos_gui_value_bar_component *const value_bar, Rectangle *out_rect)
{
	if(!out_rect)
		return;

	// start out with invalid rectangle
	Rectangle slider_knob = {0};
	*out_rect = slider_knob;

	// get owner of value bar
	phos_gui_elem *owner = pluto_cs_get_owner(value_bar);
	if(!owner)
		return;

	Rectangle track = owner->content_free_bounds.rect;

	slider_knob.width = value_bar->slider_knob_span;
	slider_knob.height = slider_knob.width;

	float t = value_bar->curr_value / value_bar->max_value;
	t = Clamp(t, 0.0f, 1.0f);

	// where to put slider knob horizontally:
	float half_knob_width = slider_knob.width / 2.0f;
	float min_center_x = track.x + half_knob_width;
	float max_center_x = track.x + track.width - half_knob_width;
	float center_x = min_center_x + t * (max_center_x - min_center_x);

	slider_knob.x = center_x - half_knob_width;
	slider_knob.y = track.y + ((track.height - slider_knob.height) / 2.0f);

	*out_rect = slider_knob;
}
static void get_scroll_bar_rects(const phos_gui_scroll_pane_component *const scroll_pane, Rectangle *out_v_bar, Rectangle *out_v_thumb, Rectangle *out_h_bar, Rectangle *out_h_thumb)
{
	// no scrolling enabled, return
	if(!scroll_pane->v_bar.active && !scroll_pane->h_bar.active)
		return;

	// get owner of scroll pane
	phos_gui_elem *elem = pluto_cs_get_owner(scroll_pane);
	if(!elem)
		return;

	// get elem rects
	Rectangle whole_content_bounds = get_calculated_elem_rect(elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);
	Rectangle usable_content_bounds = get_calculated_elem_rect(elem, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

	// vertical scroll bar pos
	float v_scroll_bar_x = whole_content_bounds.x + whole_content_bounds.width - scroll_pane->v_bar.span;
	float v_scroll_bar_y = usable_content_bounds.y;

	// horizontal scroll bar pos
	float h_scroll_bar_x = usable_content_bounds.x;
	float h_scroll_bar_y = whole_content_bounds.y + whole_content_bounds.height - scroll_pane->h_bar.span;

	// get elem's total content width and height
	float total_content_width = get_elem_total_content_width(elem);
	float total_content_height = get_elem_total_content_height(elem);

	// vertical scroll thumb
	float v_scroll_thumb_height = (usable_content_bounds.height / total_content_height) * usable_content_bounds.height;
	v_scroll_thumb_height = fmax(v_scroll_thumb_height, MIN_SCROLL_THUMB_LENGTH);
	float v_scroll_thumb_interval = usable_content_bounds.height - v_scroll_thumb_height;

	float v_scroll_thumb_y = usable_content_bounds.y;
	if(total_content_height > usable_content_bounds.height)
	{
		float max_scroll = total_content_height - usable_content_bounds.height;
		v_scroll_thumb_y = usable_content_bounds.y + (scroll_pane->scroll_y / max_scroll) * v_scroll_thumb_interval;
	}
	else
		v_scroll_thumb_height = usable_content_bounds.height;

	// horizontal scroll thumb
	float h_scroll_thumb_width = (usable_content_bounds.width / total_content_width) * usable_content_bounds.width;
	h_scroll_thumb_width = fmax(h_scroll_thumb_width, MIN_SCROLL_THUMB_LENGTH);
	float h_scroll_thumb_interval = usable_content_bounds.width - h_scroll_thumb_width;

	float h_scroll_thumb_x = usable_content_bounds.x;
	if(total_content_width > usable_content_bounds.width)
	{
		float max_scroll = total_content_width - usable_content_bounds.width;
		h_scroll_thumb_x = usable_content_bounds.x + (scroll_pane->scroll_x / max_scroll) * h_scroll_thumb_interval;
	}
	else
		h_scroll_thumb_width = usable_content_bounds.width;

	Rectangle v_bar = {0};
	v_bar.x = v_scroll_bar_x;
	v_bar.y = v_scroll_bar_y;
	v_bar.width = scroll_pane->v_bar.span;
	v_bar.height = usable_content_bounds.height;

	Rectangle v_thumb = {0};
	v_thumb.x = v_scroll_bar_x;
	v_thumb.y = v_scroll_thumb_y;
	v_thumb.width = scroll_pane->v_bar.span;
	v_thumb.height = v_scroll_thumb_height;

	Rectangle h_bar = {0};
	h_bar.x = h_scroll_bar_x;
	h_bar.y = h_scroll_bar_y;
	h_bar.width = usable_content_bounds.width;
	h_bar.height = scroll_pane->h_bar.span;

	Rectangle h_thumb = {0};
	h_thumb.x = h_scroll_thumb_x;
	h_thumb.y = h_scroll_bar_y;
	h_thumb.width = h_scroll_thumb_width;
	h_thumb.height = scroll_pane->h_bar.span;

	if(scroll_pane->v_bar.active)
	{
		if(out_v_bar)
			*out_v_bar = v_bar;
		if(out_v_thumb)
			*out_v_thumb = v_thumb;
	}
	if(scroll_pane->h_bar.active)
	{
		if(out_h_bar)
			*out_h_bar = h_bar;
		if(out_h_thumb)
			*out_h_thumb = h_thumb;
	}
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

	// get total content bounds of elem
	Rectangle whole_content_bounds = get_calculated_elem_rect(owner, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);

	Rectangle drag_bar_rect = {0};

	switch(drag_pane->drag_bar_orientation)
	{
		case PHOS_GUI_DRAG_BAR_HORIZONTAL_TOP:
			drag_bar_rect.x = whole_content_bounds.x;
			drag_bar_rect.y = whole_content_bounds.y;
			drag_bar_rect.width = whole_content_bounds.width;
			drag_bar_rect.height = drag_pane->span;
			break;
		case PHOS_GUI_DRAG_BAR_VERTICAL_LEFT:
			drag_bar_rect.x = whole_content_bounds.x;
			drag_bar_rect.y = whole_content_bounds.y;
			drag_bar_rect.width = drag_pane->span;
			drag_bar_rect.height = whole_content_bounds.height;
			break;
		case PHOS_GUI_DRAG_BAR_VERTICAL_RIGHT:
			drag_bar_rect.x = whole_content_bounds.x + whole_content_bounds.width - drag_bar_rect.width;
			drag_bar_rect.y = whole_content_bounds.y;
			drag_bar_rect.width = drag_pane->span;
			drag_bar_rect.height = whole_content_bounds.height;
			break;
		case PHOS_GUI_DRAG_BAR_HORIZONTAL_BOTTOM:
			drag_bar_rect.x = whole_content_bounds.x;
			drag_bar_rect.y = whole_content_bounds.y + whole_content_bounds.height - drag_bar_rect.height;
			drag_bar_rect.width = whole_content_bounds.width;
			drag_bar_rect.height = drag_pane->span;
			break;
		default:
			vl_delay_log(VL_ERROR, 2.0f, "Invalid drag bar orientation: %d!\n", drag_pane->drag_bar_orientation);
			break;
	}

	*out_rect = drag_bar_rect;
}
// see if any children are clipped, which affects how the mouse pos interacts with the elem:
static bool elem_in_parent_clip(phos_gui_elem *e, Vector2 mouse_pos)
{
	phos_gui_elem *child = e;
	phos_gui_elem *parent = e->parent;

	while(parent)
	{
		// child inherits parent clip region if parent is clipped:
		if(parent->clipped)
		{
			Rectangle clip_rect = get_calculated_elem_rect(parent, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

			// if mouse not in clip region, mouse cannot interact with any elems
			if(!CheckCollisionPointRec(mouse_pos, clip_rect))
				return false;
		}

		// go to next parent and child relationship
		child = parent;
		parent = parent->parent;
	}

	// return true by default, as the while loop already guarantees the mouse has to be in the clip region
	return true;
}
// checks mouse collision with elem and children:
static phos_gui_elem *get_elem_mouse_target(phos_gui_elem *e, Vector2 mouse_pos)
{
	// check for elems that can't accept input, except for PHOS_GUI_TYPE_BLANK
	if(e->disabled)
		return NULL;
	if(e -> type == PHOS_GUI_TYPE_INVALID)
		return NULL;

	// if elem isn't rendered, it can't be the mouse target either
	if(!e->auto_render)
		return NULL;

	// then see if mouse is within the parent's clip region
	if(!elem_in_parent_clip(e, mouse_pos))
		return NULL;

	// check children in reverse order:
	for(int i = e->num_children - 1; i >= 0; --i)
	{
		phos_gui_elem *target = get_elem_mouse_target(e->children[i], mouse_pos);

		if(target)
			return target;
	}

	// after checking children, if no target still found, check given elem state again
	if(e->type == PHOS_GUI_TYPE_BLANK) // check for PHOS_GUI_TYPE_BLANK here so that the children list is processed above
		return NULL;

	// check whether the elem is clickable
	Rectangle input_rect = e->input_test_bounds == PHOS_GUI_ELEM_BOUNDS_REAL ? e->bounds : get_calculated_elem_rect(e, e->input_test_bounds);

	if(!CheckCollisionPointRec(mouse_pos, input_rect))
		return NULL;

	// no child hit, return parent elem
	return e;
}
// checks mouse collision with all GUI elems and children:
static phos_gui_elem *get_gui_mouse_target(phos_gui *gui, Vector2 mouse_pos)
{
	// when mouse pos is (0, 0) it's outside window's bounds, and therefore cannot interact with any element in the GUI
	if(mouse_pos.x == 0.0f && mouse_pos.y == 0.0f)
		return NULL;

	// iterate over all elems in the GUI
	for(int i = gui->num_elems - 1; i >= 0; --i)
	{
		phos_gui_elem *target = get_elem_mouse_target(gui->elems[i], mouse_pos);
		
		// if target found, return it
		if(target)
			return target;
	}

	// no mouse target
	return NULL;
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
	// skip basic elements as well by going to update_children tag
	else if(e->type == PHOS_GUI_TYPE_BLANK)
		goto update_children;

	// if element is not being rendered, it shouldn't be updated either
	if(!e->auto_render)
		goto update_children;

	// obtain all input state here:
	Vector2 mouse_pos = phos_gui_get_mouse_pos();
	Vector2 mouse_delta = GetMouseDelta();
	Vector2 mouse_wheel_move = GetMouseWheelMoveV();
	bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
	bool mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
	bool mouse_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
	// see if mouse over element (if the current elem is the mouse target, that guarantees that the mouse is over the elem
	bool mouse_over_elem = e == mouse_target;

	// if mouse is not in window, manually clear out mouse delta to stop all dragging logic from occurring
	if(mouse_pos.x <= 0.0f || mouse_pos.y <= 0.0f || mouse_pos.x >= GetRenderWidth() || mouse_pos.y >= GetRenderHeight())
		mouse_delta = Vector2Zero();

	// calculate and get all rects for the elem:
	e->content_total_bounds.rect = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);
	e->content_free_bounds.rect = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);
	e->total_bounds.rect = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_TOTAL);

	// update mouse listener component:
	phos_gui_mouse_listener_component *mouse_listener = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_MOUSE_LISTENER);
	phos_gui_text_component *text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_TEXT);
	if(mouse_listener)
	{
		// keep track of whether or not elem gained focus
		bool no_focus = !mouse_listener->has_focus;

		// reset mouse state
		mouse_listener->hovered = false;
		mouse_listener->pressed = false;
		mouse_listener->clicked = false;

		if(mouse_over_elem)
		{
			// if mouse is over elem and all clip regions, handle elem-mouse logic:
			mouse_listener->hovered = true;

			// default mouse listener logic:
			if(mouse_listener->type == PHOS_GUI_MOUSE_LISTENER_DEFAULT)
			{
				if(mouse_clicked)
				{
					mouse_listener->clicked = true;
					mouse_listener->has_focus = true;
				}
				else if(mouse_down)
				{
					mouse_listener->pressed = true;
					mouse_listener->has_focus = true;
				}
			}
			// toggle-style mouse listener:
			else if(mouse_listener->type == PHOS_GUI_MOUSE_LISTENER_TOGGLED)
			{
				if(mouse_clicked)
				{
					mouse_listener->toggled = true;
					mouse_listener->toggled_on = !mouse_listener->toggled_on;
					mouse_listener->has_focus = true;
				}
				else if(mouse_down)
				{
					mouse_listener->pressed = true;
					mouse_listener->has_focus = true;
				}
			}
		}
		// else if user clicks OFF of the element
		else if(mouse_clicked || mouse_down)
		{
			// ensure elem loses focus
			mouse_listener->has_focus = false;

			// if curr_gui_elem_num points to this elem, reset it
			if(curr_travel_elem == e)
				curr_travel_elem = NULL;
		}
		else
		{
			// if no mouse input detected, check to see if user reached the elem and is using keyboard input instead
			if(curr_travel_elem == e)
			{
				// if this elem has a text component, hitting enter would insert a new line, and it has focus, ENTER should not act as a mouse-press
				if(text && !text->enter_inserts_new_line && !mouse_listener->has_focus)
				{
					if(IsKeyPressed(KEY_ENTER))
					{
						mouse_listener->pressed = true;
						mouse_listener->clicked = true;
					}
					else if(IsKeyDown(KEY_ENTER))
						mouse_listener->pressed = true;
				}
			}
		}

		// if elem now has focus, it gained focus
		if(no_focus && mouse_listener->has_focus)
			mouse_listener->gained_focus = true;
		else
			mouse_listener->gained_focus = false;
	}

	// update text components:
	phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_SCROLL_PANE);
	if(text)
	{
		// reset 'edited' field
		text->edited = false;

		// only type into text if it can be edited and has focus (requires mouse listener component)
		if(text->editable && mouse_listener && mouse_listener->has_focus)
		{
			// collect char pressed
			char c = GetCharPressed();

			while(c > 0)
			{
				// get type of c
				bool letter = isalpha(c);
				bool num = isdigit(c);
				bool special = !letter && !num;

				// see if text accepts this type of char:

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
				// let SPACE through the special char check
				if(special && !text->accept_specials && c != ' ')
				{
					c = GetCharPressed();
					continue;
				}

				// assign char typed
				text->char_typed = c;

				// insert char into string at cursor pos (if possible)
				insert_char_text(text, c, scroll_pane);

				// get next char pressed
				c = GetCharPressed();
			}

			update_key_timer(text, dt, &backspace_timer, backspace);
			update_key_timer(text, dt, &del_timer, delete);
			update_key_timer(text, dt, &left_arrow_timer, move_cursor_left);
			update_key_timer(text, dt, &up_arrow_timer, move_cursor_up);
			update_key_timer(text, dt, &right_arrow_timer, move_cursor_right);
			update_key_timer(text, dt, &down_arrow_timer, move_cursor_down);
			update_key_timer(text, dt, &enter_timer, new_line);
			update_key_timer(text, dt, &tab_timer, indent);
		}
		else
			text->edited = false;

		// update text scrolling if edited
		if(text->edited)
			update_text_scrolling(text);
	}

	// if elem has focus and ESC pressed, lose focus
	if(mouse_listener && mouse_listener->has_focus && IsKeyPressed(KEY_ESCAPE))
	{
		mouse_listener->has_focus = false;
		mouse_listener->gained_focus = false;

		// if curr_travel_elem points to this elem, reset it
		if(curr_travel_elem == e)
			curr_travel_elem = NULL;
	}

	// update input panes:
	phos_gui_drag_pane_component *drag_pane = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_DRAG_PANE);

	// is user currently scrolling the scroll pane?
	bool scrolling_scroll_pane = scroll_pane && (scroll_pane->v_bar.thumb_grabbed || scroll_pane->h_bar.thumb_grabbed);
	// is user currently dragging the drag pane?
	bool dragging_drag_pane = drag_pane && drag_pane->grabbed;

	if(scroll_pane)
	{
		// first, determine max scroll values for element (unless text already handled scrolling)
		if(!text)
		{
			scroll_pane->max_scroll_x = get_max_scroll_x(e);
			scroll_pane->max_scroll_y = get_max_scroll_y(e);
		}

		// store current scroll value
		float prev_scroll_x = scroll_pane->scroll_x;
		float prev_scroll_y = scroll_pane->scroll_y;

		// define rects around scroll thumbs:
		Rectangle v_bar, v_thumb, h_bar, h_thumb;
		get_scroll_bar_rects(scroll_pane, &v_bar, &v_thumb, &h_bar, &h_thumb);

		// reset mouse state on scroll thumb
		scroll_pane->v_bar.thumb_has_focus = false;
		scroll_pane->h_bar.thumb_has_focus = false;

		// see if mouse is over the thumb
		bool mouse_over_v_thumb = scroll_pane->v_bar.active && scroll_pane->v_bar.rendered && phos_gui_is_mouse_over_rect(v_thumb);
		bool mouse_over_h_thumb = scroll_pane->h_bar.active && scroll_pane->h_bar.rendered && phos_gui_is_mouse_over_rect(h_thumb);

		// see if user is hovered over the thumb and not currently dragging pane
		if(mouse_over_v_thumb && !dragging_drag_pane)
			// give scroll thumb focus
			scroll_pane->v_bar.thumb_has_focus = true;
		if(mouse_over_h_thumb && !dragging_drag_pane)
			// give scroll thumb focus
			scroll_pane->h_bar.thumb_has_focus = true;

		// check for user dragging bars:
		if(!dragging_drag_pane && mouse_down || (mouse_down && (scroll_pane->v_bar.thumb_grabbed || scroll_pane->h_bar.thumb_grabbed)))
		{
			// start initial grabs
			if(mouse_over_v_thumb && !scroll_pane->v_bar.thumb_grabbed)
			{
				scroll_pane->v_bar.thumb_grabbed = true;
				scroll_pane->v_bar.thumb_grab_offset = mouse_pos.y - v_thumb.y;
			}
			if(mouse_over_h_thumb && !scroll_pane->h_bar.thumb_grabbed)
			{
				scroll_pane->h_bar.thumb_grabbed = true;
				scroll_pane->h_bar.thumb_grab_offset = mouse_pos.x - h_thumb.x;
			}

			// continue grabs
			if(scroll_pane->v_bar.thumb_grabbed)
			{
				float desired_thumb_y = mouse_pos.y - scroll_pane->v_bar.thumb_grab_offset;
				float thumb_travel = v_bar.height - v_thumb.height;
				float thumb_offset = desired_thumb_y - v_bar.y;
				float t = thumb_offset / thumb_travel;
				t = Clamp(t, 0.0f, 1.0f);
				scroll_pane->scroll_y = t * scroll_pane->max_scroll_y;
			}
			if(scroll_pane->h_bar.thumb_grabbed)
			{
				float desired_thumb_x = mouse_pos.x - scroll_pane->h_bar.thumb_grab_offset;
				float thumb_travel = h_bar.width - h_thumb.width;
				float thumb_offset = desired_thumb_x - h_bar.x;
				float t = thumb_offset / thumb_travel;
				t = Clamp(t, 0.0f, 1.0f);
				scroll_pane->scroll_x = t * scroll_pane->max_scroll_x;
			}
		}
		// user is using mouse wheel instead: (requires 'use_mouse_wheel_input' to be true)
		else if(phos_gui_is_mouse_over_rect(get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL)) && scroll_pane->use_mouse_wheel_input)
		{
			// user is trying to scroll using mouse wheel:

			// ticks becomes mouse wheel movement instead (include px_per_tick since the mouse wheel is being used)
			float h_ticks = mouse_wheel_move.x * scroll_pane->px_per_tick;
			float v_ticks = mouse_wheel_move.y * scroll_pane->px_per_tick;

			// use ticks * px_per_tick to add to total scroll offset
			scroll_pane->scroll_y -= v_ticks;
			scroll_pane->scroll_x -= h_ticks;

			// thumb no longer grabbed
			scroll_pane->v_bar.thumb_grabbed = false;
			scroll_pane->h_bar.thumb_grabbed = false;
		}
		else
		{
			// thumb no longer grabbed or focused
			scroll_pane->v_bar.thumb_grabbed = false;
			scroll_pane->h_bar.thumb_grabbed = false;
		}

		// clamp scroll amounts
		if(scroll_pane->scroll_x < 0.0f)
			scroll_pane->scroll_x = 0.0f;
		else if(scroll_pane->scroll_x > scroll_pane->max_scroll_x)
			scroll_pane->scroll_x = scroll_pane->max_scroll_x;

		if(scroll_pane->scroll_y < 0.0f)
			scroll_pane->scroll_y = 0.0f;
		else if(scroll_pane->scroll_y > scroll_pane->max_scroll_y)
			scroll_pane->scroll_y = scroll_pane->max_scroll_y;

		float delta_x = scroll_pane->scroll_x - prev_scroll_x;
		float delta_y = scroll_pane->scroll_y - prev_scroll_y;

		// translate children based on scroll pane translation (only if user scrolled)
		for(size_t i = 0; i < e->num_children; ++i)
			phos_gui_move_elem_xy(e->children[i], -delta_x, -delta_y, PHOS_GUI_OPTS_NONE);
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

			// see if mouse target was already taken:
			bool mouse_target_exists = (mouse_target != NULL && mouse_target != e);

			// determine how to interpret mouse state
			if(drag_pane->use_drag_bar)
			{
				// check for mouse over drag bar
				bool mouse_over_bar = phos_gui_is_mouse_over_rect(drag_bar_rect);

				// if mouse over bar was already over an elem, do not drag:
				if(!mouse_target_exists && (mouse_over_bar || drag_pane->grabbed))
				{
					// if mouse down and over the bar, it becomes grabbed
					drag_pane->grabbed = true;
					drag_pane->drag_delta = mouse_delta;

					// add to elem pos based on mouse delta
					phos_gui_move_elem_xy(e, mouse_delta.x, mouse_delta.y, drag_pane->drag_opts);

					// move drag bar based on mouse delta
					/*drag_pane->drag_bar_pos.x += mouse_delta.x;
					drag_pane->drag_bar_pos.y += mouse_delta.y;	*/
				}
			}
			else
			{
				// check for mouse over free content rect
				bool mouse_over_free_content = phos_gui_is_mouse_over_rect(e->content_free_bounds.rect);

				// to grab the whole element, the mouse has to be over the free content rect but not over another elem
				if(!mouse_target_exists && (mouse_over_free_content || drag_pane->grabbed))
				{
					// if mouse down and over the elem, it becomes grabbed
					drag_pane->grabbed = true;
					drag_pane->drag_delta = mouse_delta;

					// add to elem pos based on mouse delta
					phos_gui_move_elem_xy(e, mouse_delta.x, mouse_delta.y, drag_pane->drag_opts);
				}
			}
		}
		else
			// when mouse not down, the drag pane is no longer grabbed
			drag_pane->grabbed = false;
	}

	// drop down logic for the drop down button (must be combined with mouse listener to be interacted with)
	phos_gui_drop_down_component *drop_down = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_DROP_DOWN);
	if(drop_down && mouse_listener)
	{
		// see if button was clicked, and if so, toggle drop down's expanded state
		if(mouse_listener->clicked)
			drop_down->expanded = !drop_down->expanded;

		bool selection_made = false;

		// see if the drop down was supplied a container
		if(drop_down->container)
		{
			// reset scroll amounts on scroll pane if necessary
			phos_gui_scroll_pane_component *container_scroll_pane = pluto_cs_get_component(drop_down->container, PHOS_GUI_COMPONENT_SCROLL_PANE);
			if(mouse_listener->clicked && container_scroll_pane)
			{
				// reset positions of children
				for(size_t i = 0; i < drop_down->container->num_children; ++i)
					phos_gui_move_elem_xy(drop_down->container->children[i], container_scroll_pane->scroll_x, container_scroll_pane->scroll_y, PHOS_GUI_OPTS_NONE);
				// reset scroll values to 0
				container_scroll_pane->scroll_x = 0.0f;
				container_scroll_pane->scroll_y = 0.0f;
			}

			// the container will render if the drop down is expanded
			drop_down->container->auto_render = drop_down->expanded;

			// only update elements within container if it's expanded
			if(drop_down->expanded)
			{
				// see which element in the container was clicked
				for(size_t i = 0; i < drop_down->container->num_children; ++i)
				{
					phos_gui_elem *container_option_elem = drop_down->container->children[i];

					phos_gui_mouse_listener_component *container_option_elem_ml = pluto_cs_get_component(container_option_elem, PHOS_GUI_COMPONENT_MOUSE_LISTENER);
					if(container_option_elem_ml)
					{
						if(container_option_elem_ml->clicked)
						{
							drop_down->selection = container_option_elem;
							selection_made = true;
							break;
						}
					}
				}
			}

			// see if an element was chosen by user
			if(drop_down->selection && selection_made)
			{
				// modify contents of drop down button to match selection's text
				phos_gui_text_component *container_option_elem_text = pluto_cs_get_component(drop_down->selection, PHOS_GUI_COMPONENT_TEXT);
				if(container_option_elem_text && text)
					phos_gui_set_text_contents(text, PHOS_GUI_TARGET_MAIN_TEXT, container_option_elem_text->str, PHOS_GUI_OPTS_NONE);

				// collapse drop down list
				drop_down->expanded = false;
				// the container is guaranteed to be a valid pointer because 'selection_made' is true
				drop_down->container->auto_render = false;
			}
		}
	}

	// drop down logic for the container (parent of 'e' must have a drop down component)
	phos_gui_drop_down_component *parent_drop_down = NULL;
	if(e->parent)
		parent_drop_down = pluto_cs_get_component(e->parent, PHOS_GUI_COMPONENT_DROP_DOWN);
	// see if the parent has a drop down and 'e' has a mouse listener
	if(parent_drop_down && mouse_listener)
	{
		// if 'e' was clicked, that means the selected elem in 'parent_drop_down' becomes 'e'
		if(mouse_listener->clicked)
			parent_drop_down->selection = e;

		// now the text on the parent element should reflect the text on 'e'
		phos_gui_text_component *child_text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_TEXT);
		phos_gui_text_component *parent_text = pluto_cs_get_component(e->parent, PHOS_GUI_COMPONENT_TEXT);
		// both elems must have a text component for this to work
		if(child_text && parent_text)
			// parent text becomes child text
			phos_gui_set_text_contents(parent_text, PHOS_GUI_TARGET_MAIN_TEXT, child_text->str, PHOS_GUI_OPTS_NONE);
	}

	// slider knob logic
	phos_gui_value_bar_component *value_bar = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_VALUE_BAR);
	if(value_bar)
	{
		// get slider knob rect and check for mouse over it
		Rectangle slider_knob_rect;
		get_slider_knob_rect(value_bar, &slider_knob_rect);

		float progress_bar_width = (value_bar->curr_value / value_bar->max_value) * e->content_free_bounds.rect.width;

		// reset focus state
		value_bar->slider_knob_has_focus = false;

		// same logic as scroll pane (let user grab once and then be able to move mouse anywhere and it remain grabbed)
		bool mouse_over_slider_knob = phos_gui_is_mouse_over_rect(slider_knob_rect);
		if(mouse_over_slider_knob)
			value_bar->slider_knob_has_focus = true;

		// check to see if user lets go of knob
		value_bar->slider_knob_released = false;
		if(value_bar->slider_knob_grabbed && mouse_released)
			value_bar->slider_knob_released = true;

		if(mouse_down && mouse_over_slider_knob || (mouse_down && value_bar->slider_knob_grabbed))
		{
			value_bar->slider_knob_has_focus = true;

			if(!value_bar->slider_knob_grabbed)
			{
				value_bar->slider_knob_grabbed = true;
				value_bar->slider_knob_grab_offset = mouse_pos.x - slider_knob_rect.x;
			}

			float track_x = e->content_free_bounds.rect.x;
			float knob_travel = e->content_free_bounds.rect.width - slider_knob_rect.width;

			if(knob_travel > 0.0f)
			{
				float desired_knob_x = mouse_pos.x - value_bar->slider_knob_grab_offset;
				float knob_offset = desired_knob_x - track_x;
				float t = knob_offset / knob_travel;
				t = Clamp(t, 0.0f, 1.0f);
				value_bar->curr_value = value_bar->min_value + t * (value_bar->max_value - value_bar->min_value);
			}
		}
		else
			value_bar->slider_knob_grabbed = false;
	}

	// update child elements
	update_children:
	for(size_t i = 0; i < e->num_children; ++i)
		update_elem(e->children[i], dt);
}

void phos_gui_launch()
{
	if(!init)
	{
		vl_log(VL_ERROR, "Cannot launch loop, PhosphorusGUI was never initialized!\n");
		return;
	}

	while(!WindowShouldClose())
	{
		float dt = GetFrameTime();

		vl_update(dt);
		phos_gui_update(dt);

		BeginDrawing();
		ClearBackground(window_bg_color);

		phos_gui_render();

		EndDrawing();
	}
}
// return true on action executed, false on failure
static bool run_event_listener(phos_gui_event_listener *listener)
{
	phos_gui_event_type event = listener->event;
	phos_gui_elem *elem = listener->elem;
	phos_gui_event_listener_action action = listener->action;

	// validate listener
	if(event == PHOS_GUI_EVENT_NONE)
	{
		vl_log(VL_ERROR, "Invalid event listener event: %d!\n", event);
		return false;
	}
	if(!action)
	{
		vl_log(VL_WARNING, "This event listener has a null action!\n");
		return false;
	}

	Rectangle window_rect = { 0, 0, GetRenderWidth(), GetRenderHeight() };

	// see if elem has a mouse listener component 
	const phos_gui_mouse_listener_component *const mouse_listener = elem ? pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_MOUSE_LISTENER) : NULL;

	// check event conditions:
	bool mouse_clicked = elem && mouse_listener ? mouse_listener->clicked : IsMouseButtonPressed(listener->target_btn);
	bool mouse_down = elem && mouse_listener ? mouse_listener->pressed : IsMouseButtonDown(listener->target_btn);
	bool mouse_hovered = elem && mouse_listener ? mouse_listener->hovered : phos_gui_is_mouse_over_rect(window_rect);
	bool key_clicked = IsKeyPressed(listener->target_btn);
	bool key_down = IsKeyDown(listener->target_btn);

	bool can_execute = false;
	switch(event)
	{
		case PHOS_GUI_EVENT_MOUSE_CLICK:
			can_execute = mouse_clicked;
			break;
		case PHOS_GUI_EVENT_MOUSE_DOWN:
			can_execute = mouse_down;
			break;
		case PHOS_GUI_EVENT_KEY_CLICK:
			can_execute = key_clicked;
			break;
		case PHOS_GUI_EVENT_KEY_DOWN:
			can_execute = key_down;
			break;
		case PHOS_GUI_EVENT_HOVER:
			can_execute = mouse_hovered;
			break;
		default:
			break;
	}

	// execute action if conditions are true
	if(can_execute)
		action(elem, listener->opts);

	return can_execute;
}
static void update_timer(phos_gui_timer *timer, float dt)
{
	if(!timer)
	{
		vl_delay_log(VL_ERROR, 3.0f, "Unable to update NULL timer!\n");
		return;
	}

	// if timer has 0 execution_count or less than -1 execution_count by default, do not execute
	if(timer->execution_count == 0 || timer->execution_count < -1)
		return;

	// add delta time to curr time
	timer->curr_time += dt;
	// reset curr time if it hits target time
	if(timer->curr_time >= timer->target_time)
	{
		timer->curr_time = 0.0f;

		// execute timer action
		if(timer->action)
			timer->action(timer->args);

		// handle timer execution_count (-1 means loop forever)
		if(timer->execution_count > 0)
			// minus one loop from timer
			timer->execution_count--;
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

	if(resolve_focus_on_start_elem)
	{
		for(size_t i = 0; i < curr_gui->num_elems; ++i)
		{
			// get elem at i
			phos_gui_elem *elem = curr_gui->elems[i];

			// focus_on_start requires mouse listener component
			phos_gui_mouse_listener_component *mouse_listener = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_MOUSE_LISTENER);
			if(mouse_listener && mouse_listener->focus_on_start)
			{
				// verify valid elem type
				if(elem->type == PHOS_GUI_TYPE_INVALID || elem->type == PHOS_GUI_TYPE_BLANK || elem->disabled)
					vl_log(VL_ERROR, "Cannot focus this element '%s' on start because it has an invalid type!\n", elem->ID);
				else
				{
					curr_travel_elem = elem;
					mouse_listener->has_focus = true;
					break;
				}
			}
		}

		// signal that the focus_on_start elem was resolved
		resolve_focus_on_start_elem = false;
	}

	// find mouse target
	mouse_target = get_gui_mouse_target(curr_gui, phos_gui_get_mouse_pos());

	// update elems:
	for(size_t i = 0; i < curr_gui->num_elems; ++i)
	{
		// get elem at i
		phos_gui_elem *elem = curr_gui->elems[i];

		// update the element and its children
		update_elem(elem, dt);

		// if elem gained focus, make this elem the current one (requires mouse listener component)
		phos_gui_mouse_listener_component *mouse_listener = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_MOUSE_LISTENER);
		if(mouse_listener && mouse_listener->gained_focus)
			curr_travel_elem = elem;
	}

	// update event listeners
	for(size_t i = 0; i < curr_gui->num_listeners; ++i)
		run_event_listener(&curr_gui->listeners[i]);

	// update timers
	size_t num_timers = curr_gui->num_timers;
	for(size_t i = 0; i < num_timers; ++i)
	{
		phos_gui_timer *timer = &curr_gui->timers[i];
		// use local copy of curr_gui->num_timers because update_timer(...) modifies curr_gui->num_timers
		update_timer(timer, dt);

		// check to see if this timer should be removed
		if(timer->execution_count == 0)
		{
			// move all timers after current timer one to left
			memmove(curr_gui->timers + i, curr_gui->timers + i + 1, (num_timers - i - 1) * sizeof(phos_gui_timer));
			curr_gui->num_timers--;
		}
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

static Color resolve_elem_bg_color(const phos_gui_elem *const e)
{
	if(e->disabled)
		return e->disabled_color;

	// get the normal color of elem
	Color color = e->bg_color;

	// see if elem has a mouse listener component
	phos_gui_mouse_listener_component *mouse_listener = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_MOUSE_LISTENER);
	if(mouse_listener)
	{
		// first see if the mouse listener is using toggle style
		if(mouse_listener->type == PHOS_GUI_MOUSE_LISTENER_TOGGLED)
		{
			// see if it's on/off
			if(mouse_listener->toggled_on)
			{
				// check to see if mouse button held down over element
				if(mouse_listener->pressed)
					color = ColorTint(mouse_listener->bg_focus_color, mouse_listener->bg_press_color);
				// check to see if they only have the mouse over the element
				else if(mouse_listener->hovered)
					color = ColorTint(mouse_listener->bg_focus_color, mouse_listener->bg_hover_color);
				else if(!mouse_listener->toggled_on)
					// elem's primary bg color is the toggled-off color
					color = e->bg_color;
				// check to see if the elem has focus
				else if(mouse_listener->has_focus)
					color = mouse_listener->bg_focus_color;

				return color;
			}
			else
			{
				// check to see if mouse button held down over element
				if(mouse_listener->pressed)
					color = ColorTint(e->bg_color, mouse_listener->bg_press_color);
				// check to see if they only have the mouse over the element
				else if(mouse_listener->hovered)
					color = ColorTint(e->bg_color, mouse_listener->bg_hover_color);
				else if(!mouse_listener->toggled_on)
					// elem's primary bg color is the toggled-off color
					color = e->bg_color;
				// check to see if the elem has focus
				else if(mouse_listener->has_focus)
					color = ColorTint(e->bg_color, mouse_listener->bg_focus_color);

				return color;
			}
		}

		// check to see if mouse button held down over element
		if(mouse_listener->pressed)
			color = mouse_listener->bg_press_color;
		// check to see if they only have the mouse over the element
		else if(mouse_listener->hovered)
			color = mouse_listener->bg_hover_color;
		// check to see if the elem has focus
		else if(mouse_listener->has_focus)
			color = mouse_listener->bg_focus_color;
	}

	return color;
}
static Color resolve_elem_outline_color(const phos_gui_elem *const e)
{
	if(e->disabled)
		return e->disabled_color;

	// get the normal color of elem
	Color color = e->outline_color;

	// see if elem has a mouse listener component
	phos_gui_mouse_listener_component *mouse_listener = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_MOUSE_LISTENER);
	if(mouse_listener)
	{
		// first see if the mouse listener is using toggle style
		if(mouse_listener->type == PHOS_GUI_MOUSE_LISTENER_TOGGLED)
		{
			// see if it's on/off
			if(mouse_listener->toggled_on)
			{
				// check to see if mouse button held down over element
				if(mouse_listener->pressed)
					color = ColorTint(mouse_listener->outline_focus_color, mouse_listener->outline_press_color);
				// check to see if they only have the mouse over the element
				else if(mouse_listener->hovered)
					color = ColorTint(mouse_listener->outline_focus_color, mouse_listener->outline_hover_color);
				else if(!mouse_listener->toggled_on)
					// elem's primary bg color is the toggled-off color
					color = e->outline_color;
				// check to see if the elem has focus
				else if(mouse_listener->has_focus)
					color = mouse_listener->outline_focus_color;

				return color;
			}
			else
			{
				// check to see if mouse button held down over element
				if(mouse_listener->pressed)
					color = ColorTint(e->outline_color, mouse_listener->outline_press_color);
				// check to see if they only have the mouse over the element
				else if(mouse_listener->hovered)
					color = ColorTint(e->outline_color, mouse_listener->outline_hover_color);
				else if(!mouse_listener->toggled_on)
					// elem's primary bg color is the toggled-off color
					color = e->outline_color;
				// check to see if the elem has focus
				else if(mouse_listener->has_focus)
					color = ColorTint(e->outline_color, mouse_listener->outline_focus_color);

				return color;
			}
		}

		// check to see if mouse button held down over element
		if(mouse_listener->pressed)
			color = mouse_listener->outline_press_color;
		// check to see if they only have the mouse over the element
		else if(mouse_listener->hovered)
			color = mouse_listener->outline_hover_color;
		// check to see if the elem has focus
		else if(mouse_listener->has_focus)
			color = mouse_listener->outline_focus_color;
	}

	return color;
}

static void render_children(phos_gui_elem *e, phos_gui_elem_bounding_box bounds);
static void render_elem(phos_gui_elem *e)
{
	// if elem should not be auto-rendered, skip it
	if(!e->auto_render)
		return;

	// cannot render invalid elements
	if(e->type == PHOS_GUI_TYPE_INVALID)
	{
		vl_delay_log(VL_ERROR, 5.0f, "Cannot render element with invalid type: '%s'!\n", e->ID);
		return;
	}
	// skip elements with no render mode by going to render_children tag
	else if(e->render_mode == PHOS_GUI_RENDER_BLANK)
		goto render_children;

	// get color of elem
	const Color primary_color = resolve_elem_bg_color(e);

	// if empty size, cannot render
	if(e->bounds.width <= 0 || e->bounds.height <= 0)
	{
		vl_delay_log(VL_ERROR, 5.0f, "Cannot render element '%s' with negative visual bounds: %.2f, %.2f!\n", e->ID, e->bounds.width, e->bounds.height);
		return;
	}

	// create elem rects:
	const Rectangle whole_content_bounds = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_TOTAL);
	const Rectangle usable_content_bounds = get_calculated_elem_rect(e, PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE);

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

	// draw elem texture if it has a texture component and render mode indicates the texture should be rendered
	phos_gui_texture_component *texture = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_TEXTURE);
	if(e->render_mode == PHOS_GUI_RENDER_TEXTURE && texture && texture->src && IsTextureValid(*texture->src))
	{
		if(primary_color.a == 0)
			vl_delay_log(VL_WARNING, 5.0f, "Cannot render element with 0 alpha: '%s'!\n", e->ID);

		Rectangle src_rect = { 0, 0, texture->src->width, texture->src->height };
		DrawTexturePro(*texture->src, src_rect, e->bounds, PHOS_GUI_WINDOW_ORIGIN, 0.0f, primary_color);
	}
	// else just draw base shape (if set)
	else if(e->render_mode == PHOS_GUI_RENDER_FILL_OUTLINE || e->render_mode == PHOS_GUI_RENDER_FILL)
	{
		if(primary_color.a == 0)
			vl_delay_log(VL_WARNING, 5.0f, "Cannot render element with 0 alpha: '%s'!\n", e->ID);

		phos_gui_fill_shape(e->shape, e->bounds.x, e->bounds.y, e->bounds.width, e->bounds.height, e->outline_thickness, e->corner_radius, primary_color);
	}

	// render progress bar over background immediately
	phos_gui_value_bar_component *value_bar = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_VALUE_BAR);
	if(value_bar)
	{
		// calculate percentage of progress (curr / max)
		float max_value = value_bar->max_value;
		float curr_value = value_bar->curr_value;
		float percentage_complete = curr_value / max_value;

		/*
		   draw rectangle starting at same position as element.
		   it size is equal to 'percentage_complete' *
		   element's width.
		   */
		float value_bar_width = percentage_complete * usable_content_bounds.width;

		DrawRectangle(usable_content_bounds.x, usable_content_bounds.y, value_bar_width, usable_content_bounds.height, value_bar->progress_color);

		// if value bar is a slider, render slider knob
		if(value_bar->editable)
		{
			Rectangle slider_knob_rect;
			get_slider_knob_rect(value_bar, &slider_knob_rect);

			Color slider_knob_color = value_bar->slider_knob_has_focus ? value_bar->slider_knob_focus_color : value_bar->slider_knob_color;
			phos_gui_fill_shape(value_bar->slider_knob_shape, slider_knob_rect.x, slider_knob_rect.y, slider_knob_rect.width, slider_knob_rect.height, 1.0f, value_bar->slider_knob_corner_radius, slider_knob_color);
		}
	}

	// get mouse listener component
	const phos_gui_mouse_listener_component *const mouse_listener = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_MOUSE_LISTENER);

	// get scroll pane component
	const phos_gui_scroll_pane_component *const scroll_pane = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_SCROLL_PANE);

	// render text component of element (if valid):
	const phos_gui_text_component *const text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_TEXT);
	if(text)
	{
		// get placeholder data as well (will be NULL if no placeholder text extension found)
		const phos_gui_placeholder_text_extension *const placeholder_text = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT);

		if(text->font && IsFontValid(*text->font))
		{
			if(text->font_size <= 0.0f || ColorIsEqual(text->color, BLANK))
				vl_delay_log(VL_WARNING, 1.0f, "This element's ('%s') text component will not render correctly due to invalid font size, or the color's alpha is 0!\n", e->ID);
			else
			{
				// create clip around text
				Rectangle text_clip_bounds = usable_content_bounds;

				/*
				   begin scissor mode to cut off text that has been scrolled off (use usable content bounds)
				*/
				if(phos_gui_new_clip_r(text_clip_bounds))
				{
					// calculate where to draw the text
					const Vector2 draw_pos = get_text_draw_pos(text, scroll_pane);

					// determine if text's main text, or placeholder text should be rendered
					if(placeholder_text && strlen(text->str) == 0 && strlen(placeholder_text->str) > 0)
						DrawTextEx(*text->font, placeholder_text->str, draw_pos, text->font_size, 0.0f, placeholder_text->color);
					else
						DrawTextEx(*text->font, text->str, draw_pos, text->font_size, 0.0f, text->color);

					// render cursor (only if placeholder text is not being rendered and text has focus)
					if(strlen(text->str) > 0 && text->editable && mouse_listener && mouse_listener->has_focus)
					{
						Vector2 cursor_pos = get_cursor_draw_pos(text, scroll_pane);
						DrawRectangle(cursor_pos.x, cursor_pos.y, CURSOR_WIDTH, text->font_size, text->color);
					}

					// end clip
					phos_gui_end_clip();
				}
			}
		}
		else
			vl_delay_log(VL_WARNING, 5.0f, "Cannot render text component on element '%s' because it does not have a valid font!!\n", e->ID);
	}

	// see if this elem has a shadow component
	phos_gui_shadow_component *shadow = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_SHADOW);
	if(shadow)
	{
		// first, imagine shadow over entire element's visual bounds
		Rectangle shadow_rect = e->bounds;

		switch(shadow->edges)
		{
			case PHOS_GUI_SHADOW_LEFT:
				shadow_rect.x = e->bounds.x - shadow->length;
				shadow_rect.width = shadow->length;
				DrawRectangleGradientH(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->fade_color, shadow->initial_color);
				break;

			case PHOS_GUI_SHADOW_TOP:
				shadow_rect.y = e->bounds.y - shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientV(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->fade_color, shadow->initial_color);
				break;

			case PHOS_GUI_SHADOW_RIGHT:
				shadow_rect.x = e->bounds.x + e->bounds.width;
				shadow_rect.width = shadow->length;
				DrawRectangleGradientH(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->initial_color, shadow->fade_color);
				break;

			case PHOS_GUI_SHADOW_BOTTOM:
				shadow_rect.y = e->bounds.y + e->bounds.height;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientV(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->initial_color, shadow->fade_color);
				break;

			case PHOS_GUI_SHADOW_TOP_LEFT:
				shadow_rect.y = e->bounds.y - shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientV(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->fade_color, shadow->initial_color);

				shadow_rect = e->bounds;
				shadow_rect.x = e->bounds.x - shadow->length;
				shadow_rect.width = shadow->length;
				DrawRectangleGradientH(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->fade_color, shadow->initial_color);

				shadow_rect.x = e->bounds.x - shadow->length;
				shadow_rect.y = e->bounds.y - shadow->length;
				shadow_rect.width = shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientEx(shadow_rect, shadow->fade_color, shadow->fade_color, shadow->initial_color, shadow->fade_color);
				break;

			case PHOS_GUI_SHADOW_TOP_RIGHT:
				shadow_rect.y = e->bounds.y - shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientV(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->fade_color, shadow->initial_color);

				shadow_rect = e->bounds;
				shadow_rect.x = e->bounds.x + e->bounds.width;
				shadow_rect.width = shadow->length;
				DrawRectangleGradientH(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->initial_color, shadow->fade_color);

				shadow_rect.x = e->bounds.x + e->bounds.width;
				shadow_rect.y = e->bounds.y - shadow->length;
				shadow_rect.width = shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientEx(shadow_rect, shadow->fade_color, shadow->initial_color, shadow->fade_color, shadow->fade_color);
				break;

			case PHOS_GUI_SHADOW_BOTTOM_LEFT:
				shadow_rect.y = e->bounds.y + e->bounds.height;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientV(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->initial_color, shadow->fade_color);

				shadow_rect = e->bounds;
				shadow_rect.x = e->bounds.x - shadow->length;
				shadow_rect.width = shadow->length;
				DrawRectangleGradientH(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->fade_color, shadow->initial_color);

				shadow_rect.x = e->bounds.x - shadow->length;
				shadow_rect.y = e->bounds.y + e->bounds.height;
				shadow_rect.width = shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientEx(shadow_rect, shadow->fade_color, shadow->fade_color, shadow->fade_color, shadow->initial_color);
				break;

			case PHOS_GUI_SHADOW_BOTTOM_RIGHT:
				shadow_rect.y = e->bounds.y + e->bounds.height;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientV(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->initial_color, shadow->fade_color);

				shadow_rect = e->bounds;
				shadow_rect.x = e->bounds.x + e->bounds.width;
				shadow_rect.width = shadow->length;
				DrawRectangleGradientH(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->initial_color, shadow->fade_color);

				shadow_rect.x = e->bounds.x + e->bounds.width;
				shadow_rect.y = e->bounds.y + e->bounds.height;
				shadow_rect.width = shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientEx(shadow_rect, shadow->initial_color, shadow->fade_color, shadow->fade_color, shadow->fade_color);
				break;

			case PHOS_GUI_SHADOW_ALL:
				// top left shadow
				shadow_rect.y = e->bounds.y - shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientV(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->fade_color, shadow->initial_color);

				shadow_rect = e->bounds;
				shadow_rect.x = e->bounds.x - shadow->length;
				shadow_rect.width = shadow->length;
				DrawRectangleGradientH(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->fade_color, shadow->initial_color);

				shadow_rect.x = e->bounds.x - shadow->length;
				shadow_rect.y = e->bounds.y - shadow->length;
				shadow_rect.width = shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientEx(shadow_rect, shadow->fade_color, shadow->fade_color, shadow->initial_color, shadow->fade_color);

				// bottom right shadow
				shadow_rect = e->bounds;
				shadow_rect.y = e->bounds.y + e->bounds.height;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientV(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->initial_color, shadow->fade_color);

				shadow_rect = e->bounds;
				shadow_rect.x = e->bounds.x + e->bounds.width;
				shadow_rect.width = shadow->length;
				DrawRectangleGradientH(shadow_rect.x, shadow_rect.y, shadow_rect.width, shadow_rect.height, shadow->initial_color, shadow->fade_color);

				shadow_rect.x = e->bounds.x + e->bounds.width;
				shadow_rect.y = e->bounds.y + e->bounds.height;
				shadow_rect.width = shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientEx(shadow_rect, shadow->initial_color, shadow->fade_color, shadow->fade_color, shadow->fade_color);

				// top right corner
				shadow_rect.x = e->bounds.x + e->bounds.width;
				shadow_rect.y = e->bounds.y - shadow->length;
				shadow_rect.width = shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientEx(shadow_rect, shadow->fade_color, shadow->initial_color, shadow->fade_color, shadow->fade_color);

				// bottom left corner
				shadow_rect.x = e->bounds.x - shadow->length;
				shadow_rect.y = e->bounds.y + e->bounds.height;
				shadow_rect.width = shadow->length;
				shadow_rect.height = shadow->length;
				DrawRectangleGradientEx(shadow_rect, shadow->fade_color, shadow->fade_color, shadow->fade_color, shadow->initial_color);
				break;

			default:
				vl_log(VL_ERROR, "A shadow component requires a valid position. See phos_gui_shadow_component.edges!\n");
				break;
		}
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
			Color outline_color = resolve_elem_outline_color(e);

			if(outline_color.a == 0)
				vl_delay_log(VL_WARNING, 5.0f, "Cannot render element outline with 0 alpha: '%s'!\n", e->ID);

			phos_gui_outline_shape(e->shape, e->bounds.x, e->bounds.y, e->bounds.width, e->bounds.height, e->outline_thickness, e->corner_radius, outline_color);
		}
	}

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

	// should there be a clip rect around the element's children?
	phos_gui_elem_bounding_box child_clip_bounds = PHOS_GUI_ELEM_BOUNDS_NONE;

	// render scroll bar if necessary
	if(scroll_pane)
	{
		// when using a scroll pane, clip around free content rect
		child_clip_bounds = PHOS_GUI_ELEM_BOUNDS_CONTENT_FREE;

		// obtain bar and thumb rects
		Rectangle v_bar, v_thumb, h_bar, h_thumb;
		get_scroll_bar_rects(scroll_pane, &v_bar, &v_thumb, &h_bar, &h_thumb);

		// resolve thumb color
		Color v_thumb_color = scroll_pane->v_bar.thumb_has_focus || scroll_pane->v_bar.thumb_grabbed ? scroll_pane->v_bar.thumb_focus_color : scroll_pane->v_bar.thumb_color;
		Color h_thumb_color = scroll_pane->h_bar.thumb_has_focus || scroll_pane->h_bar.thumb_grabbed ? scroll_pane->h_bar.thumb_focus_color : scroll_pane->h_bar.thumb_color;

		// if rendering scroll bar:
		if(scroll_pane->v_bar.rendered && scroll_pane->v_bar.active)
		{
			// render vertical scroll bar
			DrawRectangleRec(v_bar, scroll_pane->v_bar.bg_color);

			// render vertical scroll thumb based on thumb shape
			switch(scroll_pane->v_bar.thumb_shape)
			{
				case PHOS_GUI_SHAPE_RECT:
					DrawRectanglePro(v_thumb, PHOS_GUI_WINDOW_ORIGIN, 0.0f, v_thumb_color);
					break;
				case PHOS_GUI_SHAPE_ELLIPSE:
					{
						float v_thumb_rx = v_thumb.width / 2.0f;
						float v_thumb_ry = v_thumb.height / 2.0f;

						DrawEllipse(v_thumb.x + v_thumb_rx, v_thumb.y + v_thumb_ry, v_thumb_rx, v_thumb_ry, v_thumb_color);
						break;
					}
				case PHOS_GUI_SHAPE_ROUND_RECT:
					DrawRectangleRounded(v_thumb, scroll_pane->v_bar.thumb_corner_radius, ROUND_RECT_SEGMENTS, v_thumb_color);
					break;
				default:
					vl_log(VL_ERROR, "Invalid scroll thumb shape: %d!\n", scroll_pane->v_bar.thumb_shape);
					break;
			}
		}
		if(scroll_pane->h_bar.rendered && scroll_pane->h_bar.active)
		{
			// render horizontal scroll bar
			DrawRectangleRec(h_bar, scroll_pane->h_bar.bg_color);

			// render horizontal scroll thumb based on thumb shape
			switch(scroll_pane->h_bar.thumb_shape)
			{
				case PHOS_GUI_SHAPE_RECT:
					DrawRectanglePro(h_thumb, PHOS_GUI_WINDOW_ORIGIN, 0.0f, h_thumb_color);
					break;
				case PHOS_GUI_SHAPE_ELLIPSE:
					{
						float h_thumb_rx = h_thumb.width / 2.0f;
						float h_thumb_ry = h_thumb.height / 2.0f;

						DrawEllipse(h_thumb.x + h_thumb_rx, h_thumb.y + h_thumb_ry, h_thumb_rx, h_thumb_ry, h_thumb_color);
						break;
					}
				case PHOS_GUI_SHAPE_ROUND_RECT:
					DrawRectangleRounded(h_thumb, scroll_pane->h_bar.thumb_corner_radius, ROUND_RECT_SEGMENTS, h_thumb_color);
					break;
				default:
					vl_log(VL_ERROR, "Invalid scroll thumb shape: %d!\n", scroll_pane->h_bar.thumb_shape);
					break;
			}
		}
	}

	// see if this elem has a drop down component
	phos_gui_drop_down_component *drop_down = pluto_cs_get_component(e, PHOS_GUI_COMPONENT_DROP_DOWN);
	if(drop_down)
	{
		// render down arrow icon on drop down button when it's not expanded
		Texture2D* down_arrow = phos_gui_get_icon_id(PHOS_GUI_ICON_DOWN_ARROW);
		if(!drop_down->expanded && down_arrow)
		{
			float x = usable_content_bounds.x + usable_content_bounds.width - down_arrow->width * 1.25f;
			float y = usable_content_bounds.y + ((usable_content_bounds.height - down_arrow->height) / 2.0f);
			DrawTexture(*down_arrow, x, y, default_theme.icon_color);
		}
	}

	// render child elements:
	render_children:
	render_children(e, child_clip_bounds);
}
static void render_children(phos_gui_elem *e, phos_gui_elem_bounding_box bounds)
{
	// start a new clip around children based on bounding box given
	bool parent_clipped = false;
	Rectangle clip_rect = {0};
	if(bounds != PHOS_GUI_ELEM_BOUNDS_NONE)
	{
		// elem must have clipping enabled and the clip rect must have been created
		clip_rect = get_calculated_elem_rect(e, bounds);
		parent_clipped = e->clipped && phos_gui_new_clip_r(clip_rect);
	}

	for(size_t i = 0; i < e->num_children; ++i)
	{
		phos_gui_elem *child = e->children[i];

		// TODO SHOULD THIS SECTION BE REMOVED?
		// handle non-inherited clip regions:
		/*if(parent_clip && !child->clipped)
		{
			// temporarily remove parent clip
			phos_gui_end_clip();

			// render child with no clip:
			render_elem(child);

			// restore and reuse parent clip
			parent_clip = phos_gui_new_clip_r(clip_rect);
		}
		else*/

		// automatically inherit parent clip
		render_elem(child);
	}

	// end clip if necessary
	if(parent_clipped)
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

	// handle screen tint
	if(!ColorIsEqual(screen_tint, BLANK))
		DrawRectangleRec(PHOS_GUI_WINDOW_RECT, screen_tint);
}
void phos_gui_fill_shape(phos_gui_shape shape, float x, float y, float w, float h, float outline_thickness, float round_rect_corner_radius, Color color)
{
	Rectangle rect = { x, y, w, h };
	switch(shape)
	{
		case PHOS_GUI_SHAPE_RECT:
			DrawRectanglePro(rect, PHOS_GUI_WINDOW_ORIGIN, 0.0f, color);
			break;
		case PHOS_GUI_SHAPE_ELLIPSE:
			DrawEllipse(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f, rect.width / 2.0f, rect.height / 2.0f, color);
			break;
		case PHOS_GUI_SHAPE_ROUND_RECT:
			{
				rect.x += outline_thickness;
				rect.y += outline_thickness;
				rect.width -= outline_thickness * 2.0f;
				rect.height -= outline_thickness * 2.0f;

				DrawRectangleRounded(rect, round_rect_corner_radius, ROUND_RECT_SEGMENTS, color);
				break;
			}
		default:
			vl_log(VL_ERROR, "Cannot render invalid shape: %d!\n", shape);
			break;
	}
}
void phos_gui_outline_shape(phos_gui_shape shape, float x, float y, float w, float h, float outline_thickness, float round_rect_corner_radius, Color color)
{
	Rectangle rect = { x, y, w, h };
	switch(shape)
	{
		case PHOS_GUI_SHAPE_RECT:
			DrawRectangleLinesEx(rect, outline_thickness, color);
			break;
		case PHOS_GUI_SHAPE_ELLIPSE:
			render_ellipse_outline(phos_gui_get_rect_pos(rect), rect.width / 2.0f, rect.height / 2.0f, outline_thickness, color);
			break;
		case PHOS_GUI_SHAPE_ROUND_RECT:
			{
				rect.x += outline_thickness;
				rect.y += outline_thickness;
				rect.width -= outline_thickness * 2.0f;
				rect.height -= outline_thickness * 2.0f;

				DrawRectangleRoundedLinesEx(rect, round_rect_corner_radius, ROUND_RECT_SEGMENTS, outline_thickness, color);
				break;
			}
		default:
			vl_log(VL_ERROR, "Cannot render invalid shape: %d!\n", shape);
			break;
	}
}
void phos_gui_render_elem(phos_gui_elem *elem)
{
	if(!elem)
	{
		vl_delay_log(VL_ERROR, 3.0f, "Cannot render NULL element!\n");
		return;
	}

	render_elem(elem);
}
Color phos_gui_random_color()
{
	return (Color) { GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
}
phos_gui_theme phos_gui_get_default_theme()
{
	return default_theme;
}
phos_gui_theme phos_gui_create_theme_basic(Color base_color)
{
	phos_gui_theme theme = {0};

	theme.bg_color = base_color;
	theme.outline_color = ColorBrightness(base_color, -0.5f);
	theme.bg_hover_color = ColorBrightness(base_color, -0.1f);
	theme.bg_press_color = ColorBrightness(base_color, -0.2f);
	theme.bg_focus_color = base_color;
	theme.outline_hover_color = theme.outline_color;
	theme.outline_press_color = theme.outline_color;
	theme.outline_focus_color = theme.outline_color;
	theme.decoration_color = ColorContrast(base_color, -0.3f);
	theme.text_color = ColorBrightness(base_color, -0.75f);
	theme.icon_color = ColorContrast(base_color, 0.5f);
	theme.window_bg_color = ColorBrightness(base_color, -0.9f);
	theme.outline_thickness = 5.0f;

	return theme;
}
phos_gui_theme phos_gui_create_theme_accented(Color base_color, Color accent_color)
{
	phos_gui_theme theme = {0};

	theme.bg_color = base_color;
	theme.outline_color = accent_color;
	theme.bg_hover_color = ColorBrightness(base_color, -0.1f);
	theme.bg_press_color = ColorBrightness(base_color, -0.2f);
	theme.bg_focus_color = base_color;
	theme.outline_hover_color = theme.outline_color;
	theme.outline_press_color = theme.outline_color;
	theme.outline_focus_color = theme.outline_color;
	theme.decoration_color = ColorContrast(accent_color, -0.3f);
	theme.text_color = ColorBrightness(accent_color, -0.75f);
	theme.icon_color = ColorContrast(accent_color, 0.5f);
	theme.window_bg_color = PHOS_GUI_COLOR_MIX(ColorContrast(accent_color, -0.65f), ColorBrightness(accent_color, -0.8f));
	theme.outline_thickness = 5.0f;

	return theme;
}
phos_gui_theme phos_gui_create_theme_full(Color base_color, Color accent_color, Color decoration_color, Color text_color, Color icon_color, Color window_bg_color)
{
	phos_gui_theme theme = {0};

	// first create theme using base and accent colors
	theme = phos_gui_create_theme_accented(base_color, accent_color);

	// then override specific colors given
	theme.decoration_color = decoration_color;
	theme.text_color = text_color;
	theme.icon_color = icon_color;
	theme.window_bg_color = window_bg_color;

	return theme;
}
void phos_gui_apply_theme_to_gui(phos_gui *gui, phos_gui_theme theme)
{
	for(size_t i = 0; i < gui->num_elems; ++i)
	{
		// apply theme to parent elem
		phos_gui_elem *elem = gui->elems[i];
		phos_gui_apply_theme_to_elem(elem, theme);

		// then apply theme to all children
		for(size_t j = 0; j < elem->num_children; ++j)
			phos_gui_apply_theme_to_elem(elem->children[j], theme);
	}
}
void phos_gui_apply_theme_to_elem(phos_gui_elem *elem, phos_gui_theme theme)
{
	// elem-specific attributes:

	elem->bg_color = theme.bg_color;
	elem->outline_color = theme.outline_color;
	elem->outline_thickness = theme.outline_thickness;

	phos_gui_mouse_listener_component *mouse_listener = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_MOUSE_LISTENER);
	if(mouse_listener)
	{
		mouse_listener->bg_hover_color = theme.bg_hover_color;
		mouse_listener->bg_press_color = theme.bg_press_color;
		mouse_listener->bg_focus_color = theme.bg_focus_color;
		mouse_listener->outline_hover_color = theme.outline_hover_color;
		mouse_listener->outline_press_color = theme.outline_press_color;
		mouse_listener->outline_focus_color = theme.outline_focus_color;
	}

	phos_gui_text_component *text = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_TEXT);
	if(text)
		text->color = theme.text_color;

	phos_gui_placeholder_text_extension *placeholder_text = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_PLACEHOLDER_TEXT);
	if(placeholder_text)
		placeholder_text->color = ColorContrast(theme.text_color, -0.3f);

	phos_gui_scroll_pane_component *scroll_pane = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_SCROLL_PANE);
	if(scroll_pane)
	{
		scroll_pane->v_bar.bg_color = theme.decoration_color;
		scroll_pane->v_bar.thumb_color = scroll_pane->h_bar.thumb_color = ColorBrightness(theme.decoration_color, -0.4f);
		scroll_pane->v_bar.thumb_focus_color = scroll_pane->h_bar.thumb_focus_color = ColorBrightness(theme.decoration_color, 0.4f);
	}

	phos_gui_drag_pane_component *drag_pane = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_DRAG_PANE);
	if(drag_pane)
		drag_pane->drag_bar_color = theme.decoration_color;

	phos_gui_shadow_component *shadow = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_SHADOW);
	if(shadow)
	{
		shadow->initial_color = ColorBrightness(theme.window_bg_color, -0.3f);
		shadow->fade_color = theme.window_bg_color;
	}

	phos_gui_value_bar_component *value_bar = pluto_cs_get_component(elem, PHOS_GUI_COMPONENT_VALUE_BAR);
	if(value_bar)
	{
		value_bar->progress_color = ColorBrightness(ColorContrast(theme.bg_color, 0.6f), -0.2f);
		value_bar->slider_knob_color = ColorBrightness(value_bar->progress_color, -0.4f);
		value_bar->slider_knob_focus_color = ColorBrightness(value_bar->progress_color, -0.2f);
	}


	// force recalculation of elem rects because outline thickness changed:
	force_calculate_elem_rects(elem);

	// other attributes:

	phos_gui_set_window_bg_color(theme.window_bg_color);
}
void phos_gui_set_default_theme(phos_gui_theme theme)
{
	default_theme = theme;
}
phos_gui_theme phos_gui_brighten_theme(phos_gui_theme theme, float factor)
{
	phos_gui_theme new_theme = theme;

	new_theme.bg_color = ColorBrightness(theme.bg_color, factor);
	new_theme.outline_color = ColorBrightness(theme.outline_color, factor);
	new_theme.bg_hover_color = ColorBrightness(theme.bg_hover_color, factor);
	new_theme.bg_press_color = ColorBrightness(theme.bg_press_color, factor);
	new_theme.bg_focus_color = ColorBrightness(theme.bg_focus_color, factor);
	new_theme.outline_hover_color = ColorBrightness(theme.outline_hover_color, factor);
	new_theme.outline_press_color = ColorBrightness(theme.outline_press_color, factor);
	new_theme.outline_focus_color = ColorBrightness(theme.outline_focus_color, factor);
	new_theme.text_color = ColorBrightness(theme.text_color, factor);
	new_theme.window_bg_color = ColorBrightness(theme.window_bg_color, factor);

	return new_theme;
}
phos_gui_theme phos_gui_saturate_theme(phos_gui_theme theme, float factor)
{
	phos_gui_theme new_theme = theme;

	new_theme.bg_color = ColorContrast(theme.bg_color, factor);
	new_theme.outline_color = ColorContrast(theme.outline_color, factor);
	new_theme.bg_hover_color = ColorContrast(theme.bg_hover_color, factor);
	new_theme.bg_press_color = ColorContrast(theme.bg_press_color, factor);
	new_theme.bg_focus_color = ColorContrast(theme.bg_focus_color, factor);
	new_theme.outline_hover_color = ColorContrast(theme.outline_hover_color, factor);
	new_theme.outline_press_color = ColorContrast(theme.outline_press_color, factor);
	new_theme.outline_focus_color = ColorContrast(theme.outline_focus_color, factor);
	new_theme.text_color = ColorContrast(theme.text_color, factor);
	new_theme.window_bg_color = ColorContrast(theme.window_bg_color, factor);

	return new_theme;
}
void phos_gui_apply_screen_tint(Color color)
{
	screen_tint = color;
}
Color phos_gui_get_screen_tint()
{
	return screen_tint;
}
void phos_gui_set_window_bg_color(Color color)
{
	if(ColorIsEqual(color, BLANK))
		color = WHITE;
	window_bg_color = color;
}
Color phos_gui_get_window_bg_color()
{
	return window_bg_color;
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

	arr_add(&textures, pg_tex, NULL);

	vl_log(VL_SUCCESS, "Loaded texture: '%s'!\n", file_path);

	return &textures.data[textures.size - 1].tex;
}
Texture2D *phos_gui_get_icon_id(phos_gui_icon icon)
{
	// return loaded texture at icon's file path in the icon map
	const char **icon_file_path_value = NULL;
	dynmaps_get(&icons, icon, icon_file_path_value);
	if(icon_file_path_value)
	{
		const char *icon_file_path = *icon_file_path_value;
		return phos_gui_load_texture(icon_file_path);
	}

	vl_log(VL_ERROR, "Failed to obtain icon texture: %d!\n", icon);
	return NULL;
}
// TODO should this still be used? how to incorporate into text components like docs say?
/*Texture2D *phos_gui_get_icon_str(const char *str)
{
	// example str:   '<icon_id=0>'
	// example str 2: '<icon=DOWN_ARROW>'

	// parse an ID:
	if(strncmp(str, "<icon_id=", 9) == 0)
	{
		// get characters after '=' but before closing '>'
		const char *e = str + 9;

		// max of 3 digits
		char icon_id[3];
		size_t icon_id_idx = 0;

		// walk forward until closing '>' is found
		while(*e + 1 != '>' && icon_id_idx < sizeof(icon_id))
		{
			icon_id[icon_id_idx++] = *e;
			e++;
		}
		icon_id[icon_id_idx] = '\0';

		// parse ID
		int real_icon_id = strtol(icon_id, NULL, 10);

		// return texture matching the icon ID parsed, or NULL if invalid ID given
		return phos_gui_get_icon_id(real_icon_id);
	}
	// parse a name:
	else if(strncmp(str, "<icon=", 6) == 0)
	{
		// get characters after '=' but before closing '>'
		const char *e = str + 6;

		char icon_name[64];
		size_t icon_id_idx = 0;

		// walk forward until closing '>' is found
		while(*e + 1 != '>' && icon_id_idx < sizeof(icon_name))
		{
			icon_name[icon_id_idx++] = *e;
			e++;
		}
		icon_name[icon_id_idx] = '\0';

		// parse name
		if(strcmp(icon_name, "DOWN_ARROW"))
			return phos_gui_get_icon_id(PHOS_GUI_ICON_DOWN_ARROW);
		else
		{
			vl_log(VL_ERROR, "Invalid icon name given: '%s'!\n", icon_name);
			return NULL;
		}
	}

	// unable to parse because an invalid string was given
	return NULL;
}*/
void phos_gui_set_icon(phos_gui_icon icon, const char *file_path)
{
	map_add(&icons, icon, file_path);
	vl_log(VL_INFO, "Icon %d loaded with texture '%s'!\n", icon, file_path);
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

	arr_add(&fonts, pg_font, NULL);

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

float phos_gui_randf(float start, float end)
{
	return ((float) rand() / (float) (RAND_MAX)) * (end - start);
}
