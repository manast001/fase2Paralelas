#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpi.h"

#define MASTER_TO_SLAVE_TAG 1 //tag for messages sent from master to slaves
#define SLAVE_TO_MASTER_TAG 10 //tag for messages sent from slaves to master

#define filterWidth 5
#define filterHeight 5

#pragma pack(1)
typedef struct
{
        unsigned char R;
        unsigned char G;
        unsigned char B;
}pixel;

struct fileHeader {
       char tipo[2]; //tipo de archivo BitMap
       int size; //tamaño del archivo BMP (en bytes)
       short int reserved1; //espacio reservado para la aplicacion que crea la imagen
       short int reserved2; //espacio reservado para la aplicacion que crea la imagen
       int offset; //desplazamiento a la matriz de pixeles, desde el inicio del archivo       
};

struct imageHeader {
       int size; //tamaño en bytes de esta cabecera
       signed int width; //ancho de la imagen, en pixeles
       signed int height; //alto de la imagen en pixeles
       short int colorPlanes; //numero de planos de color, debe setearse a 1
       short int bpp; //bits por pixel, o profundidad de color
       int compression; //metodo de compresion para la imagen
       int imageSize; //tamaño de la imagen (de la matriz de pixeles, incluyendo el padding de cada row)
       signed int resolutionY; //resolucion horizontal de la imagen, en pixeles/metro
       signed int resolutionX; //resolucion vertical de la imagen, en pixeles/metro
       int colorPalette; //numero de colores de la paleta de colores, 0 para dejarlo por defecto hasta 2^n
       int importantColors; //numero de colores importantes, 0 si todos son importantes
};

struct image {
       struct fileHeader fh;
       struct imageHeader ih;
       pixel *array;
};

void setPixel(pixel *m, int row, int col,  unsigned char r, unsigned char g, unsigned char b, int width)
{
     int i=0;
     i = width * row + col;
     
     m[i].R = r;
     m[i].G = g;
     m[i].B = b;
}

pixel getPixel(pixel *m, int x, int y, int width)
{
     int i=0;
     i = width * x + y;
     return m[i];
}

int nextMultiple(int x)
{
    while(x%4 != 0) x++;
    return x;
}

void initializeImage(struct image *im, signed int width, signed int height, short int bpp)
{
     //setting up the FILEHEADER/////
     //BM image type
     memcpy((*im).fh.tipo, "\x42\x4d", 2);
     
     //calculating the bmp file size
     int rowSize = nextMultiple(width * (bpp/8));
     (*im).fh.size = 14 + 40 + rowSize * height;
     
     //adding the reserved space
     short int r=0;
     (*im).fh.reserved1 = r;
     (*im).fh.reserved2 = r;
     
     //adding the pixel array offset
     (*im).fh.offset = 54;
     ////////////////////////////////
     
     //size of the BITMAPINFOHEADER which we are using
     (*im).ih.size = 40;
     
     //width of the image, in pixels
     (*im).ih.width = width;
     
     //height of the image, in pixels
     (*im).ih.height = height;
     
     //number of color planes
     (*im).ih.colorPlanes = 1;
     
     //bits per pixel or color depth
     (*im).ih.bpp = bpp;
     
     //compression of the image
     (*im).ih.compression = 0;
     
     //////////////////////////////////////////////////////////////
     //size of the raw bitmap data, can be 0 if no compression used
     int tmpwidth = ((*im).ih.bpp/8) * ((*im).ih.width);
     
     //round up to a multiple of 4 for the row
     while(tmpwidth%4 != 0) {tmpwidth++;}
     (*im).ih.imageSize = tmpwidth*height;
     //////////////////////////////////////////////////////////////
     
     //horizontal resolution of the image, in pixels/meter
     (*im).ih.resolutionY = 2835;
     
     //vertical resolution of the image, in pixels/meter
     (*im).ih.resolutionX = 2835;
     
     //color palette, 0 for default
     (*im).ih.colorPalette = 0;
     
     //important colors, 0 mean all important
     (*im).ih.importantColors = 0;
     
     //allocating memory for the pixel array
     (*im).array = (pixel *)calloc(width * height, sizeof(pixel));
     
     
}

