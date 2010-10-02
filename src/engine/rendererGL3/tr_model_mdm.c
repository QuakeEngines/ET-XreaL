/*
===========================================================================
Copyright (C) 2009-2010 Robert Beckebans <trebor_7@users.sourceforge.net>

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
// tr_models_mdm.c -- Enemy Territory .mdm model loading and caching

#include "tr_local.h"
#include "tr_model_skel.h"

#define	LL(x) x=LittleLong(x)
#define	LF(x) x=LittleFloat(x)


/*
=================
R_LoadMDM
=================
*/
qboolean R_LoadMDM(model_t * mod, void *buffer, const char *modName)
{
	int             i, j, k;
	
	mdmHeader_t    *mdm;
//    mdmFrame_t            *frame;
	mdmSurface_t   *mdmSurf;
	mdmTriangle_t  *mdmTri;
	mdmVertex_t    *mdmVertex;
	mdmTag_t       *mdmTag;
	int             version;
	int             size;
	shader_t       *sh;
	int32_t        *collapseMap, *collapseMapOut, *boneref, *bonerefOut;


	mdmModel_t     *mdmModel;
	mdmTagIntern_t			*tag;
	mdmSurfaceIntern_t		*surf;
	srfTriangle_t			*tri;
	md5Vertex_t             *v;

	mdm = (mdmHeader_t *) buffer;

	version = LittleLong(mdm->version);
	if(version != MDM_VERSION)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMDM: %s has wrong version (%i should be %i)\n", modName, version, MDM_VERSION);
		return qfalse;
	}

	mod->type = MOD_MDM;
	size = LittleLong(mdm->ofsEnd);
	mod->dataSize += sizeof(mdmModel_t);
	
	//mdm = mod->mdm = ri.Hunk_Alloc(size, h_low);
	//memcpy(mdm, buffer, LittleLong(pinmodel->ofsEnd));

	mdmModel = mod->mdm = ri.Hunk_Alloc(sizeof(mdmModel_t), h_low);

	LL(mdm->ident);
	LL(mdm->version);
//    LL(mdm->numFrames);
	LL(mdm->numTags);
	LL(mdm->numSurfaces);
//    LL(mdm->ofsFrames);
	LL(mdm->ofsTags);
	LL(mdm->ofsEnd);
	LL(mdm->ofsSurfaces);
	
	mdmModel->lodBias = LittleFloat(mdm->lodBias);
	mdmModel->lodScale = LittleFloat(mdm->lodScale);

