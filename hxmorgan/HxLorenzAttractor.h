#pragma once

#include <hxmorgan/api.h>

#include <hxcore/HxPortIntSlider.h>
#include <hxcore/HxCompModule.h>
#include <hxcore/HxPortDoIt.h>

/// Converts a vertex set into a cluster object.
class HXMORGAN_API HxLorenzAttractor : public HxCompModule
{
    HX_HEADER(HxLorenzAttractor);

public:
    /// Number of lineset points.
    HxPortIntSlider portNumPoints;

    /// Button to start the computation.
    HxPortDoIt portAction;

    /// Update method.
    virtual void update();

    /// Compute method.
    virtual void compute();
};
