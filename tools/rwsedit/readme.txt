--------------------------------------------------------------------------
                  Stream Editor for RenderWare Graphics
--------------------------------------------------------------------------

rwstool
rwsedit is the RenderWare Stream editing tool.

This tool is used to edit a stream which may or may not contain labels.
It provides a small set of options to move, copy and delete data chunks.
Labels can be created, moved, copied, renamed or deleted.

A label is used to group several chunks under a single name purely for
reference and identification.

It is analogous to a folder to group similar files together. Each label
must have a name for identification purposes. Labels may contain sub labels,
allowing data chunks to be arranged in a hierarchy within the stream.

Each label is stored as two seperate chunks, a start marker and an end marker.
These are identified as rwID_CHUNKGROUPSTART and rwID_CHUNKGROUPEND
respectively. The name of the label is stored as a string with the start
marker. The end marker does not contain any data. Data chunks which are
part of the label are stored in between the two markers.

The tool's options are

Create : -mkdir <src>

Creates a label. A new empty label is created. Sub labels are created
if required.

Move : -mv <src> <dest>

Moves a label or chunk to a new location. If the last item in the path
is a number, rwsedit assumes it is the n'th item inside the parent label. 
Otherwise, rwsedit assumes it is a label. If the source is a label, all 
its contents are also moved.
This function does not create a new label so the destination must already
exist. The source cannot be moved into its own sub label.

Copy : -cp <src> <dest>

Copies a label or chunk to a new location. If the last item is a number, rwsedit 
assumes it is the n'th item inside the parent label. Otherwise, rwsedit assumes 
it is a label. If the source is a label, all its contents are also copied.
The function does not create a new label so the destination must already
exist.

Rename : -rn <src> <newname>

Renames a label with a new name. The source must be a label and not a chunk.
An error will occur if the name already exists.

Remove : -rm <src>

Deletes a label or a chunk. If the last item is a number, rwsedit assumes it is
the n'th item inside the parent label. Otherwise, rwsedit assumes it is a label. If
the source is a label, all its contents are also deleted.

List : -l <depth>

Lists the contents of the file. The listing is directed to the report callback.
The list can be rescursive. The level of recursion is set by the depth
parameter.

Path : -p <src>

Sets the base to the current path, otherwise the root will be used. All
relative paths will be referenced from this path as the base. The path
must be a label and not a data chunk.

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
--------------------------------------------------------------------------
Thu Feb 12 12:56:16 2004 -- build main eval ( 148160 )
