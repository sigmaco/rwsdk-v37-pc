--------------------------------------------------------------------------
                 Stream Splitting Tool for RenderWare Graphics
--------------------------------------------------------------------------

rwstool
rwsplit is used to split up a single RenderWare binary stream into
seperate components.

Each data chunk within the stream is written out into its own individual
file. The output names for the various types of data chunks are as follows:

Clump : <src>_clump<num>.rws
Atomic : <src>_atomic<num>.rws
World : <src>_world<num>.rws
Texture : <src>_text<num>.rws
Native texture : <src>_textnative<num>.rws
Hierarchical Animation : <src>_hanim<num>.rws
Texture Dictionary : <src>_txd<num>.rws
PI Texture Dictionary : <src>_pitxd<num>.rws
DMoprh Animation : <src>_dmorph<num>.rws

--------------------------------------------------------------------------
This program is copyright Criterion Software Limited 2003. 
Criterion Software grants you a license to use it only in the form as 
supplied. You may not disassemble, decompile or reverse engineer this 
program.

This program is provided as is with no warranties of any kind. Criterion
Software will not, under any circumstances, be liable for any lost revenue
or other damages arising from the use of this program.

RenderWare is a registered trademark of Canon Inc.
Other trademarks acknowledged.
--------------------------------------------------------------------------Thu Feb 12 12:56:16 2004 -- build main eval ( 148160 )
