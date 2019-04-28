----------------------------------------------------------------------------

           RenderWare Graphics Viewer Program: Stream Viewer

----------------------------------------------------------------------------

STREAM VIEWER

This Windows program may be used to obtain a simple, yet detailed, 
visualization of the contents of a RenderWare binary stream file, including 
the sizes of individual components and the hierarchical structure of the 
stream.

RenderWare Graphics stream files are in the following formats:
* DFF - contains one or more clumps
* BSP - contains a static world
* RWS - can contain clumps, static worlds, hierarchical and delta morphing
  animation and spline data

To view one of these files simply drop the file onto the viewer icon or open 
the application and load from the 'File' menu. You may also wish to create a 
file association between DFF and BSP files and this application. 

Further Information:
* User Guide Volume I discusses streams in the Serialization chapter.
* API Reference has more information about streaming, clumps, materials,
  textures, hierarchical animation and delta morphing animation.
* Exporting BSP, DFF and RWS files using 3ds max and Maya is discussed in 
  the exporter artist guides.

----------------------------------------------------------------------------

VIEWING DFF FILES

Clumps contain a frame list, atomics, geometries, material lists, materials 
and textures. The size of each component is given in bytes, which includes 
the sizes of all sub-components. For a geometry the number of triangles, 
vertices and keyframes is also indicated. Materials are identified with 
their RGBA color values. Textures contain two strings which identify the 
texture's name and the texture's mask name, if any. You will also notice 
that on most components an extension is given, which refers to the size of 
all plugin data associated with the type of the component.

----------------------------------------------------------------------------

VIEWING BSP FILES

Static worlds contain a material list, materials, textures, boundary 
sectors and leaf sectors (world sectors). The size of each component is 
given in bytes, which includes the sizes of all sub-components. The world 
is identified with the total number of triangles, vertices and world 
sectors. Materials are indicated with their RGBA colors and textures with
their name and mask name, is any. Again, extension data may also exist 
and refers to any, and all, plugin data associated with the type.

----------------------------------------------------------------------------

VIEWING RWS FILES

RWS files contain all information in BSP and DFF files as stated above. 
In addition they also contain spline, hierarchical animation and delta 
morph animation data.

The size of each component is given in bytes, This includes the size
of all sub-components. Materials are indicated with their RGBA colors and
textures with their name and mask name, if any. 

----------------------------------------------------------------------------
This software is copyright Criterion Software Limited 2001.
Criterion Software grants you a license to use it only in the form as 
supplied. You may not disassemble, decompile or reverse engineer this 
software.

This software is provided as is with no warranties of any kind. Criterion 
Software will not, under any circumstances, be liable for any lost revenue 
or other damages arising from the use of this software.

RenderWare is a registered trademark of Canon Inc.
Other trademarks acknowledged.

----------------------------------------------------------------------------
