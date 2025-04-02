/*
gcc -o poly poly.c `pkg-config --cflags --libs gtk+-3.0` -lm
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>

typedef struct {
    int a, b, c;
    double r1, r2;
    int nb_racine;
} Polynome;

typedef struct {
    GtkEntry *entree_a;
    GtkEntry *entree_b;
    GtkEntry *entree_c;
} Entree;

Polynome qdata;

void calculRacines(Polynome *data) {
    double delta = pow((double)data->b, 2) - (4 * data->a * data->c);
    if (delta > 0) {
        data->r1 = (-data->b - sqrt(delta)) / (2.0 * data->a);
        data->r2 = (-data->b + sqrt(delta)) / (2.0 * data->a);
        data->nb_racine = 2;
    } else if (delta == 0) {
        data->r1 = -data->b / (2.0 * data->a);
        data->nb_racine = 1;
    } else {
        data->nb_racine = 0;
    }
}

static gboolean draw_curve(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    Polynome *data = (Polynome *)user_data;
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, 0, height / 2); 
    cairo_line_to(cr, width, height / 2);
    cairo_move_to(cr, width / 2, 0); 
    cairo_line_to(cr, width / 2, height);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0, 0, 1);
    double scale_x = 20.0;
    double scale_y = 20.0;
    cairo_move_to(cr, 0, height / 2 - (data->c / scale_y));

    for (int x_pixel = 0; x_pixel <= width; x_pixel++) {
        double x = (x_pixel - width / 2) / scale_x;
        double y = data->a * x * x + data->b * x + data->c;
        double y_pixel = height / 2 - (y * scale_y);
        cairo_line_to(cr, x_pixel, y_pixel);
    }
    cairo_stroke(cr);

    return FALSE;
}

static void on_calculate_clicked(GtkButton *button, gpointer user_data) {
    Entree *entries = (Entree *)user_data;
    const char *a_text = gtk_entry_get_text(entries->entree_a);
    const char *b_text = gtk_entry_get_text(entries->entree_b);
    const char *c_text = gtk_entry_get_text(entries->entree_c);
    GtkLabel *result_label = g_object_get_data(G_OBJECT(button), "result_label");
    GtkWidget *drawing_area = g_object_get_data(G_OBJECT(button), "drawing_area");

    qdata.a = atoi(a_text);
    qdata.b = atoi(b_text);
    qdata.c = atoi(c_text);

    calculRacines(&qdata);

    char result[256];
    if (qdata.nb_racine == 2) {
        snprintf(result, sizeof(result), "Racines : %.2f and %.2f", qdata.r1, qdata.r2);
    } else if (qdata.nb_racine == 1) {
        snprintf(result, sizeof(result), "Racine : %.2f", qdata.r1);
    } else {
        snprintf(result, sizeof(result), "Pas de racones réeles !");
    }
    gtk_label_set_text(result_label, result);

    gtk_widget_queue_draw(drawing_area);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *grid, *label_a, *label_b, *label_c, *entree_a, *entree_b, *entree_c;
    GtkWidget *button, *result_label, *drawing_area;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Calculatrice des racines d'un polynome du 2nd degret");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    label_a = gtk_label_new("a:");
    entree_a = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), label_a, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entree_a, 1, 0, 1, 1);

    label_b = gtk_label_new("b:");
    entree_b = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), label_b, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entree_b, 1, 1, 1, 1);

    label_c = gtk_label_new("c:");
    entree_c = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), label_c, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entree_c, 1, 2, 1, 1);

    button = gtk_button_new_with_label("Calculer");
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 2, 1);

    result_label = gtk_label_new("Entréer des valeurs et cliquez sur 'Calculer'");
    gtk_grid_attach(GTK_GRID(grid), result_label, 0, 4, 2, 1);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 400, 300);
    gtk_grid_attach(GTK_GRID(grid), drawing_area, 2, 0, 1, 5);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_curve), &qdata);

    Entree *entrees = g_new(Entree, 1);
    entrees->entree_a = GTK_ENTRY(entree_a);
    entrees->entree_b = GTK_ENTRY(entree_b);
    entrees->entree_c = GTK_ENTRY(entree_c);

    g_object_set_data(G_OBJECT(button), "result_label", result_label);
    g_object_set_data(G_OBJECT(button), "drawing_area", drawing_area);
    g_signal_connect(button, "clicked", G_CALLBACK(on_calculate_clicked), entrees);

    g_signal_connect(window, "destroy", G_CALLBACK(g_free), entrees);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
