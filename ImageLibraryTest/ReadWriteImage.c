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
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  jpeg_start_compress(&cinfo, TRUE);

  row_stride = Img_src.Width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
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
  int i, j, k;
  
    k = (height-line)*3*width;
    for(i=0; i<3*width; i+=3)
    {
      rgbpix[k+i]   = buffer[i];
      rgbpix[k+i+1] = buffer[i+1];
      rgbpix[k+i+2] = buffer[i+2];
    }
} /* end put_scanline */

/*
	Create New Image
*/
struct Image CreateNewImage(struct Image Prototype)
{
	struct Image Img_dst;
	
	Img_dst.ColorSpace = Prototype.ColorSpace;
	Img_dst.Height = Prototype.Height;
	Img_dst.Width = Prototype.Width;
	Img_dst.Num_channels = Prototype.Num_channels;
	Img_dst.rgbpix = (unsigned char *)calloc(Img_dst.Height * Img_dst.Width * Img_dst.Num_channels, sizeof(unsigned char));
	if (Img_dst.rgbpix == NULL) 
	{
		printf("cannot allocate memory for the new image\n"); 
		Img_dst.isLoaded = 0;
	}
	else 
		Img_dst.isLoaded = 1;

	return Img_dst;
}

/*
	B L U R  image function
*/
struct Image BlurImage(struct Image Img_src, struct point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression)
{
	struct Image Img_dst;
	double MaxRatio = 0;
	double Distance = 0;
	double DistanceRatio = 0;
	double *matrix = (double *)calloc(Img_src.Width * Img_src.Height, sizeof(double));
	double Chislo = 0, Chislo2 = 0;
	int Sybiraemo = 0;
	int i, j, z, t, l;

	Img_dst = CreateNewImage(Img_src);
	if (Img_dst.isLoaded != 1) return Img_dst;

	/* Only odd nubers are allowed (bigger than 5*/
	if (BlurPixelRadius % 2 == 0) BlurPixelRadius += 1;
	if (BlurPixelRadius < 5) BlurPixelRadius = 5;

	MaxRatio = (double)MAX(CentralPoint.X - ((double)SizeOfBlur * Img_dst.Width / 100), Img_dst.Width - CentralPoint.X + ((double)SizeOfBlur * Img_dst.Width / 100)) / ((double)SizeOfBlur * Img_dst.Width / 100);
	for (i = 0; i < Img_dst.Height; i++)
	{
		for (j = 0; j < Img_dst.Width; j++)
		{
			//luma = ui->imageData[row * width + col];
			Distance = sqrt(pow(abs((double)CentralPoint.X - j), 2) + pow(abs((double)CentralPoint.Y - i), 2));
			if (Distance < ((double)SizeOfBlur * Img_dst.Width / 100))
			{
				matrix[i * Img_dst.Width + j] = 1;
			}
			else
			{
				DistanceRatio = Distance / ((double)SizeOfBlur * Img_dst.Width / 100);
				matrix[i * Img_dst.Width + j] = 1 - ((double)BlurAgression / 100 * (DistanceRatio / MaxRatio));
				if (matrix[i * Img_dst.Width + j] < 0) matrix[i * Img_dst.Width + j] = 0;
			}
		}
	}

