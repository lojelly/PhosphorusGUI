#include "vibrant_logs.h"
#include "phosphorus_gui.h"

int main(void)
{
	vl_init();
	phos_gui_init();

	InitWindow(720, 720, "Test");
	SetTargetFPS(60);

	phos_gui gui = {0};
	gui.num_elems = 0;

	phos_gui_elem elem = {0};
	elem.type = PHOS_GUI_BUTTON;
	elem.shape = PHOS_GUI_ELLIPSE;
	elem.render_mode = PHOS_GUI_OUTLINE;
	strcpy(elem.id, "<auto-gen>");
	phos_gui_set_elem_bounds(&elem, 50, 50, 150, 300);
	phos_gui_gen_elem_colors(&elem, WHITE, -0.25f, -0.4f);
	phos_gui_set_elem_outline(&elem, BLACK, 10.0f);
	elem.text.font = phos_gui_load_font("../test_font.ttf");
	elem.text.font_size = 18;
	elem.text.color = BLACK;
	elem.text.editable = false;
	phos_gui_set_text_contents(&elem, "HELLO!");
	elem.text.pos = phos_gui_get_elem_center_with_text(&elem);

	phos_gui_add_elem(&gui, &elem);

	phos_gui_elem elem2 = (phos_gui_elem) {0};
	elem2.type = PHOS_GUI_BUTTON;
	strcpy(elem2.id, "<auto-gen>");
	phos_gui_set_elem_bounds(&elem2, 200, 300, 150, 150);
	phos_gui_gen_elem_colors(&elem2, WHITE, -0.25f, -0.4f);
	elem2.bg_texture = phos_gui_load_texture("../test_btn.png");
	elem2.text.font = phos_gui_load_font("../test_font.ttf");

	phos_gui_add_elem(&gui, &elem2);

	while(!WindowShouldClose())
	{
		float dt = GetFrameTime();

		// update:
		vl_update(dt);

		// update gui
		phos_gui_update(&gui);


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
