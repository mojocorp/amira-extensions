#pragma once

#include <hxmorgan/api.h>

#include <hxcore/HxModule.h>
#include <hxcore/HxPortRadioBox.h>

class SoSeparator;
class SoMaterial;
class SoVertexProperty;
class SoIndexedFaceSet;

class HXMORGAN_API HxOIVDisplaySurface : public HxModule
{
    HX_HEADER(HxOIVDisplaySurface);
public:
    /// Constructor.
    HxOIVDisplaySurface();
    
    HxPortRadioBox portRenderCaching;
    
    /// Update method.
    virtual void update();

    /// Compute method.
    virtual void compute();
protected:
    /// Destructor.
    virtual ~HxOIVDisplaySurface();
private:
    SoSeparator* m_p_root;
    SoMaterial* m_p_material;
    SoVertexProperty* m_p_vertexProperty;
    SoIndexedFaceSet* m_p_faceSet;
};
