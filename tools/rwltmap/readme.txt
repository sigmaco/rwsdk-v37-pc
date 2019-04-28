--------------------------------------------------------------------------
                 Lightmap Generator for RenderWare Graphics
--------------------------------------------------------------------------

rwstool
rwltmap is use to generate lightmaps for RenderWare models. The models must
have been exported with the necessary additional data needed to generate
lightmaps. For more information, please see the exporter reference guide.

This tool is similar to the lightmap example found in the examples directory
except it can be used in a batch process. The lightmaps created are placed in
a texture dictionary. This can be the input dictionary, if specified, or a
new texture dictionary can be created. Because lightmaps can be
used on different target platforms, all textures are converted and stored
in a platform-independant format.

The tool's options are

Generate darkmap : -dm

Post processes the lightmap textures into 'darkmap'. Darkmaps are used for
rendering lightmaps on the PlayStation 2.
See RtLtMapSkyLightMapMakeDarkMap for more information.

Generate luminance : -lum <index>.

Post processes the base textures to calculate a 'luminance' value for each
texel. The luminance is used with darkmaps for rendering lightmaps on
the PlayStation 2. The index refers to which method is used.
0 : None.
1 : Select RtLtMapSkyLumCalcMaxCallBack.
2 : Select RtLtMapSkyLumCalcSigmaCallBack.
See RtLtMapSkyLightingSessionBaseTexturesProcess for more information.

Lightmap objects : -obj <filename>

Adds the objects in the given file to the scene to be lightmapped. The file
can be a texture dictionary or a clump. More than one object file can be
specified, each prefixed by the -obj option.

Occluder objects : -occ <filename>

Adds the objects in the given file to the scene to cast shadows. The file
must contain valid objects, a clump. The objects will act as occluders
but will not be lightmapped.  More than one occluder file can be specified,
each prefixed by the -occ option.

Vertex lit object : -vlt <filename>

Adds the objects in the given file to the scene to be vertex lit. The file 
must contain valid objects, a clump. The object will be vertex lit but not
lightmapped. More than one vertex lit file can be specified, each prefixed
by the -vlt option.

Enable area light : -al

Enables area light source in the scene.

Area light density : -ald <real>

Sets the area light sampling density. This option is only applicable if
area light is enabled.
See RtLtMapAreaLightGroupCreate for more information.

Area light density modifier : -alm <real>

Sets the area light density modifier. This option is only applicable if
area light is enabled.
See RtLtMapSetAreaLightDensityModifier for more information.

Area light radius modifier : -alr <real>

Sets the light radius modifier for the area light. This option is only
applicable if area light is enabled.
See RtLtMapSetAreaLightRadiusModifier for more information.

Area light cutoff : -alc <real>

Sets the error cutoff for the area light. This option is only applicable
if area light is enabled.
See RtLtMapSetAreaLightErrorCutoff for more information.

Vertex Weld Threshold : -vtw <real>

Sets the vertex weld threshold during lightmap creation.
See RtLtMapSetVertexWeldThreshold for more information.

Sliver Threshold : -slv <real>

Sets the sliver area threshold during lightmap creation.
See RtLtMapSetSliverAreaThreshold for more information.

Lightmap density : -d <int>

Sets the lightmap density used during lightmap creation. All light objects
will use this value when generating lightmap uv co-ordinates.
See RtLtMapLightMapsCreate for more information.

Lightmap size : -s <int>

Sets the size of the lightmaps textures. All lightmaps will be created with
this size.
See RtLtMapLightMapSetDefaultSize for more information.

Lightmap supersample : -ss <int>

Sets the supersample ratio during lightmap illumination.
See RtLtMapIlluminate for more information.

Lightmap format : -f <bitfield>

Sets the texture format for the lightmap textures. Depending on the
target platform, different texture formats are required. The texture format
is represented as a hexadecimal number storing the individual format flags.
See RpLtMapSetRasterFormat for information.


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