void saveImage(struct image *im, char *filename)
{
     //openin file to write on it
     FILE * file = fopen(filename, "wb");
     //writing the file header
     fwrite(&(*im).fh, sizeof(struct fileHeader), 1, file);
     //writing the image header
     fwrite(&(*im).ih, sizeof(struct imageHeader), 1, file);
     
     pixel tmp;
     
     int i=0, j=0, aux=0;
     for(i=0; i < (*im).ih.height; i++)
     {
      for(j=0; j < (*im).ih.width; j++)
      {
       //write a 3 bytes (24bits) pixel       
       tmp = getPixel((*im).array, i, j, (*im).ih.width);
       fwrite(&tmp.R, 1, 1, file);
       fwrite(&tmp.G, 1, 1, file);
       fwrite(&tmp.B, 1, 1, file);
      }
      
      //write the padding for each row up to the following multiple of 4
      aux = (j)*((*im).ih.bpp)/8;
      while(aux%4 != 0) {fwrite("\x0ff", 1, 1, file); aux++;}
     }  
     
     fclose(file);
}

void clear(struct image *im, int r, int g, int b)
{
     int i=0, j=0;
     for(i=0; i < (*im).ih.height; i++)
     {
      for(j=0; j < (*im).ih.width; j++)
      {
       //write a 3 bytes (24bits) pixel       
       setPixel((*im).array, i, j, r, g, b, (*im).ih.width);
      }
     }
}

void loadImage(struct image *im, char *filename)
{
     //struct image im_tmp;
     FILE * file = fopen(filename, "rb");
     fread(&(*im).fh, sizeof(struct fileHeader), 1, file);
     fread(&(*im).ih, sizeof(struct imageHeader), 1, file);
     
     //allocate memory for the pixel array
     (*im).array = (pixel *)calloc((*im).ih.width * (*im).ih.height, sizeof(pixel));
     
     //loading the pixel array in the image
     int i=0, j=0;
     //pixel *p;
     
     //int total_pixels = (*im).ih.width * (*im).ih.height;
     //int aux = total_pixels / (*im).ih.height;

     int auxi, ps;
     char *temp;
      
     for (i=0; i<(*im).ih.height; i++)
     {
         for (j=0; j<(*im).ih.width; j++)
         {
             ps = (*im).ih.width * i + j;
             fread(&(*im).array[ps], sizeof(pixel), 1, file);
         }
         
         //write the padding for each row up to the following multiple of 4
         auxi = (j)*((*im).ih.bpp)/8;
         while (auxi%4 != 0)
         {
               //move the pointer 1 byte per each padding pixel
               fread(&temp, 1, 1, file);
               auxi++;
         }
     }
     
     fclose(file);
}

void invertColor(struct image *im)
{
     int i,j;
     pixel tmp;
     
     for (i=0; i<(*im).ih.height; i++)
     {
         for (j=0; j<(*im).ih.width; j++)
         {
             tmp = getPixel((*im).array, i, j, (*im).ih.width);
             setPixel((*im).array, i, j, 255-tmp.R, 255-tmp.G, 255-tmp.B, (*im).ih.width);
         }
     }
}

void brightImage(struct image *im, int c)
{
     int i,j;
     pixel tmp;
     
     for (i=0; i<(*im).ih.height; i++)
     {
         for (j=0; j<(*im).ih.width; j++)
         {
             tmp = getPixel((*im).array, i, j, (*im).ih.width);
             
             if(tmp.R + c > 255) tmp.R = 255;
				else
					if(tmp.R + c < 0) tmp.R = 0;
						else tmp.R = tmp.R + c;

             if(tmp.G + c > 255) tmp.G = 255;
				else
					if(tmp.G + c < 0) tmp.G = 0;
						else tmp.G = tmp.G + c;

             if(tmp.B + c > 255) tmp.B = 255;
				else
					if(tmp.B + c < 0) tmp.B = 0;
						else tmp.B = tmp.B + c;

             setPixel((*im).array, i, j, tmp.R, tmp.G, tmp.B, (*im).ih.width);
         }
     }
}

void grayScale(struct image *im)
{
     int i,j,lum;
     pixel tmp;
     
     for (i=0; i<(*im).ih.height; i++)
     {
         for (j=0; j<(*im).ih.width; j++)
         {
             tmp = getPixel((*im).array, i, j, (*im).ih.width);
             lum = (tmp.R*0.30) + (tmp.G*0.59) + (tmp.B*0.11);

             setPixel((*im).array, i, j, lum, lum, lum, (*im).ih.width);
         }
     }
}

