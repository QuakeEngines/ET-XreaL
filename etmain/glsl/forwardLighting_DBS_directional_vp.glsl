/*
===========================================================================
Copyright (C) 2009 Robert Beckebans <trebor_7@users.sourceforge.net>

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

/* forwardLighting_DBS_directional_vp.glsl */

attribute vec4		attr_Position;
attribute vec4		attr_TexCoord0;
attribute vec3		attr_Tangent;
attribute vec3		attr_Binormal;
attribute vec3		attr_Normal;
attribute vec4		attr_Color;
#if defined(r_VertexSkinning)
attribute vec4		attr_BoneIndexes;
attribute vec4		attr_BoneWeights;
uniform int			u_VertexSkinning;
uniform mat4		u_BoneMatrix[MAX_GLSL_BONES];
#endif

uniform mat4		u_DiffuseTextureMatrix;
uniform mat4		u_NormalTextureMatrix;
uniform mat4		u_SpecularTextureMatrix;
//uniform int			u_InverseVertexColor;
uniform mat4		u_LightAttenuationMatrix;
uniform int			u_ShadowCompare;
uniform mat4		u_ShadowMatrix[MAX_SHADOWMAPS];
uniform vec4		u_ShadowParallelSplitDistances;
uniform mat4		u_ModelMatrix;
uniform mat4		u_ModelViewMatrix;
uniform mat4		u_ModelViewProjectionMatrix;

uniform int			u_DeformGen;
uniform vec4		u_DeformWave;	// [base amplitude phase freq]
uniform vec3		u_DeformBulge;	// [width height speed]
uniform float		u_DeformSpread;
uniform float		u_Time;

varying vec4		var_Position;
varying vec4		var_TexDiffuse;
varying vec4		var_TexNormal;
#if defined(r_NormalMapping)
varying vec2		var_TexSpecular;
#endif
varying vec4		var_TexAtten;
varying vec4		var_Tangent;
varying vec4		var_Binormal;
varying vec4		var_Normal;


float triangle(float x)
{
	return max(1.0 - abs(x), 0);
}

float sawtooth(float x)
{
	return x - floor(x);
}

vec4 DeformPosition(const vec4 pos, const vec3 normal, const vec2 st)
{
	vec4 deformed = pos;
	
	/*
		define	WAVEVALUE( table, base, amplitude, phase, freq ) \
			((base) + table[ Q_ftol( ( ( (phase) + backEnd.refdef.floatTime * (freq) ) * FUNCTABLE_SIZE ) ) & FUNCTABLE_MASK ] * (amplitude))
	*/

	if(u_DeformGen == DGEN_WAVE_SIN)
	{
		float off = (pos.x + pos.y + pos.z) * u_DeformSpread;
		float scale = u_DeformWave.x  + sin(off + u_DeformWave.z + (u_Time * u_DeformWave.w)) * u_DeformWave.y;
		vec3 offset = normal * scale;

		deformed.xyz += offset;
	}
	
	if(u_DeformGen == DGEN_WAVE_SQUARE)
	{
		float off = (pos.x + pos.y + pos.z) * u_DeformSpread;
		float scale = u_DeformWave.x  + sign(sin(off + u_DeformWave.z + (u_Time * u_DeformWave.w))) * u_DeformWave.y;
		vec3 offset = normal * scale;

		deformed.xyz += offset;
	}
	
	if(u_DeformGen == DGEN_WAVE_TRIANGLE)
	{
		float off = (pos.x + pos.y + pos.z) * u_DeformSpread;
		float scale = u_DeformWave.x  + triangle(off + u_DeformWave.z + (u_Time * u_DeformWave.w)) * u_DeformWave.y;
		vec3 offset = normal * scale;

		deformed.xyz += offset;
	}
	
	if(u_DeformGen == DGEN_WAVE_SAWTOOTH)
	{
		float off = (pos.x + pos.y + pos.z) * u_DeformSpread;
		float scale = u_DeformWave.x  + sawtooth(off + u_DeformWave.z + (u_Time * u_DeformWave.w)) * u_DeformWave.y;
		vec3 offset = normal * scale;

		deformed.xyz += offset;
	}
	
	if(u_DeformGen == DGEN_WAVE_INVERSE_SAWTOOTH)
	{
		float off = (pos.x + pos.y + pos.z) * u_DeformSpread;
		float scale = u_DeformWave.x + (1.0 - sawtooth(off + u_DeformWave.z + (u_Time * u_DeformWave.w))) * u_DeformWave.y;
		vec3 offset = normal * scale;

		deformed.xyz += offset;
	}
	
	if(u_DeformGen == DGEN_BULGE)
	{
		float bulgeWidth = u_DeformBulge.x;
		float bulgeHeight = u_DeformBulge.y;
		float bulgeSpeed = u_DeformBulge.z;
	
		float now = u_Time * bulgeSpeed;

		float off = (M_PI * 0.25) * st.x * bulgeWidth + now; 
		float scale = sin(off) * bulgeHeight;
		vec3 offset = normal * scale;

		deformed.xyz += offset;
	}

	return deformed;
}

