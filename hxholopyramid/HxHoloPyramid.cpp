#include "HxHoloPyramid.h"

#include <QDesktopWidget>

#include <hxcore/HxMain.h>
#include <hxholopyramid/QxHoloPyramidWidget.h>

HX_INIT_CLASS(HxHoloPyramid, HxModule);

HxHoloPyramid::HxHoloPyramid()
   : HxModule(),
   portDisplay(this, "display", tr("Display")),
   portOptions(this, "options"),
   m_viewer(new QxHoloPyramidWidget(theMainWindow))
{
    m_viewer->resize(800, 600);
    m_viewer->show();
    m_viewer->raise();

    QDesktopWidget desktop;
    portDisplay.setMinMax(0, desktop.screenCount() - 1);

    portOptions.insertCheckBox(0, "fullscreen");

    setFlag( HxObject::CAN_BE_REMOVED_ALL, false );
}

HxHoloPyramid::~HxHoloPyramid()
{
}

void HxHoloPyramid::update()
{
    if (portDisplay.isNew()) {
        portOptions.setValue(0, 0);
    }
}

void HxHoloPyramid::compute()
{
    if (portDisplay.isNew()) {
        QDesktopWidget desktop;
        QRect geometry = desktop.availableGeometry(portDisplay.getValue());
        m_viewer->move(geometry.topLeft());
    }

    if (portOptions.isItemNew(0)) {
        if (portOptions.getValue(0) == 1) {
            m_viewer->showFullScreen();
        } else {
            m_viewer->showNormal();
        }
    }
}