	for (i = BlurPixelRadius / 2; i < Img_dst.Height - BlurPixelRadius / 2; i++)
	{
		for (j = BlurPixelRadius / 2; j < Img_dst.Width - BlurPixelRadius / 2; j++)
		{
			for (l = 0; l < 3; l++)
			{
				if (Img_src.rgbpix[3 * (i*Img_dst.Width + j) + l] > 255)
					Chislo = 0;

				Sybiraemo = 0;
				if (BlurOrSharp == 0)
					Chislo2 = ((double)(matrix[i * Img_dst.Width + j]) / (pow((double)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
				else
					Chislo2 = ((double)(1 - matrix[i * Img_dst.Width + j]) / (pow((double)BlurPixelRadius, 2) - 1 - (12 + (2 * (BlurPixelRadius - 5)))));
				for (z = 0; z < BlurPixelRadius / 2; z++)
				{
					for (t = 0; t < BlurPixelRadius / 2; t++)
					{
						if (z == 0 && t == 0) continue;
						Sybiraemo += Img_src.rgbpix[3*((i - z)*Img_dst.Width + j - t) + l];
						Sybiraemo += Img_src.rgbpix[3*((i - z)*Img_dst.Width + j + t) + l];
						Sybiraemo += Img_src.rgbpix[3*((i + z)*Img_dst.Width + j - t) + l];
						Sybiraemo += Img_src.rgbpix[3*((i + z)*Img_dst.Width + j + t) + l];
					}
				}

				Chislo2 *= Sybiraemo;
				Chislo = 0;
				if (BlurOrSharp == 0)
					Chislo = (1 - matrix[i * Img_dst.Width + j])*Img_src.rgbpix[3*(i*Img_dst.Width + j) + l] + (int)Chislo2;
				else
					Chislo = (matrix[i * Img_dst.Width + j])*Img_src.rgbpix[3*(i*Img_dst.Width + j) + l] + (int)Chislo2;
				if (Chislo > 255)
					Chislo = 255;
				if (Chislo < 0)
					Chislo = 0;
				Img_dst.rgbpix[3 * (i*Img_dst.Width + j) + l] = Chislo;
			}
		}
	}

	return Img_dst;
}

/*
	Affine Transformation: Rotation
*/
struct Image RotateImage(struct Image Img_src, double RotationAngle, struct point_xy CentralPoint)
{
	struct Image Img_dst;
	float Sx, Sy, Matrix[2][2], temp, Cos, Sin;
	int i, j, k, z;
	int x_new = 0, y_new = 0;
	int currentPixel_smallImage = 0;

	Img_dst = CreateNewImage(Img_src);
	if (Img_dst.isLoaded != 1) return Img_dst;

	Sx = (-RotationAngle * 3.14) / 180;
	Cos = cos(Sx);
	Sin = sin(Sx);
	Matrix[0][0] = Cos;
	Matrix[0][1] = -Sin;
	Matrix[1][0] = Sin;
	Matrix[1][1] = Cos;

	for (i = 0; i < Img_dst.Height; i++)
	{
		for (j = 0; j < Img_dst.Width; j++)
		{
			// za vsqko i,j (ot staoroto izobrajenie se namirat novi x_new  i  y_new na koito s epipisvat stoinostite za RGB
			x_new = Matrix[0][0] * (j - CentralPoint.X) + Matrix[0][1] * (i - CentralPoint.Y) + CentralPoint.X;
			y_new = Matrix[1][0] * (j - CentralPoint.X) + Matrix[1][1] * (i - CentralPoint.Y) + CentralPoint.Y;
			if (x_new > Img_dst.Width) x_new = Img_dst.Width;
			if (x_new < 0) x_new = 0;
			if (y_new > Img_dst.Height) y_new = Img_dst.Height;
			if (y_new < 0) y_new = 0;
			Img_dst.rgbpix[y_new * 3 * Img_dst.Width + 3 * x_new + 0] = Img_src.rgbpix[i * 3 * Img_dst.Width + 3 * j + 0];
			Img_dst.rgbpix[y_new * 3 * Img_dst.Width + 3 * x_new + 1] = Img_src.rgbpix[i * 3 * Img_dst.Width + 3 * j + 1];
			Img_dst.rgbpix[y_new * 3 * Img_dst.Width + 3 * x_new + 2] = Img_src.rgbpix[i * 3 * Img_dst.Width + 3 * j + 2];
		}
	}

	return Img_dst;
}

/* Correct Brighness */
struct Image BrightnessCorrection(struct Image Img_src, double percentage)
{
	struct Image Img_dst;
	int i, j, z, l;
	Img_dst = CreateNewImage(Img_src);
	if (Img_dst.isLoaded != 1) return Img_dst;
	
	if (abs(percentage) > 1) percentage /= 100;

	for (i = 0; i < Img_dst.Height; i++)
	{
		for (j = 0; j < Img_dst.Width; j++)
		{
			for (l = 0; l < 3; l++)
			{
				if (percentage * Img_src.rgbpix[3 * (i*Img_dst.Width + j) + l] + Img_src.rgbpix[3 * (i*Img_dst.Width + j) + l]> 255)
					Img_dst.rgbpix[3 * (i*Img_dst.Width + j) + l] = 255;
				else
				{
					if (percentage * Img_src.rgbpix[3 * (i*Img_dst.Width + j) + l] + Img_src.rgbpix[3 * (i*Img_dst.Width + j) + l] < 0)
						Img_dst.rgbpix[3 * (i*Img_dst.Width + j) + l] = 0;
					else	
						Img_dst.rgbpix[3 * (i*Img_dst.Width + j) + l] = percentage * Img_src.rgbpix[3 * (i*Img_dst.Width + j) + l] + Img_src.rgbpix[3 * (i*Img_dst.Width + j) + l];
				}
			}
		}
	}

	return Img_dst;
}

/* Correct Contrass */
struct Image ContrastCorrection(struct Image Img_src, double percentage)
{
	/* The percentage value should be between -100 and 100*/
	double pixel = 0;
	double contrast = 0;
	struct Image Img_dst;
	int i, j, z, l;

	Img_dst = CreateNewImage(Img_src);
	if (Img_dst.isLoaded != 1) return Img_dst;

	if (percentage < -100) percentage = -100;
	if (percentage > 100) percentage = 100;
	contrast = (100.0 + percentage) / 100.0;

	contrast *= contrast;

	for (i = 0; i < Img_dst.Height; i++)
	{
		for (j = 0; j < Img_dst.Width; j++)
		{
			for (l = 0; l < 3; l++)
			{
				pixel = (double)Img_src.rgbpix[3 * (i*Img_dst.Width + j) + l] / 255;
				pixel -= 0.5;
				pixel *= contrast;
				pixel += 0.5;
				pixel *= 255;
				if (pixel < 0) pixel = 0;
				if (pixel > 255) pixel = 255;
				Img_dst.rgbpix[3 * (i*Img_dst.Width + j) + l] = pixel;
			}
		}
	}

	return Img_dst;
}

/* Correct WhiteBalance */
struct Image WhiteBalanceCorrection(struct Image Img_src, double percentage, int Algotype)
{
	struct Image Img_dst;
	Img_dst = CreateNewImage(Img_src);
	if (Img_dst.isLoaded != 1) return Img_dst;


	return Img_dst;
}

/* Correct Noise */
struct Image NoiseCorrection(struct Image Img_src, double percentage, int Algotype)
{
	int i, j, z, l;
	struct Image Img_dst;
	int CurrentValue;
	int ProbablityValue = 0;
	Img_dst = CreateNewImage(Img_src);
	memcpy(&Img_dst, &Img_src,sizeof(Image));

	if (Img_dst.isLoaded != 1) return Img_dst;
	/* if the current pixel is X % different from the pixels around -> it is noise*/

	for (i = 1; i < Img_dst.Height - 1; i++)
	{
		for (j = 1; j < Img_dst.Width - 1; j++)
		{
			for (z = 0; z < 3; z++)
			{
				ProbablityValue = 0;
				CurrentValue = Img_src.rgbpix[i * 3 * Img_src.Width + 3 * j + z];
				
				if (percentage * CurrentValue < Img_src.rgbpix[(i-1) * 3 * Img_src.Width + 3 * j + z] || CurrentValue > percentage *  Img_src.rgbpix[(i-1) * 3 * Img_src.Width + 3 * j + z]) ProbablityValue++;
				if (percentage * CurrentValue < Img_src.rgbpix[(i+1) * 3 * Img_src.Width + 3 * j + z] || CurrentValue > percentage *  Img_src.rgbpix[(i+1) * 3 * Img_src.Width + 3 * j + z]) ProbablityValue++;
				if (percentage * CurrentValue < Img_src.rgbpix[(i) * 3 * Img_src.Width + 3 * (j-1) + z] || CurrentValue > percentage *  Img_src.rgbpix[(i) * 3 * Img_src.Width + 3 * (j-1) + z]) ProbablityValue++;
				if (percentage * CurrentValue < Img_src.rgbpix[(i) * 3 * Img_src.Width + 3 * (j+1) + z] || CurrentValue > percentage *  Img_src.rgbpix[(i) * 3 * Img_src.Width + 3 * (j+1) + z]) ProbablityValue++;
				
				if(ProbablityValue >= 3)
				{
					Img_dst.rgbpix[(i) * 3 * Img_src.Width + 3 * j + z] = ( Img_src.rgbpix[(i-1) * 3 * Img_src.Width + 3 * j + z] + Img_src.rgbpix[(i+1) * 3 * Img_src.Width + 3 * j + z] + Img_src.rgbpix[(i) * 3 * Img_src.Width + 3 * (j-1) + z] + Img_src.rgbpix[(i) * 3 * Img_src.Width + 3 * (j+1) + z])/4;
				}
				//	arr_image[i * 3 * Cropped_width + 3 * j + 0] = (arr_image[(i - 1) * 3 * Cropped_width + 3 * j + 0] + arr_image[(i + 1) * 3 * Cropped_width + 3 * j + 0] + arr_image[i * 3 * Cropped_width + 3 * (j + 1) + 0] + arr_image[(i - 1) * 3 * Cropped_width + 3 * (j - 1) + 0]) / 4;
				//arr_image[i * 3 * Cropped_width + 3 * j + 1] = (arr_image[(i - 1) * 3 * Cropped_width + 3 * j + 1] + arr_image[(i + 1) * 3 * Cropped_width + 3 * j + 1] + arr_image[i * 3 * Cropped_width + 3 * (j + 1) + 1] + arr_image[(i - 1) * 3 * Cropped_width + 3 * (j - 1) + 1]) / 4;
				//arr_image[i * 3 * Cropped_width + 3 * j + 2] = (arr_image[(i - 1) * 3 * Cropped_width + 3 * j + 2] + arr_image[(i + 1) * 3 * Cropped_width + 3 * j + 2] + arr_image[i * 3 * Cropped_width + 3 * (j + 1) + 2] + arr_image[(i - 1) * 3 * Cropped_width + 3 * (j - 1) + 2]) / 4;
			}
		}
	}
	
	return Img_dst;
}
struct Image GammaCorrection(struct Image Img_src, double RedGamma, double GreenGamma, double BlueGamma)
{
	struct Image Img_dst;
	Img_dst = CreateNewImage(Img_src);
	if (Img_dst.isLoaded != 1) return Img_dst;


	return Img_dst;
}

void getPositionFromIndex(struct Image Img_src, int pixIdx, int *red, int *col)
{
	*red = pixIdx / Img_src.Width;
	*col = pixIdx - ((*red)*Img_src.Width);
}

int getPixelIndex(struct Image Img_src, int *pixIdx, int red, int col)
{
	int pixelIndex = 0;

	*pixIdx = red * Img_src.Width + col;
	pixelIndex = *pixIdx;

	return pixelIndex;
}