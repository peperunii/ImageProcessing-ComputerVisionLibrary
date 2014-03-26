
#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <math.h>
#include "CV_Library.h"
#include <string.h>
#include <stdlib.h>

static void put_scanline(unsigned char buffer[], int line, int width,
                         int height, unsigned char *rgbpix);
static int debug = 0;                /* = 1; prints every pixel */


#ifndef MAX
#define MAX(a,b) (a>b?a:b)
#endif

#ifndef MIN
#define MIN(a,b) (a<b?a:b)
#endif

/*
	O P E N   I M A G E
*/
struct Image ReadImage(char *FileName)
{
	FILE *f_ptr = NULL;
	struct Image Img_src;
	int isJpeg = 0;
	int Counter = 0;
	Img_src.isLoaded = 0;
	/* Open the file only with extension JPEG or JPG*/
#pragma warning (disable : 4996)
	f_ptr = fopen(FileName, "rb");
	if (f_ptr == NULL)
	{
		printf("Error opening file. Program will now close\n");
		_getch();
		return Img_src;
	}
	/* Check filename extension */
	for (Counter = 0; Counter < 255; Counter++)
	{
		if (FileName[Counter] == '.' && Counter < 253)
		{
			if (FileName[Counter + 1] == 'J' || FileName[Counter + 1] == 'j')
			{
				if (FileName[Counter + 2] == 'P' || FileName[Counter + 2] == 'p')
				{
					isJpeg = 1;
				}
			}
		}
	}
	if (isJpeg == 0)
	{
		printf("Only Jpeg file extensions are allowed\n");
		_getch();
		return Img_src;
	}

	Img_src = read_Image_file(f_ptr);
	Img_src.Image_FileName = FileName;
	Img_src.isLoaded = 1;
	return Img_src;
}


/*
	W R I T E   I M A G E
*/
GLOBAL(void) WriteImage(char * filename, struct Image Img_src, int quality)//uint16 *image_buffer, int image_width, int image_height)
{
  struct jpeg_compress_struct cinfo;

  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);
  //filename = "C:\\Users\\Petar\\Downloads\\pic.jpg";
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
	return;
  }
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = Img_src.Width; 	/* image width and height, in pixels */
  cinfo.image_height = Img_src.Height;
  if (Img_src.Num_channels == 3)
  {
	  cinfo.input_components = 3;		/* # of color components per pixel */
	  /*Currently HSL and Lab are not supported so we will save it as RGB.*/
	  if (Img_src.ColorSpace == 2 || Img_src.ColorSpace == 5 || Img_src.ColorSpace == 4)
		  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	  else if (Img_src.ColorSpace == 3)
		  cinfo.in_color_space = JCS_YCbCr;
	  
	  row_stride = Img_src.Width * 3;
  }
  else if (Img_src.Num_channels == 1)
  {
	  cinfo.input_components = 1;		/* # of color components per pixel */
	  cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
	  row_stride = Img_src.Width;
  }
  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  jpeg_start_compress(&cinfo, TRUE);

  	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) 
  {
    row_pointer[0] = & Img_src.rgbpix[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  fclose(outfile);

  jpeg_destroy_compress(&cinfo);

}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  (*cinfo->err->output_message) (cinfo);

  longjmp(myerr->setjmp_buffer, 1);
}

/*
	R E A D   I M A G E
*/
struct Image read_Image_file(FILE * infile)
{
  struct jpeg_decompress_struct cinfo;

  struct Image Img_src;
  struct my_error_mgr jerr;
 
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
	return Img_src;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);


  (void) jpeg_start_decompress(&cinfo);

  row_stride = cinfo.output_width * cinfo.output_components;

  /* Fill the structure */
  Img_src.Width = cinfo.image_width;
  Img_src.Height = cinfo.image_height;
  Img_src.Num_channels = cinfo.num_components;
  Img_src.ColorSpace = cinfo.jpeg_color_space;
  
  
  /* TO  DELETE */
  Img_src.ColorSpace = 2;
  /***************/
  
  
  Img_src.rgbpix = (unsigned char *)calloc(Img_src.Num_channels * Img_src.Width*Img_src.Height, sizeof(unsigned char));
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


  while (cinfo.output_scanline < cinfo.output_height) 
  {

    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

    if(debug)printf("cinfo.output_scanline=%d\n", cinfo.output_scanline);
    if(debug)printf("cinfo.output_height=%d\n", cinfo.output_height);
    if(debug)printf("row_stride=%d\n", row_stride);
    put_scanline(buffer[0], cinfo.output_scanline, cinfo.output_width,
                 cinfo.output_height, Img_src.rgbpix);
  }

  (void) jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

  fclose(infile);

  return Img_src;
}

static void put_scanline(unsigned char buffer[], int line, int width,
                         int height, unsigned char *rgbpix)
{
  int i, k;
  
    //k = (height-line)*3*width;
	k = (line-1) * 3 * width;
    for(i=0; i<3*width; i+=3)
    {
      rgbpix[k+i]   = buffer[i];
      rgbpix[k+i+1] = buffer[i+1];
      rgbpix[k+i+2] = buffer[i+2];
    }
} /* end put_scanline */

/*
	C R E A T E -  New Image - Based On Prototype
*/
struct Image CreateNewImage_BasedOnPrototype(struct Image *Prototype, struct Image *Img_dst)
{
	Img_dst->ColorSpace = Prototype->ColorSpace;
	Img_dst->Height = Prototype->Height;
	Img_dst->Width = Prototype->Width;
	Img_dst->Num_channels = Prototype->Num_channels;
	Img_dst->ColorSpace = Prototype->ColorSpace;

	Img_dst->rgbpix = (unsigned char *)calloc(Img_dst->Height * Img_dst->Width * Img_dst->Num_channels, sizeof(unsigned char));
	if (Img_dst->rgbpix == NULL) 
	{
		printf("cannot allocate memory for the new image\n"); 
		Img_dst->isLoaded = 0;
	}
	else 
		Img_dst->isLoaded = 1;

	memcpy(Img_dst->rgbpix, Prototype->rgbpix, Prototype->Num_channels * Prototype->Width * Prototype->Height * sizeof(unsigned char));

	return *Img_dst;
}

/*
	C R E A T E -  New Image
*/
struct Image CreateNewImage(struct Image *Img_dst, int Width, int Height, int NumChannels, int ColorSpace)
{
	FILE *fdebug = NULL;
	Img_dst->ColorSpace = ColorSpace;
	Img_dst->Height = Height;
	Img_dst->Width = Width;
	Img_dst->Num_channels = NumChannels;

	/* If we have Binary or Grayscale image, NumChannels should be == 1*/
	if (ColorSpace < 2 && NumChannels != 1)
	{
		Img_dst->isLoaded = 0;
		return *Img_dst;
	}
	Img_dst->ColorSpace = ColorSpace;

	Img_dst->rgbpix = (unsigned char *)calloc(Img_dst->Height * Img_dst->Width * Img_dst->Num_channels, sizeof(unsigned char));
	if (Img_dst->rgbpix == NULL)
	{

		#ifdef DEBUG_FILE
			fdebug = fopen(DEBUG_FILE, "wt");
			fprintf(fdebug,"cannot allocate memory for the new image\n");
			fclose(fdebug);
		#endif // DEBUG_FILE
		
		Img_dst->isLoaded = 0;
	}
	else
		Img_dst->isLoaded = 1;

	return *Img_dst;
}

/* 
	S E T   D E S T I N A T I O N  - to match the size of the prototype
*/
struct Image SetDestination(struct Image *Prototype, struct Image *Img_dst)
{
	FILE *fdebug = NULL;
	
	Img_dst->ColorSpace = Prototype->ColorSpace;
	Img_dst->Height = Prototype->Height;
	Img_dst->Width = Prototype->Width;

	Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));
	if (Img_dst->rgbpix == NULL)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "cannot allocate memory for the new image\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		Img_dst->isLoaded = 0;
	}
	else
		Img_dst->isLoaded = 1;
}

/*
	D E S T R O Y  image
*/
void DestroyImage(struct Image *Img)
{
	free(Img->rgbpix);
}

/*
	B L U R  image function - around point
*/
struct Image BlurImageAroundPoint(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression)
{
	double MaxRatio = 0;
	double Distance = 0;
	double DistanceRatio = 0;
	double *matrix = (double *)calloc(Img_src->Width * Img_src->Height, sizeof(double));
	double Chislo = 0, Chislo2 = 0;
	int Sybiraemo = 0;
	int i, j, z, t, l;

	/* Only odd nubers are allowed (bigger than 5*/
	if (BlurPixelRadius % 2 == 0) BlurPixelRadius += 1;
	if (BlurPixelRadius < 5) BlurPixelRadius = 5;

