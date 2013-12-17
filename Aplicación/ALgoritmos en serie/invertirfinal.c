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
#include <time.h>


void imagen_invertida(MagickWand *imagen,MagickWand *invertida){

	//Creacion de los iteradores para imagren principal y imagen auxiliar
	PixelIterator *iterator;
	PixelIterator *iterator2;

	// Creacion de imagen auxiliar
 	MagickNewImage(invertida,MagickGetImageWidth(imagen),MagickGetImageHeight(imagen),NewPixelWand());

 	//PIxel principal (pixels) y pixels2 (auxiliar)
	PixelWand **pixels;
	PixelWand **pixels2;

	int alto = MagickGetImageHeight(imagen);

	iterator = NewPixelIterator(imagen);
	iterator2 = NewPixelIterator(invertida);

	MagickPixelPacket pixel;	
	MagickPixelPacket pixel2 ;
	
	
	register ssize_t x;
	size_t width;
	ssize_t y;


	//******************ITERACION PIXEL POR PIXEL***********************
	for (y=0; y < MagickGetImageHeight(imagen);	 y++)
	{	
		//Obtengo fila de la imagen principal
		pixels = PixelGetNextIteratorRow(iterator,&width);

		// Se posiciona en la ultima fila de imagen auxiliar
		PixelSetIteratorRow(iterator2,(ssize_t)alto-y-1);	
		(void) PixelSyncIterator(iterator2);	

		//Se obtiene la ultima fila de imagen auxiliar
		pixels2 = PixelGetNextIteratorRow(iterator2,&width);

		if (pixels == (PixelWand **) NULL) break;
		
		//saco la primera fila
		for (x=0; x < (ssize_t) width; x++)
		{
			//Se pasan los datos de la primera fila de imagen original a la 
			// ultima fila de la imagen secundaria
			PixelGetMagickColor(pixels[x],&pixel);

			PixelSetMagickColor(pixels2[x],&pixel);	

		}
		(void) PixelSyncIterator(iterator);
		(void) PixelSyncIterator(iterator2);
		
	//******************************************************************
	}
	iterator = DestroyPixelIterator(iterator);
	iterator = DestroyPixelIterator(iterator2);
}


			

int main(int argc,char **argv)
{
        if(argc!=2)
        {
          double start, end;//--codigo nuevo
          start = clock(); // tiempo de inicio--codigo nuevo
        //creamos las imagenes
                MagickWand *imagen=NewMagickWand();
                MagickWand *invertida=NewMagickWand();
        //cargamos la imagen
                MagickReadImage(imagen,argv[1]);
        
        //
                imagen_invertida(imagen,invertida);
        
                MagickWriteImages(invertida,argv[2],MagickTrue);
        
        //liberamos memoria destruyendo la imagen
                imagen = DestroyMagickWand(imagen);
                imagen = DestroyMagickWand(invertida);
                
        
        //terminamos ejecucion de la libreria
                MagickWandTerminus();
         end = clock(); // tiempo de tÃ©rmino--codigo nuevo
         printf("Tiempo %f s\n", (end - start) / CLOCKS_PER_SEC);//--codigo nuevo
        }
        return 0;
}
