--------------------------------------------------------------------------
                  MakeFont for RenderWare Graphics
--------------------------------------------------------------------------

PURPOSE

This Microsoft Windows console program can be used to create METRIC1 BMP
fonts from True Type fonts, for use with the Rt2d toolkit.  The program
creates the BMP file and the MET file for the font.

The typeface name of the font is passed on the command line as well as any
options for creating the BMP, the resulting BMP and MET files are created
with the name of the font and it's point size e.g. "courier10.bmp" and
"courier10.met"

This program can only create BMP fonts for the fonts that are on your
system, if a font is not available a substitute font will be selected.

For example to create the BMP and MET file for a Times New Roman font with
a point size of 10, on the command line do:

     makefont -p10 "times new roman"
--------------------------------------------------------------------------

COMMAND LINE OPTIONS

-s          - Supersample the font.
-w          - The width in pixels of the generated BMP file.
-h          - The height in pixels of the generated BMP file.
-p          - The point size of the font.
-r          - The right margin spacing between glyph cells.
-b          - The bottom margin spacing between glyph cells.
-f          - The ASCII code to start at, min = 32 max =255. 
              Characters prior to this map to ' '
-e          - The ASCII code to end at, min = 32 max =255.

If any options are not entered the defaults will be used:

-s          - 1.
-w          - 128.
-h          - 128.
-p          - 9.
-r          - 1.
-b          - 1.
-f          - 32.
-e          - 127.

--------------------------------------------------------------------------
This program is copyright Criterion Software Limited 2002. 
Criterion Software grants you a license to use it only in the form as 
supplied. You may not disassemble, decompile or reverse engineer this 
program.

This program is provided as is with no warranties of any kind. Criterion
Software will not, under any circumstances, be liable for any lost revenue
or other damages arising from the use of this program.

RenderWare is a registered trademark of Canon Inc.
Other trademarks acknowledged.
--------------------------------------------------------------------------
Thu Feb 12 12:56:16 2004 -- build main eval ( 148160 )
