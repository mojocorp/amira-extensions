#pragma once

#include <hxholopyramid/api.h>

#include <hxcore/HxModule.h>

/// Converts a vertex set into a cluster object.
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
};