void grayScale_Parallel(struct image *im, int size, int rank)
{
     int i,j,lum,aux,r;
     
     int total_pixels = (*im).ih.width * (*im).ih.height;
     int qty = total_pixels/(size-1);
     int rest = total_pixels % (size-1);
     MPI_Status status;
     
     //printf("\n%d\n", rank);
     
     if(rank == 0)
     {
		 for(i=1; i<size; i++){
		 j = i*qty - qty;
		 aux = j;

		 if(rest != 0 && i==size-1) {qty=qty+rest;} //para distrubuir toda la carga
		 //printf("\nj: %d  qty: %d  rest: %d\n", j, qty, rest);

		 MPI_Send(&aux, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD);
		 MPI_Send(&qty, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG+2, MPI_COMM_WORLD);
         MPI_Send(&(*im).array[j], qty*3, MPI_BYTE, i, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
			
         //printf("\nSending to node=%d, sender node=%d\n", i, rank);
 		}
		 
	 }
	 else
	 {
		MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD,&status);
		MPI_Recv(&qty, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+2, MPI_COMM_WORLD,&status);
		
		pixel *arreglo = (pixel *)calloc(qty, sizeof(pixel));
		MPI_Recv(&arreglo[0], qty*3, MPI_BYTE, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD,&status);
        //printf("Receiving node=%d, message=%d\n", rank, aux);
		
		for(i=0;i<qty;i++)
		{
			lum = (arreglo[i].R*0.30) + (arreglo[i].G*0.59) + (arreglo[i].B*0.11);
			arreglo[i].R = lum;
			arreglo[i].G = lum;
			arreglo[i].B = lum;
		}
		
		MPI_Send(&aux, 1, MPI_INT, 0, SLAVE_TO_MASTER_TAG+1, MPI_COMM_WORLD);
		MPI_Send(&qty, 1, MPI_INT, 0, SLAVE_TO_MASTER_TAG+2, MPI_COMM_WORLD);
		MPI_Send(&arreglo[0], qty*3, MPI_BYTE, 0, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD);
		
		free(arreglo);
	 }


	if (rank==0){
		//printf("\nrank: %d\n", rank);
		for (i=1; i<size; i++) // untill all slaves have handed back the processed data
		{
			MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG+1, MPI_COMM_WORLD,&status);
			MPI_Recv(&qty, 1, MPI_INT, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG+2, MPI_COMM_WORLD,&status);
			MPI_Recv(&(*im).array[aux], qty*3, MPI_BYTE, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD,&status);
		}
	}
}

void invertColor_Parallel(struct image *im, int size, int rank)
{
     int i,j,lum,aux,r;
     
     int total_pixels = (*im).ih.width * (*im).ih.height;
     int qty = total_pixels/(size-1);
     int rest = total_pixels % (size-1);
     MPI_Status status;
     
     //printf("\n%d\n", rank);
     
     if(rank == 0)
     {
		 for(i=1; i<size; i++){
		 j = i*qty - qty;
		 aux = j;

		 if(rest != 0 && i==size-1) {qty=qty+rest;} //para distrubuir toda la carga
		 //printf("\nj: %d  qty: %d  rest: %d\n", j, qty, rest);

		 MPI_Send(&aux, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD);
		 MPI_Send(&qty, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG+2, MPI_COMM_WORLD);
         MPI_Send(&(*im).array[j], qty*3, MPI_BYTE, i, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
			
         //printf("\nSending to node=%d, sender node=%d\n", i, rank);
 		}
		 
	 }
	 else
	 {
		MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD,&status);
		MPI_Recv(&qty, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+2, MPI_COMM_WORLD,&status);
		
		pixel *arreglo = (pixel *)calloc(qty, sizeof(pixel));
		MPI_Recv(&arreglo[0], qty*3, MPI_BYTE, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD,&status);
        //printf("Receiving node=%d, message=%d\n", rank, aux);
		
		for(i=0;i<qty;i++)
		{
			arreglo[i].R = 255-arreglo[i].R;
			arreglo[i].G = 255-arreglo[i].G;
			arreglo[i].B = 255-arreglo[i].B;
		}
		
		MPI_Send(&aux, 1, MPI_INT, 0, SLAVE_TO_MASTER_TAG+1, MPI_COMM_WORLD);
		MPI_Send(&qty, 1, MPI_INT, 0, SLAVE_TO_MASTER_TAG+2, MPI_COMM_WORLD);
		MPI_Send(&arreglo[0], qty*3, MPI_BYTE, 0, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD);
		
		free(arreglo);
	 }


	if (rank==0){
		//printf("\nrank: %d\n", rank);
		for (i=1; i<size; i++) // untill all slaves have handed back the processed data
		{
			MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG+1, MPI_COMM_WORLD,&status);
			MPI_Recv(&qty, 1, MPI_INT, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG+2, MPI_COMM_WORLD,&status);
			MPI_Recv(&(*im).array[aux], qty*3, MPI_BYTE, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD,&status);
		}
	}
}

