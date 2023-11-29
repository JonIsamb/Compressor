#ifndef _QTC_IO_
  #define _QTC_IO_

	#include <qtree.h>

  /* écriture QuadTree filtré -> file.qtc                  */
	bool qtree_fwrite_Q1(qtree *qt, char *filename);

  /* A ECRIRE lecture fichier <.qtc> éventuellement filtré */
	bool qtree_fread_Q1(qtree **qt, char *filename);

#endif
