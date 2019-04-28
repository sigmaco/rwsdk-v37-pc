--------------------------------------------------------------------------
              PVS Convert Tool for RenderWare Graphics
--------------------------------------------------------------------------

Introduction:

pvscnvrt can be run as a windows application, or from the command-line
with parameters.  (The windows applications offers more control over
processing operations.)

pvscnvrt overloads the texture read callback so texture information is
retained.  This means the textures for the bsp are not required when
the bsp is loaded.

Each .bsp file is overwritten, and suitable backups should therefore
be made.

Each file may have one of four operations performed on it.
The options available are:
	- convert old-style PVS data found
	- delete all existing (old or new) PVS data
	- enhance existing (new) PVS data with extra PVS data
	  (good for repairing poor results)
	- generate new PVS data

The last two options can have a density value supplied in the
range [0.01..1.0] that is the separation of sampling points -
this figure is the separation along the major axis and is defaulted
to 1.0.



Windows help:

* Click on "pvscnvrt.exe" to invoke the tool.

* A window is presented in which you can select one or more .bsp files to
process.

* A prompt is then presented to specify/change the operation performed on
the current .bsp file.
	- convert
	- delete
	- enhance
	- generate

* If more than one file is selected, there exists the
'run without prompting' check-box.  This runs the selected operation on
each file (if applicable) without prompting.

* A warning prompt is presented if the operations 'enhance' or
'generate' are likely to take a significant amount of time.  (This warning
is based on a complexity analysis, and test of processor speed - however,
it is ultimately dependent on the juxtaposition of the polygons and sectors
in the scene, and is thus only a rough estimate.)

* During generation or enhancement, the percentage completed and
estimated time remaining are presented.  The time remaining is
only a rough estimate.

* A final prompt is presented that informs of the success or failure of
the operation.



Command line help:

The syntax is pvscnvrt [filename [-x [val]]], where:
	* filename is the name of the file you want to process
	* x is one of:
		c - convert
		d - delete
		e - enhance
		g - generate
	* val is the sampling density and should be in the range [0.01, 1.0]

if val is omitted, the default of 1.0 is used.
if -x is also omitted, the default operation of generate is performed.
if filename is also omitted, the command invokes the windows version
of the tool.
Note, batch processing is not supported directly, but can be achieved using
FOR %i IN (*.bsp*) DO pvscnvrt %i [-x [val]].  It is recommended however, to use
the windows version since progress feedback is supported.

--------------------------------------------------------------------------
This program is copyright Criterion Software Limited 2001. 
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
