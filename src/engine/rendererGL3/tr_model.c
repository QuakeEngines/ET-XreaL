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

qboolean R_LoadMD3(model_t * mod, int lod, void *buffer, int bufferSize, const char *name, qboolean forceStatic);
qboolean R_LoadMDC(model_t * mod, int lod, void *buffer, int bufferSize, const char *name);
static qboolean R_LoadMDM(model_t * mod, void *buffer, const char *name);
static qboolean R_LoadMDX(model_t * mod, void *buffer, const char *name);

#if defined(USE_REFENTITY_ANIMATIONSYSTEM)
qboolean R_LoadMD5(model_t * mod, void *buffer, int bufferSize, const char *name);
qboolean R_LoadPSK(model_t * mod, void *buffer, int bufferSize, const char *name);
#endif

model_t        *loadmodel;

/*
** R_GetModelByHandle
*/
model_t        *R_GetModelByHandle(qhandle_t index)
{
	model_t        *mod;

	// out of range gets the defualt model
	if(index < 1 || index >= tr.numModels)
	{
		return tr.models[0];
	}

	mod = tr.models[index];

	return mod;
}

//===============================================================================

/*
** R_AllocModel
*/
model_t        *R_AllocModel(void)
{
	model_t        *mod;

	if(tr.numModels == MAX_MOD_KNOWN)
	{
		return NULL;
	}

	mod = ri.Hunk_Alloc(sizeof(*tr.models[tr.numModels]), h_low);
	mod->index = tr.numModels;
	tr.models[tr.numModels] = mod;
	tr.numModels++;

	return mod;
}

