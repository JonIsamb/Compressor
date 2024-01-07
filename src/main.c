/*!=================================================================!*/
/*!= E.Incerti - eric.incerti@univ-eiffel.fr                       =!*/
/*!= Université Gustave Eiffel                                     =!*/
/*!= Code "squelette" pour prototypage avec libg2x.6e              =!*/
/*!=================================================================!*/

/* le seul #include nécessaire a priori
 * contient les libs C standards et OpenGl */
#include <qtc.h>

/* tailles de la fenêtre graphique (en pixels)     */

/* limites de la zone reelle associee a la fenetre
 * ATTENTION : ces valeurs doivent être compatibles avec
 *             les tailles WWIDTH et WHEIGHT
 *             (wxmax-wxmin)/(wymax-wymin) = WWIDTH/WHEIGHT
 **/
bool VERB = true;
static double     wxmin,wymin,wxmax,wymax,xyratio;
static char       rootname[128]="";

/* -----------------------------------------------------------------------
 * ici, en général pas mal de variables GLOBALES
 * - les variables de données globales (points, vecteurs....)
 * - les FLAGS de dialogues
 * - les paramètres de dialogue
 * - ......
 * Pas trop le choix, puisque TOUT passe par des fonctions <void f(void)>
 * ----------------------------------------------------------------------- */
static qtree *qt = NULL;
static G2Xpixmap *img = NULL;
static double alpha = 0.0;
static double beta = 1.0;
static bool COMPRESSION_MODE = false;
char *input_file = NULL;
char *output_file = NULL;

/* la fonction d'initialisation : appelée 1 seule fois, au début     */
static void init(void)
{
  if (COMPRESSION_MODE) {
    g2x_PixmapPreload(img);
    pixmap_to_qtree(img, &qt);
  } else {
    g2x_PixmapPreload(img);
  }
}

static void write_to_file(void)
{
  static int id=0;
  char fullname[256];
  char extension[4];

  if (COMPRESSION_MODE) {
    sprintf(extension, "qtc");
  } else {
    sprintf(extension, "pgm");
  }

  if (output_file == NULL) {
    sprintf(fullname,"%s.%02d.%s",rootname, id, extension);
  } else {
    sprintf(fullname, "%s.%s", output_file, extension);
  }
  
  fprintf(stderr,"\n");

  if (COMPRESSION_MODE) {
    qtree_fwrite_Q1(qt, fullname);
  } else {
    pixmap_to_pgm(img, fullname);
  }
  
  id++;
}

/* la fonction de contrôle : appelée 1 seule fois, juste APRES <init> */
static void ctrl(void)
{
  g2x_CreateButton("IMG/QTREE", "switch image / qtree");
  g2x_CreateButton("GRID", "show grid");
  g2x_CreatePopUp ("SAVE",write_to_file,"Save to File");
  g2x_CreateScrollv_d("alpha", &alpha, 0.,+1.,"alpha");
}

/* la fonction de contrôle : appelée 1 seule fois, juste APRES <init> */
static void evts(void)
{
}

/* la fonction de dessin : appelée en boucle (indispensable) */
static void draw(void)
{
  switch(g2x_GetButton())
  {
    case 0 : qtree_show_bloc(qt); break;
    case 1 : qtree_show_grid(qt); break;
    default : g2x_PixmapRecall(img, true); break;
  }
}

/* la fonction d'animation : appelée en boucle draw/anim/draw/anim... (facultatif) */
static void anim(void)
{
}

/* la fonction de sortie  (facultatif) */
static void quit(void)
{
  g2x_PixmapFree(&img);
  qtree_free(&qt);
}

/***************************************************************************/
/* La fonction principale : NE CHANGE (presque) JAMAIS                     */
/***************************************************************************/
int main(int argc, char **argv)
{
  int opt;
  int c_flag = 0, d_flag = 0;

  while ((opt = getopt(argc, argv, "i:o:cd")) != -1) {
    switch (opt) {
      case 'i':
        input_file = optarg;
        break;
      case 'o':
        output_file = optarg;
        break;
      case 'c':
        if (d_flag) {
          fprintf(stderr, "Erreur: Impossible d'utiliser les options -c and -d en même temps.\n");
          return 1;
        }
        c_flag = 1;
        break;
      case 'd':
        if (c_flag) {
          fprintf(stderr, "Erreur: Impossible d'utiliser les options -c and -d en même temps.\n");
          return 1;
        }
        d_flag = 1;
        break;
      default:
        fprintf(stderr, "Usage: %s [-i <path_to_image_input>] [-c]|[-d] [-o <path_to_image_output>]\n", argv[0]);
        return 1;
    }
  }

    if (!input_file) {
      fprintf(stderr, "Erreur: Veuillez renseigner l'image de départ.\n");
      fprintf(stderr, "Usage: %s -i <input> -d|-c -o <output>\n", argv[0]);
      return 1;
    }

    printf("Input file: %s\n", input_file);
    printf("Output file: %s\n", output_file);
    if (c_flag) {
      COMPRESSION_MODE = true;
    }
    if (d_flag) {
      COMPRESSION_MODE = false;
    }

  if (optind > argc) {
    fprintf(stderr, "Usage: %s [-i <path_to_image_input>] [-c]|[-d] [-o <path_to_image_output>]\n", argv[0]);
    return 1;
  }

  // Pour ce genre d'application, on commence par charger l'image
  // => ça permet de connaître les dimensions pour ajuster les fenêtres
  // => et en cas de pépin, on s'arrête là.
  if (COMPRESSION_MODE) {
    if (!g2x_AnyToPixmap(&img,input_file))
    { fprintf(stderr,"\e[43m<%s>\e[0m Erreur d'ouverture image <%s>\n",argv[0],input_file); return 1; }
  } else {
    qtree_fread_Q1(&qt, input_file);
    qtree_to_pixmap(qt, &img);
  }

  char* sep;
  sep=strrchr(input_file,'/');
	if (sep) input_file=sep+1;

  sep=strrchr(input_file,'.');
 *sep=0;

  sprintf(rootname,"%s",input_file);

  int width = MIN(img->width,1600);
  width = MAX(width,512);
  int height = MIN(img->height,800);
  height = MAX(height,512);

  qtree_alloc(&qt, log2(width));

  g2x_InitWindow(rootname,width,height);
  // ratio x/y
  xyratio=((double)img->width)/((double)img->height);
  // zone réelle
  wxmin = -1.; wymin = -xyratio;
  wxmax = +1.; wymax = +xyratio;
  g2x_SetWindowCoord(wxmin,wymin,wxmax,wymax);

  /* 3°) association des fonctions */
  g2x_SetInitFunction(init); /* fonction d'initialisation */
  g2x_SetCtrlFunction(ctrl); /* fonction de contrôle      */
  g2x_SetEvtsFunction(NULL); /* fonction d'événements     */
  g2x_SetDrawFunction(draw); /* fonction de dessin        */
  g2x_SetAnimFunction(anim); /* fonction d'animation      */
  g2x_SetExitFunction(quit); /* fonction de sortie        */

  /* 4°) lancement de la boucle principale */
  return g2x_MainStart();
  /* RIEN APRES CA */
}
