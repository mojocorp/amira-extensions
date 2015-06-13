#include "HxOIVDisplaySurface.h"

#include <hxcore/HxResource.h>
#include <hxcore/HxMatDatabase.h>
#include <hxcore/HxMessage.h>
#include <hxsurface/HxSurface.h>
#include <hxfield/HxUniformColorField3.h>
#include <hxsurface/HxSurfaceComplexScalarField.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoBBox.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoBufferedShape.h>
#include <Inventor/devices/SoCpuBufferObject.h>

HX_INIT_CLASS(HxOIVDisplaySurface,HxModule)

HxOIVDisplaySurface::HxOIVDisplaySurface() : 
    HxModule(HxSurface::getClassTypeId()),
    portEmissiveColor(this, "emissiveColor", tr("Emissive Color")),
    portTexture(this, "texture", tr("Texture"), HxUniformColorField3::getClassTypeId()),
    portTextureCoord(this, "textureCoord", tr("Texture Coordinates"), HxSurfaceComplexScalarField::getClassTypeId())
{
    portEmissiveColor.setMinMax(0, 255);
    portEmissiveColor.setValue(0);

    m_p_root = new SoSeparator;
    m_p_root->ref();

    m_p_material = new SoMaterial;
    //m_p_material->ambientColor.setValue(SbColor(0.19225, 0.19225, 0.19225));
    //m_p_material->diffuseColor.setValue(SbColor(255.f/250.f, 255.f/224.f, 255.f/195.f));
    //m_p_material->specularColor.setValue(SbColor(255.f/131.f, 255.f/34.f, 255.f/40.f));
    //m_p_material->emissiveColor.setValue(SbColor(1,1,1));
    m_p_root->addChild(m_p_material);
    
    m_p_texture = new SoTexture2;
    m_p_texture->model = SoTexture2::MODULATE;
    m_p_root->addChild(m_p_texture);

    m_p_boundingBoxNode = new SoBBox;
    m_p_boundingBoxNode->mode.setValue(SoBBox::USER_DEFINED);
    m_p_root->addChild(m_p_boundingBoxNode);

    m_p_vertices = new SoGLBufferObject(SoGLBufferObject::DYNAMIC_DRAW);
    m_p_normals = new SoGLBufferObject(SoGLBufferObject::STATIC_DRAW);
    m_p_texcoords = new SoGLBufferObject(SoGLBufferObject::STATIC_DRAW);
    m_p_indices = new SoGLBufferObject(SoGLBufferObject::STATIC_DRAW);

    m_p_shape = new SoBufferedShape;
    m_p_shape->shapeType = SoBufferedShape::TRIANGLES;
    m_p_shape->indexType = SbDataType::UNSIGNED_INT32;
    m_p_shape->normalBuffer = m_p_normals;
    m_p_shape->vertexBuffer = m_p_vertices;
    m_p_shape->indexBuffer = m_p_indices;
    m_p_shape->texCoordsBuffer = m_p_texcoords;

    m_p_root->addChild(m_p_shape);
}

HxOIVDisplaySurface::~HxOIVDisplaySurface()
{
    hideGeom(m_p_root);

    m_p_root->unref();
}

void HxOIVDisplaySurface::update()
{

}