	MaxRatio = (double)MAX(CentralPoint.X - ((double)SizeOfBlur * Img_dst->Width / 100), Img_dst->Width - CentralPoint.X + ((double)SizeOfBlur * Img_dst->Width / 100)) / ((double)SizeOfBlur * Img_dst->Width / 100);
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			//luma = ui->imageData[row * width + col];
			Distance = sqrt(pow(abs((double)CentralPoint.X - j), 2) + pow(abs((double)CentralPoint.Y - i), 2));
			if (Distance < ((double)SizeOfBlur * Img_dst->Width / 100))
			{
				matrix[i * Img_dst->Width + j] = 1;
			}
			else
			{
				DistanceRatio = Distance / ((double)SizeOfBlur * Img_dst->Width / 100);
				matrix[i * Img_dst->Width + j] = 1 - ((double)BlurAgression / 100 * (DistanceRatio / MaxRatio));
				if (matrix[i * Img_dst->Width + j] < 0) matrix[i * Img_dst->Width + j] = 0;
			}
		}
	}

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{

			if (i < BlurPixelRadius / 2 || j < BlurPixelRadius / 2 || j >= Img_dst->Width - BlurPixelRadius / 2 || i >= Img_dst->Height - BlurPixelRadius / 2)
			{
				for (l = 0; l < Img_dst->Num_channels; l++)
				{
					Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
				}
				continue;
			}
			for (l = 0; l < 3; l++)
			{
				if (Img_src->rgbpix[3 * (i*Img_dst->Width + j) + l] > 255)
					Chislo = 0;

				Sybiraemo = 0;
				if (BlurOrSharp == 0)
					Chislo2 = ((double)(matrix[i * Img_dst->Width + j]) / (pow((double)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
				else
					Chislo2 = ((double)(1 - matrix[i * Img_dst->Width + j]) / (pow((double)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
				for (z = 0; z < BlurPixelRadius / 2; z++)
				{
					for (t = 0; t < BlurPixelRadius / 2; t++)
					{
						if (z == 0 && t == 0) continue;
						Sybiraemo += Img_src->rgbpix[3*((i - z)*Img_dst->Width + j - t) + l];
						Sybiraemo += Img_src->rgbpix[3*((i - z)*Img_dst->Width + j + t) + l];
						Sybiraemo += Img_src->rgbpix[3*((i + z)*Img_dst->Width + j - t) + l];
						Sybiraemo += Img_src->rgbpix[3*((i + z)*Img_dst->Width + j + t) + l];
					}
				}

				Chislo2 *= Sybiraemo;
				Chislo = 0;
				if (BlurOrSharp == 0)
					Chislo = (1 - matrix[i * Img_dst->Width + j])*Img_src->rgbpix[3*(i*Img_dst->Width + j) + l] + (int)Chislo2;
				else
					Chislo = (matrix[i * Img_dst->Width + j])*Img_src->rgbpix[3*(i*Img_dst->Width + j) + l] + (int)Chislo2;
				if (Chislo > 255)
					Chislo = 255;
				if (Chislo < 0)
					Chislo = 0;
				Img_dst->rgbpix[3 * (i*Img_dst->Width + j) + l] = Chislo;
			}
		}
	}

	return *Img_dst;
}

/*
	B L U R  image function - Gaussian
*/
struct Image BlurImageGussian(struct Image *Img_src, struct Image *Img_dst, int BlurPixelRadius, double NeighborCoefficient)
{
	int i, j, l, z, t;
	int Sybiraemo = 0;
	double Chislo = 0;
	double Chislo2 = 0;
	if (BlurPixelRadius < 5) BlurPixelRadius = 5;
	if (NeighborCoefficient > 100) NeighborCoefficient /= 100;
	if (NeighborCoefficient < 0) NeighborCoefficient *= -1;

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			if (i < BlurPixelRadius / 2 || j < BlurPixelRadius / 2 || j >= Img_dst->Width - BlurPixelRadius / 2 || i >= Img_dst->Height - BlurPixelRadius / 2)
			{
				for (l = 0; l < Img_dst->Num_channels; l++)
				{
					Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
				}
				continue;
			}
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				Sybiraemo = 0;
				Chislo2 = ((double)(NeighborCoefficient) / (pow((double)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
				
				for (z = 0; z < BlurPixelRadius / 2; z++)
				{
					for (t = 0; t < BlurPixelRadius / 2; t++)
					{
						if (z == 0 && t == 0) continue;
						Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i - z)*Img_dst->Width + j - t) + l];
						Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i - z)*Img_dst->Width + j + t) + l];
						Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i + z)*Img_dst->Width + j - t) + l];
						Sybiraemo += Img_src->rgbpix[Img_dst->Num_channels * ((i + z)*Img_dst->Width + j + t) + l];
					}
				}

				Chislo2 *= Sybiraemo;
				Chislo = 0;
				Chislo = (1 - NeighborCoefficient)*Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + (int)Chislo2;
				
				if (Chislo > 255)
					Chislo = 255;
				if (Chislo < 0)
					Chislo = 0;
				Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Chislo;
			}
		}
	}
	return *Img_dst;
}

/* 
	Correct   B R I G H T N E S S  - RGB and Lab only !
*/
struct Image BrightnessCorrection(struct Image *Img_src, struct Image *Img_dst, double Algo_paramBrightnessOrEV, int Algotype)
{
	int i, j, l;

	if (Img_src->Num_channels != Img_dst->Num_channels)
		return *Img_dst;
	if (Img_src->ColorSpace != Img_dst->ColorSpace)
		SetDestination(Img_src, Img_dst);

	if (Img_src->ColorSpace == 2) //RGB
	{
		if (Algotype == 1)
		{
			if (abs(Algo_paramBrightnessOrEV) > 1) Algo_paramBrightnessOrEV /= 100;

			for (i = 0; i < Img_dst->Height; i++)
			{
				for (j = 0; j < Img_dst->Width; j++)
				{
					for (l = 0; l < Img_dst->Num_channels; l++)
					{
						if (Algo_paramBrightnessOrEV * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l]> 255)
							Img_dst->rgbpix[3 * (i*Img_dst->Width + j) + l] = 255;
						else
						{
							if (Algo_paramBrightnessOrEV * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] < 0)
								Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = 0;
							else
								Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = Algo_paramBrightnessOrEV * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] + Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
						}
					}
				}
			}
		}
		else if (Algotype == 2)
		{
			for (i = 0; i < Img_dst->Height; i++)
			{
				for (j = 0; j < Img_dst->Width; j++)
				{
					for (l = 0; l < Img_dst->Num_channels; l++)
					{
						if (pow(2, Algo_paramBrightnessOrEV) * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] > 255)
							Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = 255;
						else if (pow(2, Algo_paramBrightnessOrEV) * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] < 0)
							Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = 0;
						else
							Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = pow(2, Algo_paramBrightnessOrEV) * Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l];
					}
				}
			}
		}
	}
	else if (Img_src->ColorSpace == 4) // Lab
	{
		if (abs(Algo_paramBrightnessOrEV) > 1) Algo_paramBrightnessOrEV /= 100;

		for (i = 0; i < Img_dst->Height; i++)
		{
			for (j = 0; j < Img_dst->Width; j++)
			{
				float L, a, b;
				L = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];
				L = Algo_paramBrightnessOrEV * L  + L;

				Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] = L;
				Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];
				Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];
			}
		}
	}

	return *Img_dst;
}

/* 
	Correct    C O N T R A S T 
*/
struct Image ContrastCorrection(struct Image *Img_src, struct Image *Img_dst, double percentage)
{
	/* The percentage value should be between -100 and 100*/
	double pixel = 0;
	double contrast = 0;
	int i, j, l;

	if (Img_src->Num_channels != Img_dst->Num_channels)
		return *Img_dst;

	if (percentage < -100) percentage = -100;
	if (percentage > 100) percentage = 100;
	contrast = (100.0 + percentage) / 100.0;

	contrast *= contrast;

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				pixel = (double)Img_src->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] / 255;
				pixel -= 0.5;
				pixel *= contrast;
				pixel += 0.5;
				pixel *= 255;
				if (pixel < 0) pixel = 0;
				if (pixel > 255) pixel = 255;
				Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + l] = pixel;
			}
		}
	}

	return *Img_dst;
}

/* 
	Correct     W H I T E  B A L A N C E  - RGB
*/
struct Image WhiteBalanceCorrectionRGB(struct Image *Img_src, struct Image *Img_dst, int Algotype)
{
	int i, j;

	int MaxR = 0, MaxG = 0, MaxB = 0;
	double GtoR_Ratio = 1;
	double GtoB_Ratio = 1;
	long SumG = 0;
	long SumR = 0;
	long SumB = 0;
	double Gto255_Ratio = 1;
	int check3Values = 0;

	if ((Img_src->Num_channels != Img_dst->Num_channels) || (Img_src->Num_channels != 3))
		return *Img_dst;

	/* Only RGB is supported for WhiteBalance algos*/
	if (Img_src->ColorSpace != 2) 
		return *Img_dst;

	if (Algotype == 1)
	{
		//Green world - automatic white detection
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				check3Values = 0;
				if (Img_src->rgbpix[3 * (i*Img_src->Width + j) ] > MaxR) { MaxR = Img_src->rgbpix[3 * (i*Img_src->Width + j)]; check3Values++; }
				if (Img_src->rgbpix[3 * (i*Img_src->Width + j)  + 1] > MaxG) { MaxG = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1]; check3Values++;}
				if (Img_src->rgbpix[3 * (i*Img_src->Width + j)  + 2] > MaxB) { MaxB = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2]; check3Values++; }
				
				if (check3Values == 3)
				{
					//Calculate ratios
					GtoR_Ratio = (double)Img_src->rgbpix[3 * (i*Img_src->Width + j) +1] / Img_src->rgbpix[3 * (i*Img_src->Width + j) ];
					GtoB_Ratio = (double)Img_src->rgbpix[3 * (i*Img_src->Width + j) +1] / Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
				}
			}
		}
		/*Calculate new values based on GtoR amd GtoB ratios*/
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)] <= 255)
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) ] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
				else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
				if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
				else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;
				
				Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
			}
		}
	}
	else if (Algotype == 2)
	{
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				if (Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] > MaxG) 
				{ 
					MaxG = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
					GtoR_Ratio = (double)MaxG / Img_src->rgbpix[3 * (i*Img_src->Width + j)];
					GtoB_Ratio = (double)MaxG / Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
				}
			}
		}
		/*Calculate new values based on GtoR and GtoB ratios*/
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)] <= 255)
					Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
				else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
				if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
				else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;

				Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
			}
		}

	}
	else if (Algotype == 3)
	{
		//Green world - automatic white detection
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				check3Values = 0;
				if (Img_src->rgbpix[3 * (i*Img_src->Width + j)] > MaxR) { MaxR = Img_src->rgbpix[3 * (i*Img_src->Width + j)]; check3Values++; }
				if (Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] > MaxG) { MaxG = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1]; check3Values++; }
				if (Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] > MaxB) { MaxB = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2]; check3Values++; }

				if (check3Values == 3)
				{
					//Calculate ratios
					Gto255_Ratio = 255 / MaxG;
					GtoR_Ratio = (double)(Gto255_Ratio *(MaxG / MaxR));
					GtoB_Ratio = (double)(Gto255_Ratio *(MaxG / MaxB));
				}
			}
		}
		/*Calculate new values based on GtoR amd GtoB ratios*/
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)] <= 255)
					Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
				else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
				if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
				else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;
				if (Gto255_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] <= 255)
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Gto255_Ratio* Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
				else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = 255;
			}
		}
	}
	else if (Algotype == 4)
	{
		//Green world - automatic white detection
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				SumR += Img_src->rgbpix[3 * (i*Img_src->Width + j)    ];
				SumG += Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
				SumB += Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
			}
		}
		GtoR_Ratio =   (double)SumG / SumR;
		GtoB_Ratio =   (double)SumG / SumB;
		Gto255_Ratio = (double)SumG / (255 * Img_src->Height * Img_src->Width);
	/*	if (GtoB_Ratio < 0.8 || GtoR_Ratio < 0.8)
		{
			GtoR_Ratio = GtoR_Ratio * ;
			GtoB_Ratio = (double)SumG / SumB;
		}*/
		/*Calculate new values based on GtoR amd GtoB ratios*/
		for (i = 0; i < Img_src->Height; i++)
		{
			for (j = 0; j < Img_src->Width; j++)
			{
				if (GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) ] <= 255)
					Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = GtoR_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j)];
				else Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
				if (GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2] <= 255)
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = GtoB_Ratio * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
				else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;
				
				Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1];
			}
		}
	}
	else return *Img_src;

	return *Img_dst;
}