void brightImage_Parallel(struct image *im, int c, int size, int rank)
{
     int i,j,aux,r;
     
     int total_pixels = (*im).ih.width * (*im).ih.height;
     int qty = total_pixels/(size-1);
     int rest = total_pixels % (size-1);
     MPI_Status status;
     
     //printf("\n%d\n", rank);
     
     if(rank == 0)
     {
		 for(i=1; i<size; i++){
		 j = i*qty - qty;
		 aux = j;

		 if(rest != 0 && i==size-1) {qty=qty+rest;} //para distrubuir toda la carga
		 //printf("\nj: %d  qty: %d  rest: %d\n", j, qty, rest);

		 MPI_Send(&aux, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD);
		 MPI_Send(&qty, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG+2, MPI_COMM_WORLD);
         MPI_Send(&(*im).array[j], qty*3, MPI_BYTE, i, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
			
         //printf("\nSending to node=%d, sender node=%d\n", i, rank);
 		}
		 
	 }
	 else
	 {
		MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD,&status);
		MPI_Recv(&qty, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+2, MPI_COMM_WORLD,&status);
		
		pixel *arreglo = (pixel *)calloc(qty, sizeof(pixel));
		MPI_Recv(&arreglo[0], qty*3, MPI_BYTE, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD,&status);
        //printf("Receiving node=%d, message=%d\n", rank, aux);
		
		for(i=0;i<qty;i++)
		{
			
             if(arreglo[i].R + c > 255) arreglo[i].R = 255;
				else
					if(arreglo[i].R + c < 0) arreglo[i].R = 0;
						else arreglo[i].R = arreglo[i].R + c;

             if(arreglo[i].G + c > 255) arreglo[i].G = 255;
				else
					if(arreglo[i].G + c < 0) arreglo[i].G = 0;
						else arreglo[i].G = arreglo[i].G + c;

             if(arreglo[i].B + c > 255) arreglo[i].B = 255;
				else
					if(arreglo[i].B + c < 0) arreglo[i].B = 0;
						else arreglo[i].B = arreglo[i].B + c;
						
		}
		
		MPI_Send(&aux, 1, MPI_INT, 0, SLAVE_TO_MASTER_TAG+1, MPI_COMM_WORLD);
		MPI_Send(&qty, 1, MPI_INT, 0, SLAVE_TO_MASTER_TAG+2, MPI_COMM_WORLD);
		MPI_Send(&arreglo[0], qty*3, MPI_BYTE, 0, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD);
		
		free(arreglo);
	 }


	if (rank==0){
		//printf("\nrank: %d\n", rank);
		for (i=1; i<size; i++) // untill all slaves have handed back the processed data
		{
			MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG+1, MPI_COMM_WORLD,&status);
			MPI_Recv(&qty, 1, MPI_INT, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG+2, MPI_COMM_WORLD,&status);
			MPI_Recv(&(*im).array[aux], qty*3, MPI_BYTE, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD,&status);
		}
	}
}

