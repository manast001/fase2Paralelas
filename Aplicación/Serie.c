/*	
	Autor: Adrian Gutierrez
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>


void Negativo (MagickWand *imagen);

int main(int argc,char **argv)
{
	//creamos la imagen
	MagickWand *imagen;
	
	//inicializamos la imagen
	imagen = NewMagickWand();
	
	//cargamos la imagen
	MagickReadImage(imagen, argv[1]); //Primer argumento para cargar la imagen (ejemplo.bmp)
	
	//llamamos a la funcion Negativa
	Negativo(imagen);

	//guardamos la imagen en disco
	MagickWriteImages(imagen, argv[2], MagickTrue); // Segundo argumento para almacenar imagen (negativo.bmp)
	
	//liberamos memoria destruyendo la imagen
	imagen = DestroyMagickWand(imagen);
	
	//terminamos ejecucion de la libreria
	MagickWandTerminus();
	
	return 0;
}

void Negativo (MagickWand *imagen)
{
	PixelIterator *iterator;
	PixelWand **pixels;
	
	iterator = NewPixelIterator(imagen); //Permite recorrer los pixeles de la imagen como vector
	
	double verde, rojo, azul; //Los valores estan en proporciones en als escalas dentro de la imagen

	register ssize_t x; //registrara el valor del indice del pixel de la imagen
	size_t width; //Anchura de la imagen
	ssize_t y; //registrara la altura de la imagen

	
	//******************ITERACION PIXEL POR PIXEL***********************
	for (y=0; y < (ssize_t) MagickGetImageHeight(imagen); y++)
	{
		
		pixels = PixelGetNextIteratorRow(iterator,&width); 
		if (pixels == (PixelWand **) NULL) break; //Permite verificar si la imagen esta cargada
		
		for (x=0; x < (ssize_t) width; x++)
		{
			verde 	= PixelGetGreen(pixels[x]); //Recuperamos el valor del eemento en el pixel
			rojo 	= PixelGetRed(pixels[x]);
			azul	= PixelGetBlue(pixels[x]);
			
			/* Los componentes de los elementos dentro del pixel estan en porcentajes(%) en la escala del pixel (0.000 - 1.000)  */
			
			PixelSetGreen(pixels[x], 1-verde);  //Negativo es el complemento.... por lo que esta en porcentaje es 1-color.
			PixelSetRed(pixels[x], 1-rojo);
			PixelSetBlue(pixels[x], 1-azul);
		}
		(void) PixelSyncIterator(iterator); 
		
	}
	//******************************************************************
	
	iterator = DestroyPixelIterator(iterator); //Liberamos memoria del iterador de pixeles
} 
