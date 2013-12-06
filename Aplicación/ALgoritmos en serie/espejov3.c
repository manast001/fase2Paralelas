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

void invertirColores(MagickWand *imagen)
{
	PixelIterator *iterator;
	PixelWand **pixels;
	int red,blue,green,ancho;
	int ancho2 = MagickGetImageWidth(imagen);
	iterator = NewPixelIterator(imagen);
	MagickPixelPacket pixel;

	
	register ssize_t x;
	size_t width;
	ssize_t y;
	int fin = (int)( ( ancho2/2 ));

	//******************ITERACION PIXEL POR PIXEL***********************
	for (y=0; y < (ssize_t) MagickGetImageHeight(imagen); y++)
	{
		pixels = PixelGetNextIteratorRow(iterator,&width);
		if (pixels == (PixelWand **) NULL) break;
		
		for (x=0; x <= fin; x++)
		{

			ancho=(((ssize_t) width)-x-1);

			PixelGetMagickColor(pixels[x],&pixel);
			
			red = pixel.red;
			green = pixel.green;
			blue = pixel.blue;

			PixelGetMagickColor(pixels[ancho],&pixel);
			PixelSetMagickColor(pixels[x],&pixel);


			pixel.red = red;
			pixel.green = green;
			pixel.blue = blue;

			PixelSetMagickColor(pixels[ancho],&pixel);


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
	invertirColores(imagen);

	//guardamos la imagen en disco
	MagickWriteImages(imagen,argv[2],MagickTrue);
	
	//liberamos memoria destruyendo la imagen
	imagen = DestroyMagickWand(imagen);
	
	//terminamos ejecucion de la libreria
	MagickWandTerminus();
	
	return 0;
}
