/*
===========================================================================
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

/* genericSingle_vp.glsl */

attribute vec4		attr_Position;
attribute vec4		attr_TexCoord0;
attribute vec3		attr_Normal;
attribute vec4		attr_Color;
#if defined(r_VertexSkinning)
attribute vec4		attr_BoneIndexes;
attribute vec4		attr_BoneWeights;
uniform int			u_VertexSkinning;
uniform mat4		u_BoneMatrix[MAX_GLSL_BONES];
#endif

uniform mat4		u_ColorTextureMatrix;
uniform vec3		u_ViewOrigin;
uniform int			u_TCGen_Environment;
uniform int			u_DeformGen;
uniform vec4		u_DeformWave;	// [base amplitude phase freq]
uniform vec3		u_DeformBulge;	// [width height speed]
uniform float		u_DeformSpread;
uniform float		u_Time;
uniform int			u_ColorGen;
uniform int			u_AlphaGen;
uniform vec4		u_Color;
uniform mat4		u_ModelMatrix;
//uniform mat4		u_ProjectionMatrix;
uniform mat4		u_ModelViewProjectionMatrix;

varying vec3		var_Position;
varying vec2		var_Tex;
varying vec4		var_Color;



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
	vec4 position = vec4(0.0);

#if defined(r_VertexSkinning)
	if(bool(u_VertexSkinning))
	{
		for(int i = 0; i < 4; i++)
		{
			int boneIndex = int(attr_BoneIndexes[i]);
			float boneWeight = attr_BoneWeights[i];
			mat4  boneMatrix = u_BoneMatrix[boneIndex];
			
			position += (boneMatrix * attr_Position) * boneWeight;
		}
		
		position = DeformPosition(position, attr_Normal, attr_TexCoord0.st);

		// transform vertex position into homogenous clip-space
		gl_Position = u_ModelViewProjectionMatrix * position;
		
		// transform position into world space
		var_Position = (u_ModelMatrix * position).xyz;
	}
	else
#endif
	{
		position = DeformPosition(attr_Position, attr_Normal, attr_TexCoord0.st);
	
		// transform vertex position into homogenous clip-space
		gl_Position = u_ModelViewProjectionMatrix * position;
		
		// transform position into world space
		var_Position = (u_ModelMatrix * position).xyz;
	}
	
	// transform texcoords
	if(bool(u_TCGen_Environment))
	{
		vec3 viewer = normalize(u_ViewOrigin - position.xyz);

		float d = dot(attr_Normal, viewer);

		vec3 reflected = attr_Normal * 2.0 * d - viewer;
		
		var_Tex.s = 0.5 + reflected.y * 0.5;
		var_Tex.t = 0.5 - reflected.z * 0.5;
	}
	else
	{
		var_Tex = (u_ColorTextureMatrix * attr_TexCoord0).st;
	}
	
	// assign color
	if(u_ColorGen == CGEN_VERTEX)
	{
		var_Color.r = attr_Color.r;
		var_Color.g = attr_Color.g;
		var_Color.b = attr_Color.b;
	}
	else if(u_ColorGen == CGEN_ONE_MINUS_VERTEX)
	{
		var_Color.r = 1.0 - attr_Color.r;
		var_Color.g = 1.0 - attr_Color.g;
		var_Color.b = 1.0 - attr_Color.b;
	}
	else
	{
		var_Color.rgb = u_Color.rgb;
	}
	
	if(u_AlphaGen == AGEN_VERTEX)
	{
		var_Color.a = attr_Color.a;
	}
	else if(u_AlphaGen == AGEN_ONE_MINUS_VERTEX)
	{
		var_Color.a = 1.0 - attr_Color.a;
	}
	else
	{
		var_Color.a = u_Color.a;
	}
}
