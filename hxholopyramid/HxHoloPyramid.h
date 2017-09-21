#pragma once

#include <QScopedPointer>

#include <hxholopyramid/api.h>

#include <hxcore/HxModule.h>
#include <hxcore/HxPortIntSlider.h>
#include <hxcore/HxPortGeneric.h>

class QxHoloPyramidWidget;

class HXHOLOPYRAMID_API HxHoloPyramid : public HxModule 
{
    HX_HEADER(HxHoloPyramid);

  public:

    /// 
    HxPortIntSlider portDisplay;
    HxPortGeneric portOptions;

    /// Update method.
    virtual void update();

    /// Compute method.
    virtual void compute();

  protected:

    QScopedPointer<QxHoloPyramidWidget> m_viewer;
};
