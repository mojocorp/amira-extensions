#include "HxCreateTiles.h"

#include <hxcore/HxMessage.h>
#include <hxcore/internal/HxWorkArea.h>

#include <hxfield/HxUniformScalarField3.h>

#include <mclib/McDim3l.h>
#include <mclib/McException.h>
#include <mclib/McRawData3D.h>

#include <QTime>

namespace
{
template <class T>
McHandle<T>
createOrReuseResult(const HxUniformScalarField3* data, const McDim3l& dims, HxData* connectedResult)
{
    McHandle<HxUniformScalarField3> result(mcinterface_cast<HxUniformScalarField3>(connectedResult));
    if (!result || result->primType() != data->primType())
    {
        result = new HxUniformScalarField3(dims, data->primType());
        result->composeLabel(data->getLabel(), "tiled");
    }
    const McDim3l& srcDims = data->lattice().getDims();
    const McVec3f bboxSize = data->getBoundingBox().getSize();
    const McVec3f bboxMin = data->getBoundingBox().getMin();
    const McVec3f bboxMax(bboxMin[0] + bboxSize[0] * dims[0] / float(srcDims[0]),
                          bboxMin[1] + bboxSize[1] * dims[1] / float(srcDims[1]),
                          bboxMin[2] + bboxSize[2] * dims[2] / float(srcDims[2]));

    result->lattice().resize(dims);
    result->lattice().setBoundingBox(McBox3f(bboxMin, bboxMax));
    return result;
}
}

HX_INIT_CLASS(HxCreateTiles, HxCompModule);

HxCreateTiles::HxCreateTiles()
    : HxCompModule(HxUniformScalarField3::getClassTypeId())
    , portInfo(this, "info", tr("Info"))
    , portNumTiles(this, "numberTiles", tr("Number Of Tiles"), 3)
    , portAction(this, "action", tr("Action"))
{
    portNumTiles.setLabel(0, "X");
    portNumTiles.setLabel(1, "Y");
    portNumTiles.setLabel(2, "Z");

    portNumTiles.setMinMax(0, 1, 1000);
    portNumTiles.setMinMax(1, 1, 1000);
    portNumTiles.setMinMax(2, 1, 1000);
}

HxCreateTiles::~HxCreateTiles()
{
}

void HxCreateTiles::update()
{
    const HxUniformScalarField3* data = hxconnection_cast<HxUniformScalarField3>(portData);
    if (data)
    {
        if (portData.isNew())
        {
            portNumTiles.setValue(0, 1);
            portNumTiles.setValue(1, 1);
            portNumTiles.setValue(2, 1);
        }
    }
    updateInfo();
}

void HxCreateTiles::compute()
{
    if (!portAction.wasHit())
    {
        return;
    }

    const HxUniformScalarField3* data = hxconnection_cast<HxUniformScalarField3>(portData);
    if (!data)
    {
        mcthrow("Missing input 'data'.");
    }

    const McDim3l numTiles(portNumTiles.getValue(0),
                           portNumTiles.getValue(1),
                           portNumTiles.getValue(2));

    generateTiles(data, numTiles, theWorkArea);
}

void HxCreateTiles::updateInfo()
{
    const HxUniformScalarField3* data = hxconnection_cast<HxUniformScalarField3>(portData);
    if (!data)
    {
        portInfo.printf("--");
        return;
    }
    const McDim3l& srcDims = data->lattice().getDims();
    const McDim3l dstDims(srcDims.nx * portNumTiles.getValue(0),
                          srcDims.ny * portNumTiles.getValue(1),
                          srcDims.nz * portNumTiles.getValue(2));

    portInfo.printf("%dx%dx%d --> %dx%dx%d", srcDims[0], srcDims[1], srcDims[2], dstDims[0], dstDims[1], dstDims[2]);
}

void HxCreateTiles::generateTiles(const HxUniformScalarField3* src, const McDim3l& numTiles, McProgressInterface* progress)
{
    const McDim3l& srcDims = src->lattice().getDims();
    const McDim3l dstDims(srcDims.nx * numTiles[0],
                          srcDims.ny * numTiles[1],
                          srcDims.nz * numTiles[2]);

    // create or reuse result
    McHandle<HxUniformScalarField3> dst = createOrReuseResult<HxUniformScalarField3>(src, dstDims, getResult());

    const mcint64 updates = std::max<mcuint64>(1, numTiles.nbVoxel() / 100);

    QTime time;
    time.start();

    progress->startWorking("Generating Tiles. This may take a while...");
    McScopeExitStopWorking guard(progress);

    McRawData3D srcRawData(srcDims.nx, srcDims.ny, srcDims.nz, src->lattice().primType().getTypeSize(), src->lattice().dataPtr());
    McRawData3D dstRawData(dstDims.nx, dstDims.ny, dstDims.nz, dst->lattice().primType().getTypeSize(), dst->lattice().dataPtr());

    mcint64 count = 0;
    for (mcuint64 k = 0; k < numTiles[2]; ++k)
    {
        for (mcuint64 j = 0; j < numTiles[1]; ++j)
        {
            for (mcuint64 i = 0; i < numTiles[0]; ++i)
            {
                dstRawData.replaceSubvolume(i * srcDims.nx, j * srcDims.ny, k * srcDims.nz, srcRawData);

                if (count % updates == 0)
                {
                    progress->setProgressValue(count / float(numTiles.nbVoxel()));
                    if (progress->wasInterrupted())
                    {
                        mcthrowT(McWorkInterrupted, "Tiles generation has been canceled.");
                    }
                }
                count++;
            }
        }
    }

    theMsg->printf(QString("elapsed: %1ms").arg((int)time.elapsed()));
    setResult(dst);
}
