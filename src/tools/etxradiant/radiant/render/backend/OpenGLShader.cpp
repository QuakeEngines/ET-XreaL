#include "OpenGLShader.h"
#include "GLProgramFactory.h"
#include "OpenGLStateBucketAdd.h"
#include "render/OpenGLRenderSystem.h"

#include "iuimanager.h"
#include "ishaders.h"
#include "ifilter.h"
#include "irender.h"
#include "texturelib.h"

#include <boost/bind.hpp>
#include <cstdio>

void OpenGLShader::destroy() {
	// Clear the shaderptr, so that the shared_ptr reference count is decreased 
    _material = MaterialPtr();

    for(Passes::iterator i = _shaderPasses.begin(); i != _shaderPasses.end(); ++i)
    {
      delete *i;
    }
    _shaderPasses.clear();
}

void OpenGLShader::addRenderable(const OpenGLRenderable& renderable, 
					   			 const Matrix4& modelview, 
					   			 const LightList* lights)
{
    // Iterate over the list of OpenGLStateBuckets, bumpmap and non-bumpmap
    // buckets are handled differently.
    for(Passes::iterator i = _shaderPasses.begin(); i != _shaderPasses.end(); ++i)
    {
      if(((*i)->state().renderFlags & RENDER_BUMP) != 0)
      {
        if(lights != 0)
        {
          OpenGLStateBucketAdd add(*(*i), renderable, modelview);
		  lights->forEachLight(boost::bind(&OpenGLStateBucketAdd::visit, &add, _1));
        }
      }
      else
      {
        (*i)->addRenderable(renderable, modelview);
      }
    }
}

void OpenGLShader::incrementUsed() {
    if(++m_used == 1 && _material != 0)
    { 
      _material->SetInUse(true);
    }
}

void OpenGLShader::decrementUsed() {
    if(--m_used == 0 && _material != 0)
    {
      _material->SetInUse(false);
    }
}

void OpenGLShader::realise(const std::string& name) 
{
    // Construct the shader passes based on the name
    construct(name);

    if (_material != NULL) {
		// greebo: Check the filtersystem whether we're filtered
		_material->setVisible(GlobalFilterSystem().isVisible("texture", name));

		if (m_used != 0) {
			_material->SetInUse(true);
		}
    }
    
    for(Passes::iterator i = _shaderPasses.begin(); i != _shaderPasses.end(); ++i) {
    	render::getOpenGLRenderSystem().insertSortedState(
			OpenGLStates::value_type((*i)->statePtr(), *i));
    }

    m_observers.realise();
}

void OpenGLShader::unrealise() {
    m_observers.unrealise();

    for(Passes::iterator i = _shaderPasses.begin(); i != _shaderPasses.end(); ++i)
	{
    	render::getOpenGLRenderSystem().eraseSortedState((*i)->statePtr());
    }

    destroy();
}

unsigned int OpenGLShader::getFlags() const {
    return _material->getFlags();
}

// Append a default shader pass onto the back of the state list
OpenGLState& OpenGLShader::appendDefaultPass() 
{
    _shaderPasses.push_back(new OpenGLShaderPass);
    OpenGLState& state = _shaderPasses.back()->state();
    return state;
}

// Test if we can render in bump map mode
bool OpenGLShader::canUseLightingMode() const
{
    return (
        GlobalRenderSystem().lightingSupported()  // hw supports lighting mode
    	&& GlobalRenderSystem().lightingEnabled()  // user enable lighting mode
    );
}

void OpenGLShader::setGLTexturesFromTriplet(OpenGLState& pass,
                                            const DBSTriplet& triplet)
{
    // Get texture components. If any of the triplet is missing, look up the
    // default from the shader system.
    if (triplet.diffuse)
    {
        pass.texture0 = triplet.diffuse->getTexture()->getGLTexNum();
    }
    else
    {
        pass.texture0 = GlobalMaterialManager().getDefaultInteractionTexture(
            ShaderLayer::DIFFUSE
        )->getGLTexNum();
    }
    if (triplet.bump)
    {
        pass.texture1 = triplet.bump->getTexture()->getGLTexNum();
    }
    else
    {
        pass.texture1 = GlobalMaterialManager().getDefaultInteractionTexture(
            ShaderLayer::BUMP
        )->getGLTexNum();
    }
    if (triplet.specular)
    {
        pass.texture2 = triplet.specular->getTexture()->getGLTexNum();
    }
    else
    {
        pass.texture2 = GlobalMaterialManager().getDefaultInteractionTexture(
            ShaderLayer::SPECULAR
        )->getGLTexNum();
    }
}

