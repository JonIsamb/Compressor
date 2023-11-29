#include <g2x.h>
#include <qtree.h>

extern bool qtree_alloc(qtree ** qt, int depth)
{
    if (!((*qt)=(qtree*)calloc(1,sizeof(qtree))))
    {
        fprintf(stderr, "<qtree_alloc()> Erreur d'allocation init");

        return false;
    }

    (*qt)->depth = depth;

    if (!((*qt)->map = (qnode**)calloc(1+depth, sizeof(qnode*))))
    {
        fprintf(stderr, "<qtree_alloc()> Erreur d'allocation qtree\n");

        return false;
    }

    if (!((*qt)->map[0] = (qnode*)calloc(1,sizeof(qnode))))
    {
        fprintf(stderr, "<qtree_alloc()> Erreur d'allocation niveau 0\n");

        return false;
    }

    int i,len;
    for ((i=1, len=4); i<depth+1; (i++, len*=4))
    {
        if (!((*qt)->map[i]=(qnode*)calloc(len,sizeof(qnode))))
        {
            fprintf(stderr, "<qtree_alloc()> Erreur d'allocation niveau %d\n", i);

            return false;
        }
        // chainage de l'arbre : les fils dans le niveau inférieurs
        for (int j=0; j<len; j+=4){
            (*qt)->map[i-1][j/4].next = (*qt)->map[i]+j;
        }
    }
    
    return true;
}

extern bool qtree_free(qtree ** qt)
{
    qnode** map;
	if ((*qt)      == NULL) return false;
	if ((*qt)->map == NULL) return false;
    /*! ATTENTION : on va jusqu'au niveau (depth+1) -- cf. au dessus */
    for (map=(*qt)->map; map<=(*qt)->map+(*qt)->depth; map++)
        if (*map) free(*map);
    free((*qt)->map);
    free((*qt));
	(*qt) = NULL;

	return true;
}

static uchar pixmap_to_qnode(G2Xpixmap *img, qnode * node, int x, int y, int size)
{
    ushort sum = 0;

    size /= 2;
    
    switch (size)
    {
        case 1:
            sum += (ushort)((node->next+0)->moy = *g2x_GetPixel(img, 0, x,   y));   (node->next+0)->uni = 1;
            sum += (ushort)((node->next+1)->moy = *g2x_GetPixel(img, 0, x+1, y));   (node->next+1)->uni = 1;
            sum += (ushort)((node->next+2)->moy = *g2x_GetPixel(img, 0, x+1, y+1)); (node->next+2)->uni = 1;
            sum += (ushort)((node->next+3)->moy = *g2x_GetPixel(img, 0, x,   y+1)); (node->next+3)->uni = 1;
            break;

        default:
            sum += pixmap_to_qnode(img, node->next+0, x,        y,      size);
            sum += pixmap_to_qnode(img, node->next+1, x+size,   y,      size);
            sum += pixmap_to_qnode(img, node->next+2, x+size,   y+size, size);
            sum += pixmap_to_qnode(img, node->next+3, x,        y+size, size);
            break;
    }

    node->moy = (uchar)sum/4;

    node->uni = 1;
    qnode *next;
    for (next = node->next; next < node->next+4; next++) {
        if (!next->uni || (next->moy != node->moy)) {
            node->uni = 0;
            break;
        }
    }

    // VARIANCE
    // node->var = 0;
    // for (next=node->next; next<node->next+4; next++) {
    //     node->var += next->var + SQR(sum-4*next->moy);
    // }
    // node->var /= 4.;
    
    node->err = (uchar)sum%4;
    if (node->err != 0) {
        node->uni = 0;
    }

    fprintf(stderr, "[m:%3d|e:%d|u:%d|>%p]\n",node->moy, node->err, node->uni, node->next);

    return (ushort)node->moy;

}


extern bool pixmap_to_qtree(G2Xpixmap* img, qtree** qt)
{
    size_t n = 2;

    // Image carrée
    if (img->width != img->height) {
        fprintf(stderr, "<pixmap_to_qtree()> Image non carrée");

        return false;
    }

    while ((1 << n) < img->width) {
        n++;
    }

    if ((1 << n) != img->width) {
        fprintf(stderr, "<pixmap_to_qtree()> L'image n'est pas de largeur 2**n");

        return false;
    }

    if ((*qt) != NULL && (*qt)->depth < n) {
        fprintf(stderr, "<pixmap_to_qtree()> Profondeur du qtree dépassée");
        
        return false;
    }

    if ((*qt) == NULL) {
        if (!qtree_alloc(qt, n)) {
            fprintf(stderr, "<pixmap_to_qtree()> Erreur allocation qtree");

            return false;
        }
    }

    qnode *root = (*qt)->map[0];

    pixmap_to_qnode(img, root, 0, 0, (1 << n));

    return true;
}


