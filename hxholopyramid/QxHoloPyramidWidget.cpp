#include "QxHoloPyramidWidget.h"

#include <QHBoxLayout>

#include <Inventor/Qt/viewers/SoQtViewer.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/sensors/SoNodeSensor.h>

#include <hxcore/HxController.h>
#include <hxcore/HxViewer.h>

class QxHoloViewer : public SoQtViewer
{
public:
    QxHoloViewer(QWidget* parent) : SoQtViewer(parent, "holoviewer", true, SoQtViewer::BROWSER, true) 
    {
    }

    void actualRedraw()
    {
        SoCamera* camera = getCamera();

        adjustCameraClippingPlanes();

        getSceneManager()->render(true, true);

        camera->enableNotify(false);
        rotate(SbRotation(SbVec3f(0,1,0), M_PI/2));

        adjustCameraClippingPlanes();
        getSceneManager()->render(false, false);

        rotate(SbRotation(SbVec3f(0,1,0), M_PI));

        adjustCameraClippingPlanes();
        getSceneManager()->render(false, false);
        camera->enableNotify(true);
    }

    void rotate(const SbRotation &rot)
    {
        SoCamera* camera = getCamera();

        // get center of rotation
        float radius = camera->focalDistance.getValue();

        SbVec3f forward;
        camera->orientation.getValue().multVec(SbVec3f(0,0,-1), forward);

        SbVec3f center = camera->position.getValue() + radius * forward;

        // apply new rotation to the camera
        camera->orientation = rot * camera->orientation.getValue();

        // reposition camera to look at pt of interest
        camera->orientation.getValue().multVec(SbVec3f(0,0,-1), forward);
        camera->position = center - radius * forward;
    }
};

QxHoloPyramidWidget::QxHoloPyramidWidget(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Window);
    
    root = new SoSeparator;
    camera = new SoPerspectiveCamera;

    viewer = new QxHoloViewer(this);
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(viewer->getWidget());

    HxViewer* hxviewer = theController->viewer(0);

    sensor = new SoNodeSensor;
    sensor->setPriority(0);
    sensor->setFunction(cameraChangedCB, this);
    sensor->attach(hxviewer->getCamera());

    root->addChild(camera);
    root->addChild(hxviewer->getSceneGraph());

    viewer->setSceneGraph(root);
}

QxHoloPyramidWidget::~QxHoloPyramidWidget()
{
    delete sensor;
    delete viewer;
}

void QxHoloPyramidWidget::cameraChangedCB(void *data, SoSensor *sensor)
{
    QxHoloPyramidWidget* _this = (QxHoloPyramidWidget*)data;

    HxViewer* hxviewer = theController->viewer(0);

    _this->camera->copyFieldValues(hxviewer->getCamera());
}
