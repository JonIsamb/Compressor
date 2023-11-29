/*!-----------------------------------------------------------
    E.Incerti - 22/10/2006 - Universite de Marne-la-Vallee
		Projet Programmation - L3 Informatique - 2006/2007
    Révision Oct.2023
		Interface fichier .qtc<->qtree
-----------------------------------------------------------!*/
#include <stdio.h>
#include <stdlib.h>

#include <utils.h>
#include <qtree.h>

/*==============================================================*/
/*!                 ECRITURE MODE STANDARD "Q1"                 */
/*==============================================================*/
/*!----------------------------------------------------------------------------------------------------
     Ecriture/Lecture d'un bloc de nbit<8 bits dans un buffer de uchar deja partiellement rempli/vidé.
     On travaille dans l'octet courant et si nécessaire on passe au suivant.
     Ces fonctions ne vérifient pas si la fin du buffer est atteinte, il faut donc le faire en amont :
     s'assurer que l'octet courant et le suivant sont valides.
   ----------------------------------------------------------------------------------------------------!*/

/*!------------------------------------------------------------ *
 *  Charge le qtreetree dans un buffer bit par bit [m|e|(u)]
 *  mode [m|e|(u)] - ITERATIF
 *  - node      : le noeud courant
 *  - bufpos   : le buffer
 * ------------------------------------------------------------!*/
static bool qnode_write_Q1(qtree* qt, uchar** bufpos)
{
	qnode **map,*node,*next;
  size_t wbit = UCHARSIZE;
	/*! remplissage du buffer - itératif !*/
  /*! Traitement de la	racine - niveau 0 */
	node  = *qt->map;
	push_bits(bufpos,(uchar*)&node->moy,UCHARSIZE,&wbit);
	push_bits(bufpos,(uchar*)&node->err,2        ,&wbit);

  if (node->err==0) push_bits(bufpos,(uchar*)&node->uni,1,&wbit);

	/*! Traitement des niveaux 1 a n-1 */
  int len;  /*! taille du niveau courant */
	for ( (map=qt->map, len=1); map<(qt->map+qt->depth-1); (map++, len*=4) )
  {
		for (node=*map; node<*map+len; node++)
    {
      if (node->uni) continue;
      /*! pour les 3 premiers fils */
      for (next=node->next; next<node->next+3; next++)
      {
        push_bits(bufpos,(uchar *)&next->moy,UCHARSIZE,&wbit);         /*! moy (8b) */
        push_bits(bufpos,(uchar *)&next->err,2        ,&wbit);         /*! err (2b) */
        if (next->err==0) push_bits(bufpos,(uchar*)&next->uni,1,&wbit);/*! uni (1b) */
      }
      /*! pour le 4° fils, on n'ecrit que [e|(u)] */
      push_bits(bufpos,(uchar *)&next->err,2,&wbit);                   /*! err (2b) */
      if (next->err==0) push_bits(bufpos,(uchar*)&next->uni,1,&wbit);  /*! uni (1b) */
    }
  }
	/*! Traitement du dernier niveau : les pixels */
	for (node=*map; node<*map+len; node++)
  {
    /*! on n'ecrit que les moyennes des 3 premiers fils */
    if (node->uni) continue;
    for (next=node->next; next<node->next+3; next++)
      push_bits(bufpos,(uchar*)&next->moy,UCHARSIZE,&wbit);           /*! moy (8b) */
  }
  return true;
}

/*!------------------------------------------------------------ *
 *              Sauve sur fichier au format .qtc                *
 * ------------------------------------------------------------!*/
extern bool qtree_fwrite_Q1(qtree *qt, char *filename)
{
	if (qt==NULL) return false;

	/*! mise en place du buffer :
	    taille max. des donnees codees : au pire (8+2+1) bits par "pixel" a coder +3 pour les 4°fils */
  size_t pixsize=SQR(1<<qt->depth);
	size_t bufsize=4*pixsize;
  /*! allocation */
  uchar *buffer = (uchar*)calloc(bufsize,sizeof(uchar));
  if (!buffer)
	{
    fprintf(stderr,"\e[41mErreur allocation buffer (%zu)\e[0m\n",bufsize);
    return false;
  }

  /*! remplissage selon le mode */
  uchar *bufpos = buffer;
  qnode_write_Q1(qt,&bufpos);

  size_t wbyte = (size_t)(bufpos-buffer);
  /*! écriture sur fichier !*/
  FILE  *qtcfile=NULL;
  if (!(qtcfile=fopen(filename,"w"))) _ERR_OUVERTURE_(filename);

  time_t date  = time(NULL);

	/*! Ecriture de l'en-tête : hauteur du qtree (log2(taille_image)) */
	fprintf(qtcfile,"Q1\n");
  fprintf(qtcfile,"#---------------------------------------\n");
  fprintf(qtcfile,"# creation %s",ctime(&date));
  fprintf(qtcfile,"# size %lu bytes : compression rate %.2f%%\n",wbyte,(100.*wbyte)/pixsize);
  fprintf(qtcfile,"#---------------------------------------\n");

	/*! Lecture de l'en-tête : hauteur du qtree (log2(taille_image)) */
  /*! données pour le header */
  fwrite(&qt->depth,sizeof(int),1,qtcfile);
  /*! écriture du buffer complet */
  fwrite(buffer,sizeof(uchar),wbyte,qtcfile);
  /*! fermeture/libération */
	fflush(qtcfile);
	fclose(qtcfile);
	free(buffer);

	return true;
}
