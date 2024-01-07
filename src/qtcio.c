/*!-----------------------------------------------------------
    E.Incerti - 22/10/2006 - Universite de Marne-la-Vallee
		Projet Programmation - L3 Informatique - 2006/2007
    R�vision Oct.2023
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
     Ecriture/Lecture d'un bloc de nbit<8 bits dans un buffer de uchar deja partiellement rempli/vid�.
     On travaille dans l'octet courant et si n�cessaire on passe au suivant.
     Ces fonctions ne v�rifient pas si la fin du buffer est atteinte, il faut donc le faire en amont :
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
	/*! remplissage du buffer - it�ratif !*/
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
      /*! pour le 4� fils, on n'ecrit que [e|(u)] */
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
 *              Sauve sur fichier au format .qtc  Q1            *
 * ------------------------------------------------------------!*/
extern bool qtree_fwrite_Q1(qtree *qt, char *filename)
{
	if (qt==NULL) return false;

  fprintf(stderr, "Sauvegarde au format Q1...\n");

	/*! mise en place du buffer :
	    taille max. des donnees codees : au pire (8+2+1) bits par "pixel" a coder +3 pour les 4�fils */
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
  /*! �criture sur fichier !*/
  FILE  *qtcfile=NULL;
  if (!(qtcfile=fopen(filename,"w")))     fprintf(stderr,"\e[41mErreur ouverture fichier %s\n",filename);

  time_t date  = time(NULL);

	/*! Ecriture de l'en-t�te : hauteur du qtree (log2(taille_image)) */
	fprintf(qtcfile,"Q1\n");
  fprintf(qtcfile,"#---------------------------------------\n");
  fprintf(qtcfile,"# creation %s",ctime(&date));
  fprintf(qtcfile,"# size %lu bytes : compression rate %.2f%%\n",wbyte,(100.*wbyte)/pixsize);
  fprintf(qtcfile,"#---------------------------------------\n");

  /* Affichage de l'en-tête à l'écran */
  fprintf(stderr,"Q1\n");
  fprintf(stderr,"#---------------------------------------\n");
  fprintf(stderr,"# creation %s",ctime(&date));
  fprintf(stderr,"# size %lu bytes : compression rate %.2f%%\n",wbyte,(100.*wbyte)/pixsize);
  fprintf(stderr,"#---------------------------------------\n");

	/*! Lecture de l'en-t�te : hauteur du qtree (log2(taille_image)) */
  /*! donn�es pour le header */
  fwrite(&qt->depth,sizeof(int),1,qtcfile);
  /*! �criture du buffer complet */
  fwrite(buffer,sizeof(uchar),wbyte,qtcfile);
  /*! fermeture/lib�ration */
	fflush(qtcfile);
	fclose(qtcfile);
	free(buffer);

  fprintf(stderr, "Sauvegarde réussie dans le fichier %s\n", filename);

	return true;
}

bool qnode_unify(qnode *node, ushort moy)
{
  if (node == NULL) {
    return false;
  }

  node->moy = moy;
  node->err = 0;
  node->uni = 1;

  if (node->next == NULL) {
    return false;
  }

  qnode_unify(node->next+0, moy);
  qnode_unify(node->next+1, moy);
  qnode_unify(node->next+2, moy);
  qnode_unify(node->next+3, moy);

  return true;
}

