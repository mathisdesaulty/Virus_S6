#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>

#define GRAVITY 3
#define FRICTION 0.99
#define BALL_SIZE 10
#define PI 3.14159265358979323846

typedef struct {
    double x, y;
    double vx, vy;
} Ball;

typedef struct {
    Ball *balls;
    int count;
    int capacity;
} BallList;

BallList listBall = {NULL, 0, 0};
int width;
int height;

void ballListInit() {
    listBall.capacity = 10;
    listBall.balls = malloc(listBall.capacity * sizeof(Ball));
    listBall.count = 0;
}

void addBall(double x, double y) {
    if (listBall.count >= listBall.capacity) {
        listBall.capacity *= 2;
        listBall.balls = realloc(listBall.balls, listBall.capacity * sizeof(Ball));
    }

    Ball *ball = &listBall.balls[listBall.count++];
    ball->x = x;
    ball->y = y;
    ball->vx = 0;
    ball->vy = 0;
}

void physique() {
    for (int i = 0; i < listBall.count; i++) {
        Ball *ball = &listBall.balls[i];

        ball->vy += GRAVITY;

        ball->x += ball->vx;
        ball->y += ball->vy;

        ball->vx *= FRICTION;
        ball->vy *= FRICTION;

        if (ball->x - BALL_SIZE < 0) {
            ball->x = BALL_SIZE;
            ball->vx = -ball->vx;
        }

        if (ball->x + BALL_SIZE > width) {
            ball->x = width - BALL_SIZE;
            ball->vx = -ball->vx;
        }

        if (ball->y - BALL_SIZE < 0) {
            ball->y = BALL_SIZE;
            ball->vy = -ball->vy;
        }

        if (ball->y + BALL_SIZE > height) {
            ball->y = height - BALL_SIZE;
            ball->vy = -ball->vy * 0.8;
        }
    }

    for (int i = 0; i < listBall.count - 1; i++) {
        for (int j = i + 1; j < listBall.count; j++) {
            Ball *b1 = &listBall.balls[i];
            Ball *b2 = &listBall.balls[j];
            double dx = b2->x - b1->x;
            double dy = b2->y - b1->y;
            double distance = sqrt(dx * dx + dy * dy);

            if (distance < 2 * BALL_SIZE) {
                double nx = dx / distance;
                double ny = dy / distance;
                double relativeVx = b2->vx - b1->vx;
                double relativeVy = b2->vy - b1->vy;
                double scalaire = relativeVx * nx + relativeVy * ny;

                if (scalaire < 0) {
                    double impulse = 2 * scalaire / 2;
                    b1->vx += impulse * nx;
                    b1->vy += impulse * ny;
                    b2->vx -= impulse * nx;
                    b2->vy -= impulse * ny;

                    double overlap = 2 * BALL_SIZE - distance;
                    b1->x -= overlap * nx * 0.5;
                    b1->y -= overlap * ny * 0.5;
                    b2->x += overlap * nx * 0.5;
                    b2->y += overlap * ny * 0.5;
                }
            }
        }
    }
}

static gboolean drawBalls(GtkWidget *widget, cairo_t *cr, gpointer data) {
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0, 0, 1);
    for (int i = 0; i < listBall.count; i++) {
        Ball *ball = &listBall.balls[i];
        cairo_arc(cr, ball->x, ball->y, BALL_SIZE, 0, 2 * PI);
        cairo_fill(cr);
    }

    return FALSE;
}

static gboolean update(gpointer data) {
    physique();
    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
}

static gboolean click(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if (event->type == GDK_BUTTON_PRESS) {
        addBall(event->x, event->y);
        gtk_widget_queue_draw(widget);
    }
    return TRUE;
}

static void UI(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *drawing_area;

    window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(drawBalls), NULL);
    g_signal_connect(drawing_area, "button-press-event", G_CALLBACK(click), NULL);
    g_timeout_add(16, update, drawing_area);

    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    GtkApplication *app;
    int status;

    ballListInit();

    app = gtk_application_new("gtk.balls", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(UI), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    free(listBall.balls);
    return status;
}