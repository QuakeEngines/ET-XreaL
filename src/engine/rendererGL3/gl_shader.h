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

#ifndef GL_SHADER_H
#define GL_SHADER_H

#include <vector>

#include "tr_local.h"

// *INDENT-OFF*

class GLUniform;

class GLShader
{
protected:
	int						_activeMacros;
	shaderProgram_t*		_currentProgram;

	std::vector<shaderProgram_t> _shaderPrograms;

	std::vector<GLUniform*>	_uniforms;
public:

	GLShader(int maxPermutations)
	{
		_activeMacros = 0;
		_currentProgram = NULL;

		_shaderPrograms = std::vector<shaderProgram_t>(maxPermutations);
	}

	void RegisterUniform(GLUniform* uniform)
	{
		_uniforms.push_back(uniform);
	}

	shaderProgram_t*		GetProgram() const			{ return _currentProgram; }

	void SelectProgram(int numMacros)
	{
		int index = 0;

		for(int i = 0; i < numMacros; i++)
		{
			if(_activeMacros & BIT(i))
				index += BIT(i);
		}

		_currentProgram = &_shaderPrograms[index];
	}

	void BindProgram()
	{
		GL_BindProgram(_currentProgram);
	}
};

class GLUniform
{
protected:
	GLShader*				_parent;

	GLUniform(GLShader* parent):
	  _parent(parent)
	{
		_parent->RegisterUniform(this);
	}

	virtual const char* GetName() const = 0;
};






