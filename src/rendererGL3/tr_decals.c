/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).  

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

// tr_decal.c - ydnar
// handles projection of decals (nee marks) onto brush model surfaces

#include "tr_local.h"


// TODO


/*
RE_ProjectDecal()
creates a new decal projector from a triangle
projected polygons should be 3 or 4 points
if a single point is passed in (numPoints == 1) then the decal will be omnidirectional
omnidirectional decals use points[ 0 ] as center and projection[ 3 ] as radius
pass in lifeTime < 0 for a temporary mark
*/

void RE_ProjectDecal(qhandle_t hShader, int numPoints, vec3_t * points, vec4_t projection, vec4_t color, int lifeTime,
					 int fadeTime)
{
#if 0
	int             i;
	float           radius, iDist;
	vec3_t          xyz;
	vec4_t          omniProjection;
	decalVert_t     dv[4];
	decalProjector_t *dp, temp;


	/* first frame rendered does not have a valid decals list */
	if(tr.refdef.decalProjectors == NULL)
	{
		return;
	}

	/* dummy check */
	if(numPoints != 1 && numPoints != 3 && numPoints != 4)
	{
		ri.Printf(PRINT_WARNING, "WARNING: Invalid number of decal points (%d)\n", numPoints);
		return;
	}

	/* early outs */
	if(lifeTime == 0)
	{
		return;
	}
	if(projection[3] <= 0.0f)
	{
		return;
	}

	/* set times properly */
	if(lifeTime < 0 || fadeTime < 0)
	{
		lifeTime = 0;
		fadeTime = 0;
	}

	/* basic setup */
	temp.shader = R_GetShaderByHandle(hShader);	/* debug code */
	temp.numPlanes = temp.shader->entityMergable;
	temp.color[0] = color[0] * 255;
	temp.color[1] = color[1] * 255;
	temp.color[2] = color[2] * 255;
	temp.color[3] = color[3] * 255;
	temp.numPlanes = numPoints + 2;
	temp.fadeStartTime = tr.refdef.time + lifeTime - fadeTime;
	temp.fadeEndTime = temp.fadeStartTime + fadeTime;

	/* set up decal texcoords (fixme: support arbitrary projector st coordinates in trapcall) */
	dv[0].st[0] = 0.0f;
	dv[0].st[1] = 0.0f;
	dv[1].st[0] = 0.0f;
	dv[1].st[1] = 1.0f;
	dv[2].st[0] = 1.0f;
	dv[2].st[1] = 1.0f;
	dv[3].st[0] = 1.0f;
	dv[3].st[1] = 0.0f;

	/* omnidirectional? */
	if(numPoints == 1)
	{
		/* set up omnidirectional */
		numPoints = 4;
		temp.numPlanes = 6;
		temp.omnidirectional = qtrue;
		radius = projection[3];
		Vector4Set(omniProjection, 0.0f, 0.0f, -1.0f, radius * 2.0f);
		projection = omniProjection;
		iDist = 1.0f / (radius * 2.0f);

		/* set corner */
		VectorSet(xyz, points[0][0] - radius, points[0][1] - radius, points[0][2] + radius);

		/* make x axis texture matrix (yz) */
		VectorSet(temp.texMat[0][0], 0.0f, iDist, 0.0f);
		temp.texMat[0][0][3] = -DotProduct(temp.texMat[0][0], xyz);
		VectorSet(temp.texMat[0][1], 0.0f, 0.0f, iDist);
		temp.texMat[0][1][3] = -DotProduct(temp.texMat[0][1], xyz);

		/* make y axis texture matrix (xz) */
		VectorSet(temp.texMat[1][0], iDist, 0.0f, 0.0f);
		temp.texMat[1][0][3] = -DotProduct(temp.texMat[1][0], xyz);
		VectorSet(temp.texMat[1][1], 0.0f, 0.0f, iDist);
		temp.texMat[1][1][3] = -DotProduct(temp.texMat[1][1], xyz);

		/* make z axis texture matrix (xy) */
		VectorSet(temp.texMat[2][0], iDist, 0.0f, 0.0f);
		temp.texMat[2][0][3] = -DotProduct(temp.texMat[2][0], xyz);
		VectorSet(temp.texMat[2][1], 0.0f, iDist, 0.0f);
		temp.texMat[2][1][3] = -DotProduct(temp.texMat[2][1], xyz);

		/* setup decal points */
		VectorSet(dv[0].xyz, points[0][0] - radius, points[0][1] - radius, points[0][2] + radius);
		VectorSet(dv[1].xyz, points[0][0] - radius, points[0][1] + radius, points[0][2] + radius);
		VectorSet(dv[2].xyz, points[0][0] + radius, points[0][1] + radius, points[0][2] + radius);
		VectorSet(dv[3].xyz, points[0][0] + radius, points[0][1] - radius, points[0][2] + radius);
	}
	else
	{
		/* set up unidirectional */
		temp.omnidirectional = qfalse;

		/* set up decal points */
		VectorCopy(points[0], dv[0].xyz);
		VectorCopy(points[1], dv[1].xyz);
		VectorCopy(points[2], dv[2].xyz);
		VectorCopy(points[3], dv[3].xyz);

		/* make texture matrix */
		if(!MakeTextureMatrix(temp.texMat[0], projection, &dv[0], &dv[1], &dv[2]))
		{
			return;
		}
	}

	/* bound the projector */
	ClearBounds(temp.mins, temp.maxs);
	for(i = 0; i < numPoints; i++)
	{
		AddPointToBounds(dv[i].xyz, temp.mins, temp.maxs);
		VectorMA(dv[i].xyz, projection[3], projection, xyz);
		AddPointToBounds(xyz, temp.mins, temp.maxs);
	}

	/* make bounding sphere */
	VectorAdd(temp.mins, temp.maxs, temp.center);
	VectorScale(temp.center, 0.5f, temp.center);
	VectorSubtract(temp.maxs, temp.center, xyz);
	temp.radius = VectorLength(xyz);
	temp.radius2 = temp.radius * temp.radius;

	/* frustum cull the projector (fixme: this uses a stale frustum!) */
	if(R_CullPointAndRadius(temp.center, temp.radius) == CULL_OUT)
	{
		return;
	}

	/* make the front plane */
	if(!PlaneFromPoints(temp.planes[0], dv[0].xyz, dv[1].xyz, dv[2].xyz))
	{
		return;
	}

	/* make the back plane */
	VectorSubtract(vec3_origin, temp.planes[0], temp.planes[1]);
	VectorMA(dv[0].xyz, projection[3], projection, xyz);
	temp.planes[1][3] = DotProduct(xyz, temp.planes[1]);

	/* make the side planes */
	for(i = 0; i < numPoints; i++)
	{
		VectorMA(dv[i].xyz, projection[3], projection, xyz);
		if(!PlaneFromPoints(temp.planes[i + 2], dv[(i + 1) % numPoints].xyz, dv[i].xyz, xyz))
		{
			return;
		}
	}

	/* create a new projector */
	dp = &tr.refdef.decalProjectors[r_numDecalProjectors & DECAL_PROJECTOR_MASK];
	Com_Memcpy(dp, &temp, sizeof(*dp));

	/* we have a winner */
	r_numDecalProjectors++;
#endif
}






/*
RE_ClearDecals()
clears decals from the world and entities
*/

void RE_ClearDecals(void)
{
#if 0
	int             i, j;

	/* dummy check */
	if(tr.world == NULL || tr.world->numBModels <= 0)
	{
		return;
	}

	/* clear world decals */
	for(j = 0; j < MAX_WORLD_DECALS; j++)
		tr.world->bmodels[0].decals[j].shader = NULL;

	/* clear entity decals */
	for(i = 0; i < tr.world->numBModels; i++)
		for(j = 0; j < MAX_ENTITY_DECALS; j++)
			tr.world->bmodels[i].decals[j].shader = NULL;
#endif
}