static bool qnode_to_pixmap(qnode* node, G2Xpixmap *img, int xu, int yl, int w)
{
    /*! noeud uniforme (par filtrage) :
    * on rempli un rectangle uniforme      */
    if (node->uni)
    {
        for (int i=xu;i<xu+w;i++)
            for (int j=yl;j<yl+w;j++)
                *g2x_GetPixel(img,0,i,j) = node->moy;
        return true;
    }
    
    if (!node->next) return false;
    /*! noeud non uniforme                  */
    w /= 2;    /*! on descend voir les fils */
    switch (w)
    {
        case 1 : /*! niveau terminal - pixels */
        *g2x_GetPixel(img,0,xu  ,yl  ) = (node->next+0)->moy;
        *g2x_GetPixel(img,0,xu+w,yl  ) = (node->next+1)->moy;
        *g2x_GetPixel(img,0,xu+w,yl+w) = (node->next+2)->moy;
        *g2x_GetPixel(img,0,xu  ,yl+w) = (node->next+3)->moy;
            break;
        default : /*! niveaux interm�diaires  */
        qnode_to_pixmap(node->next+0, img, xu  ,yl  ,w);
        qnode_to_pixmap(node->next+1, img, xu+w,yl  ,w);
        qnode_to_pixmap(node->next+2, img, xu+w,yl+w,w);
        qnode_to_pixmap(node->next+3, img, xu  ,yl+w,w);
        break;
	}
	return true;
}

extern bool qtree_to_pixmap(qtree* qt, G2Xpixmap **img)
{
    /*! si n�cesaire on cr�e le Pixmap */
    if (*img==NULL)
    {
        if (!g2x_PixmapAlloc(img,(1<<qt->depth),(1<<qt->depth),1,255))
        {
        fprintf(stderr,"{\e[1;31m<pixmap_to_qtree((pixmap*,(qtree**:%p))> : err. alloc (*qt)\e[0m\n",qt);
        return false;
        }
    }
    fprintf(stderr,"size:[%dx%d] layer %d depth:%d\n",(*img)->width,(*img)->height,(*img)->layer,(*img)->depth);
    return qnode_to_pixmap(qt->map[0],(*img),0,0,1<<qt->depth);
}

void save_file(qtree ** qt)
{
    //buffersize = ((64+16+4+1)*11)/8 (if size = 64)

    //buffer bufpos;
    int wbit = 8;
    // push_bit(bufpos, (uchar*)&node->moy, 8, &wbit);
    // push_bit(bufpos, (uchar*)&node->err, 2, &wbit);

    // if (node->err == 0) {
    //     push_bit(bufpos, (uchar*)&node->uni, 1, &wbit);
    // }
}

/*!========================================== *
 *            HISTOGRAMME DU QUADTREE         *
 * ==========================================!*/
static void qnode_hist(qnode *node, int *Hist, int nbsymb)
{
	if (!node || node->uni || !node->next) return;

  qnode* next=node->next;
	qnode_hist(next+0,Hist,nbsymb);
	qnode_hist(next+1,Hist,nbsymb);
	qnode_hist(next+2,Hist,nbsymb);
	qnode_hist(next+3,Hist,nbsymb);

	/* dans l'histogramme on ne met que les 3 premiers fils
   * puisque le 4� n'est pas encod� mais reconstruit      */
	for (next=node->next; next<node->next+3; next++)
	{
		if (next->moy<nbsymb) Hist[next->moy]++;
		else Hist[nbsymb]++;
	}
}

/*!--------------------------------------- *
 * ---------------------------------------!*/
extern bool qtree_hist(qtree *qt, int **Hist, int nbsymb)
{
  if (!qt) return false;

  /* (r�-)allocation �ventuelle */
	if (*Hist==NULL)
		if (!(*Hist=(int*)calloc(nbsymb,sizeof(int)))) return false;

  memset(*Hist,0,nbsymb*sizeof(int)); /* vidange */

	(*Hist)[qt->map[0][0].moy]++; /* racine */
  qnode_hist(*qt->map,*Hist,nbsymb);
  return true;
}