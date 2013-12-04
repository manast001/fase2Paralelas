/*
 * 
 * 
 * COMPILACION: $ gcc `Wand-config --cflags --cppflags` redimensionar.c -o redimensionar `Wand-config --ldflags --libs`
 * USO: $ ./redimensionar imagen.bmp salida.bmp
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>

void Redimensionar(MagickWand *imagen, int *width, int *height, float porcentaje)
{

	float proporcion;
	*width = MagickGetImageWidth(imagen);
	*height = MagickGetImageHeight(imagen);

	//Conversion porcentaje a factor de proporcion.
	if(porcentaje < 0){
		proporcion = (float) 100/ (float) abs(porcentaje); 
		if((*width /= proporcion) < 1) *width = 1;
		if((*height /= proporcion) < 1) *height = 1;
	}else{
		proporcion = (float) (100 + porcentaje)/ (float) 100;
		printf("%f",proporcion);
		if((*width *= proporcion) < 1) *width = 1;
		if((*height *= proporcion) < 1) *height = 1;
	}


	
	
	

	MagickResizeImage(imagen,*width,*height,LanczosFilter,1);
	
	MagickSetImageCompressionQuality(imagen,95);
}


int main(int argc,char **argv)
{
	//creamos la imagen
	MagickWand *imagen;
	int height, width;
	float porcentaje = strtof(argv[3],NULL);
	//inicializamos la imagen
	imagen = NewMagickWand();
	
	//cargamos la imagen
	MagickReadImage(imagen,argv[1]);
	
	//llamamos a la funcion invertir colores
	Redimensionar(imagen,&width,&height,porcentaje);

	//guardamos la imagen en disco
	MagickWriteImages(imagen,argv[2],MagickTrue);
	
	//liberamos memoria destruyendo la imagen
	imagen = DestroyMagickWand(imagen);
	
	//terminamos ejecucion de la libreria
	MagickWandTerminus();
	
	return 0;
}