/*
	W H I T E  B A L A N C E  - for Lab color space
*/
struct Image WhiteBalanceCorrectionLAB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePointXYZ)
{
	int i, j, z;
	if (Img_src->Num_channels != 3)
		return *Img_dst;
	SetDestination(Img_src, Img_dst);


}

/* 
	Correct    N O I S E 
*/
struct Image NoiseCorrection(struct Image *Img_src, struct Image *Img_dst, double threshold, int Algotype)
{
	int i, j, z;
	int CurrentValue;
	int ProbablityValue = 0;

	if (Img_src->Num_channels != Img_dst->Num_channels)
		return *Img_dst;

	memcpy(&Img_dst, &Img_src,sizeof(Image));

	/* if the current pixel is X % different from the pixels around -> it is noise*/

	if (Algotype == 1)
	{
		for (i = 1; i < Img_dst->Height - 1; i++)
		{
			for (j = 1; j < Img_dst->Width - 1; j++)
			{
				for (z = 0; z < Img_dst->Num_channels; z++)
				{
					ProbablityValue = 0;
					CurrentValue = Img_src->rgbpix[i * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z];

					if (threshold * CurrentValue < Img_src->rgbpix[(i - 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] || CurrentValue > threshold *  Img_src->rgbpix[(i - 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z]) ProbablityValue++;
					if (threshold * CurrentValue < Img_src->rgbpix[(i + 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] || CurrentValue > threshold *  Img_src->rgbpix[(i + 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z]) ProbablityValue++;
					if (threshold * CurrentValue < Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j - 1) + z] || CurrentValue > threshold *  Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j - 1) + z]) ProbablityValue++;
					if (threshold * CurrentValue < Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j + 1) + z] || CurrentValue > threshold *  Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j + 1) + z]) ProbablityValue++;

					if (ProbablityValue >= 3)
					{
						Img_dst->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] = (Img_src->rgbpix[(i - 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] + Img_src->rgbpix[(i + 1) * Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * j + z] + Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j - 1) + z] + Img_src->rgbpix[(i)* Img_dst->Num_channels * Img_src->Width + Img_dst->Num_channels * (j + 1) + z]) / 4;
					}
				}
			}
		}
	}
	else if (Algotype == 2)
	{
		//future implementation
	}

	return *Img_dst;
}

/*
	Correction   G A M M A  - RGB only
*/
struct Image GammaCorrection(struct Image *Img_src, struct Image *Img_dst, double RedGamma, double GreenGamma, double BlueGamma)
{
	int i, j;

	int redGamma[256];
	int greenGamma[256];
	int blueGamma[256];

	if ((Img_src->Num_channels != Img_dst->Num_channels) || (Img_src->Num_channels != 3))
		return *Img_dst;
	if (Img_src->ColorSpace != 2)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "GammaCorrection: The input image is not in RGB color space\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	for (i = 0; i < 256; ++i)
	{
		redGamma[i] = MIN(255, (int)((255.0 * pow((double)i / 255.0, 1.0 / RedGamma)) + 0.5));
		greenGamma[i] = MIN(255, (int)((255.0 * pow((double)i / 255.0, 1.0 / GreenGamma)) + 0.5));
		blueGamma[i] = MIN(255, (int)((255.0 * pow((double)i / 255.0, 1.0 / BlueGamma)) + 0.5));
	}

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			Img_dst->rgbpix[(i)* 3 * Img_src->Width + 3 * j] = redGamma[Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j]];// *Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j];
			Img_dst->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 1] = greenGamma[Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 1]];
			Img_dst->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 2] = blueGamma[Img_src->rgbpix[(i)* 3 * Img_src->Width + 3 * j + 2]];
		}
	}
	return *Img_dst;
}

/*
	convert  I N D E X t o P O S
*/
void getPositionFromIndex(struct Image *Img_src, int pixIdx, int *red, int *col)
{
	*red = pixIdx / (Img_src->Num_channels * Img_src->Width);
	*col = pixIdx - ((*red)*Img_src->Width * Img_src->Num_channels);
}

/*
	convert   P O S t o I N D E X
*/
int getPixelIndex(struct Image *Img_src, int *pixIdx, int red, int col)
{
	int pixelIndex = 0;

	*pixIdx = (red * Img_src->Width + col)*Img_src->Num_channels;
	pixelIndex = *pixIdx;

	return pixelIndex;
}

/* 
	convert to    G R A Y S C A L E  - 3 channels RGB
*/
struct Image ConvertToGrayscale_3Channels(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j;
	
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 0.3 * Img_src->rgbpix[3 * (i*Img_src->Width + j)] + 0.59 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] + 0.11 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
			Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_dst->rgbpix[3 * (i*Img_src->Width + j)];
			Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = Img_dst->rgbpix[3 * (i*Img_src->Width + j)];
		}
	}

	return *Img_dst;
}

/*
	convert to    G R A Y S C A L E  - 1 channel
*/
struct Image ConvertToGrayscale_1Channel(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j;
	
	if (Img_dst->isLoaded != 1) return *Img_dst;
	
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			Img_dst->rgbpix[(i*Img_src->Width + j)] = 0.3 * Img_src->rgbpix[3 * (i*Img_src->Width + j)] + 0.59 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 1] + 0.11 * Img_src->rgbpix[3 * (i*Img_src->Width + j) + 2];
		}
	}
}

/*
A F F I N E  -  Transformation: Rotation
*/
struct Image RotateImage(struct Image *Img_src, struct Image *Img_dst, double RotationAngle, struct point_xy CentralPoint)
{
	float Sx, Matrix[2][2], Cos, Sin;
	int i, j, z ;
	int x_new = 0, y_new = 0;
	int currentPixel_smallImage = 0;

	Sx = (-RotationAngle * 3.14) / 180;
	Cos = cos(Sx);
	Sin = sin(Sx);
	Matrix[0][0] = Cos;
	Matrix[0][1] = -Sin;
	Matrix[1][0] = Sin;
	Matrix[1][1] = Cos;

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			// za vsqko i,j (ot staoroto izobrajenie se namirat novi x_new  i  y_new na koito s epipisvat stoinostite za RGB
			x_new = Matrix[0][0] * (j - CentralPoint.X) + Matrix[0][1] * (i - CentralPoint.Y) + CentralPoint.X;
			y_new = Matrix[1][0] * (j - CentralPoint.X) + Matrix[1][1] * (i - CentralPoint.Y) + CentralPoint.Y;
			if (x_new > Img_dst->Width) x_new = Img_dst->Width;
			if (x_new < 0) x_new = 0;
			if (y_new > Img_dst->Height) y_new = Img_dst->Height;
			if (y_new < 0) y_new = 0;
			
			for(z = 0; z < Img_dst->Num_channels; z++)
			{
				Img_dst->rgbpix[y_new * Img_dst->Num_channels * Img_dst->Width + Img_dst->Num_channels * x_new + z] = Img_src->rgbpix[i * Img_dst->Num_channels * Img_dst->Width + Img_dst->Num_channels * j + z];
			}
		}
	}

	return *Img_dst;
}

/*
	A F F I N E  - scale (zoom) image - in/out
*/
struct Image ScaleImage(struct Image *Img_src, struct Image *Img_dst, double ScalePercentage)
{
	int i, j, z;
	int NewX = 0;
	int NewY = 0;

	/* Modify Img_dst */
	Img_dst->Width = Img_src->Width * (1 + (ScalePercentage / 100.0));
	Img_dst->Height = Img_src->Height * (1 + (ScalePercentage / 100.0));
	Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Height * Img_dst->Width * Img_dst->Num_channels* sizeof(unsigned char));

	if (ScalePercentage < 0)
	{
		for (i = 0; i < Img_dst->Height; i++)
		{
			for (j = 0; j < Img_dst->Width; j++)
			{
				NewX = j / (1 + (ScalePercentage / 100.0)); // Plus Sign because of the negative Scale Value
				NewY = i / (1 + (ScalePercentage / 100.0));

				if (NewX < 0) NewX = 0;
				if (NewY < 0) NewY = 0;
				if (NewX >= Img_src->Width) NewX = Img_src->Width - 1;
				if (NewY >= Img_src->Height) NewY = Img_src->Height - 1;

				for(z = 0; z < Img_dst->Num_channels; z++)
				{
					Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z ] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z ];
				}
			}
		}
	}
	else if (ScalePercentage > 0)
	{
		for (i = 0; i < Img_dst->Height; i++)
		{
			for (j = 0; j < Img_dst->Width; j++)
			{
				NewX = j / (1 + (ScalePercentage / 100));
				NewY = i / (1 + (ScalePercentage / 100));

				if (NewX < 0) NewX = 0;
				if (NewY < 0) NewY = 0;
				if (NewX >= Img_src->Width) NewX = Img_src->Width - 1;
				if (NewY >= Img_src->Height) NewY = Img_src->Height - 1;

				for(z = 0; z < Img_dst->Num_channels; z++)
				{
					Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z ] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z ];
				}
			}
		}
	}
	else return *Img_src;

	return *Img_dst;
}

/*
	A F F I N E  - Translation
*/
struct Image TranslateImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy ToPoint)
{
	int i, j, z;
	int NewX = 0, NewY = 0;
	
	int ShiftX = ToPoint.X - Img_dst->Width / 2;
	int ShiftY = ToPoint.Y - Img_dst->Height / 2;
	

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			NewX = j + ShiftX;
			NewY = i + ShiftY;
			if (ShiftX < 0 ) if(NewX < 0) continue;
			if (ShiftX > 0)if (NewX >= Img_dst->Width) continue;
			if (ShiftY < 0) if(NewY < 0) continue;
			if (ShiftY > 0)if (NewY >= Img_dst->Height) continue;
			
			for(z = 0; z < Img_dst->Num_channels; z++)
			{
				Img_dst->rgbpix[Img_dst->Num_channels * (i*Img_dst->Width + j) + z] = Img_src->rgbpix[Img_dst->Num_channels * ((NewY)*Img_src->Width + (NewX)) + z];
			}
		}
	}
	return *Img_dst;
}

