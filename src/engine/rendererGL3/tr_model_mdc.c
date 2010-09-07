/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_models.c -- model loading and caching
#include "tr_local.h"

#define	LL(x) x=LittleLong(x)
#define	LF(x) x=LittleFloat(x)

#define NUMMDCVERTEXNORMALS  256

// *INDENT-OFF*
static float    r_anormals[NUMMDCVERTEXNORMALS][3] = {
{1.000000, 0.000000, 0.000000},
{0.980785, 0.195090, 0.000000},
{0.923880, 0.382683, 0.000000},
{0.831470, 0.555570, 0.000000},
{0.707107, 0.707107, 0.000000},
{0.555570, 0.831470, 0.000000},
{0.382683, 0.923880, 0.000000},
{0.195090, 0.980785, 0.000000},
{-0.000000, 1.000000, 0.000000},
{-0.195090, 0.980785, 0.000000},
{-0.382683, 0.923880, 0.000000},
{-0.555570, 0.831470, 0.000000},
{-0.707107, 0.707107, 0.000000},
{-0.831470, 0.555570, 0.000000},
{-0.923880, 0.382683, 0.000000},
{-0.980785, 0.195090, 0.000000},
{-1.000000, -0.000000, 0.000000},
{-0.980785, -0.195090, 0.000000},
{-0.923880, -0.382683, 0.000000},
{-0.831470, -0.555570, 0.000000},
{-0.707107, -0.707107, 0.000000},
{-0.555570, -0.831469, 0.000000},
{-0.382684, -0.923880, 0.000000},
{-0.195090, -0.980785, 0.000000},
{0.000000, -1.000000, 0.000000},
{0.195090, -0.980785, 0.000000},
{0.382684, -0.923879, 0.000000},
{0.555570, -0.831470, 0.000000},
{0.707107, -0.707107, 0.000000},
{0.831470, -0.555570, 0.000000},
{0.923880, -0.382683, 0.000000},
{0.980785, -0.195090, 0.000000},
{0.980785, 0.000000, -0.195090},
{0.956195, 0.218245, -0.195090},
{0.883657, 0.425547, -0.195090},
{0.766809, 0.611510, -0.195090},
{0.611510, 0.766809, -0.195090},
{0.425547, 0.883657, -0.195090},
{0.218245, 0.956195, -0.195090},
{-0.000000, 0.980785, -0.195090},
{-0.218245, 0.956195, -0.195090},
{-0.425547, 0.883657, -0.195090},
{-0.611510, 0.766809, -0.195090},
{-0.766809, 0.611510, -0.195090},
{-0.883657, 0.425547, -0.195090},
{-0.956195, 0.218245, -0.195090},
{-0.980785, -0.000000, -0.195090},
{-0.956195, -0.218245, -0.195090},
{-0.883657, -0.425547, -0.195090},
{-0.766809, -0.611510, -0.195090},
{-0.611510, -0.766809, -0.195090},
{-0.425547, -0.883657, -0.195090},
{-0.218245, -0.956195, -0.195090},
{0.000000, -0.980785, -0.195090},
{0.218245, -0.956195, -0.195090},
{0.425547, -0.883657, -0.195090},
{0.611510, -0.766809, -0.195090},
{0.766809, -0.611510, -0.195090},
{0.883657, -0.425547, -0.195090},
{0.956195, -0.218245, -0.195090},
{0.923880, 0.000000, -0.382683},
{0.892399, 0.239118, -0.382683},
{0.800103, 0.461940, -0.382683},
{0.653281, 0.653281, -0.382683},
{0.461940, 0.800103, -0.382683},
{0.239118, 0.892399, -0.382683},
{-0.000000, 0.923880, -0.382683},
{-0.239118, 0.892399, -0.382683},
{-0.461940, 0.800103, -0.382683},
{-0.653281, 0.653281, -0.382683},
{-0.800103, 0.461940, -0.382683},
{-0.892399, 0.239118, -0.382683},
{-0.923880, -0.000000, -0.382683},
{-0.892399, -0.239118, -0.382683},
{-0.800103, -0.461940, -0.382683},
{-0.653282, -0.653281, -0.382683},
{-0.461940, -0.800103, -0.382683},
{-0.239118, -0.892399, -0.382683},
{0.000000, -0.923880, -0.382683},
{0.239118, -0.892399, -0.382683},
{0.461940, -0.800103, -0.382683},
{0.653281, -0.653282, -0.382683},
{0.800103, -0.461940, -0.382683},
{0.892399, -0.239117, -0.382683},
{0.831470, 0.000000, -0.555570},
{0.790775, 0.256938, -0.555570},
{0.672673, 0.488726, -0.555570},
{0.488726, 0.672673, -0.555570},
{0.256938, 0.790775, -0.555570},
{-0.000000, 0.831470, -0.555570},
{-0.256938, 0.790775, -0.555570},
{-0.488726, 0.672673, -0.555570},
{-0.672673, 0.488726, -0.555570},
{-0.790775, 0.256938, -0.555570},
{-0.831470, -0.000000, -0.555570},
{-0.790775, -0.256938, -0.555570},
{-0.672673, -0.488726, -0.555570},
{-0.488725, -0.672673, -0.555570},
{-0.256938, -0.790775, -0.555570},
{0.000000, -0.831470, -0.555570},
{0.256938, -0.790775, -0.555570},
{0.488725, -0.672673, -0.555570},
{0.672673, -0.488726, -0.555570},
{0.790775, -0.256938, -0.555570},
{0.707107, 0.000000, -0.707107},
{0.653281, 0.270598, -0.707107},
{0.500000, 0.500000, -0.707107},
{0.270598, 0.653281, -0.707107},
{-0.000000, 0.707107, -0.707107},
{-0.270598, 0.653282, -0.707107},
{-0.500000, 0.500000, -0.707107},
{-0.653281, 0.270598, -0.707107},
{-0.707107, -0.000000, -0.707107},
{-0.653281, -0.270598, -0.707107},
{-0.500000, -0.500000, -0.707107},
{-0.270598, -0.653281, -0.707107},
{0.000000, -0.707107, -0.707107},
{0.270598, -0.653281, -0.707107},
{0.500000, -0.500000, -0.707107},
{0.653282, -0.270598, -0.707107},
{0.555570, 0.000000, -0.831470},
{0.481138, 0.277785, -0.831470},
{0.277785, 0.481138, -0.831470},
{-0.000000, 0.555570, -0.831470},
{-0.277785, 0.481138, -0.831470},
{-0.481138, 0.277785, -0.831470},
{-0.555570, -0.000000, -0.831470},
{-0.481138, -0.277785, -0.831470},
{-0.277785, -0.481138, -0.831470},
{0.000000, -0.555570, -0.831470},
{0.277785, -0.481138, -0.831470},
{0.481138, -0.277785, -0.831470},
{0.382683, 0.000000, -0.923880},
{0.270598, 0.270598, -0.923880},
{-0.000000, 0.382683, -0.923880},
{-0.270598, 0.270598, -0.923880},
{-0.382683, -0.000000, -0.923880},
{-0.270598, -0.270598, -0.923880},
{0.000000, -0.382683, -0.923880},
{0.270598, -0.270598, -0.923880},
{0.195090, 0.000000, -0.980785},
{-0.000000, 0.195090, -0.980785},
{-0.195090, -0.000000, -0.980785},
{0.000000, -0.195090, -0.980785},
{0.980785, 0.000000, 0.195090},
{0.956195, 0.218245, 0.195090},
{0.883657, 0.425547, 0.195090},
{0.766809, 0.611510, 0.195090},
{0.611510, 0.766809, 0.195090},
{0.425547, 0.883657, 0.195090},
{0.218245, 0.956195, 0.195090},
{-0.000000, 0.980785, 0.195090},
{-0.218245, 0.956195, 0.195090},
{-0.425547, 0.883657, 0.195090},
{-0.611510, 0.766809, 0.195090},
{-0.766809, 0.611510, 0.195090},
{-0.883657, 0.425547, 0.195090},
{-0.956195, 0.218245, 0.195090},
{-0.980785, -0.000000, 0.195090},
{-0.956195, -0.218245, 0.195090},
{-0.883657, -0.425547, 0.195090},
{-0.766809, -0.611510, 0.195090},
{-0.611510, -0.766809, 0.195090},
{-0.425547, -0.883657, 0.195090},
{-0.218245, -0.956195, 0.195090},
{0.000000, -0.980785, 0.195090},
{0.218245, -0.956195, 0.195090},
{0.425547, -0.883657, 0.195090},
{0.611510, -0.766809, 0.195090},
{0.766809, -0.611510, 0.195090},
{0.883657, -0.425547, 0.195090},
{0.956195, -0.218245, 0.195090},
{0.923880, 0.000000, 0.382683},
{0.892399, 0.239118, 0.382683},
{0.800103, 0.461940, 0.382683},
{0.653281, 0.653281, 0.382683},
{0.461940, 0.800103, 0.382683},
{0.239118, 0.892399, 0.382683},
{-0.000000, 0.923880, 0.382683},
{-0.239118, 0.892399, 0.382683},
{-0.461940, 0.800103, 0.382683},
{-0.653281, 0.653281, 0.382683},
{-0.800103, 0.461940, 0.382683},
{-0.892399, 0.239118, 0.382683},
{-0.923880, -0.000000, 0.382683},
{-0.892399, -0.239118, 0.382683},
{-0.800103, -0.461940, 0.382683},
{-0.653282, -0.653281, 0.382683},
{-0.461940, -0.800103, 0.382683},
{-0.239118, -0.892399, 0.382683},
{0.000000, -0.923880, 0.382683},
{0.239118, -0.892399, 0.382683},
{0.461940, -0.800103, 0.382683},
{0.653281, -0.653282, 0.382683},
{0.800103, -0.461940, 0.382683},
{0.892399, -0.239117, 0.382683},
{0.831470, 0.000000, 0.555570},
{0.790775, 0.256938, 0.555570},
{0.672673, 0.488726, 0.555570},
{0.488726, 0.672673, 0.555570},
{0.256938, 0.790775, 0.555570},
{-0.000000, 0.831470, 0.555570},
{-0.256938, 0.790775, 0.555570},
{-0.488726, 0.672673, 0.555570},
{-0.672673, 0.488726, 0.555570},
{-0.790775, 0.256938, 0.555570},
{-0.831470, -0.000000, 0.555570},
{-0.790775, -0.256938, 0.555570},
{-0.672673, -0.488726, 0.555570},
{-0.488725, -0.672673, 0.555570},
{-0.256938, -0.790775, 0.555570},
{0.000000, -0.831470, 0.555570},
{0.256938, -0.790775, 0.555570},
{0.488725, -0.672673, 0.555570},
{0.672673, -0.488726, 0.555570},
{0.790775, -0.256938, 0.555570},
{0.707107, 0.000000, 0.707107},
{0.653281, 0.270598, 0.707107},
{0.500000, 0.500000, 0.707107},
{0.270598, 0.653281, 0.707107},
{-0.000000, 0.707107, 0.707107},
{-0.270598, 0.653282, 0.707107},
{-0.500000, 0.500000, 0.707107},
{-0.653281, 0.270598, 0.707107},
{-0.707107, -0.000000, 0.707107},
{-0.653281, -0.270598, 0.707107},
{-0.500000, -0.500000, 0.707107},
{-0.270598, -0.653281, 0.707107},
{0.000000, -0.707107, 0.707107},
{0.270598, -0.653281, 0.707107},
{0.500000, -0.500000, 0.707107},
{0.653282, -0.270598, 0.707107},
{0.555570, 0.000000, 0.831470},
{0.481138, 0.277785, 0.831470},
{0.277785, 0.481138, 0.831470},
{-0.000000, 0.555570, 0.831470},
{-0.277785, 0.481138, 0.831470},
{-0.481138, 0.277785, 0.831470},
{-0.555570, -0.000000, 0.831470},
{-0.481138, -0.277785, 0.831470},
{-0.277785, -0.481138, 0.831470},
{0.000000, -0.555570, 0.831470},
{0.277785, -0.481138, 0.831470},
{0.481138, -0.277785, 0.831470},
{0.382683, 0.000000, 0.923880},
{0.270598, 0.270598, 0.923880},
{-0.000000, 0.382683, 0.923880},
{-0.270598, 0.270598, 0.923880},
{-0.382683, -0.000000, 0.923880},
{-0.270598, -0.270598, 0.923880},
{0.000000, -0.382683, 0.923880},
{0.270598, -0.270598, 0.923880},
{0.195090, 0.000000, 0.980785},
{-0.000000, 0.195090, 0.980785},
{-0.195090, -0.000000, 0.980785},
{0.000000, -0.195090, 0.980785},
};
// *INDENT-ON*