// Add an interaction layer
void OpenGLShader::appendInteractionLayer(const DBSTriplet& triplet)
{
    // Append a depthfill shader pass if requested
    if (triplet.needDepthFill)
    {
        // Create depth-buffer fill pass
        OpenGLState& state = appendDefaultPass();
        state.renderFlags = RENDER_FILL 
                        | RENDER_CULLFACE 
                        | RENDER_TEXTURE_2D 
                        | RENDER_DEPTHTEST 
                        | RENDER_DEPTHWRITE 
                        | RENDER_COLOURWRITE 
                        | RENDER_PROGRAM;

        state.m_colour[0] = 0;
        state.m_colour[1] = 0;
        state.m_colour[2] = 0;
        state.m_colour[3] = 1;
        state.m_sort = OpenGLState::eSortOpaque;
        
        state.glProgram = render::GLProgramFactory::getProgram("depthFill").get();
    }
    
    // Add the DBS pass
    OpenGLState& dbsPass = appendDefaultPass();

    // Populate the textures
    setGLTexturesFromTriplet(dbsPass, triplet);

    // Set render flags
    dbsPass.renderFlags = RENDER_BLEND
                        | RENDER_FILL
                        | RENDER_TEXTURE_2D
                        | RENDER_CULLFACE
                        | RENDER_DEPTHTEST
						| RENDER_DEPTHWRITE
                        | RENDER_COLOURWRITE
                        | RENDER_SMOOTH
                        | RENDER_BUMP
                        | RENDER_PROGRAM;
    
    dbsPass.glProgram = render::GLProgramFactory::getProgram("bumpMap").get();

    // Set layer vertex colour mode and alphatest parameters
    ShaderLayer::VertexColourMode vcolMode = ShaderLayer::VERTEX_COLOUR_NONE;
    double alphaTest = -1;
    if (triplet.diffuse)
    {
        vcolMode = triplet.diffuse->getVertexColourMode();
        alphaTest = triplet.diffuse->getAlphaTest();
    }
    if (vcolMode != ShaderLayer::VERTEX_COLOUR_NONE)
    {
        // Vertex colours allowed
        dbsPass.renderFlags |= RENDER_MATERIAL_VCOL;

        if (vcolMode == ShaderLayer::VERTEX_COLOUR_INVERSE_MULTIPLY)
        {
            // Vertex colours are inverted
            dbsPass.renderFlags |= RENDER_VCOL_INVERT;
        }
    }
    applyAlphaTestToPass(dbsPass, alphaTest);

    dbsPass.m_depthfunc = GL_LEQUAL;
    dbsPass.m_sort = OpenGLState::eSortMultiFirst;
    dbsPass.m_blend_src = GL_ONE;
    dbsPass.m_blend_dst = GL_ONE;
}

void OpenGLShader::applyAlphaTestToPass(OpenGLState& pass, float alphaTest)
{
    if (alphaTest > 0)
    {
        pass.renderFlags |= RENDER_ALPHATEST;
        pass.alphaFunc = GL_GEQUAL; // alpha >= threshold
        pass.alphaThreshold = alphaTest;
    }
}

