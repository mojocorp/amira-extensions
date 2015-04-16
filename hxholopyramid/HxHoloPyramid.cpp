#include "HxHoloPyramid.h"

#include <hxcore/HxMain.h>
#include <hxholopyramid/QxHoloPyramidWidget.h>

HX_INIT_CLASS(HxHoloPyramid, HxModule);

HxHoloPyramid::HxHoloPyramid()
   : HxModule(),
   m_viewer(new QxHoloPyramidWidget(theMainWindow))
{
    m_viewer->resize(800, 600);
    m_viewer->show();
    m_viewer->raise();
}

HxHoloPyramid::~HxHoloPyramid()
{
}

void HxHoloPyramid::update()
{

}

void HxHoloPyramid::compute()
{

}
