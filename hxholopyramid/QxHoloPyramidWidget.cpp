#include "QxHoloPyramidWidget.h"

#include <QHBoxLayout>

#include <Inventor/Qt/viewers/SoQtViewer.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoViewport.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/sensors/SoNodeSensor.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>

#include <hxcore/HxController.h>
#include <hxcore/HxViewer.h>

class SxGLHoloRenderAction : public SoGLRenderAction
{
public:
    SxGLHoloRenderAction(SoCamera * cam, SoViewport * vp) 
        : SoGLRenderAction(SbVec2s(1,1)), camera(cam), viewport(vp)
    {
    }

    virtual void apply(SoNode* node)
    {
        const SbViewportRegion &vpr = getViewportRegion();
        const SbVec2s & size = vpr.getViewportSizePixels();

        const int width = size[0];
        const int height = size[1];

        const int vpsize = height / 2;

        SbVec3f position = camera->position.getValue();
        SbRotation orientation = camera->orientation.getValue();

        camera->enableNotify(false);
        viewport->enableNotify(false);
        viewport->size.setValue(vpsize, vpsize);

        // Front View
        rotateCamera(SbRotation(SbVec3f(0,0,1), M_PI));

        viewport->origin.setValue((width - vpsize) / 2, height - vpsize);

        SoGLRenderAction::apply(node);

        // Left View
        SbRotation r1(SbVec3f(0,0,1), M_PI/2);

        rotateCamera(r1*SbRotation(SbVec3f(0,1,0), M_PI/2));

        viewport->origin.setValue(width / 2, (height/2) - vpsize/2);

        SoGLRenderAction::apply(node);

        // Right View
        rotateCamera(SbRotation(SbVec3f(0,1,0), M_PI));

        viewport->origin.setValue(width / 2 - vpsize, (height/2) - vpsize/2);

        SoGLRenderAction::apply(node);

        camera->position = position;
        camera->orientation = orientation;
        camera->enableNotify(true);
        viewport->enableNotify(true);
    }

    void rotateCamera(const SbRotation &rot)
    {
        // get center of rotation
        const float radius = camera->focalDistance.getValue();

        SbVec3f forward;
        camera->orientation.getValue().multVec(SbVec3f(0,0,-1), forward);

        const SbVec3f center = camera->position.getValue() + radius * forward;

        // apply new rotation to the camera
        camera->orientation = rot * camera->orientation.getValue();

        // reposition camera to look at pt of interest
        camera->orientation.getValue().multVec(SbVec3f(0,0,-1), forward);
        camera->position = center - radius * forward;
    }
protected:
    SoCamera * camera;
    SoViewport * viewport;
};

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

        setGLRenderAction(new SxGLHoloRenderAction(camera, viewport));
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

        SoSearchAction sa;
        sa.setNode(getHeadlight());
        sa.apply(getSceneRoot());
        SoFullPath* fullPath = (SoFullPath*) sa.getPath();
        if (fullPath) {
            SoGroup *group = (SoGroup*) fullPath->getNodeFromTail(1);
            headlightRot = (SoRotation*) group->getChild(0);
            if (!headlightRot->isOfType(SoRotation::getClassTypeId()))
                headlightRot = 0;
        }
    }

    // Callback that reports whenever the camera changes.
    static void cameraChangedCB(void *data, SoSensor *)
    {
        QxHoloViewer* _this = (QxHoloViewer*)data;

        HxViewer* hxviewer = theController->viewer(0);

        _this->camera->copyFieldValues(hxviewer->getCamera());
        _this->headlightRot->rotation.setValue(_this->camera->orientation.getValue());
    }

    /// Override original method since it seems to adjust the clipping planes in a weird manner.
    /// Maybe using a screen-space projection or whatever.
    virtual void adjustCameraClippingPlanes()
    {
        if (!camera)
            return;

        SoGetBoundingBoxAction clipbox_action(getViewportRegion());
        clipbox_action.apply(root);

        SbBox3f bbox = clipbox_action.getBoundingBox();

        if (bbox.isEmpty())
            return;

        SbSphere bSphere;
        bSphere.circumscribe(bbox);

        SbVec3f forward;
        camera->orientation.getValue().multVec(SbVec3f(0,0,-1), forward);

        float denumerator = forward.length();
        float numerator = (bSphere.getCenter() - camera->position.getValue()).dot(forward);
        float distToCenter = (forward * (numerator / denumerator)).length();

        float farplane = distToCenter + bSphere.getRadius();

        // if scene is behind the camera, don't change the planes
        if (farplane < 0) return;

        float nearplane = distToCenter - bSphere.getRadius();

        if (nearplane < (0.001 * farplane)) nearplane = 0.001 * farplane;

        camera->nearDistance = nearplane;
        camera->farDistance = farplane;
    }

private:
    McHandle<SoSeparator> root;
    McHandle<SoPerspectiveCamera> camera;
    McHandle<SoViewport> viewport;
    McHandle<SoRotation> headlightRot;

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
