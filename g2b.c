#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#endif /* _WIN32 */

#include "gif_lib.h"
//#include "getarg.h"


#define required_bit_depth 4
#define MAX_FILENAME_LENGTH 100
#pragma pack(2) 

struct bmp_fileheader
{
    unsigned short    bfType;        
    unsigned int    bfSize;
    unsigned short    bfReverved1;
    unsigned short    bfReverved2;
    unsigned int    bfOffBits;
};

struct bmp_infoheader
{
    unsigned int    biSize;
    unsigned int    biWidth;
    unsigned int    biHeight;
    unsigned short    biPlanes;
    unsigned short    biBitCount;
    unsigned int	biCompression;
    unsigned int    biSizeImage;
    unsigned int    biXPelsPerMeter;
    unsigned int    biYpelsPerMeter;
    unsigned int    biClrUsed;
    unsigned int    biClrImportant;
};

GifFileType *GifFile;
GifRowType *ScreenBuffer;
GifRecordType RecordType;
//char *GifName="test.gif";
char GifName[MAX_FILENAME_LENGTH];
char OutFileName[MAX_FILENAME_LENGTH];
//char *RecordOutName="record.txt";
char RecordOutName[MAX_FILENAME_LENGTH];
int *Error;
int piclist;
int InterlacedOffset[4]={0,4,2,1};
int InterlacedJumps[4]={8,8,4,2};
GifByteType *extension;
FILE *RecordOutFile;
enum ERROR_CODE{PASS,PASS_NORECORD,ERROR_GIFREAD,ERROR_BMPWRITE,ERROR_IMGDESC,ERROR_EXTDESC,ERROR_COLORMAP,ERROR_IMGOUTLINE,ERROR_RECORDTYPE,ERROR_MEMORY,ERROR_GETLINE};

FILE* write_bmp_header(FILE *fpout)
{
    struct bmp_fileheader bfh;
    struct bmp_infoheader bih;

	unsigned int width;
	unsigned int height;
    unsigned short depth;
    unsigned long headersize;
    unsigned long filesize;

    depth=required_bit_depth;

	printf("writing bmp header...\n");

	width=GifFile->Image.Width;
	height=GifFile->Image.Height;
	headersize=14+40;
    filesize=headersize+width*height*depth;

    memset(&bfh,0,sizeof(struct bmp_fileheader));
    memset(&bih,0,sizeof(struct bmp_infoheader));
    
    //写入比较关键的几个bmp头参数
    bfh.bfType=0x4D42;
    bfh.bfSize=filesize;
    bfh.bfOffBits=headersize;

    bih.biSize=40;
    bih.biWidth=width;
    bih.biHeight=-height;
    bih.biPlanes=1;
    bih.biBitCount=(unsigned short)depth*8;
    bih.biSizeImage=width*height*depth;

    printf("filesize:%ld\n",filesize);
    printf("headersize:%ld\n",headersize);
    printf("width:%d\n",width);
    printf("height:%d\n",height);
    printf("depth:%d\n",depth);

    fwrite(&bfh,sizeof(struct bmp_fileheader),1,fpout);
    fwrite(&bih,sizeof(struct bmp_infoheader),1,fpout);

	printf("write bmp header over\n");
	return fpout;
}

/*
void GIF_EXIT(char *info)
{
	printf("%s\n",info);
	exit(EXIT_FAILURE);
}

void Gif_info_print(char *fname)
{
	if(GifFile==NULL)
	{
		printf("Gif:%s hasn't been read\n",fname);return;
	}
	printf("Screen:width=%d,height=%d\n",GifFile->SWidth,GifFile->SHeight);
	printf("ColorResolution=%d,BackgroundColor=%d\n",GifFile->SColorResolution,GifFile->SBackGroundColor);
	printf("ImageNum:%d\n",GifFile->ImageCount);
}
*/
int Gif_read()
{
	if ((GifFile = DGifOpenFileName(GifName,Error)) == NULL) 
	{  
		return ERROR_GIFREAD;
   	}
	//open gif

	if ((ScreenBuffer = (GifRowType *)  
            malloc(GifFile->SHeight * sizeof(GifRowType *))) == NULL)
	{
		return ERROR_MEMORY;
	}   
	//apply ScreenBuffer

	int Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/  
   	if ((ScreenBuffer[0] = (GifRowType) malloc(Size)) == NULL) /* First row. */  
		return ERROR_MEMORY;
	int i;
	for (i = 0; i < GifFile->SWidth; i++)  /* Set its color to BackGround. */  
		ScreenBuffer[0][i] = GifFile->SBackGroundColor;  
	for (i = 1; i < GifFile->SHeight; i++) 
	{  
       /* Allocate the other rows, and set their color to background too: */  
		if ((ScreenBuffer[i] = (GifRowType) malloc(Size)) == NULL)  
			return ERROR_MEMORY;  
     	memcpy(ScreenBuffer[i], ScreenBuffer[0], Size);  
	}
	//paint Screen in background color

	return PASS;
}