/*
	E D G E   Extraction
*/
struct ArrPoints EdgeExtraction(struct Image *Img_src, struct Image *Img_dst, int Algotype, float Algo_param1, float Algo_param2)
{
	int i, j, z, l;
	int NewX = 0, NewY = 0;
	struct ArrPoints ArrPts;
	struct Image DerrivativeX = CreateNewImage(&DerrivativeX, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image DerrivativeY = CreateNewImage(&DerrivativeY, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image Magnitude = CreateNewImage(&Magnitude, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image Magnitude2 = CreateNewImage(&Magnitude2, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image Magnitude3 = CreateNewImage(&Magnitude3, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image NMS = CreateNewImage(&NMS, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	struct Image Hysteresis = CreateNewImage(&Hysteresis, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);

	float Gx[] = 
	  { -1, 0, 1,
		-2, 0, 2,
		-1, 0, 1 };
	float Gy[] = 
	  { 1, 2, 1,
		0, 0, 0,
	   -1, -2, -1 };

	float HighPass[] = 
	{ 1 / 9 * (
	    -1, -1, -1,
		-1,  8, -1,
		-1, -1, -1) };

	float Laplace[] = 
	 {  0,  1,  0,
		1, -4,  1,
		0,  1,  0 };

	float Prewitt_X_1[] = 
	 { -5, -5, -5,
		0,  0,  0,
		5,  5,  5 };
	float Prewitt_Y_1[] =
	 { -5,  0,  5,
	   -5,  0,  5,
	   -5,  0,  5 };

	float Prewitt_X_2[] = 
	 { 5,  5,  5,
	   0,  0,  0,
	  -5, -5, -5  };
	float Prewitt_Y_2[] = 
	 { 5,  0, -5,
	   5,  0, -5,
	   5,  0, -5 };

	float Sobel_X_1[] =
	{ -1, -2, -1,
	   0,  0,  0,
	   1,  2,  1 };
	float Sobel_Y_1[] =
	{ -1,  0,  1,
	  -2,  0,  2,
	  -1,  0,  1 };

	float Sobel_X_2[] =
	{  1,  2,  1,
	   0,  0,  0,
	  -1, -2, -1 };
	float Sobel_Y_2[] =
	{  1,  0, -1,
	   2,  0, -2,
	   1,  0, -1 };

	Img_dst->Width = Img_src->Width;
	Img_dst->Height = Img_src->Height;
	Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Num_channels * Img_dst->Width * Img_dst->Height * sizeof(unsigned char));
	ArrPts.ArrayOfPoints = (struct point_xy *)calloc(50,sizeof(struct point_xy));

	

	if (Algotype < 1 || Algotype > 3)
	{
		printf("Non existing Algo for Edge extraction\n");
		return ArrPts;
	}
	/* Canny */
	if (Algotype == 1)
	{
		// Step 1: Perfrom Gaussian Blur
		BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
		FindDerrivative_XY(Img_dst, &DerrivativeX, &DerrivativeY);
		FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);
		FindNonMaximumSupp(&Magnitude, &DerrivativeX, &DerrivativeY, &NMS);
		
		FindHysteresis(&Magnitude, &NMS, &Hysteresis, Algo_param1, Algo_param2);
		memcpy(Img_dst->rgbpix, Hysteresis.rgbpix, Hysteresis.Width* Hysteresis.Height * sizeof(unsigned char));
	}
	/* Sobel */
	else if (Algotype == 2)
	{
		BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
		Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Sobel_X_1, 3);
		Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Sobel_Y_1, 3);
		FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);

		Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Sobel_X_2, 3);
		Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Sobel_Y_2, 3);
		if (Algo_param1 == 0)
		{
			FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude2);
			memcpy(Img_dst->rgbpix, Magnitude2.rgbpix, Magnitude2.Width* Magnitude2.Height * sizeof(unsigned char));
		}
		else
		{
			memcpy(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Width* DerrivativeX.Height * sizeof(unsigned char));
		}
	}
	/* Prewitt */
	else if (Algotype == 3)
	{
		BlurImageGussian(Img_src, Img_dst, (0.5 * Img_src->Width) / 100, 0.6);
		Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Prewitt_X_1, 3);
		Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Prewitt_Y_1, 3);
		FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude);

		Convolution(Img_dst->rgbpix, DerrivativeX.rgbpix, DerrivativeX.Height, DerrivativeX.Width, Prewitt_X_2, 3);
		Convolution(Img_dst->rgbpix, DerrivativeY.rgbpix, DerrivativeY.Height, DerrivativeY.Width, Prewitt_Y_2, 3);
		FindMagnitudeOfGradient(&DerrivativeX, &DerrivativeY, &Magnitude2);
		FindMagnitudeOfGradient(&Magnitude, &Magnitude2, &Magnitude3);
		if (Algo_param1 == 0)
		{
			FindNonMaximumSupp(&Magnitude3, &Magnitude, &Magnitude2, &NMS);
			FindHysteresis(&Magnitude3, &NMS, &Hysteresis, Algo_param1, Algo_param2);

			memcpy(Img_dst->rgbpix, Hysteresis.rgbpix, Hysteresis.Width* Hysteresis.Height * sizeof(unsigned char));
		}
		else
			memcpy(Img_dst->rgbpix, Magnitude3.rgbpix, Magnitude3.Width* Magnitude3.Height * sizeof(unsigned char));
	}

	//DestroyImage(&Hysteresis);
	DestroyImage(&DerrivativeX);
	DestroyImage(&DerrivativeY);
	DestroyImage(&Magnitude);
	DestroyImage(&Magnitude2);
	DestroyImage(&Magnitude3);
	DestroyImage(&NMS);
	
	return ArrPts;
}

/* 
	D E R R I V A T I V E    calculation
*/
void FindDerrivative_XY(struct Image *Img_src, struct Image *DerrivativeX_image, struct Image *DerrivativeY_image)
{
	int r, c, pos;
	int rows = Img_src->Height;
	int cols = Img_src->Width;
	
	/* Calculate X - derrivative image */
	for (r = 0; r < rows-1; r++)
	{
		pos = r * cols;
		//DerrivativeX_image->rgbpix[pos] = Img_src->rgbpix[pos + 1] - Img_src->rgbpix[pos];
		//pos++;
		for (c = 0; c < (cols - 1); c++, pos++)
		{
			DerrivativeX_image->rgbpix[pos] = abs((Img_src->rgbpix[pos] - Img_src->rgbpix[pos + cols + 1]));
		}
		DerrivativeX_image->rgbpix[pos] = abs(Img_src->rgbpix[pos] - Img_src->rgbpix[pos - 1]);
	}
	

	/* Calculate Y - derrivative image */
	for (c = 0; c < cols-1; c++)
	{
		pos = c;
		//DerrivativeY_image->rgbpix[pos] = Img_src->rgbpix[pos + cols] - Img_src->rgbpix[pos];
		//pos += cols;
		for (r = 0; r < (rows - 1); r++, pos += cols)
		{
			DerrivativeY_image->rgbpix[pos] = abs(Img_src->rgbpix[pos + 1] - Img_src->rgbpix[pos + cols]);
		}
		DerrivativeY_image->rgbpix[pos] = abs(Img_src->rgbpix[pos] - Img_src->rgbpix[pos - cols]);
	}

	//int kn = 3;
	//const int khalf = kn / 2;
	//float min = 35000, max = -3000;
	//const float Gx[] = { -1, 0, 1,
	//	-2, 0, 2,
	//	-1, 0, 1 };
	//const float Gy[] = { 1, 2, 1,
	//	0, 0, 0,
	//	-1, -2, -1 };

	//for (int m = khalf; m < Img_src->Width - khalf; m++)
	//for (int n = khalf; n < Img_src->Height - khalf; n++) {
	//	float pixel = 0.0;
	//	float pixel2 = 0.0;
	//	size_t c = 0;
	//	for (int j = -khalf; j <= khalf; j++)
	//	for (int i = -khalf; i <= khalf; i++) {
	//		pixel += Img_src->rgbpix[(n - j) * Img_src->Width + m - i] * Gx[c];
	//		pixel2 += Img_src->rgbpix[(n - j) * Img_src->Width + m - i] * Gy[c];
	//		c++;
	//	}

	//	//pixel = 255 * (pixel - min) / (max - min);
	//	DerrivativeX_image->rgbpix[n * Img_src->Width + m] = pixel;
	//	DerrivativeY_image->rgbpix[n * Img_src->Width + m] = pixel2;
	//}
}

/*
	Find   M A G N I T U D E  of Gradient
*/
void FindMagnitudeOfGradient(struct Image *DerrivativeX_image, struct Image *DerrivativeY_image, struct Image *Magnitude)
{
	int r, c, pos, sq1, sq2;
	int rows = DerrivativeX_image->Height;
	int cols = DerrivativeX_image->Width;

	for (r = 0, pos = 0; r < rows; r++)
	{
		for (c = 0; c < cols; c++, pos++)
		{
			sq1 = DerrivativeX_image->rgbpix[pos] * DerrivativeX_image->rgbpix[pos];
			sq2 = DerrivativeY_image->rgbpix[pos] * DerrivativeY_image->rgbpix[pos];
			Magnitude->rgbpix[pos] = (int)(0.5 + sqrt((float)sq1 + (float)sq2));
		}
	}
}

