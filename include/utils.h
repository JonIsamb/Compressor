#ifndef UTILS_H
	#define UTILS_H

  /*! pour les types (bool|uchar|...) et macros associ�es !*/
  #include <g2x_types.h>

  /*! renvoie le log. de base de l'entier n !*/
	uint log2_i(uint n);

  /*! Decoupe un chemin  <path>=<dir/body.ext> en  3 morceaux <path:dir>|<body>|<ext> !*/
	void path_split(char* path, char** body, char** ext);

/*!==================================================== */
/*! ECRITURE / LECTURE BIT A BIT DANS UN BUFFER BINAIRE */

/*! utilitaire : �crit sur la sortie standard d'erreur <stderr>
 *  <n> bits (n<8) de l'octet <x> puis s�parateur
 * exp : printnbin(x='01001101'(77), n=5, sep="|")  => 01101|    */
  void printnbin(uchar x, int n, char *sep);

/*! Ecriure d'un bloc de <nbit> � une adresse donn�e (buffer)
    dest   : adresse de l'octet dans lequel on ecrit
    src    : l'octet contenant la valeur a ecrire
    nbit   : nombre de bits a ecrire
    wbyte  : nombre d'octets d�j� �crits (octet courant)
    wbit   : nbe de bits disponibles en ecriture dans l'octet courant
    return : rien
    - Rem. : le passage par adresse de <byte> n'est pas indispensable.
             c'est par souci d'homogeneite avec <pull_bits>
 * ---------------------------------------!*/
  void push_bits(uchar **dest, uchar* src, int nbit, size_t *wbit);

/*! Lecture d'un bloc de <nbit> dans une adresse donn�e (buffer)
    dest   : adresse de l'octet dans lequel on ecrit
	  src    : l'octet contenant la valeur a lire
	  nbit   : nombre de bits a ecrire
	  rbit   : nombre de bits restant dans l'octet courant
    return : rien
	  Rem.   : cette fois le passage par adresse de <src> est indispensable.
 * ---------------------------------------!*/
  void pull_bits(uchar **dest, uchar* src, int nbit, size_t *rbit);


  /*! version 'simul' de <pushbits>
   *  se contente d'�valuer les tailles des donn�es � �crire
   *  nbit  : nombre de bits � �crire
   *  wbyte : nombre d'octets d�j� �crits
   *          (position de l'octet courant)
   *  wbit  : bits disponibles en ecriture
   *          dans l'octet courant
      Exp.  : en entr�e : nbit = 5, wbyte = 12, wbit = 3
              '5 bits � �crire dans le 12� octet qui n'a plus que 3 bits disponibles'
              |xxxxx...| --> |xxxxxbbb|bb......|
              -> en sortie : wbyte=13, wbit=6
   * ---------------------------------------!*/
  void countbits(size_t nbit, size_t* wbyte, size_t *wbit);


#endif