char* itoa(int value, char* result, int base) {  
        // check that the base if valid  
        if (base < 2 || base > 36) { *result = '\0'; return result; }  
  
        char* ptr = result, *ptr1 = result, tmp_char;  
        int tmp_value;  
  
        do {  
            tmp_value = value;  
            value /= base;  
            *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];  
        } while ( value );  
  
        // Apply negative sign  
        if (tmp_value < 0) *ptr++ = '-';  
        *ptr-- = '\0';  
        while(ptr1 < ptr) {  
            tmp_char = *ptr;  
            *ptr--= *ptr1;  
            *ptr1++ = tmp_char;  
        }  
        return result;  
    }

int DumpScreen2RGBA(char *foutName, GifRowType *ScreenBuffer,ColorMapObject *ColorMap, int ScreenWidth, int ScreenHeight)  
{  
	int i, j;  
	GifRowType GifRow;  
	static GifColorType *ColorMapEntry;  
	unsigned char *BufferP;
	FILE *fpout=NULL;

	//char fout[MAX_FILENAME_LENGTH];
	char listnum[MAX_FILENAME_LENGTH];
	char *after_format=".bmp";

	//for(i=0;i<MAX_FILENAME_LENGTH;i++)	fout[i]=foutName[i];
	//printf("fout=%s,foutName=%s\n",fout,foutName);

	itoa(piclist++,listnum,10);
	strcat(foutName,listnum);
	strcat(foutName,after_format);
	
	//printf("outname=%s\n",foutname);

	/*
	if((fpout=fopen("rgb.bmp","wb"))==NULL)
	{
		GIF_EXIT("cannot open rgb");
	}
	*/
	///*
	if((fpout=fopen(foutName,"wb"))==NULL)
	{
		return ERROR_BMPWRITE;
	}
	//*/
	fpout=write_bmp_header(fpout);
       
	for (i = 0; i < ScreenHeight; i++) 
	{  
		GifRow = ScreenBuffer[i];  
		//BufferP = grb_buffer + i * (ScreenWidth * 4);  
		for (j = 0; j < ScreenWidth; j++)
		{  
			ColorMapEntry = &(ColorMap->Colors[GifRow[j]]);
			/* 
			*BufferP++ = ColorMapEntry->Blue;  
			*BufferP++ = ColorMapEntry->Green;  
			*BufferP++ = ColorMapEntry->Red;  
			*BufferP++ = 0xff;
			*/
			*BufferP=0xff;
			fwrite(&ColorMapEntry->Blue,sizeof(unsigned char),1,fpout);
			fwrite(&ColorMapEntry->Green,sizeof(unsigned char),1,fpout);
			fwrite(&ColorMapEntry->Red,sizeof(unsigned char),1,fpout);
			fwrite(BufferP,sizeof(unsigned char),1,fpout);
		}  
	}
	printf("write rgb over\n\n");
	return PASS;
}

int Gif_decode_imgdesc()
{
	//printf("Pic.%d RecordType:IMAGE_DESC_RECORD_TYPE\n",GifFile->ImageCount);

	if (DGifGetImageDesc(GifFile) == GIF_ERROR) 
	{  
		return ERROR_IMGDESC;
	}  
	int Row = GifFile->Image.Top; /* Image Position relative to Screen. */  
	int Col = GifFile->Image.Left;  
	int Width = GifFile->Image.Width;  
	int Height = GifFile->Image.Height;

	//printf("Image %d at (%d, %d) [%d,%d]:     \n",GifFile->ImageCount,Col,Row,Width,Height);
	if(RecordOutFile!=NULL)	fprintf(RecordOutFile,"<I><%d(%d,%d)[%d,%d]>",piclist,Col,Row,Width,Height);


	if(GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||  
		GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) 
	{  
		return ERROR_IMGOUTLINE;
	}
	
	int Count,i,j;
	if(GifFile->Image.Interlace) 
	{
		//Need to perform 4 passes on the images:
		for(Count = i = 0; i < 4; i++)  
			for (j = Row + InterlacedOffset[i]; j < Row + Height;j += InterlacedJumps[i]) 
			{  
				printf("\b\b\b\b%-4d", Count++);  
				if (DGifGetLine(GifFile, &ScreenBuffer[j][Col], Width) == GIF_ERROR) 
				{  
					return ERROR_GETLINE;
				}  
			}  
	}  
	else 
	{  
		for (i = 0; i < Height; i++) 
		{  
			printf("\b\b\b\b%-4d",i);  
			if (DGifGetLine(GifFile, &ScreenBuffer[Row++][Col],Width) == GIF_ERROR)
			{  
				return ERROR_GETLINE;
			}  
		}  
	}
	
     
  /* Get the color map */  
	ColorMapObject *ColorMap = (GifFile->Image.ColorMap?GifFile->Image.ColorMap:GifFile->SColorMap);
  
	if(ColorMap == NULL)
	{  
		return ERROR_COLORMAP;
	}

	//printf("gif decode over\n");
	int status=PASS;
	char outname[MAX_FILENAME_LENGTH];
	strcpy(outname,OutFileName);
	
	//if((status=DumpScreen2RGBA(OutFileName, ScreenBuffer,ColorMap, GifFile->SWidth, GifFile->SHeight))!=PASS)	return status;
	if((status=DumpScreen2RGBA(outname, ScreenBuffer,ColorMap, GifFile->SWidth, GifFile->SHeight))!=PASS)	return status;
}

