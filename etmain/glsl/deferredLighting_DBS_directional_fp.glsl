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

/* deferredLighting_DBS_directional_fp.glsl */

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_DepthMap;
uniform sampler2D	u_AttenuationMapXY;
uniform sampler2D	u_AttenuationMapZ;
uniform sampler2D	u_ShadowMap0;
uniform sampler2D	u_ShadowMap1;
uniform sampler2D	u_ShadowMap2;
uniform sampler2D	u_ShadowMap3;
uniform sampler2D	u_ShadowMap4;
uniform vec3		u_ViewOrigin;
uniform vec3		u_LightOrigin;
uniform vec3		u_LightDir;
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float       u_LightScale;
uniform mat4		u_LightAttenuationMatrix;
uniform mat4		u_ShadowMatrix[MAX_SHADOWMAPS];
uniform int			u_ShadowCompare;
uniform vec4		u_ShadowParallelSplitDistances;
uniform int         u_PortalClipping;
uniform vec4		u_PortalPlane;
uniform mat4		u_ViewMatrix;
uniform mat4		u_UnprojectMatrix;


/*
Some explanations by Marco Salvi about exponential shadow mapping:

Now you are filtering exponential values which rapidly go out of range,
to avoid this issue you can filter the logarithm of these values (and the go back to exp space)

For example if you averaging two exponential value such as exp(A) and exp(B) you have:

a*exp(A) + b*exp(B) (a and b are some filter weights)

but you can rewrite the same expression as:

exp(A) * (a + b*exp(B-A)) ,

exp(A) * exp( log (a + b*exp(B-A)))),

and:

exp(A + log(a + b*exp(B-A))

Now your sum of exponential is written as a single exponential, if you take the logarithm of it you can then just work on its argument:

A + log(a + b*exp(B-A))

Basically you end up filtering the argument of your exponential functions, which are just linear depth values,
so you enjoy the same range you have with less exotic techniques.
Just don't forget to go back to exp space when you use the final filtered value.


Though hardware texture filtering is not mathematically correct in log space it just causes a some overdarkening, nothing major.

If you have your shadow map filtered in log space occlusion is just computed like this (let assume we use bilinear filtering):

float occluder = tex2D( esm_sampler, esm_uv );
float occlusion = exp( occluder - receiver );

while with filtering in exp space you have:

float exp_occluder = tex2D( esm_sampler, esm_uv );
float occlusion = exp_occluder / exp( receiver );

EDIT: if more complex filters are used (trilinear, aniso, with mip maps) you need to generate mip maps using log filteirng as well.
*/

float log_conv(float x0, float X, float y0, float Y)
{
    return (X + log(x0 + (y0 * exp(Y - X))));
}


void	main()
{
	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec2 st = gl_FragCoord.st * r_FBufScale;
	
	// scale by the screen non-power-of-two-adjust
	st *= r_NPOTScale;
		
	// reconstruct vertex position in world space
	float depth = texture2D(u_DepthMap, st).r;
	vec4 P = u_UnprojectMatrix * vec4(gl_FragCoord.xy, depth, 1.0);
	P.xyz /= P.w;
	
	// transform to camera space
	vec4 Pcam = u_ViewMatrix * vec4(P.xyz, 1.0);
	
	if(bool(u_PortalClipping))
	{
		float dist = dot(P.xyz, u_PortalPlane.xyz) - u_PortalPlane.w;
		if(dist < 0.0)
		{
			discard;
			return;
		}
	}
	
	// transform vertex position into light space
	vec4 texAtten			= u_LightAttenuationMatrix * vec4(P.xyz, 1.0);
	if(texAtten.q <= 0.0)
	{
		// point is behind the near clip plane
		discard;
		return;
	}

	float shadow = 1.0;
	
#if defined(VSM)
	if(bool(u_ShadowCompare))
	{
		vec4 shadowVert;
		
		float vertexDistanceToCamera = -Pcam.z;
		
		vec4 shadowMoments;
#if defined(r_ParallelShadowSplits_1)
		if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
		else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[1] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
			#endif
		}
#elif defined(r_ParallelShadowSplits_2)
		if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[1] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
			#endif
			
		}
		else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[2] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
			#endif
		}
#elif defined(r_ParallelShadowSplits_3)
		if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[1] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
			#endif
			
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.z)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[2] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
			#endif
		}
		else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[3] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap3, shadowVert.xyw);
			#endif
		}
#elif defined(r_ParallelShadowSplits_4)
		if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[1] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
			#endif
			
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.z)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[2] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
			#endif
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.w)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[3] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap3, shadowVert.xyw);
			#endif
		}
		else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[4] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap4, shadowVert.xyw);
			#endif
		}
