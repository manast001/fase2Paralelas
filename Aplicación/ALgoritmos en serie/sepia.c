/*
 *
 * El efecto sepia es dejar la imagen en un tono rojizo, la idea es pasar la imagen
 * a escala de grises y luego modificar los niveles de exposicion de los pixeles verde
 * y azul.
 * 
 * COMPILACION: $ gcc `Wand-config --cflags --cppflags` sepia.c -o sepia `Wand-config --ldflags --libs`
 * USO: $ ./sepia imagen.bmp salida.bmp
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>

 #define CLAMP(x)  (((x) > (255)) ? (255) : (((x) < (0)) ? (0) : (x)))

void Sepia(MagickWand *imagen)
{
	PixelIterator *iterator;
	PixelWand **pixels;
	
	iterator = NewPixelIterator(imagen);
	MagickPixelPacket pixel;
	
	register ssize_t x;
	size_t width;
	ssize_t y;


	//******************ITERACION PIXEL POR PIXEL***********************
	for (y=0; y < (ssize_t) MagickGetImageHeight(imagen); y++)
	{
		pixels = PixelGetNextIteratorRow(iterator,&width);
		if (pixels == (PixelWand **) NULL) break;
		
		for (x=0; x < (ssize_t) width; x++)
		{
			PixelGetMagickColor(pixels[x],&pixel);
			
			//**************PASO A ESCALA DE GRISES*************************
			pixel.red = CLAMP(pixel.red *0.393 + pixel.green*0.769 + pixel.blue*0.189);
			pixel.green = CLAMP(pixel.red *0.349 + pixel.green*0.686 + pixel.blue*0.168);
			pixel.blue = CLAMP(pixel.red *0.272 + pixel.green*0.534 + pixel.blue*0.131);

			//*****************EFECTO SEPIA***************************
			pixel.red = pixel.red*1;
			pixel.green = pixel.green*0.65; //menor el valor más rojiza la imagen
			pixel.blue = pixel.blue*0.52;	//menor el valor más rojiza la imagen

			PixelSetMagickColor(pixels[x],&pixel);
		}
		(void) PixelSyncIterator(iterator);
	}
	//******************************************************************
	
	iterator = DestroyPixelIterator(iterator);
}


int main(int argc,char **argv)
{
	//creamos la imagen
	MagickWand *imagen;
	
	//inicializamos la imagen
	imagen = NewMagickWand();
	
	//cargamos la imagen
	MagickReadImage(imagen,argv[1]);
	
	//llamamos a la funcion invertir colores
	Sepia(imagen);

	//guardamos la imagen en disco
	MagickWriteImages(imagen,argv[2],MagickTrue);
	
	//liberamos memoria destruyendo la imagen
	imagen = DestroyMagickWand(imagen);
	
	//terminamos ejecucion de la libreria
	MagickWandTerminus();
	
	return 0;
}