void HxOIVDisplaySurface::compute()
{
    //////////////////////////////////////////////////////
    // Emisssive Color
    //////////////////////////////////////////////////////
    if (portEmissiveColor.isNew())
    {
        const float value = portEmissiveColor.getValue() / 255.0f;

        m_p_material->emissiveColor.setValue(SbColor(value, value, value));
    }

    //////////////////////////////////////////////////////
    // Texture
    //////////////////////////////////////////////////////
    const HxUniformColorField3* colorfield = hxconnection_cast<HxUniformColorField3>(portTexture);
    if (portTexture.isNew() && colorfield)
    {
        const int* dims = colorfield->lattice.dimsInt();
        const unsigned char* srcPtr= (unsigned char*) colorfield->lattice.dataPtr();
        m_p_texture->image.setValue( SbVec2i32(dims[0], dims[1]), 4, SoSFImage::UNSIGNED_BYTE, srcPtr);
    }

    //////////////////////////////////////////////////////
    // Texture Coordinates
    //////////////////////////////////////////////////////
    const HxSurfaceComplexScalarField* textureCoords = hxconnection_cast<HxSurfaceComplexScalarField>(portTextureCoord);
    if (portTextureCoord.isNew() && textureCoords)
    {
        m_p_texcoords->setTarget(SoGLBufferObject::ARRAY_BUFFER);
        m_p_texcoords->setSize(2 * textureCoords->nDataElements() * sizeof(float));
        const float* tc   = textureCoords->dataPtr();
        float* uvptr = (float *)m_p_texcoords->map( SoBufferObject::READ_WRITE );
        for (int i=0; i<textureCoords->nDataElements(); i++)
        {
            *(uvptr++) = tc[2*i+0];
            *(uvptr++) = 1.0f - tc[2*i+1];
        }
        m_p_texcoords->unmap();
    }

    if (portData.isNew(HxData::NEW_SOURCE)) 
    {
        HxSurface* surface = hxconnection_cast<HxSurface>(portData);
        if (surface)
        {
            surface->computeNormalsPerVertexIndexed();
            //////////////////////////////////////////////////////
            // Bounding Box
            //////////////////////////////////////////////////////
            float bbox[6];
            surface->getBoundingBox(bbox);
            m_p_boundingBoxNode->boundingBox.setValue(bbox[0], bbox[2], bbox[4], bbox[1], bbox[3], bbox[5]);

            //////////////////////////////////////////////////////
            // Normals
            //////////////////////////////////////////////////////
            m_p_normals->setTarget(SoGLBufferObject::ARRAY_BUFFER);
            m_p_normals->setSize( surface->normals.size() * sizeof(SbVec3f) );
            float* normalsPtr = (float *)m_p_normals->map( SoBufferObject::READ_WRITE );
            for (int i=0; i<surface->normals.size(); i++)
            {
                *(normalsPtr++) = surface->normals[i][0];
                *(normalsPtr++) = surface->normals[i][1];
                *(normalsPtr++) = surface->normals[i][2];
            }
            m_p_normals->unmap();

            //////////////////////////////////////////////////////
            // Indices
            //////////////////////////////////////////////////////
            m_p_indices->setTarget(SoGLBufferObject::ELEMENT_ARRAY_BUFFER);
            m_p_indices->setSize(3 * surface->triangles.size() * sizeof(unsigned int));
            unsigned int* indicesPtr = (unsigned int *)m_p_indices->map( SoBufferObject::READ_WRITE );
            for (int i=0; i<surface->triangles.size(); i++)
            {
                const Surface::Triangle & tri = surface->triangles[i];

                *(indicesPtr++) = tri.points[0];
                *(indicesPtr++) = tri.points[1];
                *(indicesPtr++) = tri.points[2];
            }
            m_p_indices->unmap();

            m_p_shape->numVertices.set1Value( 0, 3 * surface->triangles.size() );

            showGeom(m_p_root);
        }
        else
        {
            hideGeom(m_p_root);
        }
    }

    if (portData.isNew())
    {
        HxSurface* surface = hxconnection_cast<HxSurface>(portData);
        if (surface)
        {
            //////////////////////////////////////////////////////
            // Vertices
            //////////////////////////////////////////////////////
            m_p_vertices->setTarget(SoGLBufferObject::ARRAY_BUFFER);
            //m_p_vertices->bind();
            m_p_vertices->setSize( surface->points.size() * sizeof(SbVec3f) );
            float* verticesPtr = (float *)m_p_vertices->map( SoBufferObject::READ_WRITE );
            for (int i=0; i<surface->points.size(); i++)
            {
                *(verticesPtr++) = surface->points[i][0];
                *(verticesPtr++) = surface->points[i][1];
                *(verticesPtr++) = surface->points[i][2];
            }
            m_p_vertices->unmap();
            //m_p_vertices->unbind();

            m_p_shape->touch();
        }
    }
}