/*	mdm->skel = RE_RegisterModel(mdm->bonesfile);
	if ( !mdm->skel ) {
		ri.Error (ERR_DROP, "R_LoadMDM: %s skeleton not found\n", mdm->bonesfile );
	}

	if ( mdm->numFrames < 1 ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDM: %s has no frames\n", modName );
		return qfalse;
	}*/

	
	
	// swap all the frames
	/*frameSize = (int) ( sizeof( mdmFrame_t ) );
	   for ( i = 0 ; i < mdm->numFrames ; i++, frame++) {
	   frame = (mdmFrame_t *) ( (byte *)mdm + mdm->ofsFrames + i * frameSize );
	   frame->radius = LittleFloat( frame->radius );
	   for ( j = 0 ; j < 3 ; j++ ) {
	   frame->bounds[0][j] = LittleFloat( frame->bounds[0][j] );
	   frame->bounds[1][j] = LittleFloat( frame->bounds[1][j] );
	   frame->localOrigin[j] = LittleFloat( frame->localOrigin[j] );
	   frame->parentOffset[j] = LittleFloat( frame->parentOffset[j] );
	   }
	   } */

	// swap all the tags
	mdmModel->numTags = mdm->numTags;
	mdmModel->tags = tag = ri.Hunk_Alloc(sizeof(*tag) * mdm->numTags, h_low);

	mdmTag = (mdmTag_t *) ((byte *) mdm + mdm->ofsTags);
	for(i = 0; i < mdm->numTags; i++, tag++)
	{
		int             ii;

		Q_strncpyz(tag->name, mdmTag->name, sizeof(tag->name));

		for(ii = 0; ii < 3; ii++)
		{
			tag->axis[ii][0] = LittleFloat(mdmTag->axis[ii][0]);
			tag->axis[ii][1] = LittleFloat(mdmTag->axis[ii][1]);
			tag->axis[ii][2] = LittleFloat(mdmTag->axis[ii][2]);
		}

		tag->boneIndex = LittleLong(mdmTag->boneIndex);
		//tag->torsoWeight = LittleFloat( tag->torsoWeight );
		tag->offset[0] = LittleFloat(mdmTag->offset[0]);
		tag->offset[1] = LittleFloat(mdmTag->offset[1]);
		tag->offset[2] = LittleFloat(mdmTag->offset[2]);

		
		LL(mdmTag->numBoneReferences);
		LL(mdmTag->ofsBoneReferences);
		LL(mdmTag->ofsEnd);

		tag->numBoneReferences = mdmTag->numBoneReferences;
		tag->boneReferences = ri.Hunk_Alloc(sizeof(*bonerefOut) * mdmTag->numBoneReferences, h_low);

		// swap the bone references
		boneref = (int32_t *)((byte *) mdmTag + mdmTag->ofsBoneReferences);
		for(j = 0, bonerefOut = tag->boneReferences; j < mdmTag->numBoneReferences; j++, boneref++, bonerefOut++)
		{
			*bonerefOut = LittleLong(*boneref);
		}
		

		// find the next tag
		mdmTag = (mdmTag_t *) ((byte *) mdmTag + mdmTag->ofsEnd);
	}
	

	// swap all the surfaces
	mdmModel->numSurfaces = mdm->numSurfaces;
	mdmModel->surfaces = ri.Hunk_Alloc(sizeof(*surf) * mdmModel->numSurfaces, h_low);

	mdmSurf = (mdmSurface_t *) ((byte *) mdm + mdm->ofsSurfaces);
	for(i = 0, surf = mdmModel->surfaces; i < mdm->numSurfaces; i++, surf++)
	{
		LL(mdmSurf->shaderIndex);
		LL(mdmSurf->ofsHeader);
		LL(mdmSurf->ofsCollapseMap);
		LL(mdmSurf->numTriangles);
		LL(mdmSurf->ofsTriangles);
		LL(mdmSurf->numVerts);
		LL(mdmSurf->ofsVerts);
		LL(mdmSurf->numBoneReferences);
		LL(mdmSurf->ofsBoneReferences);
		LL(mdmSurf->ofsEnd);

		surf->minLod = LittleLong(mdmSurf->minLod);

		// change to surface identifier
		surf->surfaceType = SF_MDM;
		surf->model = mdmModel;

		Q_strncpyz(surf->name, mdmSurf->name, sizeof(surf->name));

		if(mdmSurf->numVerts > SHADER_MAX_VERTEXES)
		{
			ri.Error(ERR_DROP, "R_LoadMDM: %s has more than %i verts on a surface (%i)",
					 modName, SHADER_MAX_VERTEXES, mdmSurf->numVerts);
		}
		
		if(mdmSurf->numTriangles > SHADER_MAX_TRIANGLES)
		{
			ri.Error(ERR_DROP, "R_LoadMDM: %s has more than %i triangles on a surface (%i)",
						modName, SHADER_MAX_TRIANGLES, mdmSurf->numTriangles);
		}

		// register the shaders
		if(mdmSurf->shader[0])
		{
			Q_strncpyz(surf->shader, mdmSurf->shader, sizeof(surf->shader));

			sh = R_FindShader(surf->shader, SHADER_3D_DYNAMIC, qtrue);
			if(sh->defaultShader)
			{
				surf->shaderIndex = 0;
			}
			else
			{
				surf->shaderIndex = sh->index;
			}
		}
		else
		{
			surf->shaderIndex = 0;
		}

		
		// swap all the triangles
		surf->numTriangles = mdmSurf->numTriangles;
		surf->triangles = ri.Hunk_Alloc(sizeof(*tri) * surf->numTriangles, h_low);

		mdmTri = (mdmTriangle_t *) ((byte *) mdmSurf + mdmSurf->ofsTriangles);
		for(j = 0, tri = surf->triangles; j < surf->numTriangles; j++, mdmTri++, tri++)
		{
			tri->indexes[0] = LittleLong(mdmTri->indexes[0]);
			tri->indexes[1] = LittleLong(mdmTri->indexes[1]);
			tri->indexes[2] = LittleLong(mdmTri->indexes[2]);
		}

		// swap all the vertexes
		surf->numVerts = mdmSurf->numVerts;
		surf->verts = ri.Hunk_Alloc(sizeof(*v) * surf->numVerts, h_low);

		mdmVertex = (mdmVertex_t *) ((byte *) mdmSurf + mdmSurf->ofsVerts);
		for(j = 0, v = surf->verts; j < mdmSurf->numVerts; j++, v++)
		{
			v->normal[0] = LittleFloat(mdmVertex->normal[0]);
			v->normal[1] = LittleFloat(mdmVertex->normal[1]);
			v->normal[2] = LittleFloat(mdmVertex->normal[2]);

			v->texCoords[0] = LittleFloat(mdmVertex->texCoords[0]);
			v->texCoords[1] = LittleFloat(mdmVertex->texCoords[1]);

			v->numWeights = LittleLong(mdmVertex->numWeights);

			if(v->numWeights > MAX_WEIGHTS)
			{
#if 0
				ri.Error(ERR_DROP, "R_LoadMDM: vertex %i requires %i instead of maximum %i weights on surface (%i) in model '%s'",
					j, v->numWeights, MAX_WEIGHTS, i, modName);
#else
				ri.Printf(PRINT_WARNING, "WARNING: R_LoadMDM: vertex %i requires %i instead of maximum %i weights on surface (%i) in model '%s'\n",
					j, v->numWeights, MAX_WEIGHTS, i, modName);
#endif
			}

			v->weights = ri.Hunk_Alloc(sizeof(*v->weights) * v->numWeights, h_low);
			for(k = 0; k < v->numWeights; k++)
			{
				md5Weight_t *weight = ri.Hunk_Alloc(sizeof(*weight), h_low);

				weight->boneIndex = LittleLong(mdmVertex->weights[k].boneIndex);
				weight->boneWeight = LittleFloat(mdmVertex->weights[k].boneWeight);
				weight->offset[0] = LittleFloat(mdmVertex->weights[k].offset[0]);
				weight->offset[1] = LittleFloat(mdmVertex->weights[k].offset[1]);
				weight->offset[2] = LittleFloat(mdmVertex->weights[k].offset[2]);

				v->weights[k] = weight;
			}

			mdmVertex = (mdmVertex_t *) &mdmVertex->weights[v->numWeights];
		}

		// swap the collapse map
		surf->collapseMap = ri.Hunk_Alloc(sizeof(*collapseMapOut) * mdmSurf->numVerts, h_low);

		collapseMap = (int32_t *)((byte *) surf + mdmSurf->ofsCollapseMap);
		for(j = 0, collapseMapOut = surf->collapseMap; j < mdmSurf->numVerts; j++, collapseMap++, collapseMapOut++)
		{
			*collapseMapOut = LittleLong(*collapseMap);
		}

		// swap the bone references
		surf->numBoneReferences = mdmSurf->numBoneReferences;
		surf->boneReferences = ri.Hunk_Alloc(sizeof(*bonerefOut) * mdmSurf->numBoneReferences, h_low);

		boneref = (int32_t *)((byte *) mdmSurf + mdmSurf->ofsBoneReferences);
		for(j = 0, bonerefOut = surf->boneReferences; j < surf->numBoneReferences; j++, boneref++, bonerefOut++)
		{
			*bonerefOut = LittleLong(*boneref);
		}

		// find the next surface
		mdmSurf = (mdmSurface_t *) ((byte *) mdmSurf + mdmSurf->ofsEnd);
	}

	return qtrue;
}