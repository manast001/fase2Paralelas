//**********************************************
// Compilación
// $ mpicc `Wand-config --cflags --cppflags` beta_invertirparalelo.c -o invertirP `Wand-config --ldflags --libs`
// Ejecutar
// $ mpirun -np X ./invertirP imagen.bmp imagen_salida.bmp
// X es el numero de procesadores.
// Probado para X = 1 pendiente comprobacion en cluster

//**********************************************



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>
#include <mpi.h>

  typedef struct
  {
    unsigned int red;
    unsigned int green;
    unsigned int blue;
    unsigned int opacity;
    unsigned int index;
    double fuzz;

  } Pixel_rgbofi;


int main(int argc, char **argv) {
    //creamos la imagen
    MagickWand *imagen;
    
    //inicializamos la imagen
    imagen = NewMagickWand();

    //cargamos la imagen
    MagickReadImage(imagen,argv[1]);

    PixelIterator *iterator;
    PixelIterator *iterator2;

    PixelWand **pixels;
    PixelWand **pixels2;
    
    iterator = NewPixelIterator(imagen);    
    iterator2 = NewPixelIterator(imagen);

    MagickPixelPacket pixel; // pixel de la imágen
    MagickPixelPacket pixel2; // pixel de la imágen
    MagickPixelPacket pixelEsclavo; // pixel de la imágen

    register ssize_t i;
    size_t width;
    ssize_t j;


    int ancho, alto, ancho_aux,red,blue,green; // propiedades imágen
    int k, inicio, fin, identificador_nodo, numero_procesos;
    MPI_Status status;
    double start,end, aux;

    /* Definición de datos para enviar la estructura rgb_pixel_t por MPI */
    int blockcounts[6] = {1, 1, 1, 1, 1, 1,};
    MPI_Aint offsets[6];
    MPI_Datatype oldtypes[6] = {MPI_UNSIGNED, MPI_UNSIGNED, MPI_UNSIGNED,MPI_UNSIGNED,MPI_UNSIGNED,MPI_DOUBLE};
    MPI_Datatype PIXEL_TYPE;

    /* Obtención de datos de la imágen */
    ancho = MagickGetImageWidth(imagen);
    alto = MagickGetImageHeight(imagen);
    Pixel_rgbofi linea_pixeles[alto]; // fila de pixeles de la imágen
    /* Inicialización de MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numero_procesos); // obtengo el total de procesos o nodos
    MPI_Comm_rank(MPI_COMM_WORLD, &identificador_nodo); // obtengo identificador del nodo

    /* offsets de la estructura pixel */
    offsets[0] = offsetof(Pixel_rgbofi, red);
    offsets[1] = offsetof(Pixel_rgbofi, green);
    offsets[2] = offsetof(Pixel_rgbofi, blue);
    offsets[3] = offsetof(Pixel_rgbofi, opacity);
    offsets[4] = offsetof(Pixel_rgbofi, index);
    offsets[5] = offsetof(Pixel_rgbofi, fuzz);

    /* Estructura del pixel para enviar/recibir vía MPI */
    MPI_Type_create_struct(6, blockcounts, offsets, oldtypes, &PIXEL_TYPE);
    MPI_Type_commit(&PIXEL_TYPE);

    /*if(identificador_nodo == 0) {
        MagickNewImage(imagen_salida,MagickGetImageWidth(imagen),MagickGetImageHeight(imagen),NewPixelWand());// estructura de salida
    }*/

    /* Medición del tiempo inicial */
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    /* Procesar pixeles */
    inicio = (ancho/numero_procesos)*identificador_nodo; // defino la fila de inicio que procesará el nodo
    fin = (ancho/numero_procesos)*(identificador_nodo+1); // defino el fin del ciclo que recorre las filas de la matriz

    for(j=inicio;j<fin;j++){

        for(i = 0; i < alto/2; i++) {

            pixels = PixelGetNextIteratorRow(iterator,&width);
            if (pixels == (PixelWand **) NULL) break;
            PixelGetMagickColor(pixels[j],&pixel);    

            PixelSetIteratorRow(iterator2,(ssize_t)alto-i-2);
            pixels2 = PixelGetNextIteratorRow(iterator2,&width);
            if (pixels2 == (PixelWand **) NULL) break;
            PixelGetMagickColor(pixels2[j],&pixel2);      


            if(identificador_nodo != 0){

                linea_pixeles[i].red = pixel2.red;
                linea_pixeles[i].blue = pixel2.blue;
                linea_pixeles[i].green = pixel2.green;
                linea_pixeles[i].opacity = pixel2.opacity;
                linea_pixeles[i].index = pixel2.index;
                linea_pixeles[i].fuzz = pixel2.fuzz;

                linea_pixeles[alto-i-1].red = pixel.red;
                linea_pixeles[alto-i-1].blue = pixel.blue;
                linea_pixeles[alto-i-1].green = pixel.green;
                linea_pixeles[alto-i-1].opacity = pixel.opacity;
                linea_pixeles[alto-i-1].index = pixel.index;
                linea_pixeles[alto-i-1].fuzz = pixel.fuzz;

            }else{
                PixelSetMagickColor(pixels[j],&pixel2);              
                PixelSetMagickColor(pixels2[j],&pixel);
            }
            (void) PixelSyncIterator(iterator);
            (void) PixelSyncIterator(iterator2);

        }
        PixelResetIterator(iterator);
        PixelResetIterator(iterator2);

        if(identificador_nodo != 0) {
            MPI_Send(&linea_pixeles, alto, PIXEL_TYPE, 0, j, MPI_COMM_WORLD);
        }
    }

     //Recibir filas procesadas, luego unirlas y generar imagen de salida 
    if(identificador_nodo == 0) {
        for(k = 1; k < numero_procesos; k++) {
            inicio = (ancho/numero_procesos)*k; // defino la fila de inicio
            fin = (ancho/numero_procesos)*(k+1); // defino el fin del ciclo            
            for(j = inicio; j < fin; j++) {
                MPI_Recv(&linea_pixeles, alto, PIXEL_TYPE, k, j, MPI_COMM_WORLD, &status);
                for(i = 0; i < alto; i++){
                    
                    pixels = PixelGetNextIteratorRow(iterator,&width);
                    if (pixels == (PixelWand **) NULL) break;

                    pixelEsclavo.red = linea_pixeles[i].red;
                    pixelEsclavo.blue = linea_pixeles[i].blue;
                    pixelEsclavo.green = linea_pixeles[i].green;
                    pixelEsclavo.opacity = linea_pixeles[i].opacity ;
                    pixelEsclavo.index = linea_pixeles[i].index; 
                    pixelEsclavo.fuzz = linea_pixeles[i].fuzz; 

                    PixelSetMagickColor(pixels[j],&pixelEsclavo); 
                    (void) PixelSyncIterator(iterator);
                }
                PixelResetIterator(iterator);
            }
        }
    }
    
    iterator = DestroyPixelIterator(iterator);
    iterator = DestroyPixelIterator(iterator2);

    /* Medición del tiempo final */
    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();

    MPI_Type_free(&PIXEL_TYPE);
    MPI_Finalize();

    /* Guardar archivo de imágen y liberar memoria */
    if(identificador_nodo == 0) {
        printf("Tiempo %f s\n", end - start);

        //guardamos la imagen en disco
        MagickWriteImages(imagen,argv[2],MagickTrue);
        
        //liberamos memoria destruyendo la imagen
        imagen = DestroyMagickWand(imagen);
        
        //terminamos ejecucion de la libreria
        MagickWandTerminus();
    }
    return 0;
}
