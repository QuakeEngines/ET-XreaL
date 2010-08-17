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

/* deluxeMapping_vp.glsl */

attribute vec4		attr_Position;
attribute vec4		attr_TexCoord0;
attribute vec4		attr_TexCoord1;
attribute vec3		attr_Tangent;
attribute vec3		attr_Binormal;
attribute vec3		attr_Normal;

uniform mat4		u_DiffuseTextureMatrix;
uniform mat4		u_NormalTextureMatrix;
uniform mat4		u_SpecularTextureMatrix;
uniform mat4		u_ModelMatrix;
uniform mat4		u_ModelViewProjectionMatrix;

uniform int			u_DeformGen;
uniform vec4		u_DeformWave;	// [base amplitude phase freq]
uniform vec3		u_DeformBulge;	// [width height speed]
uniform float		u_DeformSpread;
uniform float		u_Time;

varying vec3		var_Position;
varying vec2		var_TexDiffuse;
varying vec2		var_TexNormal;
varying vec2		var_TexSpecular;
varying vec2		var_TexLight;
varying vec3		var_Tangent;
varying vec3		var_Binormal;
varying vec3		var_Normal;


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
	vec4 position = DeformPosition(attr_Position, attr_Normal, attr_TexCoord0.st);

	// transform vertex position into homogenous clip-space
	gl_Position = u_ModelViewProjectionMatrix * position;
	
	// transform position into world space
	var_Position = (u_ModelMatrix * position).xyz;
	
	// transform diffusemap texcoords
	var_TexDiffuse = (u_DiffuseTextureMatrix * attr_TexCoord0).st;
	
	// transform normalmap texcoords
	var_TexNormal = (u_NormalTextureMatrix * attr_TexCoord0).st;
	
	// transform specularmap texcoords
	var_TexSpecular = (u_SpecularTextureMatrix * attr_TexCoord0).st;
	
	// transform lightmap texcoords
	var_TexLight = attr_TexCoord1.st;
	
	var_Tangent.xyz = (u_ModelMatrix * vec4(attr_Tangent, 0.0)).xyz;
	var_Binormal.xyz = (u_ModelMatrix * vec4(attr_Binormal, 0.0)).xyz;
	var_Normal.xyz = (u_ModelMatrix * vec4(attr_Normal, 0.0)).xyz;
}
