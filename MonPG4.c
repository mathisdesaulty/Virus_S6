/*

gcc -o pi pi.c `pkg-config --cflags --libs gtk+-3.0` -lm

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    int n;          
    double pi_approx;
} PiData;

PiData pi_data = {6, 0.0};

double calculer_pi(int n) {
    if (n < 3) return 0.0;

    double theta = 2.0 * M_PI / n;
    double cote = 2.0 * sin(theta / 2.0);
    double perimetre = n * cote;
    return perimetre / 2.0;
}

static gboolean draw_polygon(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    PiData *data = (PiData *)user_data;
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    double radius = fmin(width, height) * 0.4;
    double center_x = width / 2.0;
    double center_y = height / 2.0;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_set_line_width(cr, 2.0);

    double theta = 2.0 * M_PI / data->n;
    for (int i = 0; i < data->n; i++) {
        double x = center_x + radius * cos(i * theta);
        double y = center_y + radius * sin(i * theta);
        if (i == 0) {
            cairo_move_to(cr, x, y);
        } else {
            cairo_line_to(cr, x, y);
        }
    }
    cairo_close_path(cr);
    cairo_stroke(cr);

    return FALSE;
}

// Callback pour le bouton "Calculer"
static void on_calculate_clicked(GtkButton *button, gpointer user_data) {
    GtkEntry *entry = GTK_ENTRY(user_data);
    GtkLabel *result_label = g_object_get_data(G_OBJECT(button), "result_label");
    GtkWidget *drawing_area = g_object_get_data(G_OBJECT(button), "drawing_area");

    const char *n_text = gtk_entry_get_text(entry);
    pi_data.n = atoi(n_text);

    // Calculer π
    pi_data.pi_approx = calculer_pi(pi_data.n);

    // Mettre à jour le label
    char result[256];
    if (pi_data.pi_approx > 0) {
        snprintf(result, sizeof(result), "Approximation de π : %.15f", pi_data.pi_approx);
    } else {
        snprintf(result, sizeof(result), "Erreur : n doit être >= 3");
    }
    gtk_label_set_text(result_label, result);

    // Redessiner le polygone
    gtk_widget_queue_draw(drawing_area);
}

// Fonction d'activation de l'application
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *grid, *label_n, *entry_n, *button, *result_label, *drawing_area;

    // Créer la fenêtre
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Approximation de π (Archimède)");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    // Créer une grille
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    // Label et entrée pour n
    label_n = gtk_label_new("Nombre de côtés :");
    entry_n = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry_n), "6"); // Valeur par défaut
    gtk_grid_attach(GTK_GRID(grid), label_n, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_n, 1, 0, 1, 1);

    // Bouton Calculer
    button = gtk_button_new_with_label("Calculer");
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 2, 1);

    // Label pour le résultat
    result_label = gtk_label_new("Approximation de π : (entrez n et calculez)");
    gtk_grid_attach(GTK_GRID(grid), result_label, 0, 2, 2, 1);

    // Zone de dessin pour le polygone
    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 400, 300);
    gtk_grid_attach(GTK_GRID(grid), drawing_area, 2, 0, 1, 3);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_polygon), &pi_data);

    // Connecter le bouton
    g_object_set_data(G_OBJECT(button), "result_label", result_label);
    g_object_set_data(G_OBJECT(button), "drawing_area", drawing_area);
    g_signal_connect(button, "clicked", G_CALLBACK(on_calculate_clicked), entry_n);

    // Afficher tout
    gtk_widget_show_all(window);

    // Calcul initial avec la valeur par défaut
    pi_data.n = 6;
    pi_data.pi_approx = calculer_pi(pi_data.n);
    char initial_result[256];
    snprintf(initial_result, sizeof(initial_result), "Approximation de π : %.15f", pi_data.pi_approx);
    gtk_label_set_text(GTK_LABEL(result_label), initial_result);
}

int main(int argc, char *argv[]) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.pi_archimede", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}