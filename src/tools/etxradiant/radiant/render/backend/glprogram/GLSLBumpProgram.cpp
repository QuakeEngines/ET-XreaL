#include "GLSLBumpProgram.h"
#include "render/backend/GLProgramFactory.h"

#include "igame.h"
#include "string/string.h"

namespace render
{

namespace
{
    // Lightscale registry path
    const char* LOCAL_RKEY_LIGHTSCALE = "/defaults/lightScale";

    // Filenames of shader code
    const char* BUMP_VP_FILENAME = "interaction_vp.glsl";
    const char* BUMP_FP_FILENAME = "interaction_fp.glsl";

}

// Main construction
void GLSLBumpProgram::create()
{
	// Initialise the lightScale value
    game::IGamePtr currentGame = GlobalGameManager().currentGame();
    xml::NodeList scaleList = currentGame->getLocalXPath(LOCAL_RKEY_LIGHTSCALE);
	if (!scaleList.empty()) 
    {
		_lightScale = strToDouble(scaleList[0].getContent());
	}
	else {
		_lightScale = 1.0;
	}

    // Create the program object
    std::cout << "[renderer] Creating GLSL bump program" << std::endl;
    _programObj = GLProgramFactory::createGLSLProgram(
        BUMP_VP_FILENAME, BUMP_FP_FILENAME
    );

    // Bind vertex attribute locations and link the program
    glBindAttribLocation(_programObj, ATTR_TEXCOORD, "attr_TexCoord0");
    glBindAttribLocation(_programObj, ATTR_TANGENT, "attr_Tangent");
    glBindAttribLocation(_programObj, ATTR_BITANGENT, "attr_Bitangent");
    glBindAttribLocation(_programObj, ATTR_NORMAL, "attr_Normal");
    glLinkProgram(_programObj);
    GlobalOpenGL().assertNoErrors();

    // Set the uniform locations to the correct bound values
    _locLightOrigin = glGetUniformLocation(_programObj, "u_light_origin");
    _locLightColour = glGetUniformLocation(_programObj, "u_light_color");
    _locViewOrigin = glGetUniformLocation(_programObj, "u_view_origin");
    _locLightScale = glGetUniformLocation(_programObj, "u_light_scale");

    // Set up the texture uniforms. The renderer uses fixed texture units for
    // particular textures, so make sure they are correct here.
    // Texture 0 - diffuse
    // Texture 1 - bump
    // Texture 2 - specular
    // Texture 3 - XY attenuation map
    // Texture 4 - Z attenuation map
    
    glUseProgram(_programObj);
    GlobalOpenGL().assertNoErrors();

    GLint samplerLoc;

    samplerLoc = glGetUniformLocation(_programObj, "u_diffusemap");
    glUniform1i(samplerLoc, 0);

    samplerLoc = glGetUniformLocation(_programObj, "u_bumpmap");
    glUniform1i(samplerLoc, 1);

    samplerLoc = glGetUniformLocation(_programObj, "u_specularmap");
    glUniform1i(samplerLoc, 2);

    samplerLoc = glGetUniformLocation(_programObj, "u_attenuationmap_xy");
    glUniform1i(samplerLoc, 3);

    samplerLoc = glGetUniformLocation(_programObj, "u_attenuationmap_z");
    glUniform1i(samplerLoc, 4);

    GlobalOpenGL().assertNoErrors();
    glUseProgram(0);

    GlobalOpenGL().assertNoErrors();
}

void GLSLBumpProgram::destroy()
{
    glDeleteProgram(_programObj);
    
    GlobalOpenGL().assertNoErrors();
}

void GLSLBumpProgram::enable()
{
    glUseProgram(_programObj);

    glEnableVertexAttribArrayARB(ATTR_TEXCOORD);
    glEnableVertexAttribArrayARB(ATTR_TANGENT);
    glEnableVertexAttribArrayARB(ATTR_BITANGENT);
    glEnableVertexAttribArrayARB(ATTR_NORMAL);

    GlobalOpenGL().assertNoErrors();
}

void GLSLBumpProgram::disable()
{
    glUseProgram(0);

    glDisableVertexAttribArrayARB(ATTR_TEXCOORD);
    glDisableVertexAttribArrayARB(ATTR_TANGENT);
    glDisableVertexAttribArrayARB(ATTR_BITANGENT);
    glDisableVertexAttribArrayARB(ATTR_NORMAL);

    GlobalOpenGL().assertNoErrors();
}

void GLSLBumpProgram::applyRenderParams(const Vector3& viewer, 
                                       const Matrix4& objectToWorld, 
                                       const Vector3& origin, 
                                       const Vector3& colour, 
                                       const Matrix4& world2light,
                                       float ambientFactor)
{
    Matrix4 worldToObject(objectToWorld);
    matrix4_affine_invert(worldToObject);

    // Calculate the light origin in object space
    Vector3 localLight(origin);
    matrix4_transform_point(worldToObject, localLight);

    Matrix4 local2light(world2light);
    local2light.multiplyBy(objectToWorld); // local->world->light

    // Bind uniform parameters
    glUniform3f(
        _locViewOrigin, viewer.x(), viewer.y(), viewer.z()
    );
    glUniform3f(
        _locLightOrigin, localLight.x(), localLight.y(), localLight.z()
    );
    glUniform3f(
        _locLightColour, colour.x(), colour.y(), colour.z()
    );
    glUniform1f(_locLightScale, _lightScale);

    glActiveTexture(GL_TEXTURE3);
    glClientActiveTexture(GL_TEXTURE3);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixd(local2light);
    glMatrixMode(GL_MODELVIEW);

    GlobalOpenGL().assertNoErrors();
}

}