/*
	Find  N O N - M A X - S U P P
*/
void FindNonMaximumSupp(struct Image *Magnitude, struct Image *DerrivativeX, struct Image *DerrivativeY, struct Image *NMS)
{
	int rowcount, colcount, count;
	unsigned char *magrowptr, *magptr;
	unsigned char *gxrowptr, *gxptr;
	unsigned char *gyrowptr, *gyptr, z1, z2;
	unsigned char m00, gx = 0, gy = 0;
	float mag1 = 0, mag2 = 0, xperp = 0, yperp = 0;
	unsigned char *resultrowptr, *resultptr;
	int nrows = DerrivativeX->Height;
	int ncols = DerrivativeX->Width;

	unsigned char *result = NMS->rgbpix;
	unsigned char *mag = Magnitude->rgbpix;
	unsigned char *gradx = DerrivativeX->rgbpix;
	unsigned char *grady = DerrivativeY->rgbpix;


	/****************************************************************************
	* Zero the edges of the result image.
	****************************************************************************/
	for (count = 0, resultrowptr = NMS->rgbpix, resultptr = NMS->rgbpix + ncols*(nrows - 1);
		count<ncols; resultptr++, resultrowptr++, count++){
		*resultrowptr = *resultptr = (unsigned char)0;
	}

	for (count = 0, resultptr = NMS->rgbpix, resultrowptr = NMS->rgbpix + ncols - 1;
		count<nrows; count++, resultptr += ncols, resultrowptr += ncols){
		*resultptr = *resultrowptr = (unsigned char)0;
	}

	/****************************************************************************
	* Suppress non-maximum points.
	****************************************************************************/
	for (rowcount = 1, magrowptr = mag + ncols + 1, gxrowptr = gradx + ncols + 1,
		gyrowptr = grady + ncols + 1, resultrowptr = result + ncols + 1;
		rowcount<nrows - 2; rowcount++, magrowptr += ncols, gyrowptr += ncols, gxrowptr += ncols,
		resultrowptr += ncols)
	{
		for (colcount = 1, magptr = magrowptr, gxptr = gxrowptr, gyptr = gyrowptr,
			resultptr = resultrowptr; colcount<ncols - 2;
			colcount++, magptr++, gxptr++, gyptr++, resultptr++)
		{
			m00 = *magptr;
			if (m00 == 0){
				*resultptr = (unsigned char)NOEDGE;
			}
			else{
				xperp = -(gx = *gxptr) / ((float)m00);
				yperp = (gy = *gyptr) / ((float)m00);
			}

			if (gx >= 0){
				if (gy >= 0){
					if (gx >= gy)
					{
						/* 111 */
						/* Left point */
						z1 = *(magptr - 1);
						z2 = *(magptr - ncols - 1);

						mag1 = (m00 - z1)*xperp + (z2 - z1)*yperp;

						/* Right point */
						z1 = *(magptr + 1);
						z2 = *(magptr + ncols + 1);

						mag2 = (m00 - z1)*xperp + (z2 - z1)*yperp;
					}
					else
					{
						/* 110 */
						/* Left point */
						z1 = *(magptr - ncols);
						z2 = *(magptr - ncols - 1);

						mag1 = (z1 - z2)*xperp + (z1 - m00)*yperp;

						/* Right point */
						z1 = *(magptr + ncols);
						z2 = *(magptr + ncols + 1);

						mag2 = (z1 - z2)*xperp + (z1 - m00)*yperp;
					}
				}
				else
				{
					if (gx >= -gy)
					{
						/* 101 */
						/* Left point */
						z1 = *(magptr - 1);
						z2 = *(magptr + ncols - 1);

						mag1 = (m00 - z1)*xperp + (z1 - z2)*yperp;

						/* Right point */
						z1 = *(magptr + 1);
						z2 = *(magptr - ncols + 1);

						mag2 = (m00 - z1)*xperp + (z1 - z2)*yperp;
					}
					else
					{
						/* 100 */
						/* Left point */
						z1 = *(magptr + ncols);
						z2 = *(magptr + ncols - 1);

						mag1 = (z1 - z2)*xperp + (m00 - z1)*yperp;

						/* Right point */
						z1 = *(magptr - ncols);
						z2 = *(magptr - ncols + 1);

						mag2 = (z1 - z2)*xperp + (m00 - z1)*yperp;
					}
				}
			}
			else
			{
				if ((gy = *gyptr) >= 0)
				{
					if (-gx >= gy)
					{
						/* 011 */
						/* Left point */
						z1 = *(magptr + 1);
						z2 = *(magptr - ncols + 1);

						mag1 = (z1 - m00)*xperp + (z2 - z1)*yperp;

						/* Right point */
						z1 = *(magptr - 1);
						z2 = *(magptr + ncols - 1);

						mag2 = (z1 - m00)*xperp + (z2 - z1)*yperp;
					}
					else
					{
						/* 010 */
						/* Left point */
						z1 = *(magptr - ncols);
						z2 = *(magptr - ncols + 1);

						mag1 = (z2 - z1)*xperp + (z1 - m00)*yperp;

						/* Right point */
						z1 = *(magptr + ncols);
						z2 = *(magptr + ncols - 1);

						mag2 = (z2 - z1)*xperp + (z1 - m00)*yperp;
					}
				}
				else
				{
					if (-gx > -gy)
					{
						/* 001 */
						/* Left point */
						z1 = *(magptr + 1);
						z2 = *(magptr + ncols + 1);

						mag1 = (z1 - m00)*xperp + (z1 - z2)*yperp;

						/* Right point */
						z1 = *(magptr - 1);
						z2 = *(magptr - ncols - 1);

						mag2 = (z1 - m00)*xperp + (z1 - z2)*yperp;
					}
					else
					{
						/* 000 */
						/* Left point */
						z1 = *(magptr + ncols);
						z2 = *(magptr + ncols + 1);

						mag1 = (z2 - z1)*xperp + (m00 - z1)*yperp;

						/* Right point */
						z1 = *(magptr - ncols);
						z2 = *(magptr - ncols - 1);

						mag2 = (z2 - z1)*xperp + (m00 - z1)*yperp;
					}
				}
			}

			/* Now determine if the current point is a maximum point */

			if ((mag1 > 0.0) || (mag2 > 0.0))
			{
				*resultptr = (unsigned char)NOEDGE;
			}
			else
			{
				if (mag2 == 0.0)
					*resultptr = (unsigned char)NOEDGE;
				else
					*resultptr = (unsigned char)POSSIBLE_EDGE;
			}
		}
	}
}

/*
	Find     H Y S T E R E S I S
*/
void FindHysteresis(struct Image *Magnitude, struct Image *NMS, struct Image *Img_dst, float Algo_param1, float Algo_param2)
{
	int r, c, pos, edges, highcount, lowthreshold, highthreshold,
		i, hist[32768], rr, cc;
	unsigned char maximum_mag, sumpix;

	int rows = Img_dst->Height;
	int cols = Img_dst->Width;

	/****************************************************************************
	* Initialize the Img_dst->rgbpix map to possible Img_dst->rgbpixs everywhere the non-maximal
	* suppression suggested there could be an Img_dst->rgbpix except for the border. At
	* the border we say there can not be an Img_dst->rgbpix because it makes the
	* follow_Img_dst->rgbpixs algorithm more efficient to not worry about tracking an
	* Img_dst->rgbpix off the side of the image.
	****************************************************************************/
	for (r = 0, pos = 0; r<rows; r++){
		for (c = 0; c<cols; c++, pos++){
			if (NMS->rgbpix[pos] == POSSIBLE_EDGE) Img_dst->rgbpix[pos] = POSSIBLE_EDGE;
			else Img_dst->rgbpix[pos] = NOEDGE;
		}
	}

	for (r = 0, pos = 0; r<rows; r++, pos += cols){
		Img_dst->rgbpix[pos] = NOEDGE;
		Img_dst->rgbpix[pos + cols - 1] = NOEDGE;
	}
	pos = (rows - 1) * cols;
	for (c = 0; c<cols; c++, pos++){
		Img_dst->rgbpix[c] = NOEDGE;
		Img_dst->rgbpix[pos] = NOEDGE;
	}

	/****************************************************************************
	* Compute the histogram of the magnitude image. Then use the histogram to
	* compute hysteresis thresholds.
	****************************************************************************/
	for (r = 0; r<32768; r++) hist[r] = 0;
	for (r = 0, pos = 0; r<rows; r++)
	{
		for (c = 0; c<cols; c++, pos++)
		{
			if (Img_dst->rgbpix[pos] == POSSIBLE_EDGE) hist[Magnitude->rgbpix[pos]]++;
		}
	}

	/****************************************************************************
	* Compute the number of pixels that passed the nonmaximal suppression.
	****************************************************************************/
	for (r = 1, edges = 0; r<32768; r++)
	{
		if (hist[r] != 0) maximum_mag = r;
		edges += hist[r];
	}

	highcount = (int)(edges * Algo_param2 + 0.5);

	/****************************************************************************
	* Compute the high threshold value as the (100 * Algo_param2) percentage point
	* in the magnitude of the gradient histogram of all the pixels that passes
	* non-maximal suppression. Then calculate the low threshold as a fraction
	* of the computed high threshold value. John Canny said in his paper
	* "A Computational Approach to Img_dst->rgbpix Detection" that "The ratio of the
	* high to low threshold in the implementation is in the range two or three
	* to one." That means that in terms of this implementation, we should
	* choose Algo_param1 ~= 0.5 or 0.33333.
	****************************************************************************/
	r = 1;
	edges = hist[1];
	while ((r<(maximum_mag - 1)) && (edges < highcount))
	{
		r++;
		edges += hist[r];
	}
	highthreshold = r;
	lowthreshold = (int)(highthreshold * Algo_param1 + 0.5);


	/****************************************************************************
	* This loop looks for pixels above the highthreshold to locate Img_dst->rgbpixs and
	* then calls follow_Img_dst->rgbpixs to continue the Img_dst->rgbpix.
	****************************************************************************/
	for (r = 0, pos = 0; r<rows; r++)
	{
		for (c = 0; c<cols; c++, pos++)
		{
			if ((Img_dst->rgbpix[pos] == POSSIBLE_EDGE) && (Magnitude->rgbpix[pos] >= highthreshold)){
				Img_dst->rgbpix[pos] = EDGE;
				Follow_edges((Img_dst->rgbpix + pos), (Magnitude->rgbpix + pos), lowthreshold, cols);
			}
		}
	}

	/****************************************************************************
	* Set all the remaining possible Img_dst->rgbpixs to non-Img_dst->rgbpixs.
	****************************************************************************/
	for (r = 0, pos = 0; r<rows; r++)
	{
		for (c = 0; c<cols; c++, pos++) if (Img_dst->rgbpix[pos] != EDGE) Img_dst->rgbpix[pos] = NOEDGE;
	}
}

/*
	Follow  E D G E S
*/
void Follow_edges(unsigned char *edgemapptr, unsigned char *edgemagptr, unsigned char lowval, int cols)
{
	unsigned char *tempmagptr;
	unsigned char *tempmapptr;
	int i;
	float thethresh;
	int x[8] = { 1, 1, 0, -1, -1, -1, 0, 1 },
		y[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };

	for (i = 0; i<8; i++){
		tempmapptr = edgemapptr - y[i] * cols + x[i];
		tempmagptr = edgemagptr - y[i] * cols + x[i];

		if ((*tempmapptr == POSSIBLE_EDGE) && (*tempmagptr > lowval)){
			*tempmapptr = (unsigned char)EDGE;
			Follow_edges(tempmapptr, tempmagptr, lowval, cols);
		}
	}
}

/*
	C O N V O L U T I O N
*/
void Convolution(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize)
{
	int i, j, n, m;
	int FinalNum;
	int DevideNumber = pow((double)KernelSize, 2);

	for (i = KernelSize / 2; i < cols - KernelSize / 2; i++)
	{
		for (j = KernelSize / 2; j < rows - KernelSize / 2; j++)
		{
			//OutputArray[i * cols + j] = OutputArray[i * cols + j];
			size_t c = 0;
			FinalNum = 0;
			for (n = -KernelSize / 2; n <= KernelSize / 2; n++)
			{
				DevideNumber = 9;
				for (m = -KernelSize / 2; m <= KernelSize / 2; m++)
				{
					FinalNum += InputArray[(j - n) * cols + i - m] * Kernel[c];
					//if (FinalNum != 0)printf("%d\n", FinalNum);
					//if (Kernel[c] == 0) DevideNumber--;
					c++;
				}
			}
			if (DevideNumber <= 0) 
				FinalNum = 0;
			else 
				FinalNum = (double)FinalNum / DevideNumber;

			if (FinalNum < 0) FinalNum = 0;
			else if (FinalNum > 255) FinalNum = 255;
			//if (FinalNum > 128) FinalNum = 255;
			//else FinalNum = 0;

			OutputArray[j * cols + i] = FinalNum;
		}
	}
}


