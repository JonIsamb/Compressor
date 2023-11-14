#ifndef _QTGRAPHICS_
  #define _QTGRAPHICS_

  #include <qtc.h>

  /*!grille de segmentation   */
  void qtree_show_grid(qtree* qt);

  /*!image quadtree           */
  void qtree_show_bloc(qtree* qt);

  /*!histogramme              */
  void qtree_show_histo(int *Hist, int nsymb);

#endif
