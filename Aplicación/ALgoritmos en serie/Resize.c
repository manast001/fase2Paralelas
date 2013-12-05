 /* 
 * COMPILACION: $ gcc `Wand-config --cflags --cppflags` Resize.c -o resize `Wand-config --ldflags --libs`
 * USO: $ ./resize imagen.bmp salida.bmp "cantidad"
 * 
 *  [cantidad] = ]-oo,oo+[
 *
 *  Ej: 2 -> 200%  | -2 -> 50%  
 *	3 -> 300%  | -3 -> 33%
 *	4 -> 400%  | -4 -> 25%
 *
 *  Author: Sebastián Enrique Menéndez Sáez
 *  Grupo 2, Computación Paralela 2013
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>

void Resize(MagickWand *imagen, MagickWand *imagen2, int proporcion)
{
	int ancho, largo, factor_conversion;
	register ssize_t x;
	size_t width, width2;
	ssize_t y;
	int ity, itx, index = 0;
	MagickPixelPacket pixel, pixel2;
	PixelIterator *iterator, *iterator2;
	PixelWand **pixels, **pixels2;

	PixelWand *bgw1 = NewPixelWand();
	factor_conversion = abs(proporcion);
	if(proporcion >= 0){
		ancho = MagickGetImageWidth(imagen)*factor_conversion;
		largo = MagickGetImageHeight(imagen)*factor_conversion;
	}else{
		ancho = (MagickGetImageWidth(imagen)/factor_conversion)+1;
		largo = (MagickGetImageHeight(imagen)/factor_conversion)+1;
	}
	MagickNewImage(imagen2,ancho,largo,bgw1);

	iterator = NewPixelIterator(imagen);
	iterator2 = NewPixelIterator(imagen2);
	
	
	
	if(proporcion >= 0){
		//****************** ITERACION PIXEL POR PIXEL ***********************
		for (y=0; y < (ssize_t) MagickGetImageHeight(imagen); y++)
		{
			pixels = PixelGetNextIteratorRow(iterator,&width);
			if (pixels == (PixelWand **) NULL) break;
			for(ity=0; ity < factor_conversion; ity++){
				pixels2 = PixelGetNextIteratorRow(iterator2,&width2);
				for (x=0; x < (ssize_t) width; x++)
				{
					PixelGetMagickColor(pixels[x],&pixel);
					for(itx=0;itx<factor_conversion;itx++){
						PixelGetMagickColor(pixels2[factor_conversion*x+itx],&pixel2);
						pixel2.red = pixel.red;
						pixel2.green = pixel.green;
						pixel2.blue = pixel.blue;
						PixelSetMagickColor(pixels2[factor_conversion*x+itx],&pixel2);
					}
				}
				(void) PixelSyncIterator(iterator2);
			}
			(void) PixelSyncIterator(iterator);
		
		}
		//******************************************************************
	}else{
		//****************** ITERACION PIXEL POR PIXEL ***********************
		for (y=0; y < (ssize_t) MagickGetImageHeight(imagen); y++)
		{
			pixels = PixelGetNextIteratorRow(iterator,&width);
			if (pixels == (PixelWand **) NULL) break;
			pixels2 = PixelGetNextIteratorRow(iterator2,&width2);
			for (x=0; x < (ssize_t) width; x++)
			{
				if(x%factor_conversion == 0){
					PixelGetMagickColor(pixels[x],&pixel);
					PixelGetMagickColor(pixels2[index],&pixel2);
					pixel2.red = pixel.red;
					pixel2.green = pixel.green;
					pixel2.blue = pixel.blue;
					PixelSetMagickColor(pixels2[index],&pixel2);
					index++;
				}
			}
			index = 0;
			for(ity=1;ity<factor_conversion;ity++){
				pixels = PixelGetNextIteratorRow(iterator,&width);
			}
			(void) PixelSyncIterator(iterator2);
			(void) PixelSyncIterator(iterator);
		
		}
		//******************************************************************
	}
	iterator = DestroyPixelIterator(iterator);
	iterator = DestroyPixelIterator(iterator2);
}

int main(int argc,char **argv)
{
	
	//creamos la imagen
	MagickWand *imagen, *imagen2;
	int proporcion = atoi(argv[3]);
	
	//inicializamos la imagen
	imagen = NewMagickWand();
	imagen2 = NewMagickWand();
	
	//cargamos la imagen
	MagickReadImage(imagen,argv[1]);
	
	//imagen2 = CloneMagickWand(imagen);

	//llamamos a la funcion de redimension
	Resize(imagen,imagen2,proporcion);
	
	//guardamos la imagen en disco
	MagickWriteImages(imagen2,argv[2],MagickTrue);
	
	//liberamos memoria destruyendo la imagen
	imagen = DestroyMagickWand(imagen2);
	
	//terminamos ejecucion de la libreria
	MagickWandTerminus();
	
	return 0;
}
