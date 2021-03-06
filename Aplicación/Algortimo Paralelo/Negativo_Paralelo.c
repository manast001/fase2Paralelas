/*	
	Adrian Gutierrez
	Programa Efecto Negativo en Paralelo
*/


//**********************************************
// Compilación
// $ mpicc `Wand-config --cflags --cppflags` Negativo_Paralelo.c -o Negativo_Paralelo `Wand-config --ldflags --libs`
// Ejecutar
// $ mpirun -np X ./Negativo_Paralelo imagen.bmp imagen_salida.bmp
// X es el numero de procesadores.
//**********************************************



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>
#include <string.h>
#include <mpi.h>

#define CLAMP(x)  (((x) > (255)) ? (255) : (((x) < (0)) ? (0) : (x)))

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
    PixelWand **pixels;
    
    iterator = NewPixelIterator(imagen);    
    MagickPixelPacket pixel; // pixel de la imágen
    MagickPixelPacket pixelEsclavo; // pixel de la imágen

    register ssize_t i;
    size_t width;
    ssize_t j;

    int mayor = 0;
    int height_aux = MagickGetImageHeight(imagen);
    int width_aux = MagickGetImageWidth(imagen);
    if (width_aux > height_aux) mayor = width_aux;
    if (width_aux < height_aux) mayor = height_aux;

    Pixel_rgbofi linea_pixeles[mayor]; // fila de pixeles de la imágen

    int ancho, alto; // propiedades imágen
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


    /* Medición del tiempo inicial */
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    /* Procesar pixeles */
    inicio = (alto/numero_procesos)*identificador_nodo; // defino la fila de inicio que procesará el nodo
    fin = (alto/numero_procesos)*(identificador_nodo+1); // defino el fin del ciclo que recorre las filas de la matriz

    for(i = inicio; i < fin; i++) {//FILA

        PixelSetIteratorRow(iterator,(ssize_t)i-1);
        pixels = PixelGetNextIteratorRow(iterator,&width);
        if (pixels == (PixelWand **) NULL) break;

        for(j = 0; j < ancho; j++) {//COLUMNA
                PixelGetMagickColor(pixels[j],&pixel);
                
                //*****************EFECTO NEGATIVO***************************
                
                pixel.red   = 255 - pixel.red;
                pixel.green = 255 - pixel.green; 
                pixel.blue  = 255 - pixel.blue;   

                //*********************************************************
            
            if(identificador_nodo != 0){

                linea_pixeles[j].red = pixel.red;
                linea_pixeles[j].blue = pixel.blue;
                linea_pixeles[j].green = pixel.green;
                linea_pixeles[j].opacity = pixel.opacity;
                linea_pixeles[j].index = pixel.index;
                linea_pixeles[j].fuzz = pixel.fuzz;

           }else{
                PixelSetMagickColor(pixels[j],&pixel);
            }
        }
        (void) PixelSyncIterator(iterator);

        if(identificador_nodo != 0) {
            MPI_Send(&linea_pixeles, ancho, PIXEL_TYPE, 0, i, MPI_COMM_WORLD);
        }
    }

    /* Recibir filas procesadas, luego unirlas y generar imagen de salida */
    if(identificador_nodo == 0) {
        for(k = 1; k < numero_procesos; k++) {
            inicio = (alto/numero_procesos)*k; // defino la fila de inicio
            fin = (alto/numero_procesos)*(k+1); // defino el fin del ciclo
            
            for(i = inicio; i < fin; i++) {
                MPI_Recv(&linea_pixeles, ancho, PIXEL_TYPE, k, i, MPI_COMM_WORLD, &status);
                PixelSetIteratorRow(iterator,(ssize_t)i-1);
                pixels = PixelGetNextIteratorRow(iterator,&width);
                for(j = 0; j < ancho; j++){
                    pixelEsclavo.red = linea_pixeles[j].red;
                    pixelEsclavo.blue = linea_pixeles[j].blue;
                    pixelEsclavo.green = linea_pixeles[j].green;
                    pixelEsclavo.opacity = linea_pixeles[j].opacity ;
                    pixelEsclavo.index = linea_pixeles[j].index; 
                    pixelEsclavo.fuzz = linea_pixeles[j].fuzz;                    
                    PixelSetMagickColor(pixels[j],&pixelEsclavo); 
                }
                (void) PixelSyncIterator(iterator);
            }
        }
    }
    iterator = DestroyPixelIterator(iterator);

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