static bool qnode_read_Q1(qtree* qt, uchar** bufpos)
{
	qnode **map,*node,*next;
  size_t wbit = UCHARSIZE;
	// /*! remplissage du buffer - it�ratif !*/
  // /*! Traitement de la	racine - niveau 0 */
  qnode* racine = *qt->map;

  pull_bits(bufpos, (uchar*)&racine->moy, UCHARSIZE, &wbit);
  pull_bits(bufpos, (uchar*)&racine->err, 2, &wbit);

  racine->uni = 0;
  if (racine->err == 0) {
    pull_bits(bufpos, (uchar*)&racine->uni, 1, &wbit);
  }

  if (racine->uni == 1) {
    qnode_unify(racine, racine->moy);

    return true;
  }

  //printf("Moy : %d | Err : %d | Uni : %d\n", racine->moy, racine->err, racine->uni);

  //printf("Lecture du fichier...\n");

  /*! Traitement des niveaux 1 a n-1 */
  int len;  /*! taille du niveau courant */
	for ( (map=qt->map, len=1); map<(qt->map+qt->depth-1); (map++, len*=4) )
  {
    //printf("Nouveau niveau\n");
		for (node=*map; node<*map+len; node++)
    {
      // printf("Node du niveau pixel\n");
      // printf("NODE : Moy : %d | Err : %d | Uni : %d\n", node->moy, node->err, node->uni);
      if (node->uni == 1) {
        continue;
      }

      int sommeMoy = 0;
      /*! pour les 3 premiers fils */
      for (next=node->next; next<node->next+3; next++)
      {
        pull_bits(bufpos, (uchar*)&next->moy, UCHARSIZE, &wbit);
        sommeMoy += (int)next->moy;
        pull_bits(bufpos, (uchar*)&next->err, 2, &wbit);

        next->uni = 0;
        if (next->err == 0) {
          pull_bits(bufpos, (uchar*)&next->uni, 1, &wbit);
        }

        if (next->uni == 1) {
            qnode_unify(next, next->moy);
            continue;
        }

        // printf("Moy : %d | Err : %d | Uni : %d\n", next->moy, next->err, next->uni);
      }
      
      // Reconstitution de la dernière moyenne
      // printf("Dernière moyenne\n");
      next->moy = (ushort)((4 * node->moy) - sommeMoy + node->err); // TODO FIX
      
      pull_bits(bufpos, (uchar*)&next->err, 2, &wbit);

      next->uni = 0;
      if (next->err == 0) {
        pull_bits(bufpos, (uchar*)&next->uni, 1, &wbit);
      }

      if (next->uni == 1) {
          qnode_unify(next, next->moy);
      }

      // printf("Moy : %d | Err : %d | Uni : %d\n", next->moy, next->err, next->uni);

      // if (next->moy > 300) {
      //   printf("Somme moy : %d | MoyenneNodeParente : %d | Calcul : (4 * %d) - %d + %d= %d\n", sommeMoy, node->moy, node->moy, sommeMoy, node->err, next->moy);
      //   return false;
      // }
      // printf("--------------------------\n");
    }
  }
  //printf("Dernier niveau : pixels\n");
  /*! Traitement du dernier niveau : les pixels */
  for (node=*map; node<*map+len; node++)
  {
    // printf("NODE : Moy : %d | Err : %d | Uni : %d\n", node->moy, node->err, node->uni);
    // Uniforme
    if (node->uni == 1) {
      continue;
    }

    int sommeMoy = 0;

    for (next=node->next; next<node->next+3; next++) {
      pull_bits(bufpos, (uchar*)&next->moy, UCHARSIZE, &wbit);
      next->err = 0;
      next->uni = 1;

      sommeMoy += (int)next->moy;

      // printf("Moy : %u | Err : %d | Uni : %d\n", next->moy, next->err, next->uni);
    }

    // Reconstitution de la dernière moyenne
    // printf("Dernière moyenne\n");
    next->moy = (ushort)((4 * node->moy) - sommeMoy + node->err);
    next->err = 0;
    next->uni = 1;
    // if (next->moy > 300) {
    //   printf("Somme moy : %d | MoyenneNodeParente : %d | Calcul : (4 * %d) - %d + %d= %d\n", sommeMoy, node->moy, node->moy, sommeMoy, node->err, next->moy);
    //   return false;
    // }
    // printf("Moy : %u | Err : %d | Uni : %d\n", next->moy, next->err, next->uni);
  }

  return true;
}

