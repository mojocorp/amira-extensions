#pragma once

#include <hxmorgan/api.h>

#include <hxcore/HxCompModule.h>
#include <hxcore/HxPortInfo.h>
#include <hxcore/HxPortIntTextN.h>
#include <hxcore/HxPortDoIt.h>

#include <mclib/McDim3l.h>

class McProgressInterface;
class HxUniformScalarField3;

class HXMORGAN_API HxCreateTiles : public HxCompModule
{
    HX_HEADER(HxCreateTiles);

public:
    HxPortInfo portInfo;

    HxPortIntTextN portNumTiles;

    /// Port associated to the action button.
    HxPortDoIt portAction;

    /// Updates the ports.
    virtual void
    update();

    /// Starts the computation.
    virtual void
    compute();

private:
    void updateInfo();
    void generateTiles(const HxUniformScalarField3* src, const McDim3l& numTiles, McProgressInterface* progress);
};
