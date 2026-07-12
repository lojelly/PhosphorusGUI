#include "vibrant_logs.h"
#include "phosphorus_gui.h"

int main(void)
{
	vl_init();
	phos_gui_init();

	InitWindow(720, 720, "Test");
	SetTargetFPS(60);

	phos_gui gui = {0};

	phos_gui_elem elem1 = {0};
	phos_gui_add_elem_id(&gui, &elem1, "<auto-gen>");

	elem1.type = PHOS_GUI_TEXT_FIELD;
	elem1.shape = PHOS_GUI_RECT;
	elem1.render_mode = PHOS_GUI_TEXTURE;
	phos_gui_set_elem_bounds(&elem1, 200.0f, 300.0f, 400.0f, 150.0f);
	phos_gui_gen_elem_colors(&elem1, WHITE, -0.2f, -0.35f);
	elem1.bg_texture = phos_gui_load_texture("../test_btn.png");
	elem1.text.font = phos_gui_load_font("../test_font.ttf");
	phos_gui_init_text(&elem1, "", PHOS_GUI_DEFAULT_FONT_SIZE, BLACK);
	phos_gui_init_placeholder_text(&elem1, "Enter name:", GRAY);
	Rectangle text_bounds = phos_gui_get_text_bounds(&elem1, elem1.text.placeholder_str);
	phos_gui_set_elem_padding(&elem1, 30, 30, 30, 30);
	elem1.text.pos = phos_gui_align_text(&elem1, PHOS_GUI_ALIGN_INNER_LEFT, elem1.text.placeholder_str);
	phos_gui_center_elem(&elem1, PHOS_GUI_WIN_ORIGIN, PHOS_GUI_WIN_SIZE);

	
	phos_gui_elem elem2 = {0};
	phos_gui_add_elem_id(&gui, &elem2, "<auto-gen>");

	elem2.type = PHOS_GUI_BUTTON;
	elem2.shape = PHOS_GUI_RECT;
	elem2.outline_color = BLACK;
	elem2.outline_thickness = 3.0f;
	phos_gui_set_elem_bounds(&elem2, 100.0f, 500.0f, 200.0f, 100.0f);
	phos_gui_gen_elem_colors(&elem2, WHITE, -0.2f, -0.35f);
	elem2.text.font = elem1.text.font;
	phos_gui_init_text(&elem2, "Click Me!", PHOS_GUI_DEFAULT_FONT_SIZE, BLACK);
	elem2.text.pos = phos_gui_align_text(&elem2, PHOS_GUI_ALIGN_INNER_CENTER, elem2.text.str);
	phos_gui_set_elem_padding(&elem2, 5.0f, 5.0f, 5.0f, 5.0f);
	phos_gui_set_elem_margin(&elem2, 0.0f, 50.0f, 0.0f, 50.0f);
	phos_gui_center_elem(&elem2, PHOS_GUI_WIN_ORIGIN, PHOS_GUI_WIN_SIZE);

	phos_gui_move_elem(&elem2, 0, 100);

	phos_gui_set_gui(&gui);

	while(!WindowShouldClose())
	{
		float dt = GetFrameTime();

		// update:
		vl_update(dt);

		// update gui
		phos_gui_update(dt);

		// toggle debug mode on/off
		if(IsKeyPressed(KEY_F1))
		{
			phos_gui_toggle_debug_mode();
		}


		// render stage:
		BeginDrawing();
		ClearBackground(GRAY);

		// render here:
		phos_gui_render();

		EndDrawing();
	}

	phos_gui_shutdown();

	CloseWindow();

	return 0;
}