// LECTURE FICHIER .QTC
extern bool qtree_fread_Q1(qtree **qt, char *filename)
{
  if (*qt != NULL) {
    fprintf(stderr,"\e[41mRéinitialisation du qtree... \n");

    qtree_free(qt);
  }

  /*! Lecture sur fichier !*/
  FILE *qtcfile = NULL;
  if (!(qtcfile = fopen(filename, "r"))) {
      fprintf(stderr, "\e[41mErreur ouverture fichier %s\n", filename);
  }

  // magic number
  size_t magic_number_size = 24;
  char buffer_magic_number[magic_number_size];
  if (fgets(buffer_magic_number, sizeof(buffer_magic_number), qtcfile) == NULL) {
      fprintf(stderr, "\e[41mErreur lecture magic number \n");

      return false;
  } else if (strcmp(buffer_magic_number, "Q1\n") != 0) {
      fprintf(stderr, "\e[41mLe fichier n'est pas codé en Q1 \n");

      return false;
  }

  printf("magic number : %s", buffer_magic_number);

  // commentaires
  char c;
  char comment[1024];
  while ((c = fgetc(qtcfile)) == '#') {
      if (fgets(comment, sizeof(comment), qtcfile) == NULL) {
          fprintf(stderr, "Error reading comments\n");

          return false;
      }

      printf("#%s", comment);
  }
  ungetc(c, qtcfile);

  // depth
  int depth;
  if (fread(&depth, sizeof(depth), 1, qtcfile) != 1) {
      fprintf(stderr, "\e[41mErreur lecture depth qtree \n");

      return false;
  }

  printf("depth : %d\n", depth);

  // Allocation qtree
  qtree_alloc(qt, depth);

	/*! mise en place du buffer :
	    taille max. des donnees codees : au pire (8+2+1) bits par "pixel" a coder +3 pour les 4�fils */
	size_t bufsize=pow(4, depth) * 1.5; // 4^n * 1.5

  /*! allocation */
  uchar *buffer = (uchar*)calloc(bufsize,sizeof(uchar));
  if (!buffer)
	{
    fprintf(stderr,"\e[41mErreur allocation buffer (%zu)\e[0m\n",bufsize);

    return false;
  }

  fread(buffer, sizeof(uchar), bufsize, qtcfile);

  /*! remplissage selon le mode */
  uchar *bufpos = buffer;

  // Remplissage du qtree
  qnode_read_Q1(*qt, &bufpos);

  /*! fermeture/lib�ration */
	fflush(qtcfile);
	fclose(qtcfile);
	free(buffer);

	return true;
}


/*==============================================================*/
/*!                 ECRITURE MODE STANDARD "Q2"                 */
/*==============================================================*/
/*!----------------------------------------------------------------------------------------------------
     Ecriture/Lecture d'un bloc de nbit<8 bits dans un buffer de uchar deja partiellement rempli/vid�.
     On travaille dans l'octet courant et si n�cessaire on passe au suivant.
     Ces fonctions ne v�rifient pas si la fin du buffer est atteinte, il faut donc le faire en amont :
     s'assurer que l'octet courant et le suivant sont valides.
   ----------------------------------------------------------------------------------------------------!*/

/*!------------------------------------------------------------ *
 *  Charge le qtreetree dans un buffer bit par bit [m|e|(u)]
 *  mode [m|e|(u)] - ITERATIF
 *  - node      : le noeud courant
 *  - bufpos   : le buffer
 * ------------------------------------------------------------!*/
