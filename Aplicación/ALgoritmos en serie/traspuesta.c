#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>


void imagen_traspuesta(MagickWand *imagen,MagickWand *traspuesta)
{
	
	register ssize_t x;
    size_t width;
    ssize_t y;
	
	PixelIterator *iterator,*iterator2;
	PixelWand **pixels,**pixels2;
	
	MagickPixelPacket pixel;

	MagickNewImage(traspuesta,MagickGetImageHeight(imagen),MagickGetImageWidth(imagen),NewPixelWand());
		
	iterator = NewPixelIterator(imagen);
	
	for(y=0;y<(ssize_t) MagickGetImageHeight(imagen); y++)
	{
		printf("%f\n",y);
		pixels = PixelGetNextIteratorRow(iterator,&width);
		
		iterator2 = NewPixelIterator(traspuesta);
		
		if (pixels == (PixelWand **) NULL) 
			break;
		
		for (x=0; x < (ssize_t) width; x++)
		{
			pixels2 = PixelGetNextIteratorRow(iterator2,&width);
			
			PixelGetMagickColor(pixels[x],&pixel);
			PixelGetMagickColor(pixels2[y],&pixel);
			(void) PixelSyncIterator(iterator2);
		}
			
		iterator2 = DestroyPixelIterator(iterator2);
	}
	
	MagickWriteImages(traspuesta,"hola.bmp",MagickTrue);
	
	iterator = DestroyPixelIterator(iterator);
    
}

int main(int argc,char **argv)
{
	if(argc!=2)
	{
		
	//creamos las imagenes
		MagickWand *imagen=NewMagickWand();
		MagickWand *traspuesta=NewMagickWand();
	//cargamos la imagen
		MagickReadImage(imagen,argv[1]);
	
	//
		imagen_traspuesta(imagen,traspuesta);
	
	
	//liberamos memoria destruyendo la imagen
		imagen = DestroyMagickWand(imagen);
		imagen = DestroyMagickWand(traspuesta);
	
	//terminamos ejecucion de la libreria
		MagickWandTerminus();
	}
	return 0;
}