/*
	C O N V O L U T I O N - Binary Image
*/
void ConvolutionBinary(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize, int DilateOrErode)
{
	int i, j, n, m;
	int FinalNum;
	int DevideNumber = pow((double)KernelSize, 2);

	for (i = KernelSize / 2; i < cols - KernelSize / 2; i++)
	{
		for (j = KernelSize / 2; j < rows - KernelSize / 2; j++)
		{

			size_t c = 0;
			FinalNum = 0;
			//printf("%f", InputArray[(j)* cols + i]);
			if (InputArray[(j) * cols + i] > 0)
			{
				for (n = -KernelSize / 2; n <= KernelSize / 2; n++)
				{
					for (m = -KernelSize / 2; m <= KernelSize / 2; m++)
					{
						if (Kernel[c] == 1)
						{
							if (DilateOrErode == 0)
								OutputArray[(j - n) * cols + i - m] = 255;
							else
								OutputArray[(j - n) * cols + i - m] = 0;
						}
						else
						{
							OutputArray[(j - n) * cols + i - m] = InputArray[(j - n) * cols + i - m];
						}
						c++;
					}
				}
			}
			else
			{
				OutputArray[(j) * cols + i ] = InputArray[(j) * cols + i];
			}
		}
	}
}

/*
	M I R R O R  image - horizontal
*/
struct Image MirrorImageHorizontal(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j, l, k;

	for (i = Img_dst->Height-1; i >= 0; i--)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				Img_dst->rgbpix[((Img_dst->Height - 1 - i) * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * j) + l] = Img_src->rgbpix[(i * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * j) + l];
			}
		}
	}

	return *Img_dst;
}

/*
	M I R R O R  image - vertical
*/
struct Image MirrorImageVertical(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j, l;

	if (Img_src->Num_channels != Img_dst->Num_channels)
		return *Img_dst;

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				Img_dst->rgbpix[(i * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * (Img_dst->Width - 1 - j)) + l] = Img_src->rgbpix[(i * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * j) + l];
			}
		}
	}

	return *Img_dst;
}

/*
	C R O P  image - around given point and given new dimensions
*/
struct Image CropImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int NewWidth, int NewHeight)
{
	int i, j, l, k, z;

	if (Img_src->Num_channels != Img_dst->Num_channels)
		return *Img_dst;

	if (NewWidth >= Img_src->Width || NewHeight >= Img_src->Height)
		return *Img_dst;

	/* Modify Img_dst */
	Img_dst->Width = NewWidth;
	Img_dst->Height = NewHeight;
	Img_dst->rgbpix = (unsigned char *)realloc(Img_dst->rgbpix, Img_dst->Width * Img_dst->Height * Img_dst->Num_channels * sizeof(unsigned char));

	/* Modify Central point - shift x and y*/
	if (CentralPoint.X > Img_src->Width - 1) CentralPoint.X = Img_src->Width - 1;
	if (CentralPoint.Y > Img_src->Height - 1) CentralPoint.X = Img_src->Height - 1;
	if (CentralPoint.X < 0) CentralPoint.X = 0;
	if (CentralPoint.Y < 0) CentralPoint.X = 0;

	/* for x - too right*/
	if (Img_src->Width < (NewWidth / 2) + CentralPoint.X) CentralPoint.X = Img_src->Width - (NewWidth / 2) - 1;
	/* for y - too down */
	if (Img_src->Height < (NewHeight / 2) + CentralPoint.Y) CentralPoint.Y = Img_src->Height - (NewHeight / 2) - 1;
	/* for x - too left*/
	if (CentralPoint.X - (NewWidth / 2 ) < 0) CentralPoint.X = (NewWidth / 2) + 1;
	/* for y - too up*/
	if (CentralPoint.Y - (NewHeight / 2) < 0) CentralPoint.Y = (NewHeight / 2) + 1;

	k = CentralPoint.Y - (NewHeight / 2);
	for (i = 0; i < Img_dst->Height; i++)
	{
		k++;
		z = CentralPoint.X - (NewWidth / 2);
		for (j = 0; j < Img_dst->Width; j++)
		{
			z++;
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				Img_dst->rgbpix[(i * Img_dst->Num_channels * Img_dst->Width) + (Img_dst->Num_channels * j) + l] = Img_src->rgbpix[(k * Img_src->Num_channels * Img_src->Width) + (Img_src->Num_channels * z) + l];
			}
		}
	}

	return *Img_dst;
}

/*
	M O R P H O L O G Y  -  Dilation
*/
struct Image MorphDilate(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
	int i, j, l, k, z;
	//float StructureElement[9] = { 0, 1, 0, 1, 1, 1, 0, 1, 0 };
	float StructureElement[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	/* Only Grayscale is currently supported */
	if ((Img_src->Num_channels != Img_dst->Num_channels) && Img_dst->Num_channels != 1)
		return *Img_dst;
	if (ElementSize < 3) ElementSize = 3;
	if (NumberOfIterations < 0) NumberOfIterations = 0;
	if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);
	
	ConvolutionBinary(Img_src->rgbpix, Img_dst->rgbpix, Img_src->Height, Img_src->Width, &StructureElement, ElementSize, 0);

	if (NumberOfIterations % 2 == 0)
	{
		if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
		{
			NumberOfIterations -= 1;
			if (NumberOfIterations != 0) MorphDilate(Img_src, Img_dst, ElementSize, NumberOfIterations);
			return *Img_dst;
		}
	}
	else
	{
		if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
		{
			NumberOfIterations -= 1;
			if (NumberOfIterations != 0) MorphDilate(Img_dst, Img_src, ElementSize, NumberOfIterations);
			return *Img_src;
		}
	}
	
}


/*
	M O R P H O L O G Y  -  Erosion
*/
struct Image MorphErode(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
	int i, j, l, k, z;
	float StructureElement[9] = { 0, 1, 0, 1, 1, 1, 0, 1, 0 };

	if (Img_src->Num_channels != Img_dst->Num_channels)
		return *Img_dst;
	if (ElementSize < 3) ElementSize = 3;
	if (NumberOfIterations < 0) NumberOfIterations = 0;
	
	if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

	
	ConvolutionBinary(Img_src->rgbpix, Img_dst->rgbpix, Img_src->Height, Img_src->Width, StructureElement, ElementSize, 1);

	if (NumberOfIterations % 2 == 0)
	{
		if (NumberOfIterations > 0 && NumberOfIterations % 2 == 0)
		{
			NumberOfIterations -= 1;
			if(NumberOfIterations != 0) MorphErode(Img_src, Img_dst, ElementSize, NumberOfIterations);
			return *Img_dst;
		}
	}
	else
	{
		if (NumberOfIterations > 0 && NumberOfIterations % 2 == 1)
		{
			NumberOfIterations -= 1;
			if (NumberOfIterations != 0) MorphErode(Img_dst, Img_src, ElementSize, NumberOfIterations);
			return *Img_src;
		}
	}
}


/*
	M O R P H O L O G Y  -  Opening
*/
struct Image MorphOpen(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
	struct Image BackupImage = CreateNewImage(&BackupImage, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	
	MorphErode(Img_src, Img_dst, ElementSize, NumberOfIterations);
	MorphDilate(Img_dst, &BackupImage, ElementSize, NumberOfIterations);

	memcpy(Img_dst->rgbpix, BackupImage.rgbpix, Img_dst->Width * Img_dst->Height * sizeof(unsigned char));
	//memcpy(Img_dst, &BackupImage, sizeof(BackupImage));

	DestroyImage(&BackupImage);
	return *Img_dst;
}
/*
	M O R P H O L O G Y  -  Closing
*/
struct Image MorphClose(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberOfIterations)
{
	struct Image BackupImage = CreateNewImage(&BackupImage, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);

	MorphDilate(Img_src, Img_dst, ElementSize, NumberOfIterations);
	MorphErode(Img_dst, &BackupImage, ElementSize, NumberOfIterations);

	memcpy(Img_dst->rgbpix, BackupImage.rgbpix, Img_dst->Width * Img_dst->Height * sizeof(unsigned char));
	//memcpy(Img_dst, &BackupImage, sizeof(Img_dst));

	DestroyImage(&BackupImage);
	return *Img_dst;


}

/*
	S H A R P   image - using EdgeExtraction function
*/
struct Image SharpImageContours(struct Image *Img_src, struct Image *Img_dst, int Percentage)
{
	int i, j, l;

	Image Img_dst_Grayscale = CreateNewImage(&Img_dst_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);
	Image Img_src_Grayscale = CreateNewImage(&Img_src_Grayscale, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);

	if (abs(Percentage) > 1) Percentage /= 100;
	Percentage *= -1;
	if ((Img_src->Width != Img_dst->Width) || (Img_src->Height != Img_dst->Height)) SetDestination(Img_src, Img_dst);

	ConvertToGrayscale_1Channel(Img_src, &Img_src_Grayscale);

	EdgeExtraction(&Img_src_Grayscale, &Img_dst_Grayscale, EDGES_PREWITT, 1, 0.9);

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				if (Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] >= EDGE)//POSSIBLE_EDGE)
				{
					if (Percentage * Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
					{	
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
					}
					else if (Percentage * Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
					{
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
					}
					else
					{
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Percentage * Img_dst_Grayscale.rgbpix[(i*Img_src->Width + j)] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
					}
				}
				else
				{
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
					break;
				}
			}
		}
	}

	DestroyImage(&Img_dst_Grayscale);
	DestroyImage(&Img_src_Grayscale);

	

	return *Img_dst;
}

/*
	S H A R P   image - using Binary mask
*/
struct Image SharpImageBinary(struct Image *Img_src, struct Image *Img_dst, struct Image *Img_Binary, int Percentage)
{
	int i, j, l;

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			for (l = 0; l < Img_dst->Num_channels; l++)
			{
				if (Img_Binary->rgbpix[(i*Img_src->Width + j)] == 1)
				{
					if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] > 255)
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 255;
					else if (Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] < 0)
						Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = 0;
					else Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Percentage * Img_src->rgbpix[3 * (i*Img_src->Width + j) + l] + Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
				}
				else
					Img_dst->rgbpix[3 * (i*Img_src->Width + j) + l] = Img_src->rgbpix[3 * (i*Img_src->Width + j) + l];
			}
		}
	}

	return *Img_dst;
}