void	main()
{
	vec4 position;

#if defined(r_VertexSkinning)
	if(bool(u_VertexSkinning))
	{
		position = vec4(0.0); 
		vec3 tangent = vec3(0.0);
		vec3 binormal = vec3(0.0);
		vec3 normal = vec3(0.0);

		for(int i = 0; i < 4; i++)
		{
			int boneIndex = int(attr_BoneIndexes[i]);
			float boneWeight = attr_BoneWeights[i];
			mat4  boneMatrix = u_BoneMatrix[boneIndex];
			
			position += (boneMatrix * attr_Position) * boneWeight;
		
			tangent += (boneMatrix * vec4(attr_Tangent, 0.0)).xyz * boneWeight;
			binormal += (boneMatrix * vec4(attr_Binormal, 0.0)).xyz * boneWeight;
			normal += (boneMatrix * vec4(attr_Normal, 0.0)).xyz * boneWeight;
		}
		
		position = DeformPosition(position, attr_Normal, attr_TexCoord0.st);

		// transform vertex position into homogenous clip-space
		gl_Position = u_ModelViewProjectionMatrix * position;
		
		// transform position into world space
		var_Position = u_ModelMatrix * position;
		
		#if defined(r_NormalMapping)
		var_Tangent.xyz = (u_ModelMatrix * vec4(tangent, 0.0)).xyz;
		var_Binormal.xyz = (u_ModelMatrix * vec4(binormal, 0.0)).xyz;
		#endif
		
		var_Normal.xyz = (u_ModelMatrix * vec4(normal, 0.0)).xyz;
		
		// calc light attenuation in light space
		var_TexAtten = u_LightAttenuationMatrix * position;
	}
	else
#endif
	{
		position = DeformPosition(attr_Position, attr_Normal, attr_TexCoord0.st);
	
		// transform vertex position into homogenous clip-space
		gl_Position = u_ModelViewProjectionMatrix * position;
		
		// transform position into world space
		var_Position = u_ModelMatrix * position;
	
		#if defined(r_NormalMapping)
		var_Tangent.xyz = (u_ModelMatrix * vec4(attr_Tangent, 0.0)).xyz;
		var_Binormal.xyz = (u_ModelMatrix * vec4(attr_Binormal, 0.0)).xyz;
		#endif
		
		var_Normal.xyz = (u_ModelMatrix * vec4(attr_Normal, 0.0)).xyz;
		
		// calc light attenuation in light space
		var_TexAtten = u_LightAttenuationMatrix * position;
	}
	
	// transform diffusemap texcoords
	var_TexDiffuse.st = (u_DiffuseTextureMatrix * attr_TexCoord0).st;
	
#if defined(r_NormalMapping)
	// transform normalmap texcoords
	var_TexNormal.st = (u_NormalTextureMatrix * attr_TexCoord0).st;
	
	// transform specularmap texture coords
	var_TexSpecular = (u_SpecularTextureMatrix * attr_TexCoord0).st;
#endif
	
	// assign color
	/*
	if(bool(u_InverseVertexColor))
	{
		var_TexDiffuse.p = 1.0 - attr_Color.r;
		var_TexNormal.p = 1.0 - attr_Color.g;
		var_TexNormal.q = 1.0 - attr_Color.b;
	}
	else
	*/
	{
		var_TexDiffuse.p = attr_Color.r;
		var_TexNormal.pq = attr_Color.gb;
	}
}