static bool qnode_write_Q2(qtree* qt, uchar** bufpos)
{
	qnode **map,*node,*next;
  size_t wbit = UCHARSIZE;
	/*! remplissage du buffer - it�ratif !*/
  /*! Traitement de la	racine - niveau 0 */
	node  = *qt->map;
	push_bits(bufpos,(uchar*)&node->moy,UCHARSIZE,&wbit);
	push_bits(bufpos,(uchar*)&node->err,2        ,&wbit);

  if (node->err == 0) push_bits(bufpos,(uchar*)&node->uni,1,&wbit);

  if (node->uni != 1) push_bits(bufpos, (uchar*)&node->var, UCHARSIZE, &wbit);

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
        push_bits(bufpos,(uchar *)&next->moy,UCHARSIZE,&wbit);                       /*! moy (8b) */
        push_bits(bufpos,(uchar *)&next->err,2        ,&wbit);                       /*! err (2b) */
        if (next->err == 0) push_bits(bufpos,(uchar*)&next->uni,1,&wbit);            /*! uni (1b) */
        if (next->uni != 1) push_bits(bufpos, (uchar*)&next->var, UCHARSIZE, &wbit);  /*! var (8b)*/
      }
      /*! pour le 4� fils, on n'ecrit que [e|(u)] */
      push_bits(bufpos,(uchar *)&next->err,2,&wbit);                               /*! err (2b) */
      if (next->err == 0) push_bits(bufpos,(uchar*)&next->uni,1,&wbit);            /*! uni (1b) */
      if (next->var != 1) push_bits(bufpos, (uchar*)&next->var, UCHARSIZE, &wbit);  /*! var (8b)*/
    }
  }
	/*! Traitement du dernier niveau : les pixels */
	for (node=*map; node<*map+len; node++)
  {
    /*! on n'ecrit que les moyennes des 3 premiers fils */
    if (node->uni) continue;
    for (next=node->next; next<node->next+3; next++)
      push_bits(bufpos,(uchar*)&next->moy,UCHARSIZE,&wbit); /*! moy (8b) */
  }
  return true;
}

/*!------------------------------------------------------------ *
 *              Sauve sur fichier au format .qtc  Q2            *
 * ------------------------------------------------------------!*/
extern bool qtree_fwrite_Q2(qtree *qt, char *filename)
{
	if (qt==NULL) return false;

	/*! mise en place du buffer :
	    taille max. des donnees codees : au pire (8+2+1) bits par "pixel" a coder +3 pour les 4�fils */
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
  qnode_write_Q2(qt,&bufpos);

  size_t wbyte = (size_t)(bufpos-buffer);
  /*! �criture sur fichier !*/
  FILE  *qtcfile=NULL;
  if (!(qtcfile=fopen(filename,"w")))     fprintf(stderr,"\e[41mErreur ouverture fichier %s\n",filename);

  time_t date  = time(NULL);

	/*! Ecriture de l'en-t�te : hauteur du qtree (log2(taille_image)) */
	fprintf(qtcfile,"Q2\n");
  fprintf(qtcfile,"#---------------------------------------\n");
  fprintf(qtcfile,"# creation %s",ctime(&date));
  fprintf(qtcfile,"# size %lu bytes : compression rate %.2f%%\n",wbyte,(100.*wbyte)/pixsize);
  fprintf(qtcfile,"#---------------------------------------\n");

	/*! Lecture de l'en-t�te : hauteur du qtree (log2(taille_image)) */
  /*! donn�es pour le header */
  fwrite(&qt->depth,sizeof(int),1,qtcfile);
  /*! �criture du buffer complet */
  fwrite(buffer,sizeof(uchar),wbyte,qtcfile);
  /*! fermeture/lib�ration */
	fflush(qtcfile);
	fclose(qtcfile);
	free(buffer);

	return true;
}

bool qnode_unify2(qnode *node, ushort moy)
{
  if (node == NULL) {
    return false;
  }

  node->moy = moy;
  node->err = 0;
  node->uni = 1;
  node->var = 0;

  if (node->next == NULL) {
    return false;
  }

  qnode_unify2(node->next+0, moy);
  qnode_unify2(node->next+1, moy);
  qnode_unify2(node->next+2, moy);
  qnode_unify2(node->next+3, moy);

  return true;
}