/*
	C O L O R  -  artificial
*/
struct Image ColorFromGray(struct Image *Img_src, struct Image *Img_dst, struct ColorPoint_RGB ColorPoint)
{
	int i, j, l;
	double R_to_G_Ratio = 0;
	double B_to_G_Ratio = 0;

	// The input should be 1 channel image. The output is 3 channel
	if (Img_src->Num_channels != 1 || Img_dst->Num_channels != 3)
	{
		return *Img_dst;
	}

	R_to_G_Ratio = ColorPoint.R / (double)ColorPoint.G;
	B_to_G_Ratio = ColorPoint.B / (double)ColorPoint.G;

	//step 1: copy the gray information to RGB channels
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			if (R_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)] > 255) 
				Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = 255;
			else 
				Img_dst->rgbpix[3 * (i*Img_src->Width + j)] = R_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)];
			Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 1] = Img_src->rgbpix[(i*Img_src->Width + j)];
			if (B_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)] > 255) 
				Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = 255;
			else 
				Img_dst->rgbpix[3 * (i*Img_src->Width + j) + 2] = B_to_G_Ratio*Img_src->rgbpix[(i*Img_src->Width + j)];
		}
	}
	//step 2: Gamma correction for B and R channels
	//GammaCorrection(Img_src, Img_dst, 0.6, 0.95, 0.7);

	return *Img_dst;
}

/*
	B I N A R Y  image 
*/
struct Image ConvertToBinary(struct Image *Img_src, struct Image *Img_dst, int Threshold)
{
	int i, j, l;
	int Sum = 0;
	int AverageGray = 0;

	struct Image GrayImage = CreateNewImage(&GrayImage, Img_src->Width, Img_src->Height, 1, COLORSPACE_GRAYSCALE);

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}
	if (Img_src->Num_channels != 1)
	{
		ConvertToGrayscale_1Channel(Img_src, &GrayImage);
	}
	if (Threshold == 0)
	{
		for (i = 0; i < Img_dst->Height; i++)
		{
			for (j = 0; j < Img_dst->Width; j++)
			{
				if (Img_src->Num_channels != 1)
					Sum += GrayImage.rgbpix[i * Img_dst->Width + j];
				else
					Sum += Img_src->rgbpix[i * Img_dst->Width + j];
			}
		}
		AverageGray = Sum / (double)(Img_dst->Width * Img_dst->Height);
	}
	else
		AverageGray = Threshold;
	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			if (Img_src->Num_channels == 1)
			{
				if (Img_src->rgbpix[i * Img_src->Width + j] > AverageGray)
					Img_dst->rgbpix[i * Img_src->Width + j] = 255;
				else
					Img_dst->rgbpix[i * Img_src->Width + j] = 0;
			}
			else
			{
				if (GrayImage.rgbpix[i * Img_src->Width + j] > AverageGray)
					Img_dst->rgbpix[i * Img_src->Width + j] = 255;
				else
					Img_dst->rgbpix[i * Img_src->Width + j] = 0;
			}
		}
	}

	DestroyImage(&GrayImage);
	return *Img_dst;
}

/*
	Chech Color values - Used in HSL - RGB conversion
*/
float CheckColorValue(float TempColor, float Temporary_1, float Temporary_2)
{
	float NewColor;

	if (6 * TempColor < 1)
	{
		NewColor = Temporary_2 + ((Temporary_1 - Temporary_2) * 6 * TempColor);
	}
	else
	{
		if (2 * TempColor < 1)
		{
			NewColor = Temporary_1;
		}
		else
		{
			if (3 * TempColor < 2)
			{
				NewColor = Temporary_2 + ((Temporary_1 - Temporary_2) * 6 * (0.6666 - TempColor));
			}
			else
				NewColor = Temporary_2;
		}
	}

	return NewColor;
}

/*
	C O N V E R T -  RGB to HSV
*/
void ConvertImage_RGB_to_HSL(struct Image *Img_src, struct Image *Img_dst)
{
	FILE *fdebug = NULL;
	float R_scaled;
	float G_scaled;
	float B_scaled;
	float C_max;
	float C_min;
	float Delta;
	float Hue;
	int TestValue;

	float del_R, del_B, del_G;
	float Saturation, Luma = 0;

	int i, j;

	if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input images are not with 3 channels\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}

	if (Img_src->ColorSpace != 2)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in RGB format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	Img_dst->ColorSpace = 5;


	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			R_scaled = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 0] / (float)255;
			G_scaled = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 1] / (float)255;
			B_scaled = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 2] / (float)255;
			C_max = MAX(R_scaled, MAX(G_scaled, B_scaled));
			C_min = MIN(R_scaled, MIN(G_scaled, B_scaled));
			Delta = C_max - C_min;

			// HUE
			if (C_max == R_scaled)
			{
				Hue = ceil((60 * ( fmod(((G_scaled - B_scaled)/ Delta),6) )));
				TestValue = (60 * ( fmod(((G_scaled - B_scaled)/ Delta),6) ));
				if(Hue - TestValue < 0.5) Hue -= 1;
				if (Hue < 0) Hue = 360 + Hue;
				if (Hue > 360) Hue = fmod(Hue, 360);
			}
			else if (C_max == G_scaled)
			{
				Hue = ceil((60 * (((B_scaled - R_scaled) / Delta) + 2)));
				TestValue = (60 * (((B_scaled - R_scaled) / Delta) + 2));
				if(Hue - TestValue < 0.5) Hue -= 1;
				if (Hue < 0) Hue = 360 + Hue;
				if (Hue > 360) Hue = fmod(Hue, 360);
			}
			else if (C_max == B_scaled)
			{
				Hue = ceil((60 * (((R_scaled - G_scaled) / Delta) + 4)));
				TestValue = (60 * (((R_scaled - G_scaled) / Delta) + 4));
				if(Hue - TestValue < 0.5) Hue -= 1;

				if (Hue < 0) Hue = 360 + Hue;
				if (Hue > 360) Hue = fmod(Hue, 360);
			}
			

			// LUMA
			Luma = (C_max + C_min) / (float)2;

			// SATURATION
			if (Delta == 0) 
			{
				Saturation = 0; 
				Hue = 0;
			}
			else
			{ 
				Saturation = Luma > 0.5 ? Delta / (float)(2 - C_max - C_min) : Delta / (float)(C_max + C_min);
			}

			TestValue = (Hue / (float)360) * 100;
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] = ceil((Hue / (float)360) * 100);
			if(Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] - TestValue < 0.5) Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] -= 1;

			TestValue = Saturation * 100;
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] = ceil(Saturation * 100);
			if(Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] - TestValue < 0.5) Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] -= 1;

			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] = Luma * 100;//ceil(Luma * 100);

		}
	}
}

/*
	C O N V E R T -  HSL to RGB
*/
void ConvertImage_HSL_to_RGB(struct Image *Img_src, struct Image *Img_dst)
{
	FILE *fdebug = NULL;
	float R_temp;
	float G_temp;
	float B_temp;
	float C, X, m;
	float Hue;
	float Temporary_1, Temporary_2;
	float Saturation, Luma = 0;
	int Test_value;

	int i, j;

	if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input images are not with 3 channels\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}

	if (Img_src->ColorSpace != 5)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in HSL format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	Img_dst->ColorSpace = 2;


	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			
			Hue = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 0] / (float)100;
			
			Saturation = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 1] / (float)100;

			Luma = Img_src->rgbpix[3 * (i * Img_src->Width + j) + 2] / (float)100;
			
			if (Saturation == 0)
			{
				R_temp = Luma * 255;
				if (R_temp > 255) 
				{
					R_temp = 255; B_temp = 255; G_temp = 255;
				}
				else
					B_temp = G_temp = R_temp;
			}
			else
			{
				if (Luma >= 0.5)
				{
					Temporary_1 = ((Luma + Saturation)  - (Luma * Saturation));
				}
				else
				{
					Temporary_1 = Luma * (1 + Saturation);
				}
				Temporary_2 = 2 * Luma - Temporary_1;
				
				R_temp = Hue + 0.33333;
				if (R_temp < 0) R_temp += 1;
				if (R_temp > 1) R_temp -= 1;
				
				G_temp = Hue;
				if (G_temp < 0) G_temp += 1;
				if (G_temp > 1) G_temp -= 1;

				B_temp = Hue - 0.33333;
				if (B_temp < 0) B_temp += 1;
				if (B_temp > 1) B_temp -= 1;

				// Check R
				R_temp = CheckColorValue(R_temp, Temporary_1, Temporary_2);

				// Check G
				G_temp = CheckColorValue(G_temp, Temporary_1, Temporary_2);

				// Check B
				B_temp = CheckColorValue(B_temp, Temporary_1, Temporary_2);

				
				R_temp *= 255;
				if (R_temp > 255) R_temp = 255;
				G_temp *= 255;
				if (G_temp > 255) G_temp = 255;
				B_temp *= 255;
				if (B_temp > 255) B_temp = 255;
			}
			
			Test_value = R_temp;
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] = ceil(R_temp);
			if(Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] - Test_value > 0.5 ) Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 0] -= 1;

			Test_value = G_temp;
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] = ceil(G_temp);
			if(Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] - Test_value > 0.5 ) Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 1] -= 1;

			Test_value = B_temp;
			Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] = ceil(B_temp);
			if(Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] - Test_value > 0.5 ) Img_dst->rgbpix[3 * (i * Img_src->Width + j) + 2] -= 1;
		}
	}
}

/*
	S A T U R A T I O N 
*/
struct Image Saturation(struct Image *Img_src, struct Image *Img_dst, int percentage)
{
	FILE * fdebug = NULL;
	int i, j;
	struct Image WorkCopy = CreateNewImage(&WorkCopy, Img_src->Width, Img_src->Height, 3, Img_src->ColorSpace);

	/* If the input is RGB -> the output is also RGB. if the input is HSL -> the output is also HSL */
	if (Img_src->ColorSpace != 2 && Img_src->ColorSpace != 5)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in HSL format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if ((Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height) || (Img_src->ColorSpace != Img_dst->ColorSpace))
	{
		SetDestination(Img_src, Img_dst);
	}
	