int Gif_decode_extension()
{
	int EXT_CODE,i;
	int *mover=NULL;
	char disposal;
	char uif;
	char tcf;
	char delay_time;
	char tch;
	
	printf("Pic.%d RecordType:EXTENSION_RECORD_TYPE\n",GifFile->ImageCount);

	extension=malloc(sizeof(GifByteType));
	if (DGifGetExtension(GifFile, &EXT_CODE, &extension) == GIF_ERROR) 
	{
		    return ERROR_EXTDESC;
	}
	
	if(extension==NULL)
	{	
		printf("ext:NULL\n");
		return	ERROR_EXTDESC;
	}
	else
	{
		printf("\nexcode:%x\n",EXT_CODE);
		if(EXT_CODE==0xf9)
		{
			disposal=*(extension+1)>>2;
			uif=(*(extension+1)&2)>>1;
			tcf=*(extension+1)&1;
			delay_time=*(extension+2);
			tch=*(extension+3);
			
			printf("ext:");
			printf("size:%x\n",*extension);
			printf("disposal:%x\n",disposal);
			printf("use input flag:%x\n",uif);
			printf("transparent color flag:%x\n",tcf);
			printf("delay time:%x\n",delay_time);
			printf("transparent color index:%x\n",tch);
			//mover=(int *)extension;
			//for(i=0;i<10;i++)	printf("%x ",*(extension+i));
			//for(i=1;i<=20;i++)	printf("%x ",*(extension+i));
			if(RecordOutFile!=NULL)	fprintf(RecordOutFile,"<E><%x,%x,%x,%x,%x>",disposal,uif,tcf,delay_time,tch);
		}
	}
	

	while (extension != NULL) 
	{
		    if (DGifGetExtensionNext(GifFile, &extension) == GIF_ERROR) 
			{
				return ERROR_EXTDESC;
		    }
			
			if(extension==NULL)	printf("\n");
			else
			{
				if(EXT_CODE==0xf9)
				{
					printf("ext:%x ",*extension);
				}
			}
	}
	free(extension);
	return PASS;
}

int Gif_decode_main()
{
	int status=PASS;
	do 
	{  
	    if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) 
		{  
	        return ERROR_RECORDTYPE;
	    }  
      
	    switch (RecordType) 
		{  
			case IMAGE_DESC_RECORD_TYPE:
				status=Gif_decode_imgdesc();
				break;  
			case EXTENSION_RECORD_TYPE:
				//Gif_decode_imgdesc();
				status=Gif_decode_extension();
				break;  
			case TERMINATE_RECORD_TYPE:
				return status;
				break;  
			default:            /* Should be traps by DGifGetRecordType. */  
				break;  
		}
		if(status!=PASS)	return status;
	}
	while (RecordType != TERMINATE_RECORD_TYPE);
}

int g2b(char *gn,char *bn,char *rn)
{
	piclist=1;
	int status=PASS;

	strcpy(GifName,gn);
	strcpy(RecordOutName,rn);
	strcpy(OutFileName,bn);

	RecordOutFile=fopen(RecordOutName,"wb");

	if((status=Gif_read())!=PASS)	return status;
	if((status=Gif_decode_main())!=PASS)	return status;
	if(RecordOutFile==NULL)	return PASS_NORECORD;
	
	free(ScreenBuffer);

	return status;
}

int main()
{
	char *gn="test.gif";
	char *rn="record.txt";
	char *bn="bmpout";
	return g2b(gn,bn,rn);
}