#else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
#endif

		shadowVert.xyz /= shadowVert.w;
		
		const float	SHADOW_BIAS = 0.001;
		float vertexDistance = shadowVert.z - SHADOW_BIAS;
	
		#if defined(VSM_CLAMP)
		// convert to [-1, 1] vector space
		shadowMoments = 2.0 * (shadowMoments - 0.5);
		#endif
		
		float shadowDistance = shadowMoments.r;
		float shadowDistanceSquared = shadowMoments.a;
	
		// standard shadow map comparison
		shadow = vertexDistance <= shadowDistance ? 1.0 : 0.0;
	
		// variance shadow mapping
		float E_x2 = shadowDistanceSquared;
		float Ex_2 = shadowDistance * shadowDistance;
	
		// AndyTX: VSM_EPSILON is there to avoid some ugly numeric instability with fp16
		float variance = min(max(E_x2 - Ex_2, 0.0) + VSM_EPSILON, 1.0);
		//float variance = smoothstep(VSM_EPSILON, 1.0, max(E_x2 - Ex_2, 0.0));
	
		float mD = shadowDistance - vertexDistance;
		float mD_2 = mD * mD;
		float p = variance / (variance + mD_2);
		p = smoothstep(0.0, 1.0, p);
	
		#if defined(DEBUG_VSM)
		#extension GL_EXT_gpu_shader4 : enable
		gl_FragColor.r = (DEBUG_VSM & 1) != 0 ? variance : 0.0;
		gl_FragColor.g = (DEBUG_VSM & 2) != 0 ? mD_2 : 0.0;
		gl_FragColor.b = (DEBUG_VSM & 4) != 0 ? p : 0.0;
		gl_FragColor.a = 1.0;
		return;
		#else
		shadow = max(shadow, p);
		#endif
	}
	
	if(shadow <= 0.0)
	{
		discard;
		return;
	}
	else
#elif defined(ESM)
	if(bool(u_ShadowCompare))
	{
		vec4 shadowVert;
		
		float vertexDistanceToCamera = -Pcam.z;
		
		vec4 shadowMoments;
#if defined(r_ParallelShadowSplits_1)
		if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
		else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[1] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
			#endif
		}
#elif defined(r_ParallelShadowSplits_2)
		if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[1] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
			#endif
			
		}
		else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[2] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
			#endif
		}
#elif defined(r_ParallelShadowSplits_3)
		if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[1] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
			#endif
			
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.z)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[2] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
			#endif
		}
		else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[3] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap3, shadowVert.xyw);
			#endif
		}
#elif defined(r_ParallelShadowSplits_4)
		if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.x)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.y)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[1] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap1, shadowVert.xyw);
			#endif
			
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.z)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[2] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap2, shadowVert.xyw);
			#endif
		}
		else if(vertexDistanceToCamera < u_ShadowParallelSplitDistances.w)
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[3] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap3, shadowVert.xyw);
			#endif
		}
		else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[4] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap4, shadowVert.xyw);
			#endif
		}
#else
		{
			#if defined(r_ShowParallelShadowSplits)
			gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
			return;
			#else
			shadowVert = u_ShadowMatrix[0] * vec4(P.xyz, 1.0);
			shadowMoments = texture2DProj(u_ShadowMap0, shadowVert.xyw);
			#endif
		}
#endif

		shadowVert.xyz /= shadowVert.w;
		
		const float	SHADOW_BIAS = 0.001;
		float vertexDistance = shadowVert.z;// * r_ShadowMapDepthScale;
		//vertexDistance /= shadowVert.w;
		
		float shadowDistance = shadowMoments.a;
		
		// exponential shadow mapping
		#if 0
		shadow = vertexDistance <= shadowDistance ? 1.0 : 0.0;
		#else
		//shadow = clamp(exp(r_OverDarkeningFactor * (shadowDistance - vertexDistance)), 0.0, 1.0);
		shadow = clamp(exp(r_OverDarkeningFactor * (shadowDistance - vertexDistance)), 0.0, 1.0);
		#endif
		//shadow = smoothstep(0.0, 1.0, shadow);
		
		#if defined(DEBUG_ESM)
		#extension GL_EXT_gpu_shader4 : enable
		gl_FragColor.r = (DEBUG_ESM & 1) != 0 ? shadowDistance : 0.0;// vertexDistance;
		gl_FragColor.g = (DEBUG_ESM & 2) != 0 ? -(shadowDistance - vertexDistance) : 0.0;
		gl_FragColor.b = (DEBUG_ESM & 4) != 0 ? shadow : 0.0;
		gl_FragColor.a = 1.0;
		return;
		#endif
	}
	
	if(shadow <= 0.0)
	{
		discard;
		return;
	}
	else
#endif
	{
	
#if !defined(r_DeferredLighting)
		// compute the diffuse term
		vec4 diffuse = texture2D(u_DiffuseMap, st);
#endif

		// compute normal in world space
		vec3 N = 2.0 * (texture2D(u_NormalMap, st).xyz - 0.5);
		
		N = normalize(N);
	
		// compute light direction in world space
		vec3 L = u_LightDir;
	
#if defined(r_NormalMapping)
		// compute view direction in world space
		vec3 V = normalize(u_ViewOrigin - P.xyz);
	
		// compute half angle in world space
		vec3 H = normalize(L + V);
		
#if !defined(r_DeferredLighting)
		vec4 S = texture2D(u_SpecularMap, st);
#endif

#endif // r_NormalMapping
	
		// compute attenuation
		//vec3 attenuationXY = texture2DProj(u_AttenuationMapXY, texAtten.xyw).rgb;
		//vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(0.5 + texAtten.z, 0.0)).rgb; // FIXME
	
		// compute final color
#if defined(r_DeferredLighting)
		vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
#else
		vec4 color = diffuse;
#endif
		
		color.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
		
#if defined(r_NormalMapping)

#if defined(r_DeferredLighting)
		//color.a += pow(clamp(dot(N, H), 0.0, 1.0), r_SpecularExponent) * r_SpecularScale;
#else
		color.rgb += S.rgb * u_LightColor * pow(clamp(dot(N, H), 0.0, 1.0), r_SpecularExponent) * r_SpecularScale;
#endif

#endif // r_NormalMapping

		//color.rgb *= attenuationXY;
		//color.rgb *= attenuationZ;
		color.rgb *= u_LightScale;
		color.rgb *= shadow;
		//color.rgb = vec3(shadow);
		
		gl_FragColor = color;
	}
}