	/* We have to work in HSL color space */
	if (Img_src->ColorSpace == 5)  // if the input image is HSL
	{
		memcpy(WorkCopy.rgbpix, Img_src->rgbpix, 3 * Img_src->Width * Img_src->Height * sizeof(unsigned char));
	}
	else // if the input image is RGB
	{
		ConvertImage_RGB_to_HSL(Img_src, &WorkCopy);
	}

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			// We work over WorkCopy
			if (WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] * percentage / (float)100 + WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] > 100)
				WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] = 100;
			else if (WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] * percentage / (float)100 + WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] < 0)
				WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] = 0;
			else
				WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] = ceil((WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1] * percentage / (float)100) + WorkCopy.rgbpix[3 * (i * Img_src->Width + j) + 1]);
		}
	}

	/* We have to return Image with the same color space as the input */
	if (Img_src->ColorSpace == 5)  // if the input image is HSL
	{			
		memcpy(Img_dst->rgbpix, WorkCopy.rgbpix, 3 * WorkCopy.Width * WorkCopy.Height * sizeof(unsigned char));
		Img_dst->ColorSpace = WorkCopy.ColorSpace;
		//DestroyImage(&WorkCopy);
		return WorkCopy;
	}
	else // if the input image is RGB
	{
		ConvertImage_HSL_to_RGB(&WorkCopy, Img_dst);
		DestroyImage(&WorkCopy);
		return *Img_dst;
	}
}

/*
	C O N V E R T  - RGB to XYZ 
*/
void Convert_RGB_to_XYZ(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j;
	float X, Y, Z;
	float var_R, var_B, var_G;

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			var_R = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];
			var_G = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];
			var_B = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];

			var_R /= 255;
			var_G /= 255;
			var_B /= 255;

			if (var_R > 0.04045)
				var_R = pow(((var_R + 0.055) / 1.055), 2.4);
			else
				var_R = var_R / 12.92;
			if (var_G > 0.04045)
				var_G = pow(((var_G + 0.055) / 1.055), 2.4);
			else
				var_G = var_G / 12.92;
			if (var_B > 0.04045)
				var_B = pow(((var_B + 0.055) / 1.055), 2.4);
			else
				var_B = var_B / 12.92;

			var_R = var_R * 100;
			var_G = var_G * 100;
			var_B = var_B * 100;

			X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
			Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
			Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
			if (X < 0) X = 0;
			if (Y < 0) Y = 0;
			if (Z < 0) Z = 0;

			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] = ceil(X);
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] = ceil(Y);
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] = ceil(Z);
		}
	}
}

/*
C O N V E R T  - XYZ to RGB
*/
void Convert_XYZ_to_RGB(struct Image *Img_src, struct Image *Img_dst)
{
	int i, j;
	float var_X, var_Y, var_Z;
	float R, B, G;

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			var_X = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];
			var_Y = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];
			var_Z = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];

			var_X /= 100;
			var_Y /= 100;
			var_Z /= 100;

			R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
			G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415;
			B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;

			if (R > 0.0031308)
				R = 1.055 * pow(R, (1 / 2.4)) - 0.055;
			else
				R = 12.92 * R;
			if (G > 0.0031308)
				G = 1.055 * pow(G, (1 / 2.4)) - 0.055;
			else
				G = 12.92 * G;
			if (B > 0.0031308) 
				B = 1.055 * pow(B , (1 / 2.4)) - 0.055;
			else
				B = 12.92 * B;
			
			R = R * 255;
			if (R > 255) R = 255;
			if (R < 0) R = 0;
			G = G * 255;
			if (G > 255) G = 255;
			if (G < 0) G = 0;
			B = B * 255;
			if (B > 255) B = 255;
			if (B < 0) B = 0;

			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] = ceil(R);
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] = ceil(G);
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] = ceil(B);
		}
	}
}

/*
	C O N V E R T - RGB to L*ab
*/
void ConvertImage_RGB_to_LAB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ)
{
	FILE *fdebug = NULL;
	struct Image Img_XYZ = CreateNewImage_BasedOnPrototype(Img_src, &Img_XYZ);

	int i, j;
	float L, a, b;
	float X, Y, Z;
	float F_x, F_y, F_z;
	float RatioY, RatioX, RatioZ;
	float e = 0.008856;
	float k = 903.3;
	if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input images are not with 3 channels\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}

	if (Img_src->ColorSpace != 2)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in RGB format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	Img_dst->ColorSpace = 4;

	// 1st step: Convert to XYZ color space
	Convert_RGB_to_XYZ(Img_src, &Img_XYZ);

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			X = Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 0];
			Y = Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 1];
			Z = Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 2];

			X /= 100.0;
			Y /= 100.0;
			Z /= 100.0;

			/*
			RatioY = Y / 0.333;
			if (RatioY > 0.008856)
			{
				L = 116 * pow(RatioY, (0.333)) - 16;
			}
			else
				L = 903.3 *  RatioY;

			RatioX = X / 0.333;
			RatioY = Y / 0.333;
			RatioZ = Z / 0.333;
			
			if (RatioX > 0.008856)
			{
				NumX = pow(RatioX, 0333);
			}
			else
				NumX = 7.787 * RatioX + 16 / 116.0;

			if (RatioY > 0.008856)
			{
				NumY = pow(RatioY, 0333);
			}
			else
				NumY = 7.787 * RatioY + 16 / 116.0;
			if (RatioZ > 0.008856)
			{
				NumZ = pow(RatioZ, 0333);
			}
			else
				NumZ = 7.787 * RatioZ + 16 / 116.0;
			// Calculate a
			a = 500 * (NumX - NumY);
			// Calculate b
			b = 200 * (NumY - NumZ);
			*/
			RatioX = X / WhitePoint_XYZ.X;
			RatioY = Y / WhitePoint_XYZ.Y;
			RatioZ = Z / WhitePoint_XYZ.Z;

			if (RatioX > e)
			{
				F_x = pow(RatioX, pow(3, -1));
			}
			else
			{
				F_x = (k * RatioX + 16) / 116;
			}

			if (RatioY > e)
			{
				F_y = pow(RatioY, pow(3, -1));
			}
			else
			{
				F_y = (k * RatioY + 16) / 116;
			}

			if (RatioZ > e)
			{
				F_z = pow(RatioZ, pow(3, -1));
			}
			else
			{
				F_z = (k * RatioZ + 16) / 116;
			}

			L = 116 * F_y - 16;
			a = 500 * (F_x - F_y);
			b = 200 * (F_y - F_z);

			if (a > 255) a = 255;
			a += 128;
			if (b > 255) b = 255;
			b += 128;
			if (L < 0) L = 0;
			
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 0] = ceil(L);
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 1] = ceil(a);
			Img_dst->rgbpix[3 * (i * Img_dst->Width + j) + 2] = ceil(b);
		}
	}

	DestroyImage(&Img_XYZ);
}

/*
	C O N V E R T - L*ab to RGB
*/
void ConvertImage_LAB_to_RGB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ)
{
	FILE *fdebug = NULL;
	struct Image Img_XYZ = CreateNewImage_BasedOnPrototype(Img_src, &Img_XYZ);

	int i, j;
	float L, a, b;
	float X, Y, Z;
	float P;
	float Number;
	float RatioY, RatioX, RatioZ;
	if ((Img_src->Num_channels != 3) || (Img_dst->Num_channels != 3))
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input images are not with 3 channels\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	if (Img_src->Width * Img_src->Height != Img_dst->Width * Img_dst->Height)
	{
		SetDestination(Img_src, Img_dst);
	}

	if (Img_src->ColorSpace != 4)
	{
		#ifdef DEBUG_FILE
				fdebug = fopen(DEBUG_FILE, "wt");
				fprintf(fdebug, "The input image is not in L*ab format\n");
				fclose(fdebug);
		#endif // DEBUG_FILE
		return;
	}

	Img_dst->ColorSpace = 2;

	// 1st step: Convert to XYZ color space

	for (i = 0; i < Img_dst->Height; i++)
	{
		for (j = 0; j < Img_dst->Width; j++)
		{
			L = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 0];
			a = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 1];
			b = Img_src->rgbpix[3 * (i * Img_dst->Width + j) + 2];

			a -= 128;
			b -= 128;

			P = (L + 16) / 116.0;
			
			Number = P;
			if (Number > 6 / 29.0)
			{
				Number = pow(Number, 3);
			}
			else
			{
				Number = 3 * pow(6 / 29.0, 2) * (Number - 4 / 29.0);
			}
			Y = WhitePoint_XYZ.Y * Number;

			Number = P + a/ 500.0;
			if (Number > 6 / 29.0)
			{
				Number = pow(Number, 3);
			}
			else
			{
				Number = 3 * pow(6 / 29.0, 2) * (Number - 4 / 29.0);
			}
			X = WhitePoint_XYZ.X * Number;
			
			Number = P - b / 200;
			if (Number > 6 / 29.0)
			{
				Number = pow(Number, 3);
			}
			else
			{
				Number = 3 * pow(6 / 29.0, 2) * (Number - 4 / 29.0);
			}
			Z = WhitePoint_XYZ.Z * Number;

			Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 0] = ceil(X * 100);
			Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 1] = ceil(Y * 100);
			Img_XYZ.rgbpix[3 * (i * Img_dst->Width + j) + 2] = ceil(Z * 100);

		}
	}
	Convert_XYZ_to_RGB(&Img_XYZ, Img_dst);

	DestroyImage(&Img_XYZ);
}

/*
	 W H I T E   P O I N T
*/
/* White Balance function and structure*/
void SetWhiteBalanceValues(struct WhitePoint *WhitePoint_lab, int TYPE)
{
	if (TYPE == 0) // D50
	{
		WhitePoint_lab->X = 0.34567;
		WhitePoint_lab->Y = 0.35850;
		WhitePoint_lab->Z = 0.29583;
	}
	else if (TYPE == 1) // D55
	{
		WhitePoint_lab->X = 0.33242;
		WhitePoint_lab->Y = 0.34743;
		WhitePoint_lab->Z = 0.32015;
	}
	else if (TYPE == 2) // D65
	{
		WhitePoint_lab->X = 0.31273;
		WhitePoint_lab->Y = 0.32902;
		WhitePoint_lab->Z = 0.35825;
	}
	else if (TYPE == 3) // A
	{
		WhitePoint_lab->X = 0.44757;
		WhitePoint_lab->Y = 0.40744;
		WhitePoint_lab->Z = 0.14499;
	}
	else if (TYPE == 4) // C
	{
		WhitePoint_lab->X = 0.31006;
		WhitePoint_lab->Y = 0.31615;
		WhitePoint_lab->Z = 0.37379;
	}
	else if (TYPE == 5) // Daylight
	{
		WhitePoint_lab->X = 0.29902;
		WhitePoint_lab->Y = 0.31485;
		WhitePoint_lab->Z = 0.38613;
	}
	else if (TYPE == 6 ) // Unknow - equal
	{
		WhitePoint_lab->X = 0.333;
		WhitePoint_lab->Y = 0.333;
		WhitePoint_lab->Z = 0.333;
	}
}