#include "vibrant_logs.h"
#include "phosphorus_gui.h"

int main(void)
{
	vl_init();
	phos_gui_init();

	InitWindow(1280, 720, "Test");
	SetTargetFPS(60);

	// TODO 'scale' field in the gui struct?
	phos_gui gui = {0};
	phos_gui_set_id(&gui, "auto");
	phos_gui_set_gui(&gui);

	phos_gui_elem elem0 = {0};
	phos_gui_add_elem_id(&gui, &elem0, "auto");
	phos_gui_setup_elem(&elem0, PHOS_GUI_TYPE_CONTAINER, PHOS_GUI_RENDER_OUTLINE, 0, 0, 250, 250);
	phos_gui_fill_color_set(&elem0.primary_colors, WHITE);
	phos_gui_fill_color_set(&elem0.outline_colors, BLACK);
	elem0.outline_thickness = 3.5f;
	phos_gui_set_elem_padding(&elem0, 5.0f, 25.0f, 5.0f, 5.0f);
	phos_gui_center_elem(&elem0, PHOS_GUI_WIN_ORIGIN, PHOS_GUI_WIN_SIZE);

	phos_gui_elem elem1 = {0};
	phos_gui_set_id(&elem1, "auto");
	phos_gui_setup_elem(&elem1, PHOS_GUI_TYPE_BUTTON, PHOS_GUI_RENDER_FILL, 0, 0, 100, 100);
	phos_gui_gen_color_set(&elem1.primary_colors, WHITE, -0.1f, -0.2f, 0.0f);
	phos_gui_add_elem_to_container(&elem1, &elem0);
	phos_gui_align_elem(&elem1, PHOS_GUI_ALIGN_INNER_CENTER, &elem0);

	while(!WindowShouldClose())
	{
		float dt = GetFrameTime();

		// update:
		vl_update(dt);

		// update gui
		phos_gui_update(dt);

		// toggle debug mode on/off
		if(IsKeyPressed(KEY_F1))
			phos_gui_toggle_debug_mode();


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
