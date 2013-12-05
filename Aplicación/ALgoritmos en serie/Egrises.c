#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>
#define BYTE  unsigned char	// 8-bits
#define CLAMP(x)  (((x) > (255)) ? (255) : (((x) < (0)) ? (0) : (x)))
#define RGB2GRAY(r,g,b) (BYTE)( (b)*0.3 + (g)*0.59 + (r)*0.11 )


void Egris (MagickWand *imagen)
{
	int gris;
	PixelIterator *iterator;
	PixelWand **pixels;
	
	iterator = NewPixelIterator(imagen); 
	
	double verde, rojo, azul; 

	register ssize_t x; 
	size_t width; //Anchura de la imagen
	ssize_t y; //registrara la altura de la imagen

	
	//******************ITERACION PIXEL POR PIXEL***********************
	for (y=0; y < (ssize_t) MagickGetImageHeight(imagen); y++)
	{
		
		pixels = PixelGetNextIteratorRow(iterator,&width); 
		if (pixels == (PixelWand **) NULL) break; 
		
		for (x=0; x < (ssize_t) width; x++)
		{
			verde 	= PixelGetGreen(pixels[x]); 
			rojo 	= PixelGetRed(pixels[x]);
			azul	= PixelGetBlue(pixels[x]);
			
			PixelSetGreen(pixels[x], azul*0.3 + verde*0.59 + rojo*0.11);  //transformamos los pixeles a escalas de grises
			PixelSetRed(pixels[x], azul*0.3 + verde*0.59 + rojo*0.11);
			PixelSetBlue(pixels[x], azul*0.3 + verde*0.59 + rojo*0.11);
		}
		(void) PixelSyncIterator(iterator); 
		
	}
	//******************************************************************
	
	iterator = DestroyPixelIterator(iterator); //Liberamos memoria del iterador de pixeles
}

int main(int argc,char **argv)
{
	
	MagickWand *imagen; //creacion de la imagen
	
	
	imagen = NewMagickWand(); //la imagen se inicia
	
	//cargamos la imagen
	MagickReadImage(imagen, argv[1]);
	
	//llamamos a la funcion Negativa
	Egris(imagen);

	//guardamos la imagen en disco
	MagickWriteImages(imagen, argv[2], MagickTrue);
	
	//liberamos memoria destruyendo la imagen
	imagen = DestroyMagickWand(imagen);
	
	//terminamos ejecucion de la libreria
	MagickWandTerminus();
	
	return 0;
}

