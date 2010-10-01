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
qboolean R_LoadMDM(model_t * mod, void *buffer, const char *mod_name)
{
	int             i, j, k;
	mdmHeader_t    *pinmodel, *mdm;

//    mdmFrame_t            *frame;
	mdmSurface_t   *surf;
	mdmTriangle_t  *tri;
	mdmVertex_t    *v;
	mdmTag_t       *tag;
	int             version;
	int             size;
	shader_t       *sh;
	int            *collapseMap, *boneref;

	pinmodel = (mdmHeader_t *) buffer;

	version = LittleLong(pinmodel->version);
	if(version != MDM_VERSION)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMDM: %s has wrong version (%i should be %i)\n", mod_name, version, MDM_VERSION);
		return qfalse;
	}

	mod->type = MOD_MDM;
	size = LittleLong(pinmodel->ofsEnd);
	mod->dataSize += size;
	mdm = mod->mdm = ri.Hunk_Alloc(size, h_low);

	memcpy(mdm, buffer, LittleLong(pinmodel->ofsEnd));

	LL(mdm->ident);
	LL(mdm->version);
//    LL(mdm->numFrames);
	LL(mdm->numTags);
	LL(mdm->numSurfaces);
//    LL(mdm->ofsFrames);
	LL(mdm->ofsTags);
	LL(mdm->ofsEnd);
	LL(mdm->ofsSurfaces);
	mdm->lodBias = LittleFloat(mdm->lodBias);
	mdm->lodScale = LittleFloat(mdm->lodScale);

/*	mdm->skel = RE_RegisterModel(mdm->bonesfile);
	if ( !mdm->skel ) {
		ri.Error (ERR_DROP, "R_LoadMDM: %s skeleton not found\n", mdm->bonesfile );
	}

	if ( mdm->numFrames < 1 ) {
		ri.Printf( PRINT_WARNING, "R_LoadMDM: %s has no frames\n", mod_name );
		return qfalse;
	}*/

	if(LittleLong(1) != 1)
	{
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
		tag = (mdmTag_t *) ((byte *) mdm + mdm->ofsTags);
		for(i = 0; i < mdm->numTags; i++)
		{
			int             ii;

			for(ii = 0; ii < 3; ii++)
			{
				tag->axis[ii][0] = LittleFloat(tag->axis[ii][0]);
				tag->axis[ii][1] = LittleFloat(tag->axis[ii][1]);
				tag->axis[ii][2] = LittleFloat(tag->axis[ii][2]);
			}

			LL(tag->boneIndex);
			//tag->torsoWeight = LittleFloat( tag->torsoWeight );
			tag->offset[0] = LittleFloat(tag->offset[0]);
			tag->offset[1] = LittleFloat(tag->offset[1]);
			tag->offset[2] = LittleFloat(tag->offset[2]);

			LL(tag->numBoneReferences);
			LL(tag->ofsBoneReferences);
			LL(tag->ofsEnd);

			// swap the bone references
			boneref = (int *)((byte *) tag + tag->ofsBoneReferences);
			for(j = 0; j < tag->numBoneReferences; j++, boneref++)
			{
				*boneref = LittleLong(*boneref);
			}

			// find the next tag
			tag = (mdmTag_t *) ((byte *) tag + tag->ofsEnd);
		}
	}

	// swap all the surfaces
	surf = (mdmSurface_t *) ((byte *) mdm + mdm->ofsSurfaces);
	for(i = 0; i < mdm->numSurfaces; i++)
	{
		if(LittleLong(1) != 1)
		{
			//LL(surf->ident);
			LL(surf->shaderIndex);
			LL(surf->minLod);
			LL(surf->ofsHeader);
			LL(surf->ofsCollapseMap);
			LL(surf->numTriangles);
			LL(surf->ofsTriangles);
			LL(surf->numVerts);
			LL(surf->ofsVerts);
			LL(surf->numBoneReferences);
			LL(surf->ofsBoneReferences);
			LL(surf->ofsEnd);
		}

		// change to surface identifier
		surf->ident = SF_MDM;

		if(surf->numVerts > SHADER_MAX_VERTEXES)
		{
			ri.Error(ERR_DROP, "R_LoadMDM: %s has more than %i verts on a surface (%i)",
					 mod_name, SHADER_MAX_VERTEXES, surf->numVerts);
		}
		
		if(surf->numTriangles > SHADER_MAX_TRIANGLES)
		{
			ri.Error(ERR_DROP, "R_LoadMDM: %s has more than %i triangles on a surface (%i)",
						mod_name, SHADER_MAX_TRIANGLES, surf->numTriangles);
		}

		// register the shaders
		if(surf->shader[0])
		{
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

		if(LittleLong(1) != 1)
		{
			// swap all the triangles
			tri = (mdmTriangle_t *) ((byte *) surf + surf->ofsTriangles);
			for(j = 0; j < surf->numTriangles; j++, tri++)
			{
				LL(tri->indexes[0]);
				LL(tri->indexes[1]);
				LL(tri->indexes[2]);
			}

			// swap all the vertexes
			v = (mdmVertex_t *) ((byte *) surf + surf->ofsVerts);
			for(j = 0; j < surf->numVerts; j++)
			{
				v->normal[0] = LittleFloat(v->normal[0]);
				v->normal[1] = LittleFloat(v->normal[1]);
				v->normal[2] = LittleFloat(v->normal[2]);

				v->texCoords[0] = LittleFloat(v->texCoords[0]);
				v->texCoords[1] = LittleFloat(v->texCoords[1]);

				v->numWeights = LittleLong(v->numWeights);

				for(k = 0; k < v->numWeights; k++)
				{
					v->weights[k].boneIndex = LittleLong(v->weights[k].boneIndex);
					v->weights[k].boneWeight = LittleFloat(v->weights[k].boneWeight);
					v->weights[k].offset[0] = LittleFloat(v->weights[k].offset[0]);
					v->weights[k].offset[1] = LittleFloat(v->weights[k].offset[1]);
					v->weights[k].offset[2] = LittleFloat(v->weights[k].offset[2]);
				}

				v = (mdmVertex_t *) & v->weights[v->numWeights];
			}

			// swap the collapse map
			collapseMap = (int *)((byte *) surf + surf->ofsCollapseMap);
			for(j = 0; j < surf->numVerts; j++, collapseMap++)
			{
				*collapseMap = LittleLong(*collapseMap);
			}

			// swap the bone references
			boneref = (int *)((byte *) surf + surf->ofsBoneReferences);
			for(j = 0; j < surf->numBoneReferences; j++, boneref++)
			{
				*boneref = LittleLong(*boneref);
			}
		}

		// find the next surface
		surf = (mdmSurface_t *) ((byte *) surf + surf->ofsEnd);
	}

	return qtrue;
}