void conv_Parallel(struct image *im, int size, int rank)
{
     int i,j,aux,r;
     pixel tmp;
     
     
     int total_pixels = (*im).ih.width * (*im).ih.height;
     int qty = total_pixels/(size-1);
     int rest = total_pixels % (size-1);
	 
     
     int alto;
     int ancho;
     int st;
     int st2;
     
     MPI_Status status;
     
     ////////////////////////////////////////////
	double filter[filterWidth][filterHeight] =  {
     {0,  0,  0,  0,  0},
     {0,  0,  0,  0,  0},
    {-1, -1,  2,  0,  0},
     {0,  0,  0,  0,  0},
     {0,  0,  0,  0,  0},
		};
	
	double factor = 1.0;
	double bias = 0.0;
	
	int filterX = 0;
	int filterY = 0;
	
	int imageX;
	int imageY;
	
	double red;
	double green;
	double blue;
	
	unsigned char new_red;
	unsigned char new_green;
	unsigned char new_blue;
	
	int new_red_int, new_green_int, new_blue_int;
	
	int x;
	int y;
     ////////////////////////////////////////////
     
     
     if(rank == 0)
     {    
		 for(i=1; i<size; i++){
		 j = i*qty - qty;
		 aux = j;

		 if(rest != 0 && i==size-1) {qty=qty+rest;} //para distrubuir toda la carga
		 //printf("\nj: %d  qty: %d  rest: %d\n", j, qty, rest);

		 MPI_Send(&aux, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD);
		 MPI_Send(&qty, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG+2, MPI_COMM_WORLD);
         MPI_Send(&(*im).ih.height, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG + 3, MPI_COMM_WORLD);
         MPI_Send(&(*im).ih.width, 1, MPI_INT, i, MASTER_TO_SLAVE_TAG + 4, MPI_COMM_WORLD);
         MPI_Send(&(*im).array[j], qty*3, MPI_BYTE, i, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD);
			
         //printf("\nSending to node=%d, sender node=%d\n", i, rank);
 		}
		 
	 }
	 else
	 {
		MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+1, MPI_COMM_WORLD,&status);
		MPI_Recv(&qty, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+2, MPI_COMM_WORLD,&status);
        MPI_Recv(&alto, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+3, MPI_COMM_WORLD,&status);
        MPI_Recv(&ancho, 1, MPI_INT, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG+4, MPI_COMM_WORLD,&status);
		
		pixel *arreglo = (pixel *)calloc(qty, sizeof(pixel));
		MPI_Recv(&arreglo[0], qty*3, MPI_BYTE, MPI_ANY_SOURCE, MASTER_TO_SLAVE_TAG, MPI_COMM_WORLD,&status);
        //printf("Receiving node=%d, message=%d\n", rank, aux);
        
        pixel *arreglo_salida = (pixel *)calloc(qty, sizeof(pixel));
		

		//ancho = qty/2;
		alto = (int)qty/ancho;

		//apply the filter 
		for(x = 0; x < alto; x++) 
		for(y = 0; y < ancho; y++) 
		{ 
			red = 0.0, green = 0.0, blue = 0.0; 
         
			//multiply every value of the filter with corresponding image pixel 
			for(filterX = 0; filterX < filterHeight; filterX++) 
			for(filterY = 0; filterY < filterWidth; filterY++) 
			{ 
				imageX = (x - filterHeight / 2 + filterX + alto) % alto; 
				imageY = (y - filterWidth / 2 + filterY + ancho) % ancho; 
            
				st = ancho * imageX + imageY;
				red += arreglo[st].R * filter[filterX][filterY]; 
				green += arreglo[st].G * filter[filterX][filterY]; 
				blue += arreglo[st].B * filter[filterX][filterY]; 
			} 
         
			//truncate values smaller than zero and larger than 255 
			new_red_int = (int)(factor * red + bias);
			new_green_int = (int)(factor * green + bias);
			new_blue_int = (int)(factor * blue + bias);
        
		if(new_red_int < 0) new_red = new_red_int * -1;
        if(new_red_int > 255) new_red = 255;

        if(new_green_int < 0) new_green = new_green_int * -1;
        if(new_green_int > 255) new_green = 255;

        if(new_blue_int < 0) new_blue =  new_blue_int * -1;
        if(new_blue_int > 255) new_blue = 255;
        
        //setPixel(imagen_salida.array, x, y, new_red, new_green, new_blue, imagen_salida.ih.width);
        st2 = ancho * x + y;
        arreglo_salida[st2].R = new_red;
		arreglo_salida[st2].G = new_green;
		arreglo_salida[st2].B = new_blue;
    }
		
		MPI_Send(&aux, 1, MPI_INT, 0, SLAVE_TO_MASTER_TAG+1, MPI_COMM_WORLD);
		MPI_Send(&qty, 1, MPI_INT, 0, SLAVE_TO_MASTER_TAG+2, MPI_COMM_WORLD);
		MPI_Send(&arreglo_salida[0], qty*3, MPI_BYTE, 0, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD);
		
		free(arreglo_salida);
	 }


	if (rank==0){
		//printf("\nrank: %d\n", rank);
		for (i=1; i<size; i++) // untill all slaves have handed back the processed data
		{
			MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG+1, MPI_COMM_WORLD,&status);
			MPI_Recv(&qty, 1, MPI_INT, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG+2, MPI_COMM_WORLD,&status);
			MPI_Recv(&(*im).array[aux], qty*3, MPI_BYTE, MPI_ANY_SOURCE, SLAVE_TO_MASTER_TAG, MPI_COMM_WORLD,&status);
		}
	}
}

