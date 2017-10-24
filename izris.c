#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>

#define MAX_WIDTH 800
#define MAX_HEIGHT 800

/*
	conversion_rate_height, conversion_rate_width... ce je okno preveliko, povprecimo
		(conversion_rate_height x conversion_rate_width) polj tabele za prikaz barve na enem px 
*/

int window_width, window_height, conversion_rate_height, conversion_rate_width;

void draw_plate(cairo_t *cr)
{
	cairo_set_source_rgb(cr, 100, 0, 0);
	cairo_set_line_width(cr, 10);

	// cairo_move_to(cr, 500, 100); // starting point
	// cairo_line_to (cr, 500, 110);

	cairo_stroke(cr);
}

gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	draw_plate(cr);

	return FALSE;
}

void init_draw_plate(double** plate, int height, int width)
{
	// zagotovimo, da ne program slucajno risal npr. 1M x 1M slike
	if(height > MAX_HEIGHT) {
		window_height = MAX_HEIGHT;
		// TODO: kam zaokrozimo?
		conversion_rate_height = (height / MAX_HEIGHT);
	}
	else {
		window_height = height;
		conversion_rate_height = 1;
	}

	
	if(width > MAX_WIDTH) {
		window_width = MAX_WIDTH;
		// TODO: kam zaokrozimo?
		conversion_rate_width = (width / MAX_WIDTH);
	}
	else {
		window_width = width;
		conversion_rate_width = 1;
	}

	gtk_init(0, NULL);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);;
	GtkWidget *darea = gtk_drawing_area_new();

	gtk_container_add(GTK_CONTAINER(window), darea);

	// ob 'draw' eventu se vse skupaj ponovno narise
	g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), window_width, window_height); 
	gtk_window_set_title(GTK_WINDOW(window), "Izris temperaturne plosce");

	gtk_widget_show_all(window);

	gtk_main();

}

// https://www.cairographics.org/manual/cairo-cairo-t.html
// Compilanje celotnega programa (Linux)
// gcc temp_plosca.c -o temp_plosca `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`