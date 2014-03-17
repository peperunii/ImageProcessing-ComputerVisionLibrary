#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <conio.h>

typedef struct Image
{
	int Width;
	int Height;
	unsigned char *rgbpix;
	int Num_channels;
	char *Image_FileName;
	int ColorSpace;
	int isLoaded;
}Image;

struct point_xy
{
	double X;
	double Y;
}point;


struct Image read_Image_file(FILE *file);
GLOBAL(void) WriteImage(char *filename, struct Image, int quality);
struct Image ReadImage(char *filename);
struct Image BlurImage(struct Image Img_src, struct point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression);
struct Image CreateNewImage(struct Image Prototype);
struct Image RotateImage(struct Image Img_src, double RotationAngle, struct point_xy CentralPoint);
struct Image BrightnessCorrection(struct Image Img_src, double percentage);
struct Image ContrastCorrection(struct Image Img_src, double percentage);
struct Image WhiteBalanceCorrection(struct Image Img_src, double percentage, int Algotype);
struct Image NoiseCorrection(struct Image Img_src, double percentage, int Algotype);
struct Image GammaCorrection(struct Image Img_src, double RedGamma, double GreenGamma, double BlueGamma);
void getPositionFromIndex(struct Image Img_src, int pixIdx, int *red, int *col);
int getPixelIndex(struct Image Img_src, int *pixIdx, int red, int col);
