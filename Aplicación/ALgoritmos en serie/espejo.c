/*
 * 
 * 
 * COMPILACION: $ gcc `Wand-config --cflags --cppflags` invertir_colores.c -o invertir_colores `Wand-config --ldflags --libs`
 * USO: $ ./invertir_colores imagen.bmp salida.bmp
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>



void espejo(MagickWand *imagen){

	MagickFlopImage(imagen);
}


int main(int argc,char **argv)
{
	//creamos la imagen
	MagickWand *imagen;
	
	//inicializamos la imagen
	imagen = NewMagickWand();
	
	//cargamos la imagen
	MagickReadImage(imagen,argv[1]);
	
	//llamamos a la funcion para reflejar
	espejo(imagen);

	//guardamos la imagen en disco
	MagickWriteImages(imagen,argv[2],MagickTrue);
	
	//liberamos memoria destruyendo la imagen
	imagen = DestroyMagickWand(imagen);
	
	//terminamos ejecucion de la libreria
	MagickWandTerminus();
	
	return 0;
}