/*
====================
RE_RegisterModel

Loads in a model for the given name

Zero will be returned if the model fails to load.
An entry will be retained for failed models as an
optimization to prevent disk rescanning if they are
asked for again.
====================
*/
qhandle_t RE_RegisterModel(const char *name)
{
	model_t        *mod;
	unsigned       *buffer;
	int             bufferLen;
	int             lod;
	int             ident;
	qboolean        loaded;
	qhandle_t       hModel;
	int             numLoaded;

	if(!name || !name[0])
	{
		ri.Printf(PRINT_ALL, "RE_RegisterModel: NULL name\n");
		return 0;
	}

	if(strlen(name) >= MAX_QPATH)
	{
		Com_Printf("Model name exceeds MAX_QPATH\n");
		return 0;
	}

	// search the currently loaded models
	for(hModel = 1; hModel < tr.numModels; hModel++)
	{
		mod = tr.models[hModel];
		if(!strcmp(mod->name, name))
		{
			if(mod->type == MOD_BAD)
			{
				return 0;
			}
			return hModel;
		}
	}

	// allocate a new model_t
	if((mod = R_AllocModel()) == NULL)
	{
		ri.Printf(PRINT_WARNING, "RE_RegisterModel: R_AllocModel() failed for '%s'\n", name);
		return 0;
	}

	// only set the name after the model has been successfully loaded
	Q_strncpyz(mod->name, name, sizeof(mod->name));

	// make sure the render thread is stopped
	R_SyncRenderThread();

	mod->numLods = 0;

	// load the files
	numLoaded = 0;

	if(strstr(name, ".mds") || strstr(name, ".mdm") || strstr(name, ".mdx") || strstr(name, ".md5mesh") || strstr(name, ".psk"))
	{
		// try loading skeletal file

		loaded = qfalse;
		bufferLen = ri.FS_ReadFile(name, (void **)&buffer);
		if(buffer)
		{
			loadmodel = mod;

			ident = LittleLong(*(unsigned *)buffer);
#if 0
			if(ident == MDS_IDENT)
			{
				loaded = R_LoadMDS(mod, buffer, name);
			} else
#endif
			if(ident == MDM_IDENT)
			{
				loaded = R_LoadMDM(mod, buffer, name);
			}
			else if(ident == MDX_IDENT)
			{
				loaded = R_LoadMDX(mod, buffer, name);
			}
#if defined(USE_REFENTITY_ANIMATIONSYSTEM)
			if(!Q_stricmpn((const char *)buffer, "MD5Version", 10))
			{
				loaded = R_LoadMD5(mod, buffer, bufferLen, name);
			}
			else if(!Q_stricmpn((const char *)buffer, PSK_IDENTSTRING, PSK_IDENTLEN))
			{
				loaded = R_LoadPSK(mod, buffer, bufferLen, name);
			}
#endif
			ri.FS_FreeFile(buffer);
		}

		if(loaded)
		{
			// make sure the VBO glState entries are save
			R_BindNullVBO();
			R_BindNullIBO();

			return mod->index;
		}
	}

	for(lod = MD3_MAX_LODS - 1; lod >= 0; lod--)
	{
		char            filename[1024];

		strcpy(filename, name);

		if(lod != 0)
		{
			char            namebuf[80];

			if(strrchr(filename, '.'))
			{
				*strrchr(filename, '.') = 0;
			}
			sprintf(namebuf, "_%d.md3", lod);
			strcat(filename, namebuf);
		}

		filename[strlen(filename) - 1] = 'c';	// try MDC first
		ri.FS_ReadFile(filename, (void **)&buffer);

		if(!buffer)
		{
			filename[strlen(filename) - 1] = '3';	// try MD3 second
			ri.FS_ReadFile(filename, (void **)&buffer);
			if(!buffer)
			{
				continue;
			}
		}

		loadmodel = mod;

		ident = LittleLong(*(unsigned *)buffer);


		if(ident == MD3_IDENT)
		{
			loaded = R_LoadMD3(mod, lod, buffer, bufferLen, name, qfalse);
			ri.FS_FreeFile(buffer);
		}
		else if(ident == MDC_IDENT)
		{
			loaded = R_LoadMDC(mod, lod, buffer, bufferLen, name);
			ri.FS_FreeFile(buffer);
		}
		else
		{
			ri.FS_FreeFile(buffer);

			ri.Printf(PRINT_WARNING, "RE_RegisterModel: unknown fileid for %s\n", name);
			goto fail;
		}

		if(!loaded)
		{
			if(lod == 0)
			{
				goto fail;
			}
			else
			{
				break;
			}
		}
		else
		{
			// make sure the VBO glState entries are save
			R_BindNullVBO();
			R_BindNullIBO();

			mod->numLods++;
			numLoaded++;
			// if we have a valid model and are biased
			// so that we won't see any higher detail ones,
			// stop loading them
//          if ( lod <= r_lodbias->integer ) {
//              break;
//          }
		}
	}

	// make sure the VBO glState entries are save
	R_BindNullVBO();
	R_BindNullIBO();

	if(numLoaded)
	{
		// duplicate into higher lod spots that weren't
		// loaded, in case the user changes r_lodbias on the fly
		for(lod--; lod >= 0; lod--)
		{
			mod->numLods++;
			mod->mdv[lod] = mod->mdv[lod + 1];
		}

		return mod->index;
	}
#ifdef _DEBUG
	else
	{
		ri.Printf(PRINT_WARNING, "couldn't load '%s'\n", name);
	}
#endif

  fail:
	// we still keep the model_t around, so if the model name is asked for
	// again, we won't bother scanning the filesystem
	mod->type = MOD_BAD;

	// make sure the VBO glState entries are save
	R_BindNullVBO();
	R_BindNullIBO();

	return 0;
}



/*
=================
R_LoadMDM
=================
*/
static qboolean R_LoadMDM(model_t * mod, void *buffer, const char *mod_name)
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

/*
=================
R_LoadMDX
=================
*/
static qboolean R_LoadMDX(model_t * mod, void *buffer, const char *mod_name)
{
	int             i, j;
	mdxHeader_t    *pinmodel, *mdx;
	mdxFrame_t     *frame;
	short          *bframe;
	mdxBoneInfo_t  *bi;
	int             version;
	int             size;
	int             frameSize;

	pinmodel = (mdxHeader_t *) buffer;

	version = LittleLong(pinmodel->version);
	if(version != MDX_VERSION)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMDX: %s has wrong version (%i should be %i)\n", mod_name, version, MDX_VERSION);
		return qfalse;
	}

	mod->type = MOD_MDX;
	size = LittleLong(pinmodel->ofsEnd);
	mod->dataSize += size;
	mdx = mod->mdx = ri.Hunk_Alloc(size, h_low);

	memcpy(mdx, buffer, LittleLong(pinmodel->ofsEnd));

	LL(mdx->ident);
	LL(mdx->version);
	LL(mdx->numFrames);
	LL(mdx->numBones);
	LL(mdx->ofsFrames);
	LL(mdx->ofsBones);
	LL(mdx->ofsEnd);
	LL(mdx->torsoParent);

	if(LittleLong(1) != 1)
	{
		// swap all the frames
		frameSize = (int)(sizeof(mdxBoneFrameCompressed_t)) * mdx->numBones;
		for(i = 0; i < mdx->numFrames; i++)
		{
			frame = (mdxFrame_t *) ((byte *) mdx + mdx->ofsFrames + i * frameSize + i * sizeof(mdxFrame_t));
			frame->radius = LittleFloat(frame->radius);
			for(j = 0; j < 3; j++)
			{
				frame->bounds[0][j] = LittleFloat(frame->bounds[0][j]);
				frame->bounds[1][j] = LittleFloat(frame->bounds[1][j]);
				frame->localOrigin[j] = LittleFloat(frame->localOrigin[j]);
				frame->parentOffset[j] = LittleFloat(frame->parentOffset[j]);
			}

			bframe = (short *)((byte *) mdx + mdx->ofsFrames + i * frameSize + ((i + 1) * sizeof(mdxFrame_t)));
			for(j = 0; j < mdx->numBones * sizeof(mdxBoneFrameCompressed_t) / sizeof(short); j++)
			{
				((short *)bframe)[j] = LittleShort(((short *)bframe)[j]);
			}
		}

		// swap all the bones
		for(i = 0; i < mdx->numBones; i++)
		{
			bi = (mdxBoneInfo_t *) ((byte *) mdx + mdx->ofsBones + i * sizeof(mdxBoneInfo_t));
			LL(bi->parent);
			bi->torsoWeight = LittleFloat(bi->torsoWeight);
			bi->parentDist = LittleFloat(bi->parentDist);
			LL(bi->flags);
		}
	}

	return qtrue;
}

//=============================================================================

#if defined(USE_REFENTITY_ANIMATIONSYSTEM)











//=============================================================================






#endif // USE_REFENTITY_ANIMATIONSYSTEM

//=============================================================================

/*
=================
R_XMLError
=================
*/
void R_XMLError(void *ctx, const char *fmt, ...)
{
	va_list         argptr;
	static char     msg[4096];

	va_start(argptr, fmt);
	Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	ri.Printf(PRINT_WARNING, "%s", msg);
}

/*
=================
R_LoadDAE
=================
*/
/*
static qboolean R_LoadDAE(model_t * mod, void *buffer, int bufferLen, const char *modName)
{
	xmlDocPtr       doc;
	xmlNodePtr      node;

	// setup error function handler
	xmlInitParser();
	xmlSetGenericErrorFunc(NULL, R_XMLError);

	ri.Printf(PRINT_ALL, "...loading DAE '%s'\n", modName);

	doc = xmlParseMemory(buffer, bufferLen);
	if(doc == NULL)
	{
		ri.Printf(PRINT_WARNING, "R_LoadDAE: '%s' xmlParseMemory returned NULL\n", modName);
		return qfalse;
	}
	node = xmlDocGetRootElement(doc);

	if(node == NULL)
	{
		ri.Printf(PRINT_WARNING, "R_LoadDAE: '%s' empty document\n", modName);
		xmlFreeDoc(doc);
		return qfalse;
	}

	if(xmlStrcmp(node->name, (const xmlChar *) "COLLADA"))
	{
		ri.Printf(PRINT_WARNING, "R_LoadDAE: '%s' document of the wrong type, root node != COLLADA", modName);
		xmlFreeDoc(doc);
		return qfalse;
	}

	//TODO

	xmlFreeDoc(doc);

	ri.Printf(PRINT_ALL, "...finished DAE '%s'\n", modName);

	return qfalse;
}
*/

//=============================================================================

/*
** RE_BeginRegistration
*/
void RE_BeginRegistration(glconfig_t * glconfigOut)
{
	R_Init();

	*glconfigOut = glConfig;

	R_SyncRenderThread();

	tr.visIndex = 0;
	memset(tr.visClusters, -2, sizeof(tr.visClusters));	// force markleafs to regenerate

#if defined(USE_D3D10)
	// TODO
#else
	R_ClearFlares();
#endif

	RE_ClearScene();

	// HACK: give world entity white color for "colored" shader keyword
	tr.worldEntity.e.shaderRGBA[0] = 255;
	tr.worldEntity.e.shaderRGBA[1] = 255;
	tr.worldEntity.e.shaderRGBA[2] = 255;
	tr.worldEntity.e.shaderRGBA[3] = 255;

	// FIXME: world entity shadows always use zfail algorithm which is slower than zpass
	tr.worldEntity.needZFail = qtrue;
	tr.worldEntity.e.nonNormalizedAxes = qfalse;

	tr.registered = qtrue;

	// NOTE: this sucks, for some reason the first stretch pic is never drawn
	// without this we'd see a white flash on a level load because the very
	// first time the level shot would not be drawn
	RE_StretchPic(0, 0, 0, 0, 0, 0, 1, 1, 0);
}

//=============================================================================

/*
===============
R_ModelInit
===============
*/
void R_ModelInit(void)
{
	model_t        *mod;

	// leave a space for NULL model
	tr.numModels = 0;

	mod = R_AllocModel();
	mod->type = MOD_BAD;
}


/*
================
R_Modellist_f
================
*/
void R_Modellist_f(void)
{
	int             i, j, k;
	model_t        *mod;
	int             total;
	int             totalDataSize;
	qboolean        showFrames;

	if(!strcmp(ri.Cmd_Argv(1), "frames"))
	{
		showFrames = qtrue;
	}
	else
	{
		showFrames = qfalse;
	}

	total = 0;
	totalDataSize = 0;
	for(i = 1; i < tr.numModels; i++)
	{
		mod = tr.models[i];
		
		
		if(mod->type == MOD_MESH)
		{
			for(j = 0; j < MD3_MAX_LODS; j++)
			{
				if(mod->mdv[j] && mod->mdv[j] != mod->mdv[j - 1])
				{
					mdvModel_t			*mdvModel;
					mdvSurface_t		*mdvSurface;
					mdvTagName_t		*mdvTagName;

					mdvModel = mod->mdv[j];

					total++;
					ri.Printf(PRINT_ALL, "%d.%02d MB '%s' LOD = %i\n",	mod->dataSize / (1024 * 1024), 
															(mod->dataSize % (1024 * 1024)) * 100 / (1024 * 1024),
															mod->name, j);

					if(showFrames && mdvModel->numFrames > 1)
					{
						ri.Printf(PRINT_ALL, "\tnumSurfaces = %i\n", mdvModel->numSurfaces);
						ri.Printf(PRINT_ALL, "\tnumFrames = %i\n", mdvModel->numFrames);
						
						for(k = 0, mdvSurface = mdvModel->surfaces; k < mdvModel->numSurfaces; k++, mdvSurface++)
						{
							ri.Printf(PRINT_ALL, "\t\tmesh = '%s'\n", mdvSurface->name);
							ri.Printf(PRINT_ALL, "\t\t\tnumVertexes = %i\n", mdvSurface->numVerts);
							ri.Printf(PRINT_ALL, "\t\t\tnumTriangles = %i\n", mdvSurface->numTriangles);
						}
					}

					ri.Printf(PRINT_ALL, "\t\tnumTags = %i\n", mdvModel->numTags);
					for(k = 0, mdvTagName = mdvModel->tagNames; k < mdvModel->numTags; k++, mdvTagName++)
					{
						ri.Printf(PRINT_ALL, "\t\t\ttagName = '%s'\n", mdvTagName->name);
					}
				}
			}
		}
		else
		{
			ri.Printf(PRINT_ALL, "%d.%02d MB '%s'\n",	mod->dataSize / (1024 * 1024), 
																(mod->dataSize % (1024 * 1024)) * 100 / (1024 * 1024),
																mod->name);

			total++;
		}

		totalDataSize += mod->dataSize;
	}
	
	ri.Printf(PRINT_ALL, " %d.%02d MB total model memory\n", totalDataSize / (1024 * 1024),
			  (totalDataSize % (1024 * 1024)) * 100 / (1024 * 1024));
	ri.Printf(PRINT_ALL, " %i total models\n\n", total);

#if	0							// not working right with new hunk
	if(tr.world)
	{
		ri.Printf(PRINT_ALL, "\n%8i : %s\n", tr.world->dataSize, tr.world->name);
	}
#endif
}


//=============================================================================


/*
================
R_GetTag
================
*/
static int R_GetTag(mdvModel_t * model, int frame, const char *_tagName, int startTagIndex, mdvTag_t ** outTag)
{
	int             i;
	mdvTag_t       *tag;
	mdvTagName_t   *tagName;

	// it is possible to have a bad frame while changing models, so don't error
	frame = Q_bound(0, frame, model->numFrames - 1);

	if(startTagIndex > model->numTags)
	{
		*outTag = NULL;
		return -1;
	}

#if 1
	tag = model->tags + frame * model->numTags;
	tagName = model->tagNames;
	for(i = 0; i < model->numTags; i++, tag++, tagName++)
	{
		if((i >= startTagIndex) && !strcmp(tagName->name, _tagName))
		{
			*outTag = tag;
			return i;
		}
	}
#endif

	*outTag = NULL;
	return -1;
}

/*
================
RE_LerpTag
================
*/
int RE_LerpTag(orientation_t * tag, const refEntity_t * refent, const char *tagNameIn, int startIndex)
{
	mdvTag_t       *start, *end;
	int             i;
	float           frontLerp, backLerp;
	model_t        *model;
	char            tagName[MAX_QPATH];	//, *ch;
	int             retval;
	qhandle_t       handle;
	int             startFrame, endFrame;
	float           frac;

	handle = refent->hModel;
	startFrame = refent->oldframe;
	endFrame = refent->frame;
	frac = 1.0 - refent->backlerp;

	Q_strncpyz(tagName, tagNameIn, MAX_QPATH);
/*
	// if the tagName has a space in it, then it is passing through the starting tag number
	if (ch = strrchr(tagName, ' ')) {
		*ch = 0;
		ch++;
		startIndex = atoi(ch);
	}
*/
	model = R_GetModelByHandle(handle);
	/*
	if(!model->mdv[0]) //if(!model->model.md3[0] && !model->model.mdc[0] && !model->model.mds)
	{
		AxisClear(tag->axis);
		VectorClear(tag->origin);
		return -1;
	}
	*/

	frontLerp = frac;
	backLerp = 1.0 - frac;

	start = end = NULL;

	if(model->type == MOD_MESH)
	{
		// old MD3 style
		retval = R_GetTag(model->mdv[0], startFrame, tagName, startIndex, &start);
		retval = R_GetTag(model->mdv[0], endFrame, tagName, startIndex, &end);

	}
/*
	else if(model->type == MOD_MDS)
	{
		// use bone lerping
		retval = R_GetBoneTag(tag, model->model.mds, startIndex, refent, tagNameIn);

		if(retval >= 0)
		{
			return retval;
		}

		// failed
		return -1;

	}
	*/
	else if(model->type == MOD_MDM)
	{
		// use bone lerping
		retval = R_MDM_GetBoneTag(tag, model->mdm, startIndex, refent, tagNameIn);

		if(retval >= 0)
		{
			return retval;
		}

		// failed
		return -1;

	}
	/*
	else
	{
		// psuedo-compressed MDC tags
		mdcTag_t       *cstart, *cend;

		retval = R_GetMDCTag((byte *) model->model.mdc[0], startFrame, tagName, startIndex, &cstart);
		retval = R_GetMDCTag((byte *) model->model.mdc[0], endFrame, tagName, startIndex, &cend);

		// uncompress the MDC tags into MD3 style tags
		if(cstart && cend)
		{
			for(i = 0; i < 3; i++)
			{
				ustart.origin[i] = (float)cstart->xyz[i] * MD3_XYZ_SCALE;
				uend.origin[i] = (float)cend->xyz[i] * MD3_XYZ_SCALE;
				sangles[i] = (float)cstart->angles[i] * MDC_TAG_ANGLE_SCALE;
				eangles[i] = (float)cend->angles[i] * MDC_TAG_ANGLE_SCALE;
			}

			AnglesToAxis(sangles, ustart.axis);
			AnglesToAxis(eangles, uend.axis);

			start = &ustart;
			end = &uend;
		}
		else
		{
			start = NULL;
			end = NULL;
		}
	}
	*/

	if(!start || !end)
	{
		AxisClear(tag->axis);
		VectorClear(tag->origin);
		return -1;
	}

	for(i = 0; i < 3; i++)
	{
		tag->origin[i] = start->origin[i] * backLerp + end->origin[i] * frontLerp;
		tag->axis[0][i] = start->axis[0][i] * backLerp + end->axis[0][i] * frontLerp;
		tag->axis[1][i] = start->axis[1][i] * backLerp + end->axis[1][i] * frontLerp;
		tag->axis[2][i] = start->axis[2][i] * backLerp + end->axis[2][i] * frontLerp;
	}

	VectorNormalize(tag->axis[0]);
	VectorNormalize(tag->axis[1]);
	VectorNormalize(tag->axis[2]);

	return retval;
}

/*
================
RE_BoneIndex
================
*/
int RE_BoneIndex(qhandle_t hModel, const char *boneName)
{
	int             i;
	md5Bone_t      *bone;
	md5Model_t     *md5;
	model_t        *model;

	model = R_GetModelByHandle(hModel);
	if(!model->md5)
	{
		return -1;
	}
	else
	{
		md5 = model->md5;
	}

	for(i = 0, bone = md5->bones; i < md5->numBones; i++, bone++)
	{
		if(!Q_stricmp(bone->name, boneName))
		{
			return i;
		}
	}

	return -1;
}



/*
====================
R_ModelBounds
====================
*/
void R_ModelBounds(qhandle_t handle, vec3_t mins, vec3_t maxs)
{
	model_t        *model;
	mdvModel_t     *header;
	mdvFrame_t     *frame;

	model = R_GetModelByHandle(handle);

	if(model->bsp)
	{
		VectorCopy(model->bsp->bounds[0], mins);
		VectorCopy(model->bsp->bounds[1], maxs);
	}
	else if(model->mdv[0])
	{
		header = model->mdv[0];

		frame = header->frames;

		VectorCopy(frame->bounds[0], mins);
		VectorCopy(frame->bounds[1], maxs);
	}
	else if(model->md5)
	{
		VectorCopy(model->md5->bounds[0], mins);
		VectorCopy(model->md5->bounds[1], maxs);
	}
	else
	{
		VectorClear(mins);
		VectorClear(maxs);
	}
}
