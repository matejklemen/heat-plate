#include <cairo/cairo.h>
#include <math.h>

#define MAX_WIDTH 800
#define MAX_HEIGHT 800

/*
	conversion_rate_height, conversion_rate_width... ce je okno preveliko, povprecimo
		(conversion_rate_height x conversion_rate_width) polj tabele za prikaz barve na enem px 
*/

int window_width, window_height;
double **heatplate;

/*
	Src: http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
	Na naslove kazalcev zapise vrednosti med 0 in 1 (idealno, ker ima cairo tudi RGB predstavljen z vrednostmi od 0 do 1)
*/
void heat_to_color(double normalized_value, float *rval, float *gval, float *bval)
{
	const int NUM_COLORS = 4;
	static float color[4][3] = { {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0} };
	int idx1, idx2;
	float fract_between = 0;

	if(normalized_value <= 0) {
		idx1 = idx2 = 0;
	}
	else if(normalized_value >= 1) {
		idx1 = idx2 = NUM_COLORS-1;
	}
	else {
		normalized_value = normalized_value * (NUM_COLORS - 1);
		idx1  = floor(normalized_value);
		idx2  = idx1 + 1;
		fract_between = normalized_value - (float)idx1;
	}

	*rval   = (color[idx2][0] - color[idx1][0]) * fract_between + color[idx1][0];
	*gval = (color[idx2][1] - color[idx1][1]) * fract_between + color[idx1][1];
	*bval  = (color[idx2][2] - color[idx1][2]) * fract_between + color[idx1][2];
}

void draw_to_file(int height, int width, char *file_name)
{
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

	cairo_t *cr = cairo_create(surface);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_width(cr, 2);

	float r, g, b;

	// risemo px po px
	for (int y = 0; y < height; y += 1) {
		for (int x = 0; x < width; x += 1) {
			double normalized_heat = heatplate[y][x] / 100.0;
			heat_to_color(normalized_heat, &r, &g, &b);
			
			cairo_set_source_rgb(cr, r, g, b);
			cairo_move_to(cr, x, y);
			cairo_close_path(cr);
			cairo_stroke(cr);
		}
	}

	cairo_stroke(cr);
	// shranimo narisano v datoteko
	cairo_surface_write_to_png(surface, file_name);
	
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}

void init_draw_plate(double **plate, int height, int width)
{
	// zagotovimo, da ne program slucajno risal npr. 1M x 1M slike
	// TODO: logika za zmanjsanje ogromne plosce na dimenzije < (MAX_WIDTH x MAX_HEIGHT)
	if(height > MAX_HEIGHT || width > MAX_WIDTH)
		return;

	window_width = width;
	window_height = height;
	heatplate = plate;

	draw_to_file(height, width, "plosca.png");
}

// https://www.cairographics.org/manual/cairo-cairo-t.html
// Compilanje celotnega programa (Linux)
// gcc temp_plosca.c -o temp_plosca `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lm