static bool qnode_read_Q2(qtree* qt, uchar** bufpos)
{
	qnode **map,*node,*next;
  size_t wbit = UCHARSIZE;
	// /*! remplissage du buffer - it�ratif !*/
  // /*! Traitement de la	racine - niveau 0 */
  qnode* racine = *qt->map;

  pull_bits(bufpos, (uchar*)&racine->moy, UCHARSIZE, &wbit);
  pull_bits(bufpos, (uchar*)&racine->err, 2, &wbit);

  racine->uni = 0;
  if (racine->err == 0) {
    pull_bits(bufpos, (uchar*)&racine->uni, 1, &wbit);
  }

  racine->var = 0;
  if (racine->uni == 1) {
    qnode_unify2(racine, racine->moy);

    return true;
  }

  pull_bits(bufpos, (uchar*)&racine->var, UCHARSIZE, &wbit);

  printf("Moy : %d | Err : %d | Uni : %d | Var : %lf\n", racine->moy, racine->err, racine->uni, racine->var);

  printf("Lecture du fichier...\n");

  /*! Traitement des niveaux 1 a n-1 */
  int len;  /*! taille du niveau courant */
	for ( (map=qt->map, len=1); map<(qt->map+qt->depth-1); (map++, len*=4) )
  {
    //printf("Nouveau niveau\n");
		for (node=*map; node<*map+len; node++)
    {
      printf("Node du niveau pixel\n");
      printf("NODE : Moy : %d | Err : %d | Uni : %d | Var : %lf\n", node->moy, node->err, node->uni, node->var);
      if (node->uni == 1) {
        continue;
      }

      int sommeMoy = 0;
      /*! pour les 3 premiers fils */
      for (next=node->next; next<node->next+3; next++)
      {
        pull_bits(bufpos, (uchar*)&next->moy, UCHARSIZE, &wbit);
        sommeMoy += (int)next->moy;
        pull_bits(bufpos, (uchar*)&next->err, 2, &wbit);

        next->uni = 0;
        if (next->err == 0) {
          pull_bits(bufpos, (uchar*)&next->uni, 1, &wbit);
        }

        next->var = 0;
        if (next->uni == 1) {
            qnode_unify2(next, next->moy);
            continue;
        }

        pull_bits(bufpos, (uchar*)&racine->var, UCHARSIZE, &wbit);

        printf("Moy : %d | Err : %d | Uni : %d | Var : %lf\n", next->moy, next->err, next->uni, next->var);
      }
      
      // Reconstitution de la dernière moyenne
      // printf("Dernière moyenne\n");
      next->moy = (ushort)((4 * node->moy) - sommeMoy + node->err);
      
      pull_bits(bufpos, (uchar*)&next->err, 2, &wbit);

      next->uni = 0;
      if (next->err == 0) {
        pull_bits(bufpos, (uchar*)&next->uni, 1, &wbit);
      }

      next->var = 0;
      if (next->uni == 1) {
          qnode_unify2(next, next->moy);
      }

      pull_bits(bufpos, (uchar*)&racine->var, UCHARSIZE, &wbit);

      // printf("Moy : %d | Err : %d | Uni : %d | Var : %lf\n", next->moy, next->err, next->uni, next->var);

      // if (next->moy > 300) {
      //   printf("Somme moy : %d | MoyenneNodeParente : %d | Calcul : (4 * %d) - %d + %d= %d\n", sommeMoy, node->moy, node->moy, sommeMoy, node->err, next->moy);
      //   return false;
      // }
      printf("--------------------------\n");
    }
  }
  //printf("Dernier niveau : pixels\n");
  /*! Traitement du dernier niveau : les pixels */
  for (node=*map; node<*map+len; node++)
  {
    printf("NODE : Moy : %d | Err : %d | Uni : %d | Var : %lf\n", node->moy, node->err, node->uni, node->var);
    // Uniforme
    if (node->uni == 1) {
      continue;
    }

    int sommeMoy = 0;

    for (next=node->next; next<node->next+3; next++) {
      pull_bits(bufpos, (uchar*)&next->moy, UCHARSIZE, &wbit);
      next->err = 0;
      next->uni = 1;
      next->var = 0;

      sommeMoy += (int)next->moy;

      printf("Moy : %u | Err : %d | Uni : %d | Var : %lf\n", next->moy, next->err, next->uni, next->var);
    }

    // Reconstitution de la dernière moyenne
    // printf("Dernière moyenne\n");
    next->moy = (ushort)((4 * node->moy) - sommeMoy + node->err);
    next->err = 0;
    next->uni = 1;
    next->var = 0;

    printf("Moy : %u | Err : %d | Uni : %d | Var : %lf\n", next->moy, next->err, next->uni, next->var);
  }

  return true;
}

