#include "HxGLSLVertexView.h"

#include <hxcore/HxMessage.h>
#include <hxcore/HxResource.h>

#include <hxcore/HxVertexSet.h>

#include <mclib/McMath.h>

#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoShaderParameter.h>
#include <Inventor/nodes/SoShaderProgram.h>
#include <Inventor/nodes/SoVertexShader.h>
#include <Inventor/nodes/SoGeometryShader.h>
#include <Inventor/nodes/SoFragmentShader.h>
#include <Inventor/nodes/SoMaterial.h>

HX_INIT_CLASS(HxGLSLVertexView,HxModule);

HxGLSLVertexView::HxGLSLVertexView() 
: HxModule(HxVertexSet::getClassTypeId()),
  portDrawStyle  (this, "drawStyle", tr("Draw Style"), 4),
  portSphereScale(this, "sphereScale", tr("Sphere Scale"))
{
    portDrawStyle.setLabel(POINTS,"Points");
    portDrawStyle.setLabel(PLATES,"Plates");
    portDrawStyle.setLabel(HAZE,"Haze");
    portDrawStyle.setLabel(SPHERES,"Spheres");
    portDrawStyle.setValue(SPHERES);

    portSphereScale.setMinMax(1,10000);
    portSphereScale.setValue(1000);

    m_p_root = new SoSeparator;
    m_p_material = new SoMaterial;
    m_p_vertexProperty = new SoVertexProperty;
    
    SoPointSet* pointSet = new SoPointSet;
    pointSet->vertexProperty.setValue(m_p_vertexProperty);

    m_p_sphereSizeParameter = new SoShaderParameter1f;
    m_p_sphereSizeParameter->name = "sphere_size";
    m_p_sphereSizeParameter->value = portSphereScale.getValue();

    m_p_vertexShader = new SoVertexShader;

    m_p_geometryshader = new SoGeometryShader;

    m_p_fragmentshader = new SoFragmentShader;
    
    m_p_shaderProgram = new SoShaderProgram;
    m_p_shaderProgram->shaderObject.addNode(m_p_vertexShader);
    m_p_shaderProgram->shaderObject.addNode(m_p_geometryshader);
    m_p_shaderProgram->shaderObject.addNode(m_p_fragmentshader);
    m_p_shaderProgram->geometryInputType = SoShaderProgram::POINTS_INPUT;
    m_p_shaderProgram->geometryOutputType = SoShaderProgram::TRIANGLE_STRIP_OUTPUT;
    m_p_shaderProgram->maxGeometryOutputVertices = 4;

    m_p_root->addChild(m_p_material);
    m_p_root->addChild(m_p_shaderProgram);
    m_p_root->addChild(pointSet);
} 

HxGLSLVertexView::~HxGLSLVertexView()
{
    hideGeom(m_p_root);
}

void HxGLSLVertexView::update()
{
    if (portDrawStyle.isNew())
    {
        setRenderMode((DrawStyles)portDrawStyle.getValue());

        portSphereScale.setVisible( portDrawStyle.getValue() != POINTS );
    }

    if (portData.isNew())
    {
        HxVertexSet* data = hxconnection_cast<HxVertexSet>(portData);
        if (data)
        {
            const McVec3f size = data->getBoundingBoxSize();

            float min_size = MC_MIN3(size[0], size[1],size[2]) / 30.0f;

            portSphereScale.setMinMax(0, min_size);
            portSphereScale.setValue( min_size / 2.0f);
        }
    }
}

void HxGLSLVertexView::compute()
{
    // data has changed ?
    if (portData.isNew())
    {
        HxVertexSet* data = hxconnection_cast<HxVertexSet>(portData);
        
        if (data)
        {
            m_p_vertexProperty->vertex.setValuesPointer(data->getNumPoints(), (SbVec3f*)data->getCoords());

            showGeom(m_p_root);
        }
        else
        {
            m_p_vertexProperty->vertex.setNum(0);

            hideGeom(m_p_root);
        }
    }

    if (portSphereScale.isNew())
    {
        m_p_sphereSizeParameter->value = portSphereScale.getValue();
    }
}

void HxGLSLVertexView::setRenderMode(DrawStyles mode)
{
    const SbString shaderPath = SbString(HxResource::getRootDir()) + "/share/shaders/hxglslvertexview/";
    
    m_p_geometryshader->parameter.removeAllShaderParameters();

    if (mode == POINTS)
    {
        m_p_shaderProgram->geometryOutputType = SoShaderProgram::POINTS_OUTPUT;
        m_p_shaderProgram->maxGeometryOutputVertices = 1;
    }
    else
    {
        m_p_shaderProgram->geometryOutputType = SoShaderProgram::TRIANGLE_STRIP_OUTPUT;
        m_p_shaderProgram->maxGeometryOutputVertices = 4;

        m_p_geometryshader->parameter.addShaderParameter(m_p_sphereSizeParameter);
    }

    if (mode == HAZE)
        m_p_material->transparency =  1.0f;
    else
        m_p_material->transparency.deleteValues(0);

    switch(mode)
    {
    case POINTS:
        m_p_vertexShader->sourceProgram   = shaderPath + "points-vert.glsl";
        m_p_geometryshader->sourceProgram = shaderPath + "points-geom.glsl";
        m_p_fragmentshader->sourceProgram = shaderPath + "points-frag.glsl";
        break;
    case PLATES:
        m_p_vertexShader->sourceProgram   = shaderPath + "quad-vert.glsl";
        m_p_geometryshader->sourceProgram = shaderPath + "quad-geom.glsl";
        m_p_fragmentshader->sourceProgram = shaderPath + "plates-frag.glsl";
        break;
    case HAZE:
        m_p_vertexShader->sourceProgram   = shaderPath + "quad-vert.glsl";
        m_p_geometryshader->sourceProgram = shaderPath + "quad-geom.glsl";
        m_p_fragmentshader->sourceProgram = shaderPath + "hazes-frag.glsl";
        break;
    case SPHERES:
        m_p_vertexShader->sourceProgram   = shaderPath + "quad-vert.glsl";
        m_p_geometryshader->sourceProgram = shaderPath + "quad-geom.glsl";
        m_p_fragmentshader->sourceProgram = shaderPath + "spheres-frag.glsl";
        break;
    }
}

