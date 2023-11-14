#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils.h>

/*! retourne k pour n=2^k !*/
extern uint ulog2(uint n)
{
	register uint k=0;
	while (n)
	{
		k++;
		n >>= 1;
	}
	return k;
}

/*! Decoupe un chemin  <path>=<dir/body.ext>
 *  en 3 morceaux <path:dir>, <body>, <ext>                            !*/
extern void path_split(char* path, char** body, char** ext)
{
  *ext=path+strlen(path);
	/*! extraction de l'extension : on cherche le 1er '.' de droite a gauche */
	while (*ext>path && **ext!='/' && **ext!='.') (*ext)--;

	if (**ext=='.')
	{
		**ext=0;
		(*ext)++;
		/*! extraction du nom principal */
		*body=*ext-2;
		while (*body>path && **body!='/')
			(*body)--;
		if (**body=='/')
		{
			**body=0;
			(*body)++;
			return;
		}
		return;
	}
	if ( *ext==path) {**ext=**body='\0';        return; }
	if (**ext=='/')      {*body=*ext+1; **ext='\0'; return; }
}

/* ==================================================== */
/*  ECRITURE / LECTURE BIT A BIT DANS UN BUFFER BINAIRE */

/*! utilitaire : écrit sur la sortie erreur
 *  <n> bits (n<8) de l'octet <x> puis séparateur
 * exp : printnbin(x='01001101'(77), n=5, sep="|")  => 01101|    */
extern void printnbin(uchar x, int n, char *sep)
{
  while (n--) fprintf(stderr,"%1u",(x>>n)&1U);
  fprintf(stderr,"\e[31m(%3d)%s\e[0m",x,sep);
}

/*! version 'simul' de <pushbits>
 *  se contente d'évaluer les tailles des données à écrire
 *  nbit  : nombre de bits à écrire
 *  wbyte : nombre d'octets déjà écrits
 *          (octet courant)
 *  wbit  : bits disponibles en ecriture
 *          dans l'octet courant
    Exp.  : wbyte = 12, wbit = 3, nbit = 5
            '5 bits à écrire dans le 12° octet qui n'a plus que 3 bits dispobible'
            |xxxxx...| --> |xxxxxbbb|bb......|
            En sortie : wbyte=13, wbit=6
 * ---------------------------------------!*/
extern void countbits(size_t nbit, size_t* wbyte, size_t *wbit)
{
//  if ((*wbyte)==0) *wbit = UCHARSIZE; /*! initialisation                                     */
	if (nbit>=(*wbit))                    /*! si on ne peut pas tout copier dans l'octet courant */
	{
		nbit -= *(wbit);
	 (*wbyte)++;                        /*! on incrémente le compteur d'octets                 */
   (*wbit) = UCHARSIZE;                /*! on réinitialise la capacité (octet neuf)           */
	}
	if (nbit==0) return;                /*! si il ne reste rien : c'est fini                   */
	(*wbit) -=nbit;                       /*! si il reste de la place ou des bits a copier       */
	return;
}

/*! Ecriure d'un bloc de <nbit> à une adresse donnée (buffer)
    dest   : adresse de l'octet dans lequel on ecrit
    src    : l'octet contenant la valeur a ecrire
    nbit   : nombre de bits a ecrire
    wbyte  : nombre d'octets déjà écrits (octet courant)
    wbit   : nbe de bits disponibles en ecriture dans l'octet courant
    return : rien
    - Rem. : le passage par adresse de <byte> n'est pas indispensable.
             c'est par souci d'homogeneite avec <pull_bits>
 * ---------------------------------------!*/
extern void push_bits(uchar **dest, uchar* src, int nbit, size_t *wbit)
{
	if (nbit>=(*wbit))           /*! si on ne peut pas tout copier dans l'octet courant */
	{
		nbit -= (*wbit);
    (**dest) |= *src>>nbit;    /*! on copie ce qu'on peut                             */
	  (*dest)++;                 /*! et on passe a l'octet suivant                      */
	  (*wbit) = UCHARSIZE;       /*! en réinitialisant la capacité (octet neuf)         */
	}
	if (nbit==0) return;         /*! si il ne reste rien : c'est fini                   */
	(*wbit) -=nbit;              /*! si il reste de la place ou des bits a copier       */
  (**dest) |= (*src)<<(*wbit);
}

/*! Lecture d'un bloc de <nbit> dans une adresse donnée (buffer)
    dest   : adresse de l'octet dans lequel on ecrit
	  src    : l'octet contenant la valeur a lire
	  nbit   : nombre de bits a ecrire
	  rbit   : nombre de bits restant dans l'octet courant
    return : rien
	  Rem.   : cette fois le passage par adresse de <src> est indispensable.
 * ---------------------------------------!*/
extern void pull_bits(uchar **dest, uchar* src, int nbit, size_t *rbit)
{
	(*src)   = (**dest);
	(*src) <<= (UCHARSIZE-(*rbit));
	(*src) >>= (UCHARSIZE-  nbit );
	/*! src contient min(rbit,nbit) bits à la bonne position */
  /*! on n'a pas completement vidé l'octet courant */
	if (nbit<(*rbit))  { (*rbit) -= nbit; return; }
  /*! on a completement vidé l'octet courant */
  (*dest)++;
	if (nbit==(*rbit)) { (*rbit)=UCHARSIZE; return; }
	nbit -= (*rbit);   /*! on entame l'octet suivant avec ce qu'il reste à lire*/
  (*rbit) = UCHARSIZE-nbit;
  (*src) |= (**dest)>>(*rbit);
}

