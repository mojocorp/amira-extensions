#include "HxLorenzAttractor.h"

#include <hxlines/HxLineSet.h>

HX_INIT_CLASS(HxLorenzAttractor,HxCompModule);

HxLorenzAttractor::HxLorenzAttractor() :
    HxCompModule(HxData::getClassTypeId()),
    portNumPoints(this, "numPoints"),
    portAction(this, "action")
{
    portNumPoints.setMinMax(0, 10000);
    portNumPoints.setValue(1000);
}

HxLorenzAttractor::~HxLorenzAttractor()
{
}

void HxLorenzAttractor::update()
{
    portData.hide();
}

void HxLorenzAttractor::compute()
{
    if (portAction.wasHit())
    {
        McHandle<HxLineSet> lineset = dynamic_cast<HxLineSet*>(getResult());
        if (!lineset)
        {
            lineset = new HxLineSet();
        }

        lineset->points.resize(portNumPoints.getValue());

        lineset->lines.resize(1);
        lineset->lines[0].points.resize(portNumPoints.getValue());

        const double h = 0.01;
        const double a = 10.0;
        const double b = 28.0;
        const double c = 8.0 / 3.0;

        double x0 = 0.1;
        double y0 = 0;
        double z0 = 0;
        for (int i=0; i<portNumPoints.getValue(); i++) 
        {
            const double x1 = x0 + h * a * (y0 - x0);
            const double y1 = y0 + h * (x0 * (b - z0) - y0);
            const double z1 = z0 + h * (x0 * y0 - c * z0);
            x0 = x1;
            y0 = y1;
            z0 = z1;

            lineset->points[i].setValue(x0,y0,z0);
            lineset->lines[0].points[i] = i;
        }

        lineset->composeLabel("LorenzAttractor","lineset");
        lineset->touch();
        setResult(0, lineset);
    }
}
