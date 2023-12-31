
#include <g2x.h>
#include <qtree.h>

/*!---------------------------------------
   Affichage d'une grille depuis le QuadTree
   --   en sur-impression sur le pixmap   --
   ---------------------------------------!*/
static void square(int xu, int yl, int w)
{
  #ifdef _FILL_MODE_
    double g=1.-1./sqrt(w);
    g2x_FillRectangle(xu,yl,xu+w,yl+w,(G2Xcolor){g,g,g,0.});
  #else
    g2x_Rectangle(xu,yl,xu+w,yl+w,G2Xwa_c,1);
  #endif

}

static void qnode_show_grid(qnode* node, int xu, int yl, int w)
{
	if (!node) return; /*! sécurité - ne devrait jamais arriver */

  /*! noeud uniforme (par filtrage) : un rectangle et on s'en va */
	if (node->uni) return square(xu,yl,w);

  /*! noeud non uniforme : on descend voir les fils */
	w /= 2;
  switch (w)
  {
    case 1  : /*! niveau pixel : on trace 4 petits carrés et on sort */
      square(xu  ,yl  ,w);
      square(xu+w,yl  ,w);
      square(xu+w,yl+w,w);
      square(xu  ,yl+w,w);
      break;
    default :  /*! niveau supérieur : 4 appels récursifs */
      qnode_show_grid(node->next+0, xu  ,yl  ,w);
      qnode_show_grid(node->next+1, xu+w,yl  ,w);
      qnode_show_grid(node->next+2, xu+w,yl+w,w);
      qnode_show_grid(node->next+3, xu  ,yl+w,w);
      break;
  }
}

extern void qtree_show_grid(qtree* qt)
{
  double dx = g2x_GetZoom()*g2x_GetXPixSize(); /*! facteur d'échelle en x */
  double dy = g2x_GetZoom()*g2x_GetYPixSize(); /*! facteur d'échelle en y */
  glPushMatrix();
    glScalef(dx,-dy,1.);
    /*! appel sur racine du QTree :
     * ATTENTION A L'OFFSET (-qt->width/2) pour centrage sur l'image */
    int xu = -(1<<qt->depth)/2; /*! coin supérieur... */
    int yl = -(1<<qt->depth)/2; /*! .... gauche       */
    qnode_show_grid(qt->map[0],xu,yl,(1<<qt->depth));
  glPopMatrix();
}

/*!---------------------------------------
   Affichage d'une grille depuis le QuadTree
   --   en sur-impression sur le pixmap   --
   ---------------------------------------!*/
static void pixbloc(qnode* node, int xu, int yl, int w)
{
    double g=node->moy/255.;
    //double u=node->uni?0.8*(1.-1./w):1.0;
    g2x_FillRectangle(xu,yl,xu+w,yl+w,(G2Xcolor){g,g,g,0});
}

static void qnode_show_bloc(qnode* node, int xu, int yl, int w)
{
	if (!node) return; /*! sécurité - ne devrait jamais arriver    */
  /*! noeud uniforme (par filtrage) : un rectangle et on s'en va */
	if (node->uni) return pixbloc(node,xu,yl,w);
  /*! noeud non uniforme : on descend voir les fils */
	w /= 2;
  switch (w)
  {
    case 1  : /*! niveau pixel : on trace 4 petits carrés et on sort */
      pixbloc(node->next+0,xu  ,yl  ,w);
      pixbloc(node->next+1,xu+w,yl  ,w);
      pixbloc(node->next+2,xu+w,yl+w,w);
      pixbloc(node->next+3,xu  ,yl+w,w);
      break;
    default :  /*! niveau supérieur : 4 appels récursifs */
      qnode_show_bloc(node->next+0, xu  ,yl  ,w);
      qnode_show_bloc(node->next+1, xu+w,yl  ,w);
      qnode_show_bloc(node->next+2, xu+w,yl+w,w);
      qnode_show_bloc(node->next+3, xu  ,yl+w,w);
      break;
  }
}

extern void qtree_show_bloc(qtree* qt)
{
  double dx = g2x_GetZoom()*g2x_GetXPixSize(); /*! facteur d'échelle en x */
  double dy = g2x_GetZoom()*g2x_GetYPixSize(); /*! facteur d'échelle en y */
  glPushMatrix();
    glScalef(dx,-dy,1.);
    /*! appel sur racine du QTree :
     * ATTENTION A L'OFFSET (-qt->width/2) pour centrage sur l'image */
    int xu = -(1<<qt->depth)/2; /*! coin supérieur... */
    int yl = -(1<<qt->depth)/2; /*! .... gauche       */
    qnode_show_bloc(qt->map[0],xu,yl,(1<<qt->depth));
  glPopMatrix();
}

/*!--------------------------------------- *
 * Affichage de l'histogramme      *
 * ---------------------------------------!*/
extern void qtree_show_histo(int *Hist, int nbsymb)
{
  int    *h,i;
	double  max=0.;

  /* 1°) extraction du max. */
  for (h=Hist; h<Hist+nbsymb; h++) max = MAX(max,(double)*h);

  /* 2°) calibrage */
  max = (g2x_GetYMax()-g2x_GetYMin())/max;
  double width=(g2x_GetXMax()-g2x_GetXMin())/nbsymb;

  /* 3°) tracé */
  double xl=g2x_GetXMin(),
         xr=xl+width,
         yd=g2x_GetYMin(),
         yu;
  for ((h=Hist,i=0); h<Hist+nbsymb; (h++,i++))
  {
    yu = yd+*h*max;
    g2x_FillRectangle(xl,yd,xr,yu,G2Xr_a);
    xl  = xr;
    xr += width;
  }
}

/*!--------------------------------------- *
 * Affichage gnuplot de l'histogramme      *
 * ---------------------------------------!*/
extern bool qtree_plothisto(char* name, qtree *qt, int nsymb)
{
  FILE   *output;
  int    *h,i;
	int     max=0;
  static char filename[256]="";
  static char command[256];
	int    *Hist=NULL;

	if (!qtree_hist(qt,&Hist,nsymb)) return false;

  /* creation du fichier de points */
  sprintf(filename,"qt%d.hist",nsymb);
  if (!(output=fopen(filename,"w"))) return false;

  h = Hist;
	i=0;
	while(h<Hist+nsymb)
	{
		max = MAX(max,*h);
		fprintf(output,"%3d %5d\n",i,*h);
		h++;
		i++;
	}
  fflush(output);
	free(Hist);

  if (!(output=fopen(".plottmp","w"))) return false;
  fprintf(output,"set xrange[%d:%d]\n",0,nsymb);
  fprintf(output,"set yrange[0:%d]\n",max);
  fprintf(output,"plot \"%s\" with line\n",filename);
  fflush(output);
  fclose(output);

  system("gnuplot -persist .plottmp");
  sprintf(command,"rm -f .plottmp qt*.hist");
  system(command);

  return true;
}

