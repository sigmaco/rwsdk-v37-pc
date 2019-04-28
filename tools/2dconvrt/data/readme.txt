==================================
NOTES FOR CONVERTING EXAMPLE .SWFs
==================================

Conversion
----------
The command
   ..\2dconvrt *.swf
will convert each of the .swf files in this directory to .anm files.

The font alias file in this directory is used to make the converter look for
fonts in ..\fonts.

Playback
--------
Use the 2dviewer tool to play back the new .anm files.

Make sure that the viewer is run from the viewers\2dviewer. The viewer expects
to find a 'font' directory under the directory from which it is run.

Do this by either (a) running it from Windows Explorer or (b) changing directory
to viewers\2dviewer, then running the executable.

The sample .swf files
   static05.swf
   static07.swf
   mainmenu.swf
use fonts.

