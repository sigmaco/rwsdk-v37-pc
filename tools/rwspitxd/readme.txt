--------------------------------------------------------------------------
              Dictionary Editing Tool for RenderWare Graphics
--------------------------------------------------------------------------

rwstool
rwspitxd is use to edit a PI texture dictionary in a RenderWare
binary stream. The texture dictionary is alted in place and the input
stream will contain the modified texture at tool termination.

If more than one PI texture dictionary exists, the first one will be
used.

The tool's options are

Default U Texture co-ordinate addressing mode : -ua <u addressing mode>

Sets the default texture's U co-ordinates addressing mode for any new
textures added to the dictionary.

 Default V Texture co-ordinate addressing mode : -va <v addressing mode>

Sets the default texture's V co-ordinates addressing mode for any new
textures added to the dictionary.

Default texture filter mode : -fm <filter mode>

Sets the default texture filter mode for any new textures added to the
dictionary.

Add a texture : -a <image name>

Adds a new texture to the dictionary. The texture will be set to use the
default UV addressing and filter mode. Only PNG and BMP images can be
loaded. An error will occur if a texture of the same name already
exists. More than one texture can be extracted, each preceded by the -a option.

Remove a texture : -rm <texture name>

Removes a texture from a PI dictionary. More than one texture can be extracted,
each preceded by the -rm option.

Replace a texture : -rp <texture name> <image name>

Replaces a texture with a new image. The texture's other properties are
unchanged.

Rename a texture : -rn <old texture name> <new texture name>

The specified texture's name is replaced with the new given name.

Extract a texture : -e <texture name>

Extracts a texture from the dicionary and writes it out into a PNG image
file. More than one texture can be extracted, each preceded by the -e option.

Extract all textures : -ea

Extracts all textures in the dictionary. This overrides the -e option.

List the textures in the dictionary : -l

Lists all the textures in the dictionary by their names.

Merge a PI texture dictioanry : -m <src>

Merges a PI texture dictionary. The textures in the PI texture dictionary
from the specified file are merged with the current PI texture dictionary.

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
