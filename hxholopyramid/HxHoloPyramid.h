#pragma once

#include <QScopedPointer>

#include <hxholopyramid/api.h>

#include <hxcore/HxModule.h>

class QxHoloPyramidWidget;

class HXHOLOPYRAMID_API HxHoloPyramid : public HxModule 
{
    HX_HEADER(HxHoloPyramid);

  public:
    /// Constructor.
    HxHoloPyramid();

    /// Update method.
    virtual void update();

    /// Compute method.
    virtual void compute();

  protected:
    virtual ~HxHoloPyramid();

    QScopedPointer<QxHoloPyramidWidget> m_viewer;
};