// Construct lighting mode render passes
void OpenGLShader::constructLightingPassesFromMaterial()
{
    // Build up and add shader passes for DBS triplets as they are found. A
    // new triplet is found when (1) the same DBS layer type is seen twice, (2)
    // we have at least one DBS layer then see a blend layer, or (3) we have at
    // least one DBS layer then reach the end of the layers.

    DBSTriplet triplet;
    const ShaderLayerVector& allLayers = _material->getAllLayers();

    for (ShaderLayerVector::const_iterator i = allLayers.begin();
         i != allLayers.end();
         ++i)
    {
        switch ((*i)->getType())
        {
        case ShaderLayer::DIFFUSE:
            if (triplet.diffuse)
            {
                appendInteractionLayer(triplet);
                triplet.reset();
            }
            triplet.diffuse = *i;
            break;

        case ShaderLayer::BUMP:
            if (triplet.bump)
            {
                appendInteractionLayer(triplet);
                triplet.reset();
            }
            triplet.bump = *i;
            break;

        case ShaderLayer::SPECULAR:
            if (triplet.specular)
            {
                appendInteractionLayer(triplet);
                triplet.reset();
            }
            triplet.specular = *i;
            break;

        case ShaderLayer::BLEND:
            if (triplet.specular || triplet.bump || triplet.diffuse)
            {
                appendInteractionLayer(triplet);
                triplet.reset();
            }

            appendBlendLayer(*i);
        }
    }

    // Submit final pass if we reach the end
    if (triplet.specular || triplet.bump || triplet.diffuse)
	{
		appendInteractionLayer(triplet);
	}
}

void OpenGLShader::determineBlendModeForEditorPass(OpenGLState& pass)
{
    bool hasDiffuseLayer = false;
    int numLayers = 0;

    // Determine alphatest from first diffuse layer
    const ShaderLayerVector& allLayers = _material->getAllLayers();
    for (ShaderLayerVector::const_iterator i = allLayers.begin();
         i != allLayers.end();
         ++i)
    {
        ++numLayers;

        ShaderLayerPtr layer = *i;
        if (layer->getType() == ShaderLayer::DIFFUSE)
        {
            hasDiffuseLayer = true;
            if (layer->getAlphaTest() > 0)
            {
                applyAlphaTestToPass(pass, layer->getAlphaTest());
                break;
            }
        }
    }

    // If this is a purely blend material (no DBS layers), set the editor blend
    // mode from the first blend layer.
    if (!hasDiffuseLayer && numLayers > 0)
    {
        pass.renderFlags |= RENDER_BLEND;
        pass.m_sort = OpenGLState::eSortTranslucent;

        BlendFunc bf = allLayers[0]->getBlendFunc();
        pass.m_blend_src = bf.src;
        pass.m_blend_dst = bf.dest;
    }
}

// Construct editor-image-only render passes
void OpenGLShader::constructEditorPreviewPassFromMaterial()
{
    OpenGLState& previewPass = appendDefaultPass();

    // Render the editor texture in legacy mode
    previewPass.texture0 = _material->getEditorImage()->getGLTexNum();
    previewPass.renderFlags = RENDER_FILL
                              | RENDER_TEXTURE_2D
                              | RENDER_DEPTHTEST
                              | RENDER_DEPTHWRITE
                              | RENDER_COLOURWRITE
                              | RENDER_LIGHTING
                              | RENDER_SMOOTH;

    // Handle certain shader flags
    if ((_material->getFlags() & QER_CULL) == 0
        || _material->getCull() == Material::eCullBack)
    {
        previewPass.renderFlags |= RENDER_CULLFACE;
    }

    // Set up blend properties
    determineBlendModeForEditorPass(previewPass);

    // Set the GL color to white
    previewPass.m_colour = Vector4(1, 1, 1, 1);

    // Sort position
    if (_material->getSortRequest() == Material::SORT_DECAL)
    {
        previewPass.m_sort = OpenGLState::eSortOverlayFirst;
    }
    else if (previewPass.m_sort != OpenGLState::eSortTranslucent)
    {
        previewPass.m_sort = OpenGLState::eSortFullbright;
    }

    // Polygon offset
    previewPass.polygonOffset = _material->getPolygonOffset();
}

