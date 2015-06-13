#include "QxHoloPyramidWidget.h"

#include <QHBoxLayout>

#include <Inventor/Qt/viewers/SoQtViewer.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>
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
    SxGLHoloRenderAction(SoQtViewer * v) 
        : SoGLRenderAction(SbVec2s(1,1)), viewer(v), headlightRot(NULL)
    {

    }

    virtual void apply(SoNode* node)
    {
        if (!headlightRot) {
            SoSearchAction sa;
            sa.setNode(viewer->getHeadlight());
            sa.apply(viewer->getSceneRoot());
            SoFullPath* fullPath = (SoFullPath*) sa.getPath();
            if (fullPath) {
                SoGroup *group = (SoGroup*) fullPath->getNodeFromTail(1);
                headlightRot = (SoRotation*) group->getChild(0);
                if (!headlightRot->isOfType(SoRotation::getClassTypeId()))
                    headlightRot = 0;
            }
        }

        const SbViewportRegion vpr = getViewportRegion();
        const SbVec2s & size = vpr.getViewportSizePixels();

        const int width = size[0];
        const int height = size[1];

        const int vpsize = width / 2;

        SoCamera * camera = viewer->getCamera();

        const SbVec3f position = camera->position.getValue();
        const SbRotation orientation = camera->orientation.getValue();
        const float nearplane = camera->nearDistance.getValue();
        const float farplane = camera->farDistance.getValue();

        camera->enableNotify(false);

        // Front View
        rotateCamera(SbRotation(SbVec3f(0,0,1), M_PI));

        SbViewportRegion vp;
        vp.setViewportPixels(SbVec2s(0, height-width/2), SbVec2s(width, width/2) );
        setViewportRegion(vp);

        SoGLRenderAction::apply(node);

        // Left View
        SbRotation r1(SbVec3f(0,0,1), -M_PI/2);

        rotateCamera(r1*SbRotation(SbVec3f(0,1,0), -M_PI/2));
        
        vp.setViewportPixels(SbVec2s(0, height-width), SbVec2s(width/2, width) );
        setViewportRegion(vp);

        SoGLRenderAction::apply(node);

        // Right View
        rotateCamera(SbRotation(SbVec3f(0,1,0), -M_PI));

        vp.setViewportPixels(SbVec2s(width/2, height-width), SbVec2s(width/2, width) );
        setViewportRegion(vp);

        SoGLRenderAction::apply(node);

        setViewportRegion(vpr);

        camera->position = position;
        camera->orientation = orientation;
        camera->enableNotify(true);

        // Restore original viewport region
        setViewportRegion(vpr);
    }

    void rotateCamera(const SbRotation &rot)
    {
        SoCamera * camera = viewer->getCamera();

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

        headlightRot->rotation = camera->orientation.getValue();

        // Adjust clipping planes
        SoGetBoundingBoxAction clipbox_action(getViewportRegion());
        clipbox_action.apply(viewer->getSceneRoot());

        SbBox3f bbox = clipbox_action.getBoundingBox();

        if (bbox.isEmpty())
            return;

        SbSphere bSphere;
        bSphere.circumscribe(bbox);

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

     virtual void adjustCameraClippingPlanes()
    {
        SoCamera * camera = viewer->getCamera();
        if (!camera)
            return;

        SoGetBoundingBoxAction clipbox_action(getViewportRegion());
        clipbox_action.apply(viewer->getSceneRoot());

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
protected:
    SoQtViewer * viewer;
    McHandle<SoRotation> headlightRot;
};

class QxHoloViewer : public SoQtViewer
{
public:
    QxHoloViewer(QWidget* parent) : SoQtViewer(parent, "holoviewer", true, SoQtViewer::BROWSER, true) 
    {
        HxViewer* hxviewer = theController->viewer(0);

        sensor = new SoNodeSensor;
        sensor->setPriority(0);
        sensor->setFunction(cameraChangedCB, this);
        sensor->attach(hxviewer->getCamera());

        setGLRenderAction(new SxGLHoloRenderAction(this));
    }

    ~QxHoloViewer()
    {
        delete sensor;
    }

    // Callback that reports whenever the camera changes.
    static void cameraChangedCB(void *data, SoSensor *)
    {
        QxHoloViewer* _this = (QxHoloViewer*)data;

        HxViewer* hxviewer = theController->viewer(0);

        _this->getCamera()->copyFieldValues(hxviewer->getCamera());
    }

    /// Override original method since it seems to adjust the clipping planes in a weird manner.
    /// Maybe using a screen-space projection or whatever.
    virtual void adjustCameraClippingPlanes()
    {
        SoCamera * camera = getCamera();
        if (!camera)
            return;

        SoGetBoundingBoxAction clipbox_action(getViewportRegion());
        clipbox_action.apply(getSceneRoot());

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
