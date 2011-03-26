/*
===========================================================================
Copyright (C) 2010-2011 Robert Beckebans <trebor_7@users.sourceforge.net>

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

	const uint32_t						_vertexAttribsRequired;
	const uint32_t						_vertexAttribsOptional;
	const uint32_t						_vertexAttribsUnsupported;
	uint32_t							_vertexAttribs;
public:

	GLShader(uint32_t vertexAttribsRequired, uint32_t vertexAttribsOptional, uint32_t vertexAttribsUnsupported):
	  _vertexAttribsRequired(vertexAttribsRequired),
	  _vertexAttribsOptional(vertexAttribsOptional),
	  _vertexAttribsUnsupported(vertexAttribsUnsupported)
	{
		_activeMacros = 0;
		_currentProgram = NULL;
		_vertexAttribs = 0;
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

	bool IsVertexAtttribSet(int bit)
	{
		return (_vertexAttribs & bit) != 0;
	}

	void AddVertexAttribBit(int bit)
	{
		_vertexAttribs |= bit;
	}

	void DelVertexAttribBit(int bit)
	{
		_vertexAttribs &= ~bit;
	}

	void SetVertexAttribs()
	{
		GL_VertexAttribsState((_vertexAttribsRequired | _vertexAttribs) & ~_vertexAttribsUnsupported);
	}
};

class GLUniform
{
protected:
	GLShader*				_shader;

	GLUniform(GLShader* shader):
	  _shader(shader)
	{
		_shader->RegisterUniform(this);
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

	void SetAlphaTesting(bool enable)
	{
		if(enable)
			EnableMacro();
		else
			DisableMacro();
	}
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

	void SetPortalClipping(bool enable)
	{
		if(enable)
			EnableMacro();
		else
			DisableMacro();
	}
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

	void EnableVertexSkinning()
	{
		EnableMacro();

		_shader->AddVertexAttribBit(ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS);
	}
	void DisableVertexSkinning()
	{
		DisableMacro();

		_shader->DelVertexAttribBit(ATTR_BONE_INDEXES | ATTR_BONE_WEIGHTS);
	}

	void SetVertexSkinning(bool enable)
	{
		if(enable)
			EnableVertexSkinning();
		else
			DisableVertexSkinning();
	}
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

	void EnableVertexAnimation()
	{
		EnableMacro();

		_shader->AddVertexAttribBit(ATTR_POSITION2 | ATTR_NORMAL2);

		if(r_normalMapping->integer)
		{
			_shader->AddVertexAttribBit(ATTR_TANGENT2 | ATTR_BINORMAL2);
		}
	}

	void DisableVertexAnimation()
	{
		DisableMacro();

		_shader->DelVertexAttribBit(ATTR_POSITION2 | ATTR_NORMAL2);

		if(r_normalMapping->integer)
		{
			_shader->DelVertexAttribBit(ATTR_TANGENT2 | ATTR_BINORMAL2);
		}
	}

	void SetVertexAnimation(bool enable)
	{
		if(enable)
			EnableVertexAnimation();
		else
			DisableVertexAnimation();
	}
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

	void EnableDeformVertexes()
	{
		EnableMacro();

		_shader->AddVertexAttribBit(ATTR_NORMAL);
	}
	
	void DisableDeformVertexes()
	{
		DisableMacro();
	}

	void SetDeformVertexes(bool enable)
	{
		if(enable)
			EnableMacro();
		else
			DisableMacro();
	}
};

class GLCompileMacro_USE_TCGEN_ENVIRONMENT:
GLCompileMacro
{
public:
	GLCompileMacro_USE_TCGEN_ENVIRONMENT(GLShader* shader):
	  GLCompileMacro(shader)
	{
	}

	const char* GetName() const { return "USE_TCGEN_ENVIRONMENT"; }

	void EnableTCGenEnvironment()
	{
		EnableMacro();

		_shader->AddVertexAttribBit(ATTR_NORMAL);
	}
	
	void DisableTCGenEnvironment()
	{
		DisableMacro();
	}

	void SetTCGenEnvironment(bool enable)
	{
		if(enable)
			EnableMacro();
		else
			DisableMacro();
	}
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

	void SetParallaxMapping(bool enable)
	{
		if(enable)
			EnableMacro();
		else
			DisableMacro();
	}
};




class u_ColorTextureMatrix:
GLUniform
{
public:
	u_ColorTextureMatrix(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_ColorTextureMatrix"; }

	void SetUniform_ColorTextureMatrix(const matrix_t m)
	{
		GLSL_SetUniform_ColorTextureMatrix(_shader->GetProgram(), m);
	}
};

class u_DiffuseTextureMatrix:
GLUniform
{
public:
	u_DiffuseTextureMatrix(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_DiffuseTextureMatrix"; }

	void SetUniform_DiffuseTextureMatrix(const matrix_t m)
	{
		GLSL_SetUniform_DiffuseTextureMatrix(_shader->GetProgram(), m);
	}
};

class u_NormalTextureMatrix:
GLUniform
{
public:
	u_NormalTextureMatrix(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_NormalTextureMatrix"; }

	void SetUniform_NormalTextureMatrix(const matrix_t m)
	{
		GLSL_SetUniform_NormalTextureMatrix(_shader->GetProgram(), m);
	}
};

class u_SpecularTextureMatrix:
GLUniform
{
public:
	u_SpecularTextureMatrix(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_SpecularTextureMatrix"; }

	void SetUniform_SpecularTextureMatrix(const matrix_t m)
	{
		GLSL_SetUniform_SpecularTextureMatrix(_shader->GetProgram(), m);
	}
};


class u_AlphaTest:
GLUniform
{
public:
	u_AlphaTest(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_AlphaTest"; }

	void SetUniform_AlphaTest(uint32_t stateBits)
	{
		GLSL_SetUniform_AlphaTest(_shader->GetProgram(), stateBits);
	}
};


class u_AmbientColor:
GLUniform
{
public:
	u_AmbientColor(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_AmbientColor"; }

	void SetUniform_AmbientColor(const vec3_t v)
	{
		GLSL_SetUniform_AmbientColor(_shader->GetProgram(), v);
	}
};


class u_ViewOrigin:
GLUniform
{
public:
	u_ViewOrigin(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_ViewOrigin"; }

	void SetUniform_ViewOrigin(const vec3_t v)
	{
		GLSL_SetUniform_ViewOrigin(_shader->GetProgram(), v);
	}
};


class u_LightDir:
GLUniform
{
public:
	u_LightDir(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_LightDir"; }

	void SetUniform_LightDir(const vec3_t v)
	{
		GLSL_SetUniform_LightDir(_shader->GetProgram(), v);
	}
};


class u_LightColor:
GLUniform
{
public:
	u_LightColor(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_LightColor"; }

	void SetUniform_LightColor(const vec3_t v)
	{
		GLSL_SetUniform_LightColor(_shader->GetProgram(), v);
	}
};


class u_LightWrapAround:
GLUniform
{
public:
	u_LightWrapAround(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_LightWrapAround"; }

	void SetUniform_LightWrapAround(float value)
	{
		GLSL_SetUniform_LightWrapAround(_shader->GetProgram(), value);
	}
};


class u_Color:
GLUniform
{
public:
	u_Color(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_Color"; }

	void SetUniform_Color(const vec3_t v)
	{
		GLSL_SetUniform_Color(_shader->GetProgram(), v);
	}
};




class u_ModelMatrix:
GLUniform
{
public:
	u_ModelMatrix(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_ModelMatrix"; }

	void SetUniform_ModelMatrix(const matrix_t m)
	{
		GLSL_SetUniform_ModelMatrix(_shader->GetProgram(), m);
	}
};


class u_ModelViewProjectionMatrix:
GLUniform
{
public:
	u_ModelViewProjectionMatrix(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_ModelViewProjectionMatrix"; }

	void SetUniform_ModelViewProjectionMatrix(const matrix_t m)
	{
		GLSL_SetUniform_ModelViewProjectionMatrix(_shader->GetProgram(), m);
	}
};


class u_BoneMatrix:
GLUniform
{
public:
	u_BoneMatrix(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_BoneMatrix"; }

	void SetUniform_BoneMatrix(int numBones, const matrix_t boneMatrices[MAX_BONES])
	{
		glUniformMatrix4fvARB(_shader->GetProgram()->u_BoneMatrix, numBones, GL_FALSE, &boneMatrices[0][0]);
	}
};


class u_VertexInterpolation:
GLUniform
{
public:
	u_VertexInterpolation(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_VertexInterpolation"; }

	void SetUniform_VertexInterpolation(float value)
	{
		GLSL_SetUniform_VertexInterpolation(_shader->GetProgram(), value);
	}
};


class u_PortalPlane:
GLUniform
{
public:
	u_PortalPlane(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_PortalPlane"; }

	void SetUniform_PortalPlane(const vec4_t v)
	{
		GLSL_SetUniform_PortalPlane(_shader->GetProgram(), v);
	}
};

class u_DepthScale:
GLUniform
{
public:
	u_DepthScale(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_DepthScale"; }

	void SetUniform_DepthScale(float value)
	{
		GLSL_SetUniform_DepthScale(_shader->GetProgram(), value);
	}
};



class u_DeformGen:
GLUniform
{
public:
	u_DeformGen(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_DeformGen"; }

	void SetUniform_DeformGen(deformGen_t value)
	{
		GLSL_SetUniform_DeformGen(_shader->GetProgram(), value);
	}
};

class u_DeformWave:
GLUniform
{
public:
	u_DeformWave(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_DeformWave"; }

	void SetUniform_DeformWave(const waveForm_t * wf)
	{
		GLSL_SetUniform_DeformWave(_shader->GetProgram(), wf);
	}
};

class u_DeformSpread:
GLUniform
{
public:
	u_DeformSpread(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_DeformSpread"; }

	void SetUniform_DeformSpread(float value)
	{
		GLSL_SetUniform_DeformSpread(_shader->GetProgram(), value);
	}
};

class u_DeformBulge:
GLUniform
{
public:
	u_DeformBulge(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_DeformBulge"; }

	void SetUniform_DeformBulge(deformStage_t *ds)
	{
		GLSL_SetUniform_DeformBulge(_shader->GetProgram(), ds);
	}
};


class u_Time:
GLUniform
{
public:
	u_Time(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_Time"; }

	void SetUniform_Time(float value)
	{
		GLSL_SetUniform_Time(_shader->GetProgram(), value);
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
	GLDeformStage(GLShader* shader):
	  u_DeformGen(shader),
	  u_DeformWave(shader),
	  u_DeformSpread(shader),
	  u_DeformBulge(shader),
	  u_Time(shader)
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



class u_TCGen_Environment:
GLUniform
{
public:
	u_TCGen_Environment(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_TCGen_Environment"; }

	void SetUniform_TCGen_Environment(qboolean value)
	{
		GLSL_SetUniform_TCGen_Environment(_shader->GetProgram(), value);
	}
};

class u_ColorGen:
GLUniform
{
public:
	u_ColorGen(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_ColorGen"; }

	void SetUniform_ColorGen(colorGen_t value)
	{
		GLSL_SetUniform_ColorGen(_shader->GetProgram(), value);

		switch (value)
		{
			case CGEN_VERTEX:
			case CGEN_ONE_MINUS_VERTEX:
				_shader->AddVertexAttribBit(ATTR_COLOR);
				break;

			default:
				_shader->DelVertexAttribBit(ATTR_COLOR);
				break;
		}
	}
};

class u_ColorModulate:
GLUniform
{
public:
	u_ColorModulate(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_ColorModulate"; }

	void SetUniform_ColorModulate(colorGen_t colorGen, alphaGen_t alphaGen)
	{
		vec4_t				v;

		if(r_logFile->integer)
		{
			GLimp_LogComment(va("--- u_ColorModulate::SetUniform_ColorModulate( program = %s, colorGen = %i, alphaGen = %i ) ---\n", _shader->GetProgram()->name, colorGen, alphaGen));
		}

		switch (colorGen)
		{
			case CGEN_VERTEX:
				_shader->AddVertexAttribBit(ATTR_COLOR);
				VectorSet(v, 1, 1, 1);
				break;

			case CGEN_ONE_MINUS_VERTEX:
				_shader->AddVertexAttribBit(ATTR_COLOR);
				VectorSet(v, -1, -1, -1);
				break;

			default:
				_shader->DelVertexAttribBit(ATTR_COLOR);
				VectorSet(v, 0, 0, 0);
				break;
		}

		switch (alphaGen)
		{
			case AGEN_VERTEX:
				_shader->AddVertexAttribBit(ATTR_COLOR);
				v[3] = 1.0f;
				break;

			case AGEN_ONE_MINUS_VERTEX:
				_shader->AddVertexAttribBit(ATTR_COLOR);
				v[3] = -1.0f;
				break;

			default:
				v[3] = 0.0f;
				break;
		}

		GLSL_SetUniform_ColorModulate(_shader->GetProgram(), v);
	}
};





class u_AlphaGen:
GLUniform
{
public:
	u_AlphaGen(GLShader* shader):
	  GLUniform(shader)
	{
	}

	const char* GetName() const { return "u_AlphaGen"; }

	void SetUniform_AlphaGen(alphaGen_t value)
	{
		GLSL_SetUniform_AlphaGen(_shader->GetProgram(), value);
	}
};








class GLShader_generic:
public GLShader,
public u_ColorTextureMatrix,
public u_ViewOrigin,
public u_AlphaTest,
public u_ModelMatrix,
public u_ModelViewProjectionMatrix,
public u_ColorModulate,
public u_Color,
public u_BoneMatrix,
public u_VertexInterpolation,
public u_PortalPlane,
public GLDeformStage,
public GLCompileMacro_USE_PORTAL_CLIPPING,
public GLCompileMacro_USE_ALPHA_TESTING,
public GLCompileMacro_USE_VERTEX_SKINNING,
public GLCompileMacro_USE_VERTEX_ANIMATION,
public GLCompileMacro_USE_DEFORM_VERTEXES,
public GLCompileMacro_USE_TCGEN_ENVIRONMENT
{
public:
	GLShader_generic();
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
public u_DiffuseTextureMatrix,
public u_NormalTextureMatrix,
public u_SpecularTextureMatrix,
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


class GLShader_vertexLighting_DBS_world:
public GLShader,
public u_DiffuseTextureMatrix,
public u_NormalTextureMatrix,
public u_SpecularTextureMatrix,
public u_ColorModulate,
public u_Color,
public u_AlphaTest,
public u_ViewOrigin,
public u_ModelMatrix,
public u_ModelViewProjectionMatrix,
public u_PortalPlane,
public u_DepthScale,
public u_LightWrapAround,
public GLDeformStage,
public GLCompileMacro_USE_PORTAL_CLIPPING,
public GLCompileMacro_USE_ALPHA_TESTING,
public GLCompileMacro_USE_DEFORM_VERTEXES,
public GLCompileMacro_USE_PARALLAX_MAPPING
{
public:
	GLShader_vertexLighting_DBS_world();
};


extern GLShader_generic* gl_genericShader;
extern GLShader_lightMapping* gl_lightMappingShader;
extern GLShader_vertexLighting_DBS_entity* gl_vertexLightingShader_DBS_entity;
extern GLShader_vertexLighting_DBS_world* gl_vertexLightingShader_DBS_world;

#endif	// GL_SHADER_H
