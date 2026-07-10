#include <math.h>

#include "dynamic_array_spellbook.h"
#include "vibrant_logs.h"
#include "phosphorus_gui.h"

// registry of elems and UI contexts:
typedef struct phos_gui_elem_arr
{
	phos_gui_elem **data;
	size_t size, capacity;
} phos_gui_elem_arr;

// custom phos_gui_texture (holds file path as well as Texture2D)
typedef struct phos_gui_tex
{
	Texture2D tex;
	const char *file_path;
} phos_gui_tex;

// custom phos_gui_font (holds file path as well as Font)
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

static bool phos_gui_is_init = false;
static phos_gui_elem_arr phos_gui_elem_registry;
static phos_gui_tex_arr phos_gui_textures;
static phos_gui_font_arr phos_gui_fonts;
static int phos_gui_auto_id = 0;


void phos_gui_init()
{
	if(phos_gui_is_init)
	{
		vl_log(VL_ERROR, "Cannot re-initialize PhosphorusGUI!\n");
		return;
	}

	dynas_init(&phos_gui_elem_registry);
	dynas_init(&phos_gui_textures);
	dynas_init(&phos_gui_fonts);

	phos_gui_is_init = true;
}
void phos_gui_shutdown()
{
	if(!phos_gui_is_init)
	{
		vl_log(VL_ERROR, "PhosphorusGUI was never initialized, cannot shutdown!\n");
		return;
	}

	dynas_free(&phos_gui_elem_registry);

	// unload every texture loaded
	for(size_t i = 0; i < phos_gui_textures.size; ++i)
	{
		UnloadTexture(phos_gui_textures.data[i].tex);
		vl_log(VL_SUCCESS, "Unloaded texture: '%s'!\n", phos_gui_textures.data[i].file_path);
	}
	dynas_free(&phos_gui_textures);

	// unload every font loaded
	for(size_t i = 0; i < phos_gui_fonts.size; ++i)
	{
		UnloadFont(phos_gui_fonts.data[i].font);
		vl_log(VL_SUCCESS, "Unloaded font: '%s'!\n", phos_gui_fonts.data[i].file_path);
	}
	dynas_free(&phos_gui_fonts);

	phos_gui_is_init = false;
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

	elem -> pos = elem_centered;
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
		vl_log(VL_ERROR, "This element ('%s') has a null font, cannot obtain center with text!\n", elem -> id);
		return v;
	}

	// get center of elem
	v = phos_gui_get_elem_center(elem);

	// measure text
	Vector2 text_bounds = MeasureTextEx(*elem -> text.font, elem -> text.str, elem -> text.font_size, 0.0f);

	// minus half the text's bounds
	v.x -= text_bounds.x / 2.0f;
	v.y -= text_bounds.y / 2.0f;

	return v;
}

void phos_gui_set_elem_bounds(phos_gui_elem *elem, float x, float y, float w, float h)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set bounds of null UI element!\n");
		return;
	}

	elem -> pos = (Vector2) { x, y };
	elem -> size = (Vector2) { w, h };
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
		vl_log(VL_WARNING, "Element outline disabled: '%s'.\n", elem -> id);
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

void phos_gui_set_text_contents(phos_gui_elem *elem, const char *str)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot set string contents of a null UI element!\n");
		return;
	}
	if(!str)
	{
		vl_log(VL_ERROR, "String is null, cannot set UI element's text contents!\n");
		return;
	}

	strcpy(elem -> text.str, str);
	elem -> text.cursor_pos = strlen(str);
}

PHOS_GUI_API int phos_gui_register_elem(phos_gui_elem *elem)
{
	if(!elem)
	{
		vl_log(VL_ERROR, "Cannot register null UI element!\n");
		return 0;
	}
	if(!phos_gui_is_init)
	{
		vl_log(VL_ERROR, "Cannot register element, phos_gui_init() was never called!\n");
		return 0;
	}

	// first auto-generate element's ID if it is set to "<auto-gen>"
	if(strcmp(elem -> id, "<auto-gen>") == 0)
	{
		sprintf(elem -> id, "elem_#%d", phos_gui_auto_id++);
	}

	// find duplicate pointers and IDs:
	for(size_t i = 0; i < phos_gui_elem_registry.size; ++i)
	{
		phos_gui_elem *e = phos_gui_elem_registry.data[i];

		if(e)
		{
			if(e == elem)
			{
				vl_log(VL_ERROR, "Duplicate UI element pointer found: %p!\n", e);
				return 0;
			}
			if(strcmp(e -> id, elem -> id) == 0)
			{
				vl_log(VL_ERROR, "Duplicate ID found: '%s'!\n", e -> id);
				return 0;
			}
		}
	}

	// no duplicate, the elem can be registered
	dynas_add(&phos_gui_elem_registry, elem);

	vl_log(VL_SUCCESS, "Registered element with ID: '%s'!\n", elem -> id);

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

	return 1;
}
phos_gui_elem *phos_gui_get_elem(const char *ID)
{
	if(!ID)
	{
		vl_log(VL_ERROR, "Cannot obtain UI element with null ID!\n");
		return NULL;
	}
	if(!phos_gui_is_init)
	{
		vl_log(VL_ERROR, "Cannot obtain UI element, phos_gui_init() was never called!\n");
		return NULL;
	}

	for(size_t i = 0; i < phos_gui_elem_registry.size; ++i)
	{
		phos_gui_elem *e = phos_gui_elem_registry.data[i];

		if(e)
			if(strcmp(e -> id, ID) == 0)
				return e;
	}

	vl_log(VL_ERROR, "No UI element found with the ID: '%s'\n", ID);
	return NULL;
}

