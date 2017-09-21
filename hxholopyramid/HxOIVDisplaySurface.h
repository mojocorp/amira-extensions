#pragma once

#include <hxholopyramid/api.h>

#include <hxcore/HxModule.h>
#include <hxcore/HxPortRadioBox.h>
#include <hxcore/HxPortIntSlider.h>

class SoSeparator;
class SoMaterial;
class SoTexture2;
class SoBBox;
class SoGLBufferObject;
class SoBufferedShape;

class HXHOLOPYRAMID_API HxOIVDisplaySurface : public HxModule
{
    HX_HEADER(HxOIVDisplaySurface);
public:
    
    HxPortIntSlider portEmissiveColor;
    HxConnection portTexture;
    HxConnection portTextureCoord;

    /// Update method.
    virtual void update();

    /// Compute method.
    virtual void compute();

private:
    SoSeparator* m_p_root;
    SoMaterial* m_p_material;
    SoTexture2* m_p_texture;
    SoBBox* m_p_boundingBoxNode;
    SoGLBufferObject* m_p_vertices;
    SoGLBufferObject* m_p_texcoords;
    SoGLBufferObject* m_p_normals;
    SoGLBufferObject* m_p_indices;
    SoBufferedShape* m_p_shape;
};
