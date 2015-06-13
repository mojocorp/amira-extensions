#include "HxOIVDisplaySurface.h"

#include <hxcore/HxResource.h>
#include <hxcore/HxMatDatabase.h>
#include <hxcore/HxMessage.h>
#include <hxsurface/HxSurface.h>
#include <hxfield/HxUniformColorField3.h>
#include <hxsurface/HxSurfaceComplexScalarField.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoBBox.h>
#include <Inventor/nodes/SoIndexedTriangleSet.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoTexture2.h>

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
    /*
    SoShapeHints* shapeHints = new SoShapeHints;
    shapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
    //shapeHints->useVBO = TRUE;
    m_p_root->addChild(shapeHints);
    */
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

    m_p_vertexProperty = new SoVertexProperty;

    m_p_faceSet = new SoIndexedTriangleSet;
    m_p_faceSet->vertexProperty.setValue(m_p_vertexProperty);

    m_p_root->addChild(m_p_faceSet);
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
        const float* tc   = textureCoords->dataPtr();
        m_p_vertexProperty->texCoord.setNum(textureCoords->nDataElements());
        SbVec2f* uvptr = m_p_vertexProperty->texCoord.startEditing();
        for (int i=0; i<textureCoords->nDataElements(); i++)
        {
            SbVec2f & uv = uvptr[i];
            uv[0] = tc[2*i+0];
            uv[1] = 1.0f - tc[2*i+1];
        }
        m_p_vertexProperty->texCoord.finishEditing();
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
            m_p_vertexProperty->normal.setValues(0, surface->normals.size(), (const SbVec3f *)surface->normals.dataPtr());
            m_p_vertexProperty->normalBinding = SoVertexProperty::PER_VERTEX_INDEXED;

            //////////////////////////////////////////////////////
            // Coords Indexes
            //////////////////////////////////////////////////////
            m_p_faceSet->coordIndex.setNum(3*surface->triangles.size());

            int32_t* faceidx = m_p_faceSet->coordIndex.startEditing();
            for (int i=0; i<surface->triangles.size(); i++)
            {
                const Surface::Triangle & tri = surface->triangles[i];

                *(faceidx++) = tri.points[0];
                *(faceidx++) = tri.points[1];
                *(faceidx++) = tri.points[2];
            }
            m_p_faceSet->coordIndex.finishEditing();

            showGeom(m_p_root);
        }
        else
        {
            hideGeom(m_p_root);

            m_p_vertexProperty->vertex.setNum(0);
            m_p_faceSet->coordIndex.setNum(0);
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
            m_p_vertexProperty->vertex.setValues(0, surface->points.size(), (const SbVec3f *)surface->points.dataPtr());
        }
    }
}
