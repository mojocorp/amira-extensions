#pragma once

#include <hxmorgan/api.h>
#include <hxcore/HxModule.h>
#include <hxcore/HxPortFloatSlider.h>
#include <hxcore/HxPortRadioBox.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoShaderParameter.h>

#include <mclib/McHandle.h>

class SoMaterial;
class SoVertexProperty;
class SoShaderProgram;
class SoVertexShader;
class SoGeometryShader;
class SoFragmentShader;

/// Vertex View that use shaders.
class HXMORGAN_API HxGLSLVertexView : public HxModule 
{
  HX_HEADER(HxGLSLVertexView);
  
  public:
    /// Constructor.
    HxGLSLVertexView();

    /// Determines the way points are shown.
    enum DrawStyles 
    {
        POINTS,
        PLATES,
        HAZE,
        SPHERES
    };

    /// Draw style: points, plates, haze, spheres.
    HxPortRadioBox portDrawStyle;

    HxPortFloatSlider portSphereScale;

    /// Update method.
    virtual void update();

    /// Compute method.
    virtual void compute();

  protected:
    virtual ~HxGLSLVertexView();

    virtual void setRenderMode(DrawStyles mode);
  private:  
    McHandle<SoSeparator>           m_p_root;
    SoMaterial*                     m_p_material;
    SoShaderProgram*                m_p_shaderProgram;
    SoVertexProperty*               m_p_vertexProperty;
    SoVertexShader*                 m_p_vertexShader;
    SoGeometryShader*               m_p_geometryshader;
    SoFragmentShader*               m_p_fragmentshader;

    McHandle<SoShaderParameter1f>   m_p_sphereSizeParameter;
};
