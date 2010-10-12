/*
===========================================================================
Copyright (C) 2010 Robert Beckebans <trebor_7@users.sourceforge.net>

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
// gl_shader.cpp -- GLSL shader handling

#include "gl_shader.h"

// *INDENT-OFF*

void		 GLSL_InitGPUShader3(shaderProgram_t * program,
								const char *vertexMainShader,
								const char *fragmentMainShader,
								const char *vertexLibShaders,
								const char *fragmentLibShaders,
								const char *compileMacros,
								int attribs,
								qboolean optimize);

void		GLSL_ValidateProgram(GLhandleARB program);
void		GLSL_ShowProgramUniforms(GLhandleARB program);


GLShader_vertexLighting_DBS_entity* gl_vertexLightingShader_DBS_entity = NULL;



GLShader_vertexLighting_DBS_entity::GLShader_vertexLighting_DBS_entity():
		GLShader(MAX_PERMUTATIONS),
		u_AlphaTest(this),
		u_AmbientColor(this),
		u_ViewOrigin(this),
		u_LightDir(this),
		u_LightColor(this),
		u_ModelMatrix(this),
		u_ModelViewProjectionMatrix(this),
		u_BoneMatrix(this),
		u_VertexInterpolation(this),
		u_PortalPlane(this),
		GLDeformStage(this)
{
	char compileMacros[4096];

	
	//Com_Memset(_shaderPrograms, 0, sizeof(_shaderPrograms));

	for(int i = 0; i < MAX_PERMUTATIONS; i++)
	{
		Com_Memset(compileMacros, 0, sizeof(compileMacros));

		if(i & USE_PORTAL_CLIPPING)
		{
			Q_strcat(compileMacros, sizeof(compileMacros), "USE_PORTAL_CLIPPING ");
		}

		if(i & USE_ALPHA_TESTING)
		{
			Q_strcat(compileMacros, sizeof(compileMacros), "USE_ALPHA_TESTING ");
		}

		if(i & USE_VERTEX_SKINNING)
		{
			Q_strcat(compileMacros, sizeof(compileMacros), "USE_VERTEX_SKINNING ");
		}

		if(i & USE_VERTEX_ANIMATION)
		{
			Q_strcat(compileMacros, sizeof(compileMacros), "USE_VERTEX_ANIMATION ");
		}

		if(i & USE_DEFORM_VERTEXES)
		{
			Q_strcat(compileMacros, sizeof(compileMacros), "USE_DEFORM_VERTEXES ");
		}

		ri.Printf(PRINT_ALL, "Compile macros: '%s'\n", compileMacros);

		shaderProgram_t *shaderProgram = &_shaderPrograms[i];

		GLSL_InitGPUShader3(shaderProgram,
						"vertexLighting_DBS_entity",
						"vertexLighting_DBS_entity",
						"vertexSkinning vertexAnimation deformVertexes",
						"",
						compileMacros,
						ATTR_POSITION | ATTR_TEXCOORD | ATTR_TANGENT | ATTR_BINORMAL | ATTR_NORMAL |
						ATTR_POSITION2 | ATTR_TANGENT2 | ATTR_BINORMAL2 | ATTR_NORMAL2,
						qtrue);

		shaderProgram->u_DiffuseMap = glGetUniformLocationARB(shaderProgram->program, "u_DiffuseMap");
		shaderProgram->u_NormalMap =
			glGetUniformLocationARB(shaderProgram->program, "u_NormalMap");
		shaderProgram->u_SpecularMap =
			glGetUniformLocationARB(shaderProgram->program, "u_SpecularMap");
		shaderProgram->u_DiffuseTextureMatrix =
			glGetUniformLocationARB(shaderProgram->program, "u_DiffuseTextureMatrix");
		shaderProgram->u_NormalTextureMatrix =
			glGetUniformLocationARB(shaderProgram->program, "u_NormalTextureMatrix");
		shaderProgram->u_SpecularTextureMatrix =
			glGetUniformLocationARB(shaderProgram->program, "u_SpecularTextureMatrix");
		shaderProgram->u_AlphaTest =
			glGetUniformLocationARB(shaderProgram->program, "u_AlphaTest");
		shaderProgram->u_DeformGen =
			glGetUniformLocationARB(shaderProgram->program, "u_DeformGen");
		shaderProgram->u_DeformWave =
			glGetUniformLocationARB(shaderProgram->program, "u_DeformWave");
		shaderProgram->u_DeformBulge =
			glGetUniformLocationARB(shaderProgram->program, "u_DeformBulge");
		shaderProgram->u_DeformSpread =
			glGetUniformLocationARB(shaderProgram->program, "u_DeformSpread");
		shaderProgram->u_ViewOrigin =
			glGetUniformLocationARB(shaderProgram->program, "u_ViewOrigin");
		shaderProgram->u_AmbientColor =
			glGetUniformLocationARB(shaderProgram->program, "u_AmbientColor");
		shaderProgram->u_LightDir =
			glGetUniformLocationARB(shaderProgram->program, "u_LightDir");
		shaderProgram->u_LightColor =
			glGetUniformLocationARB(shaderProgram->program, "u_LightColor");
		shaderProgram->u_ParallaxMapping =
			glGetUniformLocationARB(shaderProgram->program, "u_ParallaxMapping");
		shaderProgram->u_DepthScale =
			glGetUniformLocationARB(shaderProgram->program, "u_DepthScale");
		shaderProgram->u_PortalClipping =
			glGetUniformLocationARB(shaderProgram->program, "u_PortalClipping");
		shaderProgram->u_PortalPlane =
			glGetUniformLocationARB(shaderProgram->program, "u_PortalPlane");
		shaderProgram->u_ModelMatrix =
			glGetUniformLocationARB(shaderProgram->program, "u_ModelMatrix");
		shaderProgram->u_ModelViewProjectionMatrix =
			glGetUniformLocationARB(shaderProgram->program, "u_ModelViewProjectionMatrix");
		shaderProgram->u_Time =
			glGetUniformLocationARB(shaderProgram->program, "u_Time");
		if(glConfig2.vboVertexSkinningAvailable)
		{
			shaderProgram->u_VertexSkinning =
				glGetUniformLocationARB(shaderProgram->program, "u_VertexSkinning");
			shaderProgram->u_BoneMatrix =
				glGetUniformLocationARB(shaderProgram->program, "u_BoneMatrix");
		}
		shaderProgram->u_VertexInterpolation =
			glGetUniformLocationARB(shaderProgram->program, "u_VertexInterpolation");

		glUseProgramObjectARB(shaderProgram->program);
		glUniform1iARB(shaderProgram->u_DiffuseMap, 0);
		glUniform1iARB(shaderProgram->u_NormalMap, 1);
		glUniform1iARB(shaderProgram->u_SpecularMap, 2);
		glUseProgramObjectARB(0);

		GLSL_ValidateProgram(shaderProgram->program);
		GLSL_ShowProgramUniforms(shaderProgram->program);
		GL_CheckErrors();
	}
}