// NOTE: MDC_MAX_ERROR is effectively the compression level. the lower this value, the higher
// the accuracy, but with lower compression ratios.
#define MDC_MAX_ERROR       0.1	// if any compressed vert is off by more than this from the
									// actual vert, make this a baseframe

#define MDC_DIST_SCALE      0.05	// lower for more accuracy, but less range

// note: we are locked in at 8 or less bits since changing to byte-encoded normals
#define MDC_BITS_PER_AXIS   8
#define MDC_MAX_OFS         127.0	// to be safe

#define MDC_MAX_DIST        ( MDC_MAX_OFS * MDC_DIST_SCALE )

#if 0
void            R_MDC_DecodeXyzCompressed(mdcXyzCompressed_t * xyzComp, vec3_t out, vec3_t normal);
#else							// optimized version
#define R_MDC_DecodeXyzCompressed( ofsVec, out, normal ) \
	( out )[0] = ( (float)( ( ofsVec ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	( out )[1] = ( (float)( ( ofsVec >> 8 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	( out )[2] = ( (float)( ( ofsVec >> 16 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	VectorCopy( ( r_anormals )[( ofsVec >> 24 )], normal );
#endif


/*
=================
R_LoadMDC
=================
*/
qboolean R_LoadMDC(model_t * mod, int lod, void *buffer, int bufferSize, const char *modName)
{
	int             i, j, k, l;

	mdcHeader_t    *mdcModel;
	md3Frame_t     *mdcFrame;
	mdcSurface_t   *mdcSurf;
	md3Shader_t    *mdcShader;
	md3Triangle_t  *mdcTri;
	md3St_t        *mdcst;
	md3XyzNormal_t *mdcxyz;
	mdcXyzCompressed_t *mdcxyzComp;
	mdcTag_t       *mdcTag;
	mdcTagName_t   *mdcTagName;

	mdvModel_t     *mdvModel;
	mdvFrame_t     *frame;
	mdvSurface_t   *surf, *surface;
	srfTriangle_t  *tri;
	mdvVertex_t    *v;
	mdvSt_t        *st;
	mdvTag_t       *tag;
	mdvTagName_t   *tagName;
	short          *ps;

	int             version;
	int             size;

	mdcModel = (mdcHeader_t *) buffer;

	version = LittleLong(mdcModel->version);
	if(version != MDC_VERSION)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMD3: %s has wrong version (%i should be %i)\n", modName, version, MDC_VERSION);
		return qfalse;
	}

	mod->type = MOD_MESH;
	size = LittleLong(mdcModel->ofsEnd);
	mod->dataSize += size;
	mdvModel = mod->mdv[lod] = ri.Hunk_Alloc(sizeof(mdvModel_t), h_low);


	LL(mdcModel->ident);
	LL(mdcModel->version);
	LL(mdcModel->numFrames);
	LL(mdcModel->numTags);
	LL(mdcModel->numSurfaces);
	LL(mdcModel->ofsFrames);
	LL(mdcModel->ofsTags);
	LL(mdcModel->ofsSurfaces);
	LL(mdcModel->ofsEnd);
	LL(mdcModel->ofsEnd);
	LL(mdcModel->flags);
	LL(mdcModel->numSkins);

	if(mdcModel->numFrames < 1)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMDC: '%s' has no frames\n", modName);
		return qfalse;
	}

	// swap all the frames
	mdvModel->numFrames = mdcModel->numFrames;
	mdvModel->frames = frame = ri.Hunk_Alloc(sizeof(*frame) * mdcModel->numFrames, h_low);

	mdcFrame = (md3Frame_t *) ((byte *) mdcModel + mdcModel->ofsFrames);
	for(i = 0; i < mdcModel->numFrames; i++, frame++, mdcFrame++)
	{
		
#if 1
		// RB: ET HACK
		if(strstr(mod->name, "sherman") || strstr(mod->name, "mg42"))
		{
			frame->radius = 256;
			for(j = 0; j < 3; j++)
			{
				frame->bounds[0][j] = 128;
				frame->bounds[1][j] = -128;
				frame->localOrigin[j] = LittleFloat(mdcFrame->localOrigin[j]);
			}
		}
		else
#endif
		{
			frame->radius = LittleFloat(mdcFrame->radius);
			for(j = 0; j < 3; j++)
			{
				frame->bounds[0][j] = LittleFloat(mdcFrame->bounds[0][j]);
				frame->bounds[1][j] = LittleFloat(mdcFrame->bounds[1][j]);
				frame->localOrigin[j] = LittleFloat(mdcFrame->localOrigin[j]);
			}
		}
	}

	// swap all the tags
	mdvModel->numTags = mdcModel->numTags;
	mdvModel->tags = tag = ri.Hunk_Alloc(sizeof(*tag) * (mdcModel->numTags * mdcModel->numFrames), h_low);

	mdcTag = (mdcTag_t *) ((byte *) mdcModel + mdcModel->ofsTags);
	for(i = 0; i < mdcModel->numTags * mdcModel->numFrames; i++, tag++, mdcTag++)
	{
		vec3_t		angles;
		for(j = 0; j < 3; j++)
		{
			tag->origin[j] = (float)LittleShort(mdcTag->xyz[j]) * MD3_XYZ_SCALE;
			angles[j] = (float)LittleShort(mdcTag->angles[j]) * MDC_TAG_ANGLE_SCALE;
		}

		AnglesToAxis(angles, tag->axis);
	}

	mdvModel->tagNames = tagName = ri.Hunk_Alloc(sizeof(*tagName) * (mdcModel->numTags), h_low);

	mdcTagName = (mdcTagName_t *) ((byte *) mdcModel + mdcModel->ofsTagNames);
	for(i = 0; i < mdcModel->numTags; i++, tagName++, mdcTagName++)
	{
		Q_strncpyz(tagName->name, mdcTagName->name, sizeof(tagName->name));
	}

	// swap all the surfaces
	mdvModel->numSurfaces = mdcModel->numSurfaces;
	mdvModel->surfaces = surf = ri.Hunk_Alloc(sizeof(*surf) * mdcModel->numSurfaces, h_low);

	mdcSurf = (mdcSurface_t *) ((byte *) mdcModel + mdcModel->ofsSurfaces);
	for(i = 0; i < mdcModel->numSurfaces; i++)
	{
		LL(mdcSurf->ident);
		LL(mdcSurf->flags);
		LL(mdcSurf->numBaseFrames);
		LL(mdcSurf->numCompFrames);
		LL(mdcSurf->numShaders);
		LL(mdcSurf->numTriangles);
		LL(mdcSurf->ofsTriangles);
		LL(mdcSurf->numVerts);
		LL(mdcSurf->ofsShaders);
		LL(mdcSurf->ofsSt);
		LL(mdcSurf->ofsXyzNormals);
		LL(mdcSurf->ofsXyzNormals);
		LL(mdcSurf->ofsXyzCompressed);
		LL(mdcSurf->ofsFrameBaseFrames);
		LL(mdcSurf->ofsFrameCompFrames);
		LL(mdcSurf->ofsEnd);


		if(mdcSurf->numVerts > SHADER_MAX_VERTEXES)
		{
			ri.Error(ERR_DROP, "R_LoadMDC: %s has more than %i verts on a surface (%i)",
					 modName, SHADER_MAX_VERTEXES, mdcSurf->numVerts);
		}
		if(mdcSurf->numTriangles > SHADER_MAX_TRIANGLES)
		{
			ri.Error(ERR_DROP, "R_LoadMDC: %s has more than %i triangles on a surface (%i)",
					 modName, SHADER_MAX_TRIANGLES, mdcSurf->numTriangles);
		}

		// change to surface identifier
		surf->surfaceType = SF_MDV;

		// give pointer to model for Tess_SurfaceMDX
		surf->model = mdvModel;

		// copy surface name
		Q_strncpyz(surf->name, mdcSurf->name, sizeof(surf->name));

		// lowercase the surface name so skin compares are faster
		Q_strlwr(surf->name);

		// strip off a trailing _1 or _2
		// this is a crutch for q3data being a mess
		j = strlen(surf->name);
		if(j > 2 && surf->name[j - 2] == '_')
		{
			surf->name[j - 2] = 0;
		}

		// register the shaders
		/*
		   surf->numShaders = md3Surf->numShaders;
		   surf->shaders = shader = ri.Hunk_Alloc(sizeof(*shader) * md3Surf->numShaders, h_low);

		   md3Shader = (md3Shader_t *) ((byte *) md3Surf + md3Surf->ofsShaders);
		   for(j = 0; j < md3Surf->numShaders; j++, shader++, md3Shader++)
		   {
		   shader_t       *sh;

		   sh = R_FindShader(md3Shader->name, SHADER_3D_DYNAMIC, qtrue);
		   if(sh->defaultShader)
		   {
		   shader->shaderIndex = 0;
		   }
		   else
		   {
		   shader->shaderIndex = sh->index;
		   }
		   }
		 */

		// only consider the first shader
		mdcShader = (md3Shader_t *) ((byte *) mdcSurf + mdcSurf->ofsShaders);
		surf->shader = R_FindShader(mdcShader->name, SHADER_3D_DYNAMIC, qtrue);

		// swap all the triangles
		surf->numTriangles = mdcSurf->numTriangles;
		surf->triangles = tri = ri.Hunk_Alloc(sizeof(*tri) * mdcSurf->numTriangles, h_low);

		mdcTri = (md3Triangle_t *) ((byte *) mdcSurf + mdcSurf->ofsTriangles);
		for(j = 0; j < mdcSurf->numTriangles; j++, tri++, mdcTri++)
		{
			tri->indexes[0] = LittleLong(mdcTri->indexes[0]);
			tri->indexes[1] = LittleLong(mdcTri->indexes[1]);
			tri->indexes[2] = LittleLong(mdcTri->indexes[2]);
		}

		R_CalcSurfaceTriangleNeighbors(surf->numTriangles, surf->triangles);

		// swap all the XyzNormals
		mdcxyz = (md3XyzNormal_t *) ((byte *) mdcSurf + mdcSurf->ofsXyzNormals);
		for(j = 0; j < mdcSurf->numVerts * mdcSurf->numBaseFrames; j++, mdcxyz++)
		{
			mdcxyz->xyz[0] = LittleShort(mdcxyz->xyz[0]);
			mdcxyz->xyz[1] = LittleShort(mdcxyz->xyz[1]);
			mdcxyz->xyz[2] = LittleShort(mdcxyz->xyz[2]);

			mdcxyz->normal = LittleShort(mdcxyz->normal);
		}

		// swap all the XyzCompressed
		mdcxyzComp = (mdcXyzCompressed_t *) ((byte *) mdcSurf + mdcSurf->ofsXyzCompressed);
		for(j = 0; j < mdcSurf->numVerts * mdcSurf->numCompFrames; j++, mdcxyzComp++)
		{
			LL(mdcxyzComp->ofsVec);
		}

		// swap the frameBaseFrames
		ps = (short *)((byte *) mdcSurf + mdcSurf->ofsFrameBaseFrames);
		for(j = 0; j < mdcModel->numFrames; j++, ps++)
		{
			*ps = LittleShort(*ps);
		}

		// swap the frameCompFrames
		ps = (short *)((byte *) mdcSurf + mdcSurf->ofsFrameCompFrames);
		for(j = 0; j < mdcModel->numFrames; j++, ps++)
		{
			*ps = LittleShort(*ps);
		}

		surf->numVerts = mdcSurf->numVerts;
		surf->verts = v = ri.Hunk_Alloc(sizeof(*v) * (mdcSurf->numVerts * mdcModel->numFrames), h_low);
		
		for(j = 0; j < mdcModel->numFrames; j++)
		{
			int             baseFrame;
			int				compFrame;

			baseFrame = (int) *((short *)((byte *) mdcSurf + mdcSurf->ofsFrameBaseFrames) + j);

			mdcxyz = (md3XyzNormal_t *)((byte *) mdcSurf + mdcSurf->ofsXyzNormals + baseFrame * mdcSurf->numVerts * sizeof(md3XyzNormal_t));

			if(mdcSurf->numCompFrames > 0)
			{
				compFrame = (int) *((short *)((byte *) mdcSurf + mdcSurf->ofsFrameCompFrames) + j);
				if(compFrame >= 0)
				{
					mdcxyzComp = (mdcXyzCompressed_t *) ((byte *) mdcSurf + mdcSurf->ofsXyzCompressed + compFrame * mdcSurf->numVerts * sizeof(mdcXyzCompressed_t));
				}
			}

			for(k = 0; k < mdcSurf->numVerts; k++, v++, mdcxyz++)
			{
				v->xyz[0] = LittleShort(mdcxyz->xyz[0]) * MD3_XYZ_SCALE;
				v->xyz[1] = LittleShort(mdcxyz->xyz[1]) * MD3_XYZ_SCALE;
				v->xyz[2] = LittleShort(mdcxyz->xyz[2]) * MD3_XYZ_SCALE;

				if(mdcSurf->numCompFrames > 0 && compFrame >= 0)
				{
					vec3_t      ofsVec;
					vec3_t		normal;

					R_MDC_DecodeXyzCompressed(LittleShort(mdcxyzComp->ofsVec), ofsVec, normal);
					VectorAdd(v->xyz, ofsVec, v->xyz);

					mdcxyzComp++;
				}
			}
		}

		// swap all the ST
		surf->st = st = ri.Hunk_Alloc(sizeof(*st) * mdcSurf->numVerts, h_low);

		mdcst = (md3St_t *) ((byte *) mdcSurf + mdcSurf->ofsSt);
		for(j = 0; j < mdcSurf->numVerts; j++, mdcst++, st++)
		{
			st->st[0] = LittleFloat(mdcst->st[0]);
			st->st[1] = LittleFloat(mdcst->st[1]);
		}

		// find the next surface
		mdcSurf = (mdcSurface_t *) ((byte *) mdcSurf + mdcSurf->ofsEnd);
		surf++;
	}

	return qtrue;
}

