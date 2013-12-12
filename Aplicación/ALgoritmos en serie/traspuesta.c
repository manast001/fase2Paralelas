#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>
#include <time.h>

//traspuesta 90°
void imagen_traspuesta90(MagickWand *imagen,MagickWand *traspuesta)
{
        
        register ssize_t x;
        size_t width,width2;
        ssize_t y,alto,ancho;
        
        alto=MagickGetImageHeight(imagen);
        ancho=MagickGetImageWidth(imagen);
        
        PixelIterator *iterator,*iterator2;
        PixelWand **pixels,**pixels2;
        
        MagickPixelPacket pixel;

        MagickNewImage(traspuesta,alto,ancho,NewPixelWand());
                
        iterator = NewPixelIterator(imagen);
        
        for(y=0;y<alto; y++)
        {

                pixels = PixelGetNextIteratorRow(iterator,&width);
                
                iterator2 = NewPixelIterator(traspuesta);
                
                if (pixels == (PixelWand **) NULL)
                        break;
                
                for (x=0; x < (ssize_t) width; x++)
                {
                        pixels2 = PixelGetNextIteratorRow(iterator2,&width2);
                        
                        PixelGetMagickColor(pixels[x],&pixel);
                        PixelSetMagickColor(pixels2[alto-y-1],&pixel);
                        (void) PixelSyncIterator(iterator2);
                }
                        
                iterator2 = DestroyPixelIterator(iterator2);
                (void) PixelSyncIterator(iterator);
        }
        
        
        iterator = DestroyPixelIterator(iterator);
    
}

//traspuesta 180°
void imagen_traspuesta180(MagickWand *imagen,MagickWand *traspuesta)
{
        
        register ssize_t x;
        size_t width,width2;
        ssize_t y,alto,ancho;
        
        alto=MagickGetImageHeight(imagen);
        ancho=MagickGetImageWidth(imagen);
        
        PixelIterator *iterator,*iterator2;
        PixelWand **pixels,**pixels2;
        
        MagickPixelPacket pixel;
        
        MagickNewImage(traspuesta,ancho,alto,NewPixelWand());
                
        iterator = NewPixelIterator(imagen);
        iterator2 = NewPixelIterator(traspuesta);
        
        for(y=0;y<alto; y++)
        {

                pixels = PixelGetNextIteratorRow(iterator,&width);
                
                
                PixelSetIteratorRow(iterator2,(ssize_t)alto-y-1);        
                (void) PixelSyncIterator(iterator2);        

                pixels2 = PixelGetNextIteratorRow(iterator2,&width);
                
                if (pixels == (PixelWand **) NULL)
                        break;
                
                for (x=0; x < (ssize_t) width; x++)
                {
                        PixelGetMagickColor(pixels[x],&pixel);
                        PixelSetMagickColor(pixels2[width-x-1],&pixel);
                        
                }
                        
                
                (void) PixelSyncIterator(iterator);
        }
        
        iterator2 = DestroyPixelIterator(iterator2);
        iterator = DestroyPixelIterator(iterator);
    
}

//traspuesta 270°
void imagen_traspuesta270(MagickWand *imagen,MagickWand *traspuesta)
{
        register ssize_t x;
        size_t width,width2;
        ssize_t y,alto,ancho;
        
        alto=MagickGetImageHeight(imagen);
        ancho=MagickGetImageWidth(imagen);
        
        PixelIterator *iterator,*iterator2;
        PixelWand **pixels,**pixels2;
        
        MagickPixelPacket pixel;

        MagickNewImage(traspuesta,alto,ancho,NewPixelWand());
                
        iterator = NewPixelIterator(imagen);
        
        for(y=0;y<alto; y++)
        {

                pixels = PixelGetNextIteratorRow(iterator,&width);
                
                iterator2 = NewPixelIterator(traspuesta);
                
                if (pixels == (PixelWand **) NULL)
                        break;
                
                for (x=0; x < (ssize_t) width; x++)
                {
                        pixels2 = PixelGetNextIteratorRow(iterator2,&width2);
                        
                        PixelGetMagickColor(pixels[width-x-1],&pixel);
                        PixelSetMagickColor(pixels2[alto-y-1],&pixel);
                        (void) PixelSyncIterator(iterator2);
                }
                        
                iterator2 = DestroyPixelIterator(iterator2);
                (void) PixelSyncIterator(iterator);
        }
        
        
        iterator = DestroyPixelIterator(iterator);
    
}

int main(int argc,char **argv)
{
        if(argc!=3)
        {
        //creamos las imagenes
                MagickWand *imagen=NewMagickWand();
                MagickWand *traspuesta=NewMagickWand();
        //cargamos la imagen
				
                MagickReadImage(imagen,argv[1]);
				
				clock_t t_ini, t_fin;
				double secs;

				t_ini = clock();
				
				switch(atoi(argv[3]))
				{
					case 1:
						imagen_traspuesta90(imagen,traspuesta);
						break;
					case 2:
						imagen_traspuesta180(imagen,traspuesta);
						break;
					case 3:
						imagen_traspuesta270(imagen,traspuesta);
						break;		
				}
				
				t_fin = clock();
				
				secs = (double)(t_fin - t_ini) / CLOCKS_PER_SEC;
				
				printf("Tiempo Algoritmo : %f segundos\n",secs);
        
                MagickWriteImages(traspuesta,argv[2],MagickTrue);
        
        //liberamos memoria destruyendo la imagen
                imagen = DestroyMagickWand(imagen);
                traspuesta = DestroyMagickWand(traspuesta);
                
                 
        
        //terminamos ejecucion de la libreria
                MagickWandTerminus();
        }
        return 0;
}