void runningTime(clock_t begin, clock_t end)
{
	double time_spent;
	
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\n\nRunning Time: %f\n\n", time_spent);
}

int main(int argc, char *argv[])
{
	//////////time counter
	clock_t begin;
	
    /*
    struct image image1;
    initializeImage(&image1, 10000, 10000, 24);
    
    
    clear(&image1, 128, 255, 0);
    //saveImage(&image1, "prueba.bmp");
    */
    
    int rank, size;
    
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status status;
    
    int op=1;
    char filename_toload[50];
    int bright_number=0;
    struct image image2;
    
	if (rank==0)
	{
    printf("Escriba el nombre de la imagen BMP que desea cargar: \n");
    scanf("%s", filename_toload);
    //sprintf(filename_toload, "paisaje.bmp");
    loadImage(&image2, filename_toload);
	}
    
    while(op != 0)
    {
		if (rank==0)
		{
		printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		printf("Bienvenido a Procesamiento Digital de Imagenes\n\n");
		printf("\t1.- Invertir colores\n");
		printf("\t2.- Aplicar escala de grises\n");
		printf("\t3.- Ajustar brillo\n");
		printf("\t4.- Convolucion: Deteccion de Bordes\n");
		printf("\t5.- Guardar imagen\n");
		printf("\t0.- Salir\n\t");
		
		scanf("%d", &op);
		}
		
		//Broadcast the user's choice to all other ranks
		MPI_Bcast(&op, 1, MPI_INT, 0, MPI_COMM_WORLD);
		
		switch(op)
		{
			case 1:
					if (rank==0) {begin = clock();}
					MPI_Barrier(MPI_COMM_WORLD);
					invertColor_Parallel(&image2, size, rank);
					MPI_Barrier(MPI_COMM_WORLD);
					if (rank==0) {runningTime(begin, clock()); printf("Se invirtieron los colores de la imagen\n\n");}
					break;
			case 2:
					if (rank==0) {begin = clock();}
					MPI_Barrier(MPI_COMM_WORLD);
					grayScale_Parallel(&image2, size, rank);
					MPI_Barrier(MPI_COMM_WORLD);
					if (rank==0) {runningTime(begin, clock()); printf("Se transformo a blanco y negro la imagen\n\n");}
					break;
			case 3:
					if (rank==0)
					{
						printf("\n\tIngrese brillo (-127 a 127): ");
						fflush(stdout);
						scanf("%d", &bright_number);
						begin = clock();
					}
					
					MPI_Bcast(&bright_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
					MPI_Barrier(MPI_COMM_WORLD);
					brightImage_Parallel(&image2, bright_number, size, rank);
					if (rank==0) {runningTime(begin, clock()); printf("Se ajusto el brillo de la imagen\n\n");}
					break;
			case 4:
					if (rank==0) {begin = clock();}
					MPI_Barrier(MPI_COMM_WORLD);
					conv_Parallel(&image2, size, rank);
					MPI_Barrier(MPI_COMM_WORLD);
					if (rank==0) {runningTime(begin, clock()); printf("Se aplico convolucion para detectar bordes\n\n");}
					break;
			case 5:
					if (rank==0) {begin = clock();}
					if (rank==0) {saveImage(&image2, "imagen_procesada.bmp");}
					MPI_Barrier(MPI_COMM_WORLD);
					if (rank==0) {runningTime(begin, clock()); printf("Imagen guardada correctamente\n\n");}
					break;
		}
	}
	
	if (rank==0) free(image2.array);
	MPI_Finalize();
	
    return 0;
}