static void phos_gui_update_elem(phos_gui_elem *e)
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
	}
}
// TODO create phos_gui_update function for specific window scaling so GetMousePosition() always works!!
void phos_gui_update(phos_gui *gui)
{
	if(!gui)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot update null phos_gui!\n");
		return;
	}

	for(size_t i = 0; i < gui -> num_elems; ++i)
		phos_gui_update_elem(gui -> elems[i]);
}

static void phos_gui_render_elem(const phos_gui_elem *const e)
{
	// get color of elem
	Color e_color = e -> color;
	
	if(e -> pressed)
		e_color = e -> press_color;
	else if(e -> hovered)
		e_color = e -> hover_color;

	// create elem rect:
	Rectangle e_rect = { e -> pos.x, e -> pos.y, e -> size.x, e -> size.y };

	// create elem ellipse info:
	float e_rx = e -> size.x / 2.0f;
	float e_ry = e -> size.y / 2.0f;

	// draw elem bg if it is valid
	if(e -> bg_texture && IsTextureValid(*e -> bg_texture))
	{
		Rectangle src = { 0, 0, e -> bg_texture -> width, e -> bg_texture -> height };
		DrawTexturePro(*e -> bg_texture, src, e_rect, PHOS_GUI_WIN_ORIGIN, e -> rotation, e_color);
	}
	// else just draw base shape (if set)
	else if(e -> render_mode == PHOS_GUI_FILL_OUTLINE || e -> render_mode == PHOS_GUI_FILL)
	{
		switch(e -> shape)
		{
			case PHOS_GUI_RECT:
				DrawRectanglePro(e_rect, PHOS_GUI_WIN_ORIGIN, e -> rotation, e_color);
				break;
			case PHOS_GUI_ELLIPSE:
				DrawEllipse(e -> pos.x + e_rx, e -> pos.y + e_ry, e_rx, e_ry, e_color);
				break;
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
				DrawRectangleLinesEx(e_rect, e -> outline_thickness, outline_color);
				break;
			case PHOS_GUI_ELLIPSE:
				for(int i = 0; i < e -> outline_thickness + 1; ++i)
					DrawEllipseLines(e -> pos.x + e_rx, e -> pos.y + e_ry, e_rx - i * 0.7f, e_ry - i * 0.7f, outline_color);
				break;
		}
	}

	// render text component of element (if valid):
	switch(e -> type)
	{
		case PHOS_GUI_BUTTON:
			if(e -> text.font && IsFontValid(*e -> text.font))
				DrawTextEx(*e -> text.font, e -> text.str, e -> text.pos, e -> text.font_size, 0.0f, e -> text.color);
			break;
		case PHOS_GUI_TEXT_FIELD:
			break;
	}
}
void phos_gui_render(const phos_gui *const gui)
{
	if(!gui)
	{
		vl_delay_log(VL_ERROR, 1.0f, "Cannot render null phos_gui!\n");
		return;
	}

	for(size_t i = 0; i < gui -> num_elems; ++i)
		phos_gui_render_elem(gui -> elems[i]);
}

Texture2D *phos_gui_load_texture(const char *file_path)
{
	if(!file_path)
	{
		vl_log(VL_ERROR, "Cannot load texture using null file path!\n");
		return NULL;
	}

	// see if an existing texture has been loaded with the given file path
	for(size_t i = 0; i < phos_gui_textures.size; ++i)
		if(strcmp(phos_gui_textures.data[i].file_path, file_path) == 0)
			return &phos_gui_textures.data[i].tex;
	
	Texture2D tex = LoadTexture(file_path);

	if(!IsTextureValid(tex))
	{
		vl_log(VL_ERROR, "Failed to load texture: '%s'!\n", file_path);
		return NULL;
	}

	phos_gui_tex pg_tex = { .tex = tex, .file_path = file_path };

	dynas_add(&phos_gui_textures, pg_tex);

	vl_log(VL_SUCCESS, "Loaded texture: '%s'!\n", file_path);

	return &phos_gui_textures.data[phos_gui_textures.size - 1].tex;
}

Font *phos_gui_load_font(const char *file_path)
{
	if(!file_path)
	{
		vl_log(VL_ERROR, "Cannot load font using null file path!\n");
		return NULL;
	}

	// see if an existing font has been loaded with the given file path
	for(size_t i = 0; i < phos_gui_fonts.size; ++i)
		if(strcmp(phos_gui_fonts.data[i].file_path, file_path) == 0)
			return &phos_gui_fonts.data[i].font;

	Font font = LoadFont(file_path);

	if(!IsFontValid(font))
	{
		vl_log(VL_ERROR, "Failed to load font: '%s'!\n", file_path);
		return NULL;
	}

	phos_gui_font pg_font = { .font = font, .file_path = file_path };

	dynas_add(&phos_gui_fonts, pg_font);

	vl_log(VL_SUCCESS, "Loaded font: '%s'!\n", file_path);

	return &phos_gui_fonts.data[phos_gui_fonts.size - 1].font;
}
