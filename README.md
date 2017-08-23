FUNCTION:GIF2BMP
SOURCE CODE:g2b_1.c
LIBRARY USE:GIFLIB
RUNNING:g2b_1

///////////////////////////////////////////////////////////////////////////////////////////////
0.api:int g2b(char *gif,char *bmp,char *record)
input parameter:
gif:gif file name.eg:*gif="test.gif"
bmp:output bmp file name.eg:if want to output as[testbmp1.bmp,testbmp2.bmp,testbmp3.bmp......],set *bmp="testbmp"
record:record file name.eg:*record="record.txt"


return value:

return pass/error code,refer to No.9 ERROR_CODE

///////////////////////////////////////////////////////////////////////////////////////////////


1.function:decode gif image(test.gif),get graphic and extension block info and output.
2.input:test.gif(url:test.gif)
3.output:
3.1.pics:1.bmp,2.bmp....n.bmp(url:n.bmp)
3.2.extension:record.txt
4.routine:
while(block is not a teminate block)
do
	if(block is a graphic block)
	do
		block => n.bmp(n:num of pics meet now)
		"<I><n(left_space,upper_space)[length,height]>"	=> record.txt
	end
	if(block is a extension block)
	do
		if(block is not a graphicm extension block)	(do not deal it)continue;
		"<E><disposal_method,user_input_flag,transparent_color_flag,delay_time(10ns),transparent_color_index>"	=> record.txt
	end
end

5.disposal_method:
0:do not use
1:remove now image
2:return to backgroundcolor
3:return to last status

6.flags:
user_input_flag:1/0-need/not need a user input to continue
transparent_color_flag:1/0-use/not use transparent color

7.functions:(if not in list,means it is a function from GIFLIB Library)
FILE* write_bmp_header(FILE *fpout)					: write bmp header,should used after Gif_read()
void GIF_EXIT(char *info)							: exit with printing info,instead EXIT_FAILURE
void Gif_info_print(char *fname)					: print GIF(filename:fname) info,include Screen_width/height,ColorResolution,BackgroundColor,number of the Image dealing now
void Gif_read()										: read GIF with name(GifName),open a new Gif File(GifFile)
char* itoa(int value, char* result, int base)		: normal itoa,incase <stdlib.h> do not support itoa()
void DumpScreen2RGBA
(char *foutName, GifRowType *ScreenBuffer,
ColorMapObject *ColorMap, int ScreenWidth, 			: dump data in screenbuffer(ScreenBuffer) to bmpfile(foutName),using colormap(ColorMap),size of screen
int ScreenHeight)									  (ScreenWidth,ScreenHeight).must be used after Gif_decode_imgdesc()

void Gif_decode_main()								: decord gifblock until meet teminate block,should used after GIF_read(),using Gif_decode_imgdesc(),Gif_decode_extension()
void Gif_decode_imgdesc()							: decord image block,get block data to buffer,use DumpScreen2RGBA() to generate bmp
void Gif_decode_extension()							: decord graphic extension block,get block data to record.txt

8.static vars:
GifFileType *GifFile;								:Gif File opened,used in GIFLIB
GifRowType *ScreenBuffer;							:Gif buffer,used in DumpScreen2RGB(),save image data
char GifName[MAX_FILENAME_LENGTH];					:gif file name	
char OutFileName[MAX_FILENAME_LENGTH];				:output bmp file name
char RecordOutName[MAX_FILENAME_LENGTH];			:record file name
int *Error;											:Error,used in GIFLIB
int piclist;										:number of now picture 
int InterlacedOffset[4]={0,4,2,1};					:used for decord gif,see gif introduction for more
int InterlacedJumps[4]={8,8,4,2};					:used for decord gif,see gif introduction for more
GifByteType *extension;								:pointer of extension block
FILE *RecordOutFile;								:pointer for File record

9.Error code
enum ERROR_CODE{PASS,PASS_NORECORD,ERROR_GIFREAD,ERROR_BMPWRITE,ERROR_IMGDESC,ERROR_EXTDESC,ERROR_COLORMAP,ERROR_IMGOUTLINE,ERROR_RECORDTYPE,ERROR_MEMORY,ERROR_GETLINE};
PASS:							pass,no error or warning occured
PASS_NORECORD:					pass,cannot open record file,meaning cannot save extension block data
ERROR_GIFREAD:					error,cannot open or read GIF file
ERROR_BMPWRITE:					error,cannot open or write BMP file
ERROR_IMGDESC:					error,cannot read image desc block with GIFLIB
ERROR_EXTDESC:					error,cannot read extension block with GIFLIB
ERROR_COLORMAP:					error,cannot read colormap with GIFLIB
ERROR_IMGOUTLINE:				error,image is out of border,gif file may be damaged
ERROR_RECORDTYPE:				error,unexpected record type occurred,gif file may be damaged
ERROR_MEMORY:					error,no enough memory
ERROR_GETLINE:					error,cannot get image data with GFIFLIB
