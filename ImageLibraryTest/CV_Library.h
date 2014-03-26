/***************************************************
*
*  This is the main header file for the CV Library
*  
*
*
*
****************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <conio.h>


/* DEBUG information - define if you  want to save debug information */
//#define DEBUG_LOG 

#ifdef DEBUG_LOG 
#define DEBUG_FILE "Debug_log.txt"
#endif // DEBUG_LOG



#define NOEDGE 255
#define POSSIBLE_EDGE 128
#define EDGE 0

/* Main structure in the program */
typedef struct Image
{
	int Width;                // Image Width
	int Height;               // Image Height
	unsigned char *rgbpix;    // Pointer to data of the image
	int Num_channels;         // Number of channels for image. 1 = Grayscale, 3 = Color
	char *Image_FileName;     // Image file name. Currently is not used
	int ColorSpace;           // Color space: 0 - Binary image, 1 -Grayscale, 2 - RGB, 3 - YCbCr, 4 - Lab, 5 - HSL
	int isLoaded;			  // This flag is raised if the image is succefully loaded
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



/* White Balance -> L*ab algorithm */
struct WhitePoint
{
	double X;
	double Y;
	double Z;

}WhitePoint;


struct ColorPoint_RGB
{
	int R;
	int G;
	int B;

}ColorPoint_RGB;

struct ColorPoint_Lab
{
	double L;
	double a;
	double b;

}ColorPoint_Lab;



struct ColorPoint_HSV
{
	int H;
	int S;
	int V;

}ColorPoint_HSV;

struct ColorPoint_YCbCr
{
	int Y;
	int Cb;
	int Cr;

}ColorPoint_YCbCr;

enum WhitePointType
{
	D50_5000K,
	D55_5500K,
	D65_6504K,
	INCADESCENT_2865K,
	NORTH_SKY_6774K,
	DAYLIGHT_7500K
};

enum ColorSpace
{
	COLORSPACE_BINARY,
	COLORSPACE_GRAYSCALE,
	COLORSPACE_RGB,
	COLORSPACE_YCbCr,
	COLORSPACE_LAB,
	COLORSPACE_HSV
};

enum AlgoType_Edges
{
	EDGES_Undefined,
	EDGES_CANNY,
	EDGES_SOBEL,
	EDGES_PREWITT
};

enum AlgoType_Brightness
{
	BRIGHTNESS_Undefined,
	BRIGHTNESS_PERCENTAGE_ALGO,
	BRIGHTNESS_EV_ALGO
};

enum AlgoType_Noise
{
	NOISE_Undefined,
	NOISE_GAUSSIAN
};

enum AlgoType_BLUR
{
	BLUR_Undefined,
	BLUR_AROUND_CENTER,
	BLUR_CENTER
};

enum AlgoType_WB
{
	WB_Undefined,
	WB_ALGO_1,
	WB_ALGO_2,
	WB_ALGO_3,
	WB_GREEN_WORLD
};

enum Write_Quality
{
	QUALITY_NONE,
	QUALITY_LOW_1, QUALITY_LOW_2, QUALITY_LOW_3, QUALITY_LOW_4, QUALITY_LOW_5, QUALITY_LOW_6, QUALITY_LOW_7, QUALITY_LOW_8, QUALITY_LOW_9,
	QUALITY_LOW_10, QUALITY_LOW_11, QUALITY_LOW_12, QUALITY_LOW_13, QUALITY_LOW_14, QUALITY_LOW_15, QUALITY_LOW_16, QUALITY_LOW_17, QUALITY_LOW_18, QUALITY_LOW_19,
	QUALITY_LOW_20, QUALITY_LOW_21, QUALITY_LOW_22, QUALITY_LOW_23, QUALITY_LOW_24, QUALITY_LOW_25, QUALITY_LOW_26, QUALITY_LOW_27, QUALITY_LOW_28, QUALITY_LOW_29,
	QUALITY_AVERAGE_30, QUALITY_AVERAGE_31, QUALITY_AVERAGE_32, QUALITY_AVERAGE_33, QUALITY_AVERAGE_34, QUALITY_AVERAGE_35, QUALITY_AVERAGE_36, QUALITY_AVERAGE_37, QUALITY_AVERAGE_38, QUALITY_AVERAGE_39,
	QUALITY_AVERAGE_40, QUALITY_AVERAGE_41, QUALITY_AVERAGE_42, QUALITY_AVERAGE_43, QUALITY_AVERAGE_44, QUALITY_AVERAGE_45, QUALITY_AVERAGE_46, QUALITY_AVERAGE_47, QUALITY_AVERAGE_48, QUALITY_AVERAGE_49,
	QUALITY_AVERAGE_50, QUALITY_AVERAGE_51, QUALITY_AVERAGE_52, QUALITY_AVERAGE_53, QUALITY_AVERAGE_54, QUALITY_AVERAGE_55, QUALITY_AVERAGE_56, QUALITY_AVERAGE_57, QUALITY_AVERAGE_58, QUALITY_AVERAGE_59,
	QUALITY_HIGH_60, QUALITY_HIGH_61, QUALITY_HIGH_62, QUALITY_HIGH_63, QUALITY_HIGH_64, QUALITY_HIGH_65, QUALITY_HIGH_66, QUALITY_HIGH_67, QUALITY_HIGH_68, QUALITY_HIGH_69,
	QUALITY_HIGH_70, QUALITY_HIGH_71, QUALITY_HIGH_72, QUALITY_HIGH_73, QUALITY_HIGH_74, QUALITY_HIGH_75, QUALITY_HIGH_76, QUALITY_HIGH_77, QUALITY_HIGH_78, QUALITY_HIGH_79,
	QUALITY_HIGH_80, QUALITY_HIGH_81, QUALITY_HIGH_82, QUALITY_HIGH_83, QUALITY_HIGH_84, QUALITY_HIGH_85, QUALITY_HIGH_86, QUALITY_HIGH_87, QUALITY_HIGH_88, QUALITY_HIGH_89,
	QUALITY_EXCELLENT_90, QUALITY_EXCELLENT_91, QUALITY_EXCELLENT_92, QUALITY_EXCELLENT_93, QUALITY_EXCELLENT_94, QUALITY_EXCELLENT_95, QUALITY_EXCELLENT_96, QUALITY_EXCELLENT_97, QUALITY_EXCELLENT_98, QUALITY_EXCELLENT_99,
	QUALITY_MAX

};


/* Function prototypes */

/* Creates new image, based on a prototype ( for width, height, Num of chanels and color space ) */
struct Image	 CreateNewImage_BasedOnPrototype(struct Image *Prototype, struct Image *Img_dst);
/* Creates new image. Specify width, height, number of channels and color space */
struct Image	 CreateNewImage(struct Image *Img_dst, int Width, int Height, int NumChannels, int ColorSpace);
/* Set destination Image parameters to match the ones in source image */
struct Image	 SetDestination(struct Image *Prototype, struct Image *Img_dst);
/* Release the memory allocated by calling CreateNewImage */
void			 DestroyImage(struct Image *Img);
/* Read Jpeg file and load it into memory */
struct Image	 ReadImage(char *filename);
struct Image	 read_Image_file(FILE *file);
/* Write Jpeg file. Parameter: quality (0, 100) */
GLOBAL(void)	 WriteImage(char *filename, struct Image, int quality);

struct Image	 BlurImageAroundPoint(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int BlurPixelRadius, int SizeOfBlur, int BlurOrSharp, int BlurAgression);
struct Image	 BlurImageGussian(struct Image *Img_src, struct Image *Img_dst, int BlurPixelRadius, double NeighborCoefficient);
struct Image	 BrightnessCorrection(struct Image *Img_src, struct Image *Img_dst, double Algo_paramBrightnessOrEV, int Algotype);
struct Image	 ContrastCorrection(struct Image *Img_src, struct Image *Img_dst, double percentage);
struct Image	 WhiteBalanceCorrectionRGB(struct Image *Img_src, struct Image *Img_dst, int Algotype);
struct Image	 WhiteBalanceCorrectionLAB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePointXYZ);
struct Image	 NoiseCorrection(struct Image *Img_src, struct Image *Img_dst, double threshold, int Algotype);
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
void			 ConvolutionBinary(unsigned char *InputArray, unsigned char *OutputArray, int rows, int cols, float *Kernel, int KernelSize, int DilateOrErode);
struct Image	 MirrorImageHorizontal(struct Image *Img_src, struct Image *Img_dst);
struct Image	 MirrorImageVertical(struct Image *Img_src, struct Image *Img_dst);
struct Image	 CropImage(struct Image *Img_src, struct Image *Img_dst, struct point_xy CentralPoint, int NewWidth, int NewHeight);
struct Image	 MorphDilate(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberofIterations);
struct Image	 MorphErode(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberofIterations);
struct Image	 MorphOpen(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberofIterations);
struct Image	 MorphClose(struct Image *Img_src, struct Image *Img_dst, int ElementSize, int NumberofIterations);
struct Image     SharpImageContours(struct Image *Img_src, struct Image *Img_dst , int Percentage);
struct Image     SharpImageBinary(struct Image *Img_src, struct Image *Img_dst, struct Image *Img_Binary , int Percentage);
struct Image     ColorFromGray(struct Image *Img_src, struct Image *Img_dst, struct ColorPoint_RGB ColorPoint);
struct Image	 ConvertToBinary(struct Image *Img_src, struct Image *Img_dst, /* 0 for automatic */ int Threshold);
/* Change image color space - RGB to HSL. Both Src and Dst have to be 3 channeled images.*/
void			 ConvertImage_RGB_to_HSL(struct Image *Img_src, struct Image *Img_dst);
/* Change image color space - HSL to RGB. Both Src and Dst have to be 3 channeled images.*/
void			 ConvertImage_HSL_to_RGB(struct Image *Img_src, struct Image *Img_dst);
/* Change image saturation by percentage. Params: 1: Input Image - HSL or RGB, 2: Output Image - HSL or RGB, Percentage to increase/decrease saturation (-100, 100) */
struct Image	 Saturation(struct Image *Img_src, struct Image *Img_dst, int percentage);
/* Change image color space - RGB to L*ab. Both Src and Dst have to be 3 channeled images.*/
void			 ConvertImage_RGB_to_LAB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ);
/* Change image color space - L*ab to RGB. Both Src and Dst have to be 3 channeled images.*/
void			 ConvertImage_LAB_to_RGB(struct Image *Img_src, struct Image *Img_dst, struct WhitePoint WhitePoint_XYZ);
void			 SetWhiteBalanceValues(struct WhitePoint *WhitePoint_lab, int TYPE);
