#include "QxHoloPyramidWidget.h"

#include <QHBoxLayout>

#include <Inventor/Qt/viewers/SoQtViewer.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoViewport.h>
#include <Inventor/sensors/SoNodeSensor.h>

#include <hxcore/HxController.h>
#include <hxcore/HxViewer.h>

class QxHoloViewer : public SoQtViewer
{
public:
    QxHoloViewer(QWidget* parent) : SoQtViewer(parent, "holoviewer", true, SoQtViewer::BROWSER, true) 
    {
        root = new SoSeparator;
        camera = new SoPerspectiveCamera;
        viewport = new SoViewport;

        sensor = new SoNodeSensor;
        sensor->setPriority(0);
        sensor->setFunction(cameraChangedCB, this);
    }

    ~QxHoloViewer()
    {
        delete sensor;
    }

    virtual void setSceneGraph (SoNode *newScene)
    {
        HxViewer* hxviewer = theController->viewer(0);

        root->removeAllChildren();
        root->addChild(viewport);
        root->addChild(camera);
        root->addChild(newScene);

        sensor->attach(hxviewer->getCamera());

        SoQtViewer::setSceneGraph(root);
    }

    void actualRedraw()
    {
        int width = getSize()[0];
        int height = getSize()[1];

        int vpsize = height / 2;

        SoCamera* camera = getCamera();
        camera->enableNotify(false);

        viewport->enableNotify(false);
        viewport->size.setValue(vpsize, vpsize);

        // Front View
        adjustCameraClippingPlanes();

        viewport->origin.setValue((width - vpsize) / 2, height - vpsize);

        getSceneManager()->render(true, true);

        // Left View
        SbRotation r1(SbVec3f(0,0,1), M_PI/2);

        rotate(r1*SbRotation(SbVec3f(0,1,0), M_PI/2));

        adjustCameraClippingPlanes();

        viewport->origin.setValue(width / 2, (height/2) - vpsize/2);

        getSceneManager()->render(false, false);

        // Right View
        rotate(SbRotation(SbVec3f(0,1,0), M_PI));

        adjustCameraClippingPlanes();

        viewport->origin.setValue(width / 2 - vpsize, (height/2) - vpsize/2);

        getSceneManager()->render(false, false);

        camera->enableNotify(true);
        viewport->enableNotify(true);
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

    // Callback that reports whenever the camera changes.
    static void cameraChangedCB(void *data, SoSensor *)
    {
        QxHoloViewer* _this = (QxHoloViewer*)data;

        HxViewer* hxviewer = theController->viewer(0);

        _this->camera->copyFieldValues(hxviewer->getCamera());
    }

private:
    McHandle<SoSeparator> root;
    McHandle<SoPerspectiveCamera> camera;
    McHandle<SoViewport> viewport;

    SoNodeSensor* sensor;
};

QxHoloPyramidWidget::QxHoloPyramidWidget(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Window);

    viewer = new QxHoloViewer(this);
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(viewer->getWidget());

    

    viewer->setSceneGraph(theController->viewer(0)->getSceneGraph());
}

QxHoloPyramidWidget::~QxHoloPyramidWidget()
{
    delete viewer;
}