// Append a blend (non-interaction) layer
void OpenGLShader::appendBlendLayer(ShaderLayerPtr layer)
{
    TexturePtr layerTex = layer->getTexture();

    OpenGLState& state = appendDefaultPass();
    state.renderFlags = RENDER_FILL
                    | RENDER_BLEND
                    | RENDER_DEPTHTEST
                    | RENDER_COLOURWRITE;

    // Set the texture
    state.texture0 = layerTex->getGLTexNum();

    // Get the blend function
    BlendFunc blendFunc = layer->getBlendFunc();
    state.m_blend_src = blendFunc.src;
    state.m_blend_dst = blendFunc.dest;

	// Alpha-tested stages or one-over-zero blends should use the depth buffer
    if (state.m_blend_src == GL_SRC_ALPHA || state.m_blend_dst == GL_SRC_ALPHA || 
		(state.m_blend_src == GL_ONE && state.m_blend_dst == GL_ZERO))
    {
		state.renderFlags |= RENDER_DEPTHWRITE;
    }

    // Set texture dimensionality (cube map or 2D)
    state.cubeMapMode = layer->getCubeMapMode();
    if (state.cubeMapMode == ShaderLayer::CUBE_MAP_CAMERA)
    {
        state.renderFlags |= RENDER_TEXTURE_CUBEMAP;
    }
    else
    {
        state.renderFlags |= RENDER_TEXTURE_2D;
    }

    // Colour modulation
    state.m_colour = Vector4(layer->getColour(), 1.0);

    state.m_sort = OpenGLState::eSortFullbright;

	// Sort position
    if (_material->getSortRequest() == Material::SORT_DECAL)
    {
        state.m_sort = OpenGLState::eSortOverlayFirst;
    }
    else
    {
        state.m_sort = OpenGLState::eSortFullbright;
	}

    // Polygon offset
    state.polygonOffset = _material->getPolygonOffset();
}

// Construct a normal shader
void OpenGLShader::constructNormalShader(const std::string& name)
{
    // Obtain the Material
    _material = GlobalMaterialManager().getMaterialForName(name);
    assert(_material);

    // Determine whether we can render this shader in lighting/bump-map mode,
    // and construct the appropriate shader passes
    if (canUseLightingMode()) 
    {
        // Full lighting, DBS and blend modes
        constructLightingPassesFromMaterial();
    }
    else
    {
        // Editor image rendering only
        constructEditorPreviewPassFromMaterial();
    }
}

