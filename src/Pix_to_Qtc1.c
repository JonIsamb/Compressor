#include <g2x.h>
#include <qtc.h>

       bool VERB = true;
static double     wxmin,wymin,wxmax,wymax,xyratio;
static char       rootname[128]="";
static G2Xpixmap *img=NULL;
static qtree     *qt =NULL; /* le QuadTree  */
static double alfa=0.0;

/*!-------------------------------------------------------------------------!*
 *!                                                                         !*
 *!-------------------------------------------------------------------------!*/
static void init(void)
{
  g2x_PixmapPreload(img);   /*! pré-chargement de l'image d'entrée        */
  pixmap_to_qtree(img,&qt); /*! remplissage du QuadTree depuis l'image    */
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

/*!-------------------------------------------------------------------------!*
 *!                                                                         !*
 *!-------------------------------------------------------------------------!*/
static void ctrl(void)
{
  g2x_CreateButton("I/QT","toggle original/quantif. sqrmap");
  g2x_CreateButton("GRID","toggle segmentation grid       ");
  g2x_CreatePopUp ("SAVE",write_to_file,"Save to File     ");
  g2x_CreateScrollv_d("alfa", &alfa, 0.,+1.,"alfa");
}

/*!-------------------------------------------------------------------------!*
 *!                                                                         !*
 *!-------------------------------------------------------------------------!*/
static void evts(void)
{
}

/*!-------------------------------------------------------------------------!*
 *!                                                                         !*
 *!-------------------------------------------------------------------------!*/
static void draw(void)
{
  switch (g2x_GetButton())
  {
    case 0  : qtree_show_bloc(qt);     break; /*! l'image en sortie du QuadTree */
    case 1  : qtree_show_grid(qt);     break; /*! la grille de segmentation     */
    default : g2x_PixmapRecall(img,true); break; /*! l'image originale             */
  }
}

/*!-------------------------------------------------------------------------!*
 *!                                                                         !*
 *!-------------------------------------------------------------------------!*/
static void quit(void)
{
  g2x_PixmapFree(&img );
  qtree_free(&qt);
}


/*! =============================================== *
 *                                                 *
 * =============================================== */
int main(int argc, char* argv[])
{
  if (!g2x_AnyToPixmap(&img,argv[1])) return 1;

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

  g2x_InitWindow(rootname,width,height);
  // ratio x/y
  xyratio=((double)img->width)/((double)img->height);
  // zone réelle
  wxmin = -1.; wymin = -xyratio;
  wxmax = +1.; wymax = +xyratio;
  g2x_SetWindowCoord(wxmin,wymin,wxmax,wymax);

  g2x_SetInitFunction(init);
  g2x_SetCtrlFunction(ctrl);
  g2x_SetDrawFunction(draw);
  g2x_SetEvtsFunction(evts);
  g2x_SetExitFunction(quit);

  return g2x_MainStart();
}
