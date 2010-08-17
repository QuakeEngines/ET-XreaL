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

/* forwardLighting_DBS_post_fp.glsl */

uniform sampler2D	u_LightMap;
uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_SpecularMap;
uniform int			u_AlphaTest;
uniform vec3		u_ViewOrigin;
uniform vec3        u_AmbientColor;
uniform int			u_ParallaxMapping;
uniform float		u_DepthScale;

varying vec2		var_TexDiffuse;
#if defined(r_NormalMapping)
varying vec2		var_TexNormal;
varying vec2		var_TexSpecular;
#endif

#if defined(r_ParallaxMapping)
varying vec4		var_Position;
varying vec4		var_Tangent;
varying vec4		var_Binormal;
varying vec4		var_Normal;
#endif


#if defined(r_ParallaxMapping)
float RayIntersectDisplaceMap(vec2 dp, vec2 ds)
{
	const int linearSearchSteps = 16;
	const int binarySearchSteps = 6;

	float depthStep = 1.0 / float(linearSearchSteps);

	// current size of search window
	float size = depthStep;

	// current depth position
	float depth = 0.0;

	// best match found (starts with last position 1.0)
	float bestDepth = 1.0;

	// search front to back for first point inside object
	for(int i = 0; i < linearSearchSteps - 1; ++i)
	{
		depth += size;
		
		vec4 t = texture2D(u_NormalMap, dp + ds * depth);

		if(bestDepth > 0.996)		// if no depth found yet
			if(depth >= t.w)
				bestDepth = depth;	// store best depth
	}

	depth = bestDepth;
	
	// recurse around first point (depth) for closest match
	for(int i = 0; i < binarySearchSteps; ++i)
	{
		size *= 0.5;

		vec4 t = texture2D(u_NormalMap, dp + ds * depth);
		
		if(depth >= t.w)
		#ifdef RM_DOUBLEDEPTH
			if(depth <= t.z)
		#endif
			{
				bestDepth = depth;
				depth -= 2.0 * size;
			}

		depth += size;
	}

	return bestDepth;
}
#endif

void	main()
{
	vec2 texDiffuse = var_TexDiffuse.st;

#if defined(r_NormalMapping)
	vec2 texNormal = var_TexNormal.st;
	vec2 texSpecular = var_TexSpecular.st;
#endif

#if defined(r_ParallaxMapping)
	if(bool(u_ParallaxMapping))
	{
		mat3 worldToTangentMatrix;
		if(gl_FrontFacing)
			worldToTangentMatrix = mat3(-var_Tangent.x, -var_Binormal.x, -var_Normal.x,
										-var_Tangent.y, -var_Binormal.y, -var_Normal.y, 
										-var_Tangent.z, -var_Binormal.z, -var_Normal.x);
		else
			worldToTangentMatrix = mat3(var_Tangent.x, var_Binormal.x, var_Normal.x,
										var_Tangent.y, var_Binormal.y, var_Normal.y, 
										var_Tangent.z, var_Binormal.z, var_Normal.x);
	
		// compute view direction in tangent space
		vec3 V = worldToTangentMatrix * (u_ViewOrigin - var_Position.xyz);
		V = normalize(V);
		
		// ray intersect in view direction
		
		// size and start position of search in texture space
		vec2 S = V.xy * -u_DepthScale / V.z;
			
#if 1
		vec2 texOffset = vec2(0.0);
		for(int i = 0; i < 4; i++) {
			vec4 Normal = texture2D(u_NormalMap, texNormal.st + texOffset);
			float height = Normal.a * 0.2 - 0.0125;
			texOffset += height * Normal.z * S;
		}
#else
		float depth = RayIntersectDisplaceMap(texNormal, S);
		
		// compute texcoords offset
		vec2 texOffset = S * depth;
#endif
		
		texNormal.st += texOffset;
		texDiffuse.st += texOffset;
		texSpecular.st += texOffset;
	}
#endif
	
#if 1
	vec4 diffuse = texture2D(u_DiffuseMap, texDiffuse);
	if(u_AlphaTest == ATEST_GT_0 && diffuse.a <= 0.0)
	{
		discard;
		return;
	}
	else if(u_AlphaTest == ATEST_LT_128 && diffuse.a >= 0.5)
	{
		discard;
		return;
	}
	else if(u_AlphaTest == ATEST_GE_128 && diffuse.a < 0.5)
	{
		discard;
		return;
	}
#endif

	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec2 fragCoord = gl_FragCoord.st * r_FBufScale;
	
	// scale by the screen non-power-of-two-adjust
	fragCoord *= r_NPOTScale;
	
	vec4 light = texture2D(u_LightMap, fragCoord);
	//light.rgb += light.aaa;
	
	// reconstruct the light equation
	vec4 color = vec4(u_AmbientColor.rgb + (diffuse.rgb * light.rgb), diffuse.a);
	//color = diffuse;
	//color.rgb = light.rgb;
	
	const vec4 LUMINANCE_VECTOR = vec4(0.2125, 0.7154, 0.0721, 0.0);
	float Y = dot(LUMINANCE_VECTOR, light);
	
#if defined(r_NormalMapping)
	vec3 specular = texture2D(u_SpecularMap, texSpecular).rgb ;
	
	specular *= Y;
	specular *= light.rgb;
	//specular *= (light.rgb + light.aaa);
	
	//specular *= pow(light.a / Y, r_SpecularExponent);
	specular *= light.a;// / Y;
	//specular = vec3(light.a);
	
	specular *= r_SpecularScale;// * 2.0;
	color.rgb += specular;
	//color.rgb = specular;
#endif

	gl_FragColor = color;
}