// Main shader construction entry point
void OpenGLShader::construct(const std::string& name)
{
	// Retrieve the highlight colour from the colourschemes (once)
	static Vector3 highLightColour = ColourSchemes().getColour("selected_brush_camera");
	
    // Check the first character of the name to see if this is a special built-in
    // shader
    switch(name[0])
    {
        case '(': // fill shader
        {
            OpenGLState& state = appendDefaultPass();
            sscanf(name.c_str(), "(%lf %lf %lf)", &state.m_colour[0], &state.m_colour[1], &state.m_colour[2]);
            state.m_colour[3] = 1.0f;
            state.renderFlags = RENDER_FILL|RENDER_LIGHTING|RENDER_DEPTHTEST|RENDER_CULLFACE|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
            state.m_sort = OpenGLState::eSortFullbright;
            break;
        }

        case '[':
        {
            OpenGLState& state = appendDefaultPass();
            sscanf(name.c_str(), "[%lf %lf %lf]", &state.m_colour[0], &state.m_colour[1], &state.m_colour[2]);
            state.m_colour[3] = 0.5f;
            state.renderFlags = RENDER_FILL|RENDER_LIGHTING|RENDER_DEPTHTEST|RENDER_CULLFACE|RENDER_COLOURWRITE|RENDER_DEPTHWRITE|RENDER_BLEND;
            state.m_sort = OpenGLState::eSortTranslucent;
            break;
        }

        case '<': // wireframe shader
        {
            OpenGLState& state = appendDefaultPass();
            sscanf(name.c_str(), "<%lf %lf %lf>", &state.m_colour[0], &state.m_colour[1], &state.m_colour[2]);
            state.m_colour[3] = 1;
            state.renderFlags = RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
            state.m_sort = OpenGLState::eSortFullbright;
            state.m_depthfunc = GL_LESS;
            state.m_linewidth = 1;
            state.m_pointsize = 1;
            break;
        }

        case '$':
        {
            OpenGLState& state = appendDefaultPass();

            if (name == "$POINT")
            {
              state.renderFlags = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortControlFirst;
              state.m_pointsize = 4;
            }
            else if (name == "$SELPOINT")
            {
              state.renderFlags = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortControlFirst + 1;
              state.m_pointsize = 4;
            }
            else if (name == "$BIGPOINT")
            {
              state.renderFlags = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortControlFirst;
              state.m_pointsize = 6;
            }
            else if (name == "$PIVOT")
            {
              state.renderFlags = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHTEST|RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortGUI1;
              state.m_linewidth = 2;
              state.m_depthfunc = GL_LEQUAL;

              OpenGLState& hiddenLine = appendDefaultPass();
              hiddenLine.renderFlags = RENDER_COLOURARRAY|RENDER_COLOURWRITE|RENDER_DEPTHTEST|RENDER_LINESTIPPLE;
              hiddenLine.m_sort = OpenGLState::eSortGUI0;
              hiddenLine.m_linewidth = 2;
              hiddenLine.m_depthfunc = GL_GREATER;
            }
            else if (name == "$LATTICE")
            {
              state.m_colour[0] = 1;
              state.m_colour[1] = 0.5;
              state.m_colour[2] = 0;
              state.m_colour[3] = 1;
              state.renderFlags = RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortControlFirst;
            }
            else if (name == "$WIREFRAME")
            {
              state.renderFlags = RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortFullbright;
            }
            else if (name == "$CAM_HIGHLIGHT")
            {
              state.m_colour[0] = highLightColour[0];
              state.m_colour[1] = highLightColour[1];
              state.m_colour[2] = highLightColour[2];
              state.m_colour[3] = 0.3f;
              state.renderFlags = RENDER_FILL|RENDER_DEPTHTEST|RENDER_CULLFACE|RENDER_BLEND|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortHighlight;
              state.m_depthfunc = GL_LEQUAL;
            }
            else if (name == "$CAM_OVERLAY")
            {
        #if 0
              state.renderFlags = RENDER_CULLFACE|RENDER_COLOURWRITE|RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortOverlayFirst;
        #else
              state.renderFlags = RENDER_CULLFACE|RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_DEPTHWRITE|RENDER_OFFSETLINE;
              state.m_sort = OpenGLState::eSortOverlayFirst + 1;
              state.m_depthfunc = GL_LEQUAL;

              OpenGLState& hiddenLine = appendDefaultPass();
              hiddenLine.m_colour[0] = 0.75;
              hiddenLine.m_colour[1] = 0.75;
              hiddenLine.m_colour[2] = 0.75;
              hiddenLine.m_colour[3] = 1;
              hiddenLine.renderFlags = RENDER_CULLFACE|RENDER_DEPTHTEST|RENDER_COLOURWRITE|RENDER_OFFSETLINE|RENDER_LINESTIPPLE;
              hiddenLine.m_sort = OpenGLState::eSortOverlayFirst;
              hiddenLine.m_depthfunc = GL_GREATER;
              hiddenLine.m_linestipple_factor = 2;
        #endif
            }
            else if (name == "$XY_OVERLAY")
            {
              Vector3 colorSelBrushes = ColourSchemes().getColour("selected_brush");
              state.m_colour[0] = colorSelBrushes[0];
              state.m_colour[1] = colorSelBrushes[1];
              state.m_colour[2] = colorSelBrushes[2];
              state.m_colour[3] = 1;
              state.renderFlags = RENDER_COLOURWRITE | RENDER_LINESTIPPLE;
              state.m_sort = OpenGLState::eSortOverlayFirst;
              state.m_linewidth = 2;
              state.m_linestipple_factor = 3;
            }
            else if (name == "$DEBUG_CLIPPED")
            {
              state.renderFlags = RENDER_COLOURARRAY | RENDER_COLOURWRITE | RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortLast;
            }
            else if (name == "$POINTFILE")
            {
              state.m_colour[0] = 1;
              state.m_colour[1] = 0;
              state.m_colour[2] = 0;
              state.m_colour[3] = 1;
              state.renderFlags = RENDER_DEPTHTEST | RENDER_COLOURWRITE | RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortFullbright;
              state.m_linewidth = 4;
            }
            else if (name == "$LIGHT_SPHERE")
            {
              state.m_colour[0] = .15f * .95f;
              state.m_colour[1] = .15f * .95f;
              state.m_colour[2] = .15f * .95f;
              state.m_colour[3] = 1;
              state.renderFlags = RENDER_CULLFACE | RENDER_DEPTHTEST | RENDER_BLEND | RENDER_FILL | RENDER_COLOURWRITE | RENDER_DEPTHWRITE;
              state.m_blend_src = GL_ONE;
              state.m_blend_dst = GL_ONE;
              state.m_sort = OpenGLState::eSortTranslucent;
            }
            else if (name == "$Q3MAP2_LIGHT_SPHERE")
            {
              state.m_colour[0] = .05f;
              state.m_colour[1] = .05f;
              state.m_colour[2] = .05f;
              state.m_colour[3] = 1;
              state.renderFlags = RENDER_CULLFACE | RENDER_DEPTHTEST | RENDER_BLEND | RENDER_FILL;
              state.m_blend_src = GL_ONE;
              state.m_blend_dst = GL_ONE;
              state.m_sort = OpenGLState::eSortTranslucent;
            }
            else if (name == "$WIRE_OVERLAY")
            {
        #if 0
              state.renderFlags = RENDER_COLOURARRAY | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE;
              state.m_sort = OpenGLState::eSortOverlayFirst;
        #else
              state.renderFlags = RENDER_COLOURARRAY | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE;
              state.m_sort = OpenGLState::eSortGUI1;
              state.m_depthfunc = GL_LEQUAL;

              OpenGLState& hiddenLine = appendDefaultPass();
              hiddenLine.renderFlags = RENDER_COLOURARRAY | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE | RENDER_LINESTIPPLE;
              hiddenLine.m_sort = OpenGLState::eSortGUI0;
              hiddenLine.m_depthfunc = GL_GREATER;
        #endif
            }
            else if (name == "$FLATSHADE_OVERLAY")
            {
              state.renderFlags = RENDER_CULLFACE | RENDER_LIGHTING | RENDER_SMOOTH | RENDER_SCALED | RENDER_COLOURARRAY | RENDER_FILL | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE;
              state.m_sort = OpenGLState::eSortGUI1;
              state.m_depthfunc = GL_LEQUAL;

              OpenGLState& hiddenLine = appendDefaultPass();
              hiddenLine.renderFlags = RENDER_CULLFACE | RENDER_LIGHTING | RENDER_SMOOTH | RENDER_SCALED | RENDER_COLOURARRAY | RENDER_FILL | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_DEPTHTEST | RENDER_OVERRIDE | RENDER_POLYGONSTIPPLE;
              hiddenLine.m_sort = OpenGLState::eSortGUI0;
              hiddenLine.m_depthfunc = GL_GREATER;
            }
            else if (name == "$CLIPPER_OVERLAY")
            {
              Vector3 colorClipper = ColourSchemes().getColour("clipper");
              state.m_colour[0] = colorClipper[0];
              state.m_colour[1] = colorClipper[1];
              state.m_colour[2] = colorClipper[2];
              state.m_colour[3] = 1;
              state.renderFlags = RENDER_CULLFACE | RENDER_COLOURWRITE | RENDER_DEPTHWRITE | RENDER_FILL | RENDER_POLYGONSTIPPLE;
              state.m_sort = OpenGLState::eSortOverlayFirst;
            }
            else if (name == "$OVERBRIGHT")
            {
              const float lightScale = 2;
              state.m_colour[0] = lightScale * 0.5f;
              state.m_colour[1] = lightScale * 0.5f;
              state.m_colour[2] = lightScale * 0.5f;
              state.m_colour[3] = 0.5;
              state.renderFlags = RENDER_FILL|RENDER_BLEND|RENDER_COLOURWRITE|RENDER_SCREEN;
              state.m_sort = OpenGLState::eSortOverbrighten;
              state.m_blend_src = GL_DST_COLOR;
              state.m_blend_dst = GL_SRC_COLOR;
            }
            else
            {
              // default to something recognisable.. =)
              ERROR_MESSAGE("hardcoded renderstate not found");
              state.m_colour[0] = 1;
              state.m_colour[1] = 0;
              state.m_colour[2] = 1;
              state.m_colour[3] = 1;
              state.renderFlags = RENDER_COLOURWRITE | RENDER_DEPTHWRITE;
              state.m_sort = OpenGLState::eSortFirst;
            }
            break;
        } // case '$'

        default:
        {
            // This is not a hard-coded shader, construct from the shader system
            constructNormalShader(name);
        }

    } // switch
}

