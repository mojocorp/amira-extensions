#include "HxOIVDisplaySurface.h"

#include <hxcore/HxResource.h>
#include <hxcore/HxMatDatabase.h>
#include <hxsurface/HxSurface.h>
#include <hxfield/HxUniformColorField3.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoMaterialBinding.h>

HX_INIT_CLASS(HxOIVDisplaySurface,HxModule)

HxOIVDisplaySurface::HxOIVDisplaySurface() : 
    HxModule(HxSurface::getClassTypeId()),
    portRenderCaching(this, "renderCaching", tr("Render Cahching"), 3)
{
    portRenderCaching.setValue(1);
    portRenderCaching.setLabel(0,"On");
    portRenderCaching.setLabel(1,"Off");
    portRenderCaching.setLabel(2,"Auto");
    
    m_p_root = new SoSeparator;
    m_p_root->renderCaching = SoSeparator::OFF; // Force VBO activation
    m_p_root->ref();
#if 0
    SoShapeHints* shapeHints = new SoShapeHints;
    shapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
    shapeHints->useVBO = TRUE;
    m_p_root->addChild(shapeHints);
#endif    
    m_p_material = new SoMaterial;
    m_p_material->diffuseColor.setValue(SbColor(1,1,1));
    m_p_root->addChild(m_p_material);
    
    SoMaterialBinding* materialBinding = new SoMaterialBinding;
    materialBinding->value = SoMaterialBinding::PER_FACE_INDEXED;
    m_p_root->addChild(materialBinding);
    
    m_p_vertexProperty = new SoVertexProperty;

    m_p_faceSet = new SoIndexedFaceSet;
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
    if (portRenderCaching.isNew())
    {
        switch(portRenderCaching.getValue())
        {
            case 0: m_p_root->renderCaching = SoSeparator::ON; break;
            case 1: m_p_root->renderCaching = SoSeparator::OFF; break;
            case 2: m_p_root->renderCaching = SoSeparator::AUTO; break;
            default: break;
        }
    }
}

void HxOIVDisplaySurface::compute()
{
    HxSurface* surface = hxconnection_cast<HxSurface>(portData);
    if (surface)
    {
        //////////////////////////////////////////////////////
        // Materials
        //////////////////////////////////////////////////////
        HxParamBundle* materials = surface->parameters.materials();
        int nMaterials = materials->nBundles();
        
        m_p_material->diffuseColor.setNum(nMaterials);
        SbColor * matptr = m_p_material->diffuseColor.startEditing();
        
        for (int k=0; k<surface->patches.size(); k++) 
        {
            const Surface::Patch* patch = surface->patches[k];
            
            int innerRegion = patch->innerRegion;
            if (innerRegion < nMaterials) 
            {
                const HxParamBundle* b = materials->bundle(innerRegion);
                SbColor color;
                if (!b->findColor(&color[0]))
                    color = matDatabase->getColor(qPrintable(b->name()));
                matptr[k] = color;
            }
        }
        m_p_material->diffuseColor.finishEditing();
        //////////////////////////////////////////////////////
        // Vertices
        //////////////////////////////////////////////////////
        m_p_vertexProperty->vertex.setNum(surface->points.size());
        
        SbVec3f* coords = m_p_vertexProperty->vertex.startEditing();
        for (int i=0; i<surface->points.size(); i++)
        {
            const McVec3f & pt = surface->points[i];

            coords[i].setValue(pt.x, pt.y, pt.z); 
        }
        m_p_vertexProperty->vertex.finishEditing();

        //////////////////////////////////////////////////////
        // Normals
        //////////////////////////////////////////////////////
        m_p_vertexProperty->normal.setNum(surface->normals.size());

        SbVec3f* normal = m_p_vertexProperty->normal.startEditing();
        for (int i=0; i<surface->normals.size(); i++)
        {
            const McVec3f & n = surface->normals[i];

            normal[i].setValue(n.x, n.y, n.z); 
        }
        m_p_vertexProperty->normal.finishEditing();

        if (surface->normals.size() == surface->triangles.size())
            m_p_vertexProperty->normalBinding = SoVertexProperty::PER_FACE;
        else
            m_p_vertexProperty->normalBinding = SoVertexProperty::PER_VERTEX;

        //////////////////////////////////////////////////////
        // Coords Indexes
        //////////////////////////////////////////////////////
        m_p_faceSet->coordIndex.setNum(4*surface->triangles.size());
        m_p_faceSet->materialIndex.setNum(surface->triangles.size());

        int32_t* faceidx = m_p_faceSet->coordIndex.startEditing();
        int32_t* matidx = m_p_faceSet->materialIndex.startEditing();
        for (int i=0; i<surface->triangles.size(); i++)
        {
            const Surface::Triangle & tri = surface->triangles[i];
            
            faceidx[4*i+0] = tri.points[0];
            faceidx[4*i+1] = tri.points[1];
            faceidx[4*i+2] = tri.points[2];
            faceidx[4*i+3] = -1;
            
            matidx[i] = tri.patch;
        }
        m_p_faceSet->coordIndex.finishEditing();
        m_p_faceSet->materialIndex.finishEditing();
        
        showGeom(m_p_root);
    }
    else
    {
        hideGeom(m_p_root);

        m_p_vertexProperty->vertex.setNum(0);
        m_p_faceSet->coordIndex.setNum(0);
    }
}
