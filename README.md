# BUT Informatique 3

## Jonathan ISAMBOURG

## Projet Compression

### Utilisation

Après avoir dézippé l'archive, ouvrez un terminal à la racine du projet et éxécutez la commande <code>make codec</code>.

Vous pouvez ensuite utiliser la commande <code>codec</code>.

### Paramètres :
- -i : Path de l'image d'origine (obligatoire)  
- -c **OU** -d : Mode du CoDec (Compression/Décompression) (obligatoire)
- -o : Nom de fichier de sortie si vous sauvegardez l'image (facultatif)

Exemple : <code>./codec -i images/image1.pgm -c -o image_format_qtc</code>

### Remarques

- Si vous ajoutez le paramètre **-d**, vous devez obligatoirement fournir en paramètre -i une image dans le format **QTC**.

- Vous pouvez trouver dans le répertoire **images/** des images compressées dans le format QTC et des images décompressées à partir de ce codec.

- Le répertoire **IMAGES.QTC/** contient des images dans le format QTC fournies lors du projet.

- Le répertoire **lib/** contient la libraire **libqtc.so** du projet.