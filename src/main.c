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



/* la fonction d'initialisation : appelée 1 seule fois, au début     */
static void init(void)
{
  g2x_PixmapPreload(img);
  pixmap_to_qtree(img, &qt);
}

static void write_to_file(void)
{
  static int id=0;
  char fullname[256];
  sprintf(fullname,"%s.%02d.qtc",rootname,id);
  fprintf(stderr,"\n");
  qtree_fwrite_Q1(qt,fullname);
  id++;
}

/* la fonction de contrôle : appelée 1 seule fois, juste APRES <init> */
static void ctrl(void)
{
  g2x_CreateButton("IMG/QTREE", "switch image / qtree");
  g2x_CreateButton("GRID", "show grid");
  g2x_CreatePopUp ("SAVE",write_to_file,"Save to File     ");
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
  if (argc<2)
  {
    fprintf(stderr,"Usage : %s <path_to_image>\n",argv[0]);
    return 1;
  }
  // Pour ce genre d'application, on commence par charger l'image
  // => ça permet de connaître les dimensions pour ajuster les fenêtres
  // => et en cas de pépin, on s'arrête là.
  if (!g2x_AnyToPixmap(&img,argv[1]))
  { fprintf(stderr,"\e[43m<%s>\e[0m Erreur d'ouverture image <%s>\n",argv[0],argv[1]); return 1; }

  char* sep;
  sep=strrchr(argv[1],'/');
	if (sep) argv[1]=sep+1;

  sep=strrchr(argv[1],'.');
 *sep=0;

  sprintf(rootname,"%s",argv[1]);

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