class u_ColorTextureMatrix:
GLUniform
{
public:
	u_ColorTextureMatrix(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_ColorTextureMatrix"; }

	void SetUniform_ColorTextureMatrix(const matrix_t m)
	{
		GLSL_SetUniform_ColorTextureMatrix(_parent->GetProgram(), m);
	}
};

class u_DiffuseTextureMatrix:
GLUniform
{
public:
	u_DiffuseTextureMatrix(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_DiffuseTextureMatrix"; }

	void SetUniform_DiffuseTextureMatrix(const matrix_t m)
	{
		GLSL_SetUniform_DiffuseTextureMatrix(_parent->GetProgram(), m);
	}
};

class u_NormalTextureMatrix:
GLUniform
{
public:
	u_NormalTextureMatrix(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_NormalTextureMatrix"; }

	void SetUniform_NormalTextureMatrix(const matrix_t m)
	{
		GLSL_SetUniform_NormalTextureMatrix(_parent->GetProgram(), m);
	}
};

class u_SpecularTextureMatrix:
GLUniform
{
public:
	u_SpecularTextureMatrix(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_SpecularTextureMatrix"; }

	void SetUniform_SpecularTextureMatrix(const matrix_t m)
	{
		GLSL_SetUniform_SpecularTextureMatrix(_parent->GetProgram(), m);
	}
};


class u_AlphaTest:
GLUniform
{
public:
	u_AlphaTest(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_AlphaTest"; }

	void SetUniform_AlphaTest(uint32_t stateBits)
	{
		GLSL_SetUniform_AlphaTest(_parent->GetProgram(), stateBits);
	}
};


class u_AmbientColor:
GLUniform
{
public:
	u_AmbientColor(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_AmbientColor"; }

	void SetUniform_AmbientColor(const vec3_t v)
	{
		GLSL_SetUniform_AmbientColor(_parent->GetProgram(), v);
	}
};


class u_ViewOrigin:
GLUniform
{
public:
	u_ViewOrigin(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_ViewOrigin"; }

	void SetUniform_ViewOrigin(const vec3_t v)
	{
		GLSL_SetUniform_ViewOrigin(_parent->GetProgram(), v);
	}
};


class u_LightDir:
GLUniform
{
public:
	u_LightDir(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_LightDir"; }

	void SetUniform_LightDir(const vec3_t v)
	{
		GLSL_SetUniform_LightDir(_parent->GetProgram(), v);
	}
};


class u_LightColor:
GLUniform
{
public:
	u_LightColor(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_LightColor"; }

	void SetUniform_LightColor(const vec3_t v)
	{
		GLSL_SetUniform_LightColor(_parent->GetProgram(), v);
	}
};


class u_ModelMatrix:
GLUniform
{
public:
	u_ModelMatrix(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_ModelMatrix"; }

	void SetUniform_ModelMatrix(const matrix_t m)
	{
		GLSL_SetUniform_ModelMatrix(_parent->GetProgram(), m);
	}
};


class u_ModelViewProjectionMatrix:
GLUniform
{
public:
	u_ModelViewProjectionMatrix(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_ModelViewProjectionMatrix"; }

	void SetUniform_ModelViewProjectionMatrix(const matrix_t m)
	{
		GLSL_SetUniform_ModelViewProjectionMatrix(_parent->GetProgram(), m);
	}
};


class u_BoneMatrix:
GLUniform
{
public:
	u_BoneMatrix(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_BoneMatrix"; }

	void SetUniform_BoneMatrix(int numBones, const matrix_t boneMatrices[MAX_BONES])
	{
		glUniformMatrix4fvARB(_parent->GetProgram()->u_BoneMatrix, numBones, GL_FALSE, &boneMatrices[0][0]);
	}
};


class u_VertexInterpolation:
GLUniform
{
public:
	u_VertexInterpolation(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_VertexInterpolation"; }

	void SetUniform_VertexInterpolation(float value)
	{
		GLSL_SetUniform_VertexInterpolation(_parent->GetProgram(), value);
	}
};


class u_PortalPlane:
GLUniform
{
public:
	u_PortalPlane(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_PortalPlane"; }

	void SetUniform_PortalPlane(const vec4_t v)
	{
		GLSL_SetUniform_PortalPlane(_parent->GetProgram(), v);
	}
};

class u_DepthScale:
GLUniform
{
public:
	u_DepthScale(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_DepthScale"; }

	void SetUniform_DepthScale(float value)
	{
		GLSL_SetUniform_DepthScale(_parent->GetProgram(), value);
	}
};



class u_DeformGen:
GLUniform
{
public:
	u_DeformGen(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_DeformGen"; }

	void SetUniform_DeformGen(deformGen_t value)
	{
		GLSL_SetUniform_DeformGen(_parent->GetProgram(), value);
	}
};

class u_DeformWave:
GLUniform
{
public:
	u_DeformWave(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_DeformWave"; }

	void SetUniform_DeformWave(const waveForm_t * wf)
	{
		GLSL_SetUniform_DeformWave(_parent->GetProgram(), wf);
	}
};

class u_DeformSpread:
GLUniform
{
public:
	u_DeformSpread(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_DeformSpread"; }

	void SetUniform_DeformSpread(float value)
	{
		GLSL_SetUniform_DeformSpread(_parent->GetProgram(), value);
	}
};

class u_DeformBulge:
GLUniform
{
public:
	u_DeformBulge(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_DeformBulge"; }

	void SetUniform_DeformBulge(deformStage_t *ds)
	{
		GLSL_SetUniform_DeformBulge(_parent->GetProgram(), ds);
	}
};


class u_Time:
GLUniform
{
public:
	u_Time(GLShader* parent):
	  GLUniform(parent)
	{
	}

	const char* GetName() const { return "u_Time"; }

	void SetUniform_Time(float value)
	{
		GLSL_SetUniform_DeformSpread(_parent->GetProgram(), value);
	}
};



class GLDeformStage:
u_DeformGen,
u_DeformWave,
u_DeformSpread,
u_DeformBulge,
u_Time
{
public:
	GLDeformStage(GLShader* parent):
	  u_DeformGen(parent),
	  u_DeformWave(parent),
	  u_DeformSpread(parent),
	  u_DeformBulge(parent),
	  u_Time(parent)
	{

	}

	void SetDeformStageUniforms(deformStage_t *ds)
	{
		switch (ds->deformation)
		{
			case DEFORM_WAVE:
				SetUniform_DeformGen((deformGen_t) ds->deformationWave.func);
				SetUniform_DeformWave(&ds->deformationWave);
				SetUniform_DeformSpread(ds->deformationSpread);
				SetUniform_Time(backEnd.refdef.floatTime);
				break;

			case DEFORM_BULGE:
				SetUniform_DeformGen(DGEN_BULGE);
				SetUniform_DeformBulge(ds);
				SetUniform_Time(backEnd.refdef.floatTime);
				break;

			default:
				SetUniform_DeformGen(DGEN_NONE);
				break;
		}
	}
};











class GLShader_lightMapping:
public GLShader,
public u_DiffuseTextureMatrix,
public u_AlphaTest,
public u_ModelViewProjectionMatrix,
public u_PortalPlane,
public GLDeformStage
{
private:
	
	enum
	{
		USE_PORTAL_CLIPPING = BIT(0),
		USE_ALPHA_TESTING = BIT(1),
		USE_DEFORM_VERTEXES = BIT(2),

		ALL_COMPILE_FLAGS = USE_PORTAL_CLIPPING |
							USE_ALPHA_TESTING |
							USE_DEFORM_VERTEXES,

		NUM_MACROS = 3,
		MAX_PERMUTATIONS = (1 << NUM_MACROS)	// same as 2^NUM_MACROS
	};


public:
	GLShader_lightMapping();
	
public:

	void EnablePortalClipping()					{	_activeMacros |= USE_PORTAL_CLIPPING; SelectProgram(NUM_MACROS); }
	void EnableAlphaTesting()					{	_activeMacros |= USE_ALPHA_TESTING; SelectProgram(NUM_MACROS); }
	void EnableDeformVertexes()					{	_activeMacros |= USE_DEFORM_VERTEXES; SelectProgram(NUM_MACROS); }

	void DisablePortalClipping()				{	_activeMacros &= ~USE_PORTAL_CLIPPING; SelectProgram(NUM_MACROS); }
	void DisableAlphaTesting()					{	_activeMacros &= ~USE_ALPHA_TESTING; SelectProgram(NUM_MACROS); }
	void DisableDeformVertexes()				{	_activeMacros &= ~USE_DEFORM_VERTEXES; SelectProgram(NUM_MACROS); }

		
};



class GLShader_vertexLighting_DBS_entity:
public GLShader,
public u_AlphaTest,
public u_AmbientColor,
public u_ViewOrigin,
public u_LightDir,
public u_LightColor,
public u_ModelMatrix,
public u_ModelViewProjectionMatrix,
public u_BoneMatrix,
public u_VertexInterpolation,
public u_PortalPlane,
public u_DepthScale,
public GLDeformStage
{
private:
	
	enum
	{
		USE_PORTAL_CLIPPING = BIT(0),
		USE_ALPHA_TESTING = BIT(1),
		USE_VERTEX_SKINNING = BIT(2),
		USE_VERTEX_ANIMATION = BIT(3),
		USE_DEFORM_VERTEXES = BIT(4),
		USE_PARALLAX_MAPPING = BIT(5),

		ALL_COMPILE_FLAGS = USE_PORTAL_CLIPPING |
							USE_ALPHA_TESTING |
							USE_VERTEX_SKINNING |
							USE_VERTEX_ANIMATION |
							USE_DEFORM_VERTEXES |
							USE_PARALLAX_MAPPING,

		NUM_MACROS = 6,
		MAX_PERMUTATIONS = (1 << NUM_MACROS)	// same as 2^NUM_MACROS
	};


public:
	GLShader_vertexLighting_DBS_entity();
	
public:

	void EnablePortalClipping()					{	_activeMacros |= USE_PORTAL_CLIPPING; SelectProgram(NUM_MACROS); }
	void EnableAlphaTesting()					{	_activeMacros |= USE_ALPHA_TESTING; SelectProgram(NUM_MACROS); }
	void EnableVertexSkinning()					{	_activeMacros |= USE_VERTEX_SKINNING; SelectProgram(NUM_MACROS); }
	void EnableVertexAnimation()				{	_activeMacros |= USE_VERTEX_ANIMATION; SelectProgram(NUM_MACROS); }
	void EnableDeformVertexes()					{	_activeMacros |= USE_DEFORM_VERTEXES; SelectProgram(NUM_MACROS); }
	void EnableParallaxMapping()				{	_activeMacros |= USE_PARALLAX_MAPPING; SelectProgram(NUM_MACROS); }

	void DisablePortalClipping()				{	_activeMacros &= ~USE_PORTAL_CLIPPING; SelectProgram(NUM_MACROS); }
	void DisableAlphaTesting()					{	_activeMacros &= ~USE_ALPHA_TESTING; SelectProgram(NUM_MACROS); }
	void DisableVertexSkinning()				{	_activeMacros &= ~USE_VERTEX_SKINNING; SelectProgram(NUM_MACROS); }
	void DisableVertexAnimation()				{	_activeMacros &= ~USE_VERTEX_ANIMATION; SelectProgram(NUM_MACROS); }
	void DisableDeformVertexes()				{	_activeMacros &= ~USE_DEFORM_VERTEXES; SelectProgram(NUM_MACROS); }
	void DisableParallaxMapping()				{	_activeMacros &= ~USE_PARALLAX_MAPPING; SelectProgram(NUM_MACROS); }

		
};


extern GLShader_lightMapping* gl_lightMappingShader;
extern GLShader_vertexLighting_DBS_entity* gl_vertexLightingShader_DBS_entity;

#endif	// GL_SHADER_H
