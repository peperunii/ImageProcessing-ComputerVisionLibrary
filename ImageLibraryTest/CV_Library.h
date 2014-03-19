#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <conio.h>

#define NOEDGE 255
#define POSSIBLE_EDGE 128
#define EDGE 0

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

struct ArrPoints
{
	struct point_xy *ArrayOfPoints;
}ArrPoints;

struct Image	 CreateNewImage(struct Image *Prototype, struct Image *Img_dst, int NumChannels);
void			 DestroyImage(struct Image *Img);
struct Image	 ReadImage(char *filename);
struct Image	 read_Image_file(FILE *file);
GLOBAL(void)	 WriteImage(char *filename, struct Image, int quality);
struct Image	 BlurImageAroundPoint(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression);
struct Image	 BlurImageGussian(struct Image *Img_src, struct Image *Img_dst, int BlurPixelRadius, double NeighborCoefficient);
struct Image	 BrightnessCorrection(struct Image *Img_src, struct Image *Img_dst, double percentage);
struct Image	 ContrastCorrection(struct Image *Img_src, struct Image *Img_dst, double percentage);
struct Image	 WhiteBalanceCorrection(struct Image *Img_src, struct Image *Img_dst, int Algotype);
struct Image	 NoiseCorrection(struct Image *Img_src, struct Image *Img_dst, double percentage, int Algotype);
struct Image	 GammaCorrection(struct Image *Img_src, struct Image *Img_dst, double RedGamma, double GreenGamma, double BlueGamma);
void			 getPositionFromIndex(struct Image *Img_src, int pixIdx, int *red, int *col);
int				 getPixelIndex(struct Image *Img_src, int *pixIdx, int red, int col);
struct Image	 ConvertToGrayscale_3Channels(struct Image *Img_src, struct Image *Img_dst);
struct Image	 ConvertToGrayscale_1Channel(struct Image *Img_src, struct Image *Img_dst);
struct Image	 ScaleImage(struct Image *Img_src, struct Image *Img_dst, double ScalePercentage);
struct Image	 TranslateImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy ToPoint);
struct Image	 RotateImage(struct Image *Img_src, struct Image *Img_dst, double RotationAngle, struct point_xy CentralPoint);
struct ArrPoints EdgeExtraction(struct Image *Img_src, struct Image *Img_dst, int Algotype, float Algo_param1, float Algo_param2);
void			 FindDerrivative_XY(struct Image *Img_src, struct Image *DerrivativeX_image, struct Image *DerrivativeY_image);
void			 FindMagnitudeOfGradient(struct Image *DerrivativeX_image, struct Image *DerrivativeY_image, struct Image *Magnitude);
void			 FindNonMaximumSupp(struct Image *Magnitude, struct Image *DerrivativeX, struct Image *DerrivativeY, struct Image *NMS);
void			 FindHysteresis(struct Image *Magnitude, struct Image *NMS, struct Image *Img_dst, float Algo_param1, float Algo_param2);
void			 Follow_edges(unsigned char *edgemapptr, unsigned char *edgemagptr, unsigned char lowval, int cols);
void			 Convolution(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize);