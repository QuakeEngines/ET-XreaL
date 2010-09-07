/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2010 Robert Beckebans <trebor_7@users.sourceforge.net>

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


/*
=================
MDXSurfaceCompare
compare function for qsort()
=================
*/
static int MDXSurfaceCompare(const void *a, const void *b)
{
	mdvSurface_t   *aa, *bb;

	aa = *(mdvSurface_t **) a;
	bb = *(mdvSurface_t **) b;

	// shader first
	if(&aa->shader < &bb->shader)
		return -1;

	else if(&aa->shader > &bb->shader)
		return 1;

	return 0;
}

/*
=================
R_LoadMD3
=================
*/
qboolean R_LoadMD3(model_t * mod, int lod, void *buffer, int bufferSize, const char *modName, qboolean forceStatic)
{
	int             i, j, k, l;

	md3Header_t    *md3Model;
	md3Frame_t     *md3Frame;
	md3Surface_t   *md3Surf;
	md3Shader_t    *md3Shader;
	md3Triangle_t  *md3Tri;
	md3St_t        *md3st;
	md3XyzNormal_t *md3xyz;
	md3Tag_t       *md3Tag;

	mdvModel_t     *mdvModel;
	mdvFrame_t     *frame;
	mdvSurface_t   *surf, *surface;
	srfTriangle_t  *tri;
	mdvVertex_t    *v;
	mdvSt_t        *st;
	mdvTag_t       *tag;
	mdvTagName_t   *tagName;

	int             version;
	int             size;

	md3Model = (md3Header_t *) buffer;

	version = LittleLong(md3Model->version);
	if(version != MD3_VERSION)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMD3: %s has wrong version (%i should be %i)\n", modName, version, MD3_VERSION);
		return qfalse;
	}

	mod->type = MOD_MESH;
	size = LittleLong(md3Model->ofsEnd);
	mod->dataSize += size;
	mdvModel = mod->mdv[lod] = ri.Hunk_Alloc(sizeof(mdvModel_t), h_low);