// LECTURE FICHIER .QTC
extern bool qtree_fread_Q2(qtree **qt, char *filename)
{
  if (*qt != NULL) {
    fprintf(stderr,"\e[41mRéinitialisation du qtree... \n");

    qtree_free(qt);
  }

  /*! Lecture sur fichier !*/
  FILE *qtcfile = NULL;
  if (!(qtcfile = fopen(filename, "r"))) {
      fprintf(stderr, "\e[41mErreur ouverture fichier %s\n", filename);
  }

  // magic number
  size_t magic_number_size = 24;
  char buffer_magic_number[magic_number_size];
  if (fgets(buffer_magic_number, sizeof(buffer_magic_number), qtcfile) == NULL) {
      fprintf(stderr, "\e[41mErreur lecture magic number \n");

      return false;
  } else if (strcmp(buffer_magic_number, "Q1\n") != 0) {
      fprintf(stderr, "\e[41mLe fichier n'est pas codé en Q1 \n");

      return false;
  }

  printf("magic number : %s", buffer_magic_number);

  // commentaires
  char c;
  char comment[1024];
  while ((c = fgetc(qtcfile)) == '#') {
      if (fgets(comment, sizeof(comment), qtcfile) == NULL) {
          fprintf(stderr, "Error reading comments\n");

          return false;
      }

      printf("#%s", comment);
  }
  ungetc(c, qtcfile);

  // depth
  int depth;
  if (fread(&depth, sizeof(depth), 1, qtcfile) != 1) {
      fprintf(stderr, "\e[41mErreur lecture depth qtree \n");

      return false;
  }

  printf("depth : %d\n", depth);

  // Allocation qtree
  qtree_alloc(qt, depth);

	/*! mise en place du buffer :
	    taille max. des donnees codees : au pire (8+2+1) bits par "pixel" a coder +3 pour les 4�fils */
	size_t bufsize=pow(4, depth) * 1.5; // 4^n * 1.5

  /*! allocation */
  uchar *buffer = (uchar*)calloc(bufsize,sizeof(uchar));
  if (!buffer)
	{
    fprintf(stderr,"\e[41mErreur allocation buffer (%zu)\e[0m\n",bufsize);

    return false;
  }

  fread(buffer, sizeof(uchar), bufsize, qtcfile);

  /*! remplissage selon le mode */
  uchar *bufpos = buffer;

  // Remplissage du qtree
  qnode_read_Q2(*qt, &bufpos);

  /*! fermeture/lib�ration */
	fflush(qtcfile);
	fclose(qtcfile);
	free(buffer);

	return true;
}

extern bool pixmap_to_pgm(G2Xpixmap *img, char *filename)
{
  FILE * pgmf = NULL;

  fprintf(stderr, "Sauvegarde au format pgm...\n");

  if (! (pgmf = fopen(filename, "w"))) return false;

  fprintf(pgmf, "P5\n%d %d 255\n",img->width, img->height);

  size_t nbpix = img->width * img->height;
  size_t wbyte = fwrite(img->map, sizeof(uchar), nbpix, pgmf);

  fflush(pgmf);
  fclose(pgmf);

  if (wbyte < nbpix) {
    fprintf(stderr, "Ecriture <%s> : données tronquées\n");

    return false;
  }

  fprintf(stderr, "Sauvegarde réussie dans le fichier %s\n", filename);

  return true;
}