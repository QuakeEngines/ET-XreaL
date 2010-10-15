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
class GLCompileMacro;

class GLShader
{
	//friend class GLCompileMacro_USE_ALPHA_TESTING;

protected:
	int									_activeMacros;

	std::vector<shaderProgram_t>		_shaderPrograms;
	shaderProgram_t*					_currentProgram;

	std::vector<GLUniform*>				_uniforms;
	std::vector<GLCompileMacro*>		_compileMacros;
public:

	GLShader()
	{
		_activeMacros = 0;
		_currentProgram = NULL;
	}

	~GLShader()
	{
		for(std::size_t i = 0; i < _shaderPrograms.size(); i++)
		{
			glDeleteObjectARB(_shaderPrograms[i].program);
		}
	}

	void RegisterUniform(GLUniform* uniform)
	{
		_uniforms.push_back(uniform);
	}

	void RegisterCompileMacro(GLCompileMacro* compileMacro)
	{
		if(_compileMacros.size() >= 9)
		{
			ri.Error(ERR_DROP, "Can't register more than 9 compile macros for a single shader");
		}

		_compileMacros.push_back(compileMacro);
	}

	const char* GetCompileMacrosString(int permutation);

	size_t GetNumOfCompiledMacros() const				{ return _compileMacros.size(); }

	shaderProgram_t*		GetProgram() const			{ return _currentProgram; }

	void SelectProgram()
	{
		int index = 0;

		size_t numMacros = _compileMacros.size();
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

	bool IsMacroSet(int bit)
	{
		return (_activeMacros & bit) != 0;
	}

	void AddMacroBit(int bit)
	{
		_activeMacros |= bit;
	}

	void DelMacroBit(int bit)
	{
		_activeMacros &= ~bit;
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


class GLCompileMacro
{
private:
	int						_bit;

protected:
	GLShader*				_shader;

	GLCompileMacro(GLShader* shader):
	  _shader(shader)
	{
		_bit = BIT(_shader->GetNumOfCompiledMacros());
		_shader->RegisterCompileMacro(this);
	}

public:
	virtual const char* GetName() const = 0;

	void EnableMacro()
	{
		int bit = GetBit();

		if(!_shader->IsMacroSet(bit))
		{
			_shader->AddMacroBit(bit);
			_shader->SelectProgram();
		}
	}

	void DisableMacro()
	{
		int bit = GetBit();

		if(_shader->IsMacroSet(bit))
		{
			_shader->DelMacroBit(bit);
			_shader->SelectProgram();
		}
	}

public:
	const int GetBit() { return _bit; }
};


class GLCompileMacro_USE_ALPHA_TESTING:
GLCompileMacro
{
public:
	GLCompileMacro_USE_ALPHA_TESTING(GLShader* shader):
	  GLCompileMacro(shader)
	{
	}

	const char* GetName() const { return "USE_ALPHA_TESTING"; }

	void EnableAlphaTesting()		{ EnableMacro(); }
	void DisableAlphaTesting()		{ DisableMacro(); }
};

class GLCompileMacro_USE_PORTAL_CLIPPING:
GLCompileMacro
{
public:
	GLCompileMacro_USE_PORTAL_CLIPPING(GLShader* shader):
	  GLCompileMacro(shader)
	{
	}

	const char* GetName() const { return "USE_PORTAL_CLIPPING"; }

	void EnablePortalClipping()		{ EnableMacro(); }
	void DisablePortalClipping()	{ DisableMacro(); }
};

class GLCompileMacro_USE_VERTEX_SKINNING:
GLCompileMacro
{
public:
	GLCompileMacro_USE_VERTEX_SKINNING(GLShader* shader):
	  GLCompileMacro(shader)
	{
	}

	const char* GetName() const { return "USE_VERTEX_SKINNING"; }

	void EnableVertexSkinning()		{ EnableMacro(); }
	void DisableVertexSkinning()	{ DisableMacro(); }
};

class GLCompileMacro_USE_VERTEX_ANIMATION:
GLCompileMacro
{
public:
	GLCompileMacro_USE_VERTEX_ANIMATION(GLShader* shader):
	  GLCompileMacro(shader)
	{
	}

	const char* GetName() const { return "USE_VERTEX_ANIMATION"; }

	void EnableVertexAnimation()		{ EnableMacro(); }
	void DisableVertexAnimation()		{ DisableMacro(); }
};

class GLCompileMacro_USE_DEFORM_VERTEXES:
GLCompileMacro
{
public:
	GLCompileMacro_USE_DEFORM_VERTEXES(GLShader* shader):
	  GLCompileMacro(shader)
	{
	}

	const char* GetName() const { return "USE_DEFORM_VERTEXES"; }

	void EnableDeformVertexes()		{ EnableMacro(); }
	void DisableDeformVertexes()	{ DisableMacro(); }
};

class GLCompileMacro_USE_PARALLAX_MAPPING:
GLCompileMacro
{
public:
	GLCompileMacro_USE_PARALLAX_MAPPING(GLShader* shader):
	  GLCompileMacro(shader)
	{
	}

	const char* GetName() const { return "USE_PARALLAX_MAPPING"; }

	void EnableParallaxMapping()	{ EnableMacro(); }
	void DisableParallaxMapping()	{ DisableMacro(); }
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
		GLSL_SetUniform_Time(_parent->GetProgram(), value);
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
public GLDeformStage,
public GLCompileMacro_USE_PORTAL_CLIPPING,
public GLCompileMacro_USE_ALPHA_TESTING,
public GLCompileMacro_USE_DEFORM_VERTEXES
{
public:
	GLShader_lightMapping();
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
public GLDeformStage,
public GLCompileMacro_USE_PORTAL_CLIPPING,
public GLCompileMacro_USE_ALPHA_TESTING,
public GLCompileMacro_USE_VERTEX_SKINNING,
public GLCompileMacro_USE_VERTEX_ANIMATION,
public GLCompileMacro_USE_DEFORM_VERTEXES,
public GLCompileMacro_USE_PARALLAX_MAPPING
{
public:
	GLShader_vertexLighting_DBS_entity();
};


extern GLShader_lightMapping* gl_lightMappingShader;
extern GLShader_vertexLighting_DBS_entity* gl_vertexLightingShader_DBS_entity;

#endif	// GL_SHADER_H