//  Com_Memcpy(mod->md3[lod], buffer, LittleLong(md3Model->ofsEnd));

	LL(md3Model->ident);
	LL(md3Model->version);
	LL(md3Model->numFrames);
	LL(md3Model->numTags);
	LL(md3Model->numSurfaces);
	LL(md3Model->ofsFrames);
	LL(md3Model->ofsTags);
	LL(md3Model->ofsSurfaces);
	LL(md3Model->ofsEnd);

	if(md3Model->numFrames < 1)
	{
		ri.Printf(PRINT_WARNING, "R_LoadMD3: %s has no frames\n", modName);
		return qfalse;
	}

	// swap all the frames
	mdvModel->numFrames = md3Model->numFrames;
	mdvModel->frames = frame = ri.Hunk_Alloc(sizeof(*frame) * md3Model->numFrames, h_low);

	md3Frame = (md3Frame_t *) ((byte *) md3Model + md3Model->ofsFrames);
	for(i = 0; i < md3Model->numFrames; i++, frame++, md3Frame++)
	{
		frame->radius = LittleFloat(md3Frame->radius);
		for(j = 0; j < 3; j++)
		{
			frame->bounds[0][j] = LittleFloat(md3Frame->bounds[0][j]);
			frame->bounds[1][j] = LittleFloat(md3Frame->bounds[1][j]);
			frame->localOrigin[j] = LittleFloat(md3Frame->localOrigin[j]);
		}
	}

	// swap all the tags
	mdvModel->numTags = md3Model->numTags;
	mdvModel->tags = tag = ri.Hunk_Alloc(sizeof(*tag) * (md3Model->numTags * md3Model->numFrames), h_low);

	md3Tag = (md3Tag_t *) ((byte *) md3Model + md3Model->ofsTags);
	for(i = 0; i < md3Model->numTags * md3Model->numFrames; i++, tag++, md3Tag++)
	{
		for(j = 0; j < 3; j++)
		{
			tag->origin[j] = LittleFloat(md3Tag->origin[j]);
			tag->axis[0][j] = LittleFloat(md3Tag->axis[0][j]);
			tag->axis[1][j] = LittleFloat(md3Tag->axis[1][j]);
			tag->axis[2][j] = LittleFloat(md3Tag->axis[2][j]);
		}
	}


	mdvModel->tagNames = tagName = ri.Hunk_Alloc(sizeof(*tagName) * (md3Model->numTags), h_low);

	md3Tag = (md3Tag_t *) ((byte *) md3Model + md3Model->ofsTags);
	for(i = 0; i < md3Model->numTags; i++, tagName++, md3Tag++)
	{
		Q_strncpyz(tagName->name, md3Tag->name, sizeof(tagName->name));
	}

	// swap all the surfaces
	mdvModel->numSurfaces = md3Model->numSurfaces;
	mdvModel->surfaces = surf = ri.Hunk_Alloc(sizeof(*surf) * md3Model->numSurfaces, h_low);

	md3Surf = (md3Surface_t *) ((byte *) md3Model + md3Model->ofsSurfaces);
	for(i = 0; i < md3Model->numSurfaces; i++)
	{
		LL(md3Surf->ident);
		LL(md3Surf->flags);
		LL(md3Surf->numFrames);
		LL(md3Surf->numShaders);
		LL(md3Surf->numTriangles);
		LL(md3Surf->ofsTriangles);
		LL(md3Surf->numVerts);
		LL(md3Surf->ofsShaders);
		LL(md3Surf->ofsSt);
		LL(md3Surf->ofsXyzNormals);
		LL(md3Surf->ofsEnd);

		if(md3Surf->numVerts > SHADER_MAX_VERTEXES)
		{
			ri.Error(ERR_DROP, "R_LoadMD3: %s has more than %i verts on a surface (%i)",
					 modName, SHADER_MAX_VERTEXES, md3Surf->numVerts);
		}
		if(md3Surf->numTriangles * 3 > SHADER_MAX_INDEXES)
		{
			ri.Error(ERR_DROP, "R_LoadMD3: %s has more than %i triangles on a surface (%i)",
					 modName, SHADER_MAX_INDEXES / 3, md3Surf->numTriangles);
		}

		// change to surface identifier
		surf->surfaceType = SF_MDV;

		// give pointer to model for Tess_SurfaceMDX
		surf->model = mdvModel;

		// copy surface name
		Q_strncpyz(surf->name, md3Surf->name, sizeof(surf->name));

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
		md3Shader = (md3Shader_t *) ((byte *) md3Surf + md3Surf->ofsShaders);
		surf->shader = R_FindShader(md3Shader->name, SHADER_3D_DYNAMIC, qtrue);

		// swap all the triangles
		surf->numTriangles = md3Surf->numTriangles;
		surf->triangles = tri = ri.Hunk_Alloc(sizeof(*tri) * md3Surf->numTriangles, h_low);

		md3Tri = (md3Triangle_t *) ((byte *) md3Surf + md3Surf->ofsTriangles);
		for(j = 0; j < md3Surf->numTriangles; j++, tri++, md3Tri++)
		{
			tri->indexes[0] = LittleLong(md3Tri->indexes[0]);
			tri->indexes[1] = LittleLong(md3Tri->indexes[1]);
			tri->indexes[2] = LittleLong(md3Tri->indexes[2]);
		}

		R_CalcSurfaceTriangleNeighbors(surf->numTriangles, surf->triangles);

		// swap all the XyzNormals
		surf->numVerts = md3Surf->numVerts;
		surf->verts = v = ri.Hunk_Alloc(sizeof(*v) * (md3Surf->numVerts * md3Surf->numFrames), h_low);

		md3xyz = (md3XyzNormal_t *) ((byte *) md3Surf + md3Surf->ofsXyzNormals);
		for(j = 0; j < md3Surf->numVerts * md3Surf->numFrames; j++, md3xyz++, v++)
		{
			v->xyz[0] = LittleShort(md3xyz->xyz[0]) * MD3_XYZ_SCALE;
			v->xyz[1] = LittleShort(md3xyz->xyz[1]) * MD3_XYZ_SCALE;
			v->xyz[2] = LittleShort(md3xyz->xyz[2]) * MD3_XYZ_SCALE;
		}

		// swap all the ST
		surf->st = st = ri.Hunk_Alloc(sizeof(*st) * md3Surf->numVerts, h_low);

		md3st = (md3St_t *) ((byte *) md3Surf + md3Surf->ofsSt);
		for(j = 0; j < md3Surf->numVerts; j++, md3st++, st++)
		{
			st->st[0] = LittleFloat(md3st->st[0]);
			st->st[1] = LittleFloat(md3st->st[1]);
		}

		// find the next surface
		md3Surf = (md3Surface_t *) ((byte *) md3Surf + md3Surf->ofsEnd);
		surf++;
	}

	// build static VBO surfaces
