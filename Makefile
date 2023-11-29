CPP = 0
DBG = 1

ifeq ($(CPP),1)
	# clang++ n'accepte pas les réels 1.0 sous type <float> => IL VEUT 1.0f => génère une erreur
	# g++ n'aime pas non plus, mais se contente d'un warning
	CC   = g++ -std=c++11
	LANG = ++
else
	CC   = clang -std=c17
	LANG =
endif

# CFLAGS : options de compil. (phase 1 .c->.o)
ifeq ($(DBG),1)
  CFLAGS = -g -Wpointer-arith -Wall
	gdb    = .dbg
else
  CFLAGS = -O2
	gdb    =
endif

# - ajoute le répertoire contenant les <g2x*.h> à la liste des répertoires standards
#   via $(incG2X) définie dans ~∕.bashrc (script d'install)
# - ajoute le rép. local ./include/ à la liste des rép. où chercher les .h
#   permet d'écrire #include <monfichier.h> plutot que #include "../include/monfichier.h"
PFLAGS = $(incG2X) -I./include

# les libs : pour la 2° phase de compil. : l'Editon de Liens
# - précise à l'Editeur de lien où trouver <freeglut> et <libg2x.[vers](++)(.gdb).so>
#   via $(libG2X) définie dans ~∕.bashrc (script d'install)
LFLAGS = $(libG2X)$(LANG)$(gdb)

# répertoire pour les sources .c
SRC = src/

# règle générique de création de xxx.o à partir de SRC/xxx.c
%.o : $(SRC)%.c
	$(CC) $(STD) $(CFLAGS) $(PFLAGS) -c $< -o $@

exec : utils.o qtgraphics.o qtree.o qtcio.o main.o
	$(CC) $^ $(LFLAGS) -o $@

# regle de compilation generique (1 seul fichier local a compiler)
% : %.o
	$(CC) $^ $(LFLAGS) -o $@

#-- Nettoyage
clean :
	rm -f *.o *~

.PHONY : clean ?

# informations sur les paramètres de compilation
? :
	@echo "----informations de compilation----"
	@echo "  COMPIL : $(CC)"
	@echo "  PFLAGS : $(PFLAGS)"
	@echo "  CFLAGS : $(CFLAGS)"
	@echo "  LFLAGS : $(LFLAGS)"

