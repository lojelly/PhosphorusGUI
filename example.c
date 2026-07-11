#include "vibrant_logs.h"
#include "phosphorus_gui.h"

int main(void)
{
	vl_init();

	vl_config *vlcfg = vl_curr_config();
	vlcfg -> print_date = false;
	vlcfg -> print_month_name = false;
	vlcfg -> print_time = true;

	phos_gui_init();

	InitWindow(720, 720, "Test");
	SetTargetFPS(60);

	phos_gui gui = {0};

	phos_gui_elem elem1 = (phos_gui_elem) {0};
	strcpy(elem1.id, "<auto-gen>");
	phos_gui_add_elem(&gui, &elem1);

	elem1.type = PHOS_GUI_TEXT_FIELD;
	phos_gui_set_elem_bounds(&elem1, 200, 300, 400, 150);
	phos_gui_gen_elem_colors(&elem1, WHITE, -0.2f, -0.35f);
	elem1.bg_texture = phos_gui_load_texture("../test_btn.png");
	elem1.text.font = phos_gui_load_font("../test_font.ttf");
	phos_gui_init_text(&elem1, "", PHOS_GUI_DEFAULT_FONT_SIZE, BLACK);
	phos_gui_init_placeholder_text(&elem1, "Enter name:", GRAY);
	elem1.left_padding = 8.5f;
	elem1.top_padding = -phos_gui_get_text_bounds(&elem1, elem1.text.placeholder_str).height / 2.0f;
	elem1.right_padding = 30.0f;
	elem1.text.pos = phos_gui_align(&elem1, PHOS_GUI_ALIGN_LEFT);
	phos_gui_center_elem(&elem1, PHOS_GUI_WIN_ORIGIN, PHOS_GUI_WIN_SIZE);
	phos_gui_move_elem(&elem1, 100, 50);

	while(!WindowShouldClose())
	{
		float dt = GetFrameTime();

		// update:
		vl_update(dt);

		// update gui
		phos_gui_update(&gui, dt);



		// render stage:
		BeginDrawing();
		ClearBackground(GRAY);

		// render here:
		phos_gui_render(&gui);

		EndDrawing();
	}

	phos_gui_shutdown();

	CloseWindow();

	return 0;
}