#if defined(USE_D3D10)
	// TODO
#else
	if(r_vboModels->integer && forceStatic)
	{
		int             vertexesNum;
		byte           *data;
		int             dataSize;
		int             dataOfs;

		GLuint          ofsTexCoords;
		GLuint          ofsTangents;
		GLuint          ofsBinormals;
		GLuint          ofsNormals;
		GLuint          ofsColors;

		int             indexesNum;
		byte           *indexes;
		int             indexesSize;
		int             indexesOfs;

		shader_t       *shader, *oldShader;

		int             numSurfaces;
		mdvSurface_t  **surfacesSorted;

		vec4_t          tmp;
		int             index;

		static vec3_t   xyzs[SHADER_MAX_VERTEXES];
		static vec2_t   texcoords[SHADER_MAX_VERTEXES];
		static vec3_t   tangents[SHADER_MAX_VERTEXES];
		static vec3_t   binormals[SHADER_MAX_VERTEXES];
		static vec3_t   normals[SHADER_MAX_VERTEXES];
		static int      indexes2[SHADER_MAX_INDEXES];

		growList_t      vboSurfaces;
		srfVBOMesh_t   *vboSurf;

		vec4_t          tmpColor = { 1, 1, 1, 1 };


		//ri.Printf(PRINT_ALL, "...trying to calculate VBOs for model '%s'\n", modName);

		// count number of surfaces that we want to merge
		numSurfaces = 0;
		for(i = 0, surf = mdvModel->surfaces; i < mdvModel->numSurfaces; i++, surf++)
		{
			// remove all deformVertexes surfaces
			shader = surf->shader;
			if(shader->numDeforms)
				continue;

			numSurfaces++;
		}

		// build surfaces list
		surfacesSorted = ri.Hunk_AllocateTempMemory(numSurfaces * sizeof(surfacesSorted[0]));

		for(i = 0, surf = mdvModel->surfaces; i < numSurfaces; i++, surf++)
		{
			surfacesSorted[i] = surf;
		}

		// sort interaction caches by shader
		qsort(surfacesSorted, numSurfaces, sizeof(surfacesSorted), MDXSurfaceCompare);

		// create a VBO for each shader
		shader = oldShader = NULL;

		Com_InitGrowList(&vboSurfaces, 10);

		for(k = 0; k < numSurfaces; k++)
		{
			surf = surfacesSorted[k];
			shader = surf->shader;

			if(shader != oldShader)
			{
				oldShader = shader;

				// count vertices and indices
				vertexesNum = 0;
				indexesNum = 0;

				for(l = k; l < numSurfaces; l++)
				{
					surface = surfacesSorted[l];

					if(surface->shader != shader)
						continue;

					indexesNum += surface->numTriangles * 3;
					vertexesNum += surface->numVerts;
				}

				if(!vertexesNum || !indexesNum)
					continue;

				//ri.Printf(PRINT_ALL, "...calculating MD3 mesh VBOs ( %s, %i verts %i tris )\n", shader->name, vertexesNum, indexesNum / 3);

				// create surface
				vboSurf = ri.Hunk_Alloc(sizeof(*vboSurf), h_low);
				Com_AddToGrowList(&vboSurfaces, vboSurf);

				vboSurf->surfaceType = SF_VBO_MESH;
				vboSurf->shader = shader;
				vboSurf->lightmapNum = -1;
				vboSurf->numIndexes = indexesNum;
				vboSurf->numVerts = vertexesNum;

				dataSize = vertexesNum * (sizeof(vec4_t) * 7);
				data = ri.Hunk_AllocateTempMemory(dataSize);
				dataOfs = 0;
				vertexesNum = 0;

				indexesSize = indexesNum * sizeof(int);
				indexes = ri.Hunk_AllocateTempMemory(indexesSize);
				indexesOfs = 0;
				indexesNum = 0;

				// build triangle indices
				for(l = k; l < numSurfaces; l++)
				{
					surface = surfacesSorted[l];

					if(surface->shader != shader)
						continue;

					// set up triangle indices
					if(surface->numTriangles)
					{
						srfTriangle_t  *tri;

						for(i = 0, tri = surface->triangles; i < surface->numTriangles; i++, tri++)
						{
							for(j = 0; j < 3; j++)
							{
								index = vertexesNum + tri->indexes[j];

								Com_Memcpy(indexes + indexesOfs, &index, sizeof(int));
								indexesOfs += sizeof(int);
							}

							for(j = 0; j < 3; j++)
							{
								indexes2[indexesNum + i * 3 + j] = vertexesNum + tri->indexes[j];
							}
						}

						indexesNum += surface->numTriangles * 3;
					}

					if(surface->numVerts)
						vertexesNum += surface->numVerts;
				}

				// don't forget to recount vertexesNum
				vertexesNum = 0;

				// feed vertex XYZ
				for(l = k; l < numSurfaces; l++)
				{
					surface = surfacesSorted[l];

					if(surface->shader != shader)
						continue;

					if(surface->numVerts)
					{
						// set up xyz array
						for(i = 0; i < surface->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								tmp[j] = surface->verts[i].xyz[j];// * MD3_XYZ_SCALE;
							}
							tmp[3] = 1;
							Com_Memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);
						}
					}
				}

				// feed vertex texcoords
				ofsTexCoords = dataOfs;
				for(l = k; l < numSurfaces; l++)
				{
					surface = surfacesSorted[l];

					if(surface->shader != shader)
						continue;

					if(surface->numVerts)
					{
						// set up xyz array
						for(i = 0; i < surface->numVerts; i++)
						{
							for(j = 0; j < 2; j++)
							{
								tmp[j] = surface->st[i].st[j];
							}
							tmp[2] = 0;
							tmp[3] = 1;
							Com_Memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
							dataOfs += sizeof(vec4_t);

						}
					}
				}

				// prepare positions and texcoords for tangent space calculations
				for(l = k; l < numSurfaces; l++)
				{
					surface = surfacesSorted[l];

					if(surface->shader != shader)
						continue;

					if(surface->numVerts)
					{
						// set up xyz array
						for(i = 0; i < surface->numVerts; i++)
						{
							for(j = 0; j < 3; j++)
							{
								xyzs[vertexesNum + i][j] = surface->verts[i].xyz[j] * MD3_XYZ_SCALE;
							}
							for(j = 0; j < 2; j++)
							{
								texcoords[vertexesNum + i][j] = surface->st[i].st[j];
							}
						}

						vertexesNum += surface->numVerts;
					}
				}

				// calc tangent spaces
				{
					float          *v;
					const float    *v0, *v1, *v2;
					const float    *t0, *t1, *t2;
					vec3_t          tangent;
					vec3_t          binormal;
					vec3_t          normal;

					for(i = 0; i < vertexesNum; i++)
					{
						VectorClear(tangents[i]);
						VectorClear(binormals[i]);
						VectorClear(normals[i]);
					}

					for(i = 0; i < indexesNum; i += 3)
					{
						v0 = xyzs[indexes2[i + 0]];
						v1 = xyzs[indexes2[i + 1]];
						v2 = xyzs[indexes2[i + 2]];

						t0 = texcoords[indexes2[i + 0]];
						t1 = texcoords[indexes2[i + 1]];
						t2 = texcoords[indexes2[i + 2]];

						R_CalcTangentSpace(tangent, binormal, normal, v0, v1, v2, t0, t1, t2);

						for(j = 0; j < 3; j++)
						{
							v = tangents[indexes2[i + j]];
							VectorAdd(v, tangent, v);

							v = binormals[indexes2[i + j]];
							VectorAdd(v, binormal, v);

							v = normals[indexes2[i + j]];
							VectorAdd(v, normal, v);
						}
					}

					for(i = 0; i < vertexesNum; i++)
					{
						VectorNormalize(tangents[i]);
						VectorNormalize(binormals[i]);
						VectorNormalize(normals[i]);
					}

					// do another extra smoothing for normals to avoid flat shading
					for(i = 0; i < vertexesNum; i++)
					{
						for(j = 0; j < vertexesNum; j++)
						{
							if(i == j)
								continue;

							if(VectorCompare(xyzs[i], xyzs[j]))
							{
								VectorAdd(normals[i], normals[j], normals[i]);
							}
						}

						VectorNormalize(normals[i]);
					}
				}

				// feed vertex tangents
				ofsTangents = dataOfs;
				for(i = 0; i < vertexesNum; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = tangents[i][j];
					}
					tmp[3] = 1;
					Com_Memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// feed vertex binormals
				ofsBinormals = dataOfs;
				for(i = 0; i < vertexesNum; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = binormals[i][j];
					}
					tmp[3] = 1;
					Com_Memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// feed vertex normals
				ofsNormals = dataOfs;
				for(i = 0; i < vertexesNum; i++)
				{
					for(j = 0; j < 3; j++)
					{
						tmp[j] = normals[i][j];
					}
					tmp[3] = 1;
					Com_Memcpy(data + dataOfs, (vec_t *) tmp, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				// feed vertex colors
				ofsColors = dataOfs;
				for(i = 0; i < vertexesNum; i++)
				{
					Com_Memcpy(data + dataOfs, tmpColor, sizeof(vec4_t));
					dataOfs += sizeof(vec4_t);
				}

				vboSurf->vbo =
					R_CreateVBO(va("staticMD3Mesh_VBO %i", vboSurfaces.currentElements), data, dataSize, VBO_USAGE_STATIC);
				vboSurf->vbo->ofsXYZ = 0;
				vboSurf->vbo->ofsTexCoords = ofsTexCoords;
				vboSurf->vbo->ofsLightCoords = ofsTexCoords;
				vboSurf->vbo->ofsTangents = ofsTangents;
				vboSurf->vbo->ofsBinormals = ofsBinormals;
				vboSurf->vbo->ofsNormals = ofsNormals;
				vboSurf->vbo->ofsColors = ofsColors;
				vboSurf->vbo->ofsLightCoords = ofsColors;		// not required anyway
				vboSurf->vbo->ofsLightDirections = ofsColors;	// not required anyway

				vboSurf->ibo =
					R_CreateIBO(va("staticMD3Mesh_IBO %i", vboSurfaces.currentElements), indexes, indexesSize,
								VBO_USAGE_STATIC);

				ri.Hunk_FreeTempMemory(indexes);
				ri.Hunk_FreeTempMemory(data);

				// megs
				/*
				   ri.Printf(PRINT_ALL, "md3 mesh data VBO size: %d.%02d MB\n", dataSize / (1024 * 1024),
				   (dataSize % (1024 * 1024)) * 100 / (1024 * 1024));
				   ri.Printf(PRINT_ALL, "md3 mesh tris VBO size: %d.%02d MB\n", indexesSize / (1024 * 1024),
				   (indexesSize % (1024 * 1024)) * 100 / (1024 * 1024));
				 */

			}
		}

		ri.Hunk_FreeTempMemory(surfacesSorted);

		// move VBO surfaces list to hunk
		mdvModel->numVBOSurfaces = vboSurfaces.currentElements;
		mdvModel->vboSurfaces = ri.Hunk_Alloc(mdvModel->numVBOSurfaces * sizeof(*mdvModel->vboSurfaces), h_low);

		for(i = 0; i < mdvModel->numVBOSurfaces; i++)
		{
			mdvModel->vboSurfaces[i] = (srfVBOMesh_t *) Com_GrowListElement(&vboSurfaces, i);
		}

		Com_DestroyGrowList(&vboSurfaces);

		//ri.Printf(PRINT_ALL, "%i MD3 VBO surfaces created\n", mdvModel->numVBOSurfaces);
	}
#endif // defined(USE_D3D10)

	return qtrue;
}

