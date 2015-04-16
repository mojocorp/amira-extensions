#pragma once

#include <QWidget>

#include <mclib/McHandle.h>

class QxHoloViewer;
class SoSensor;
class SoNodeSensor;
class SoSeparator;
class SoPerspectiveCamera;

class QxHoloPyramidWidget : public QWidget
{
public:
    QxHoloPyramidWidget(QWidget* parent);
    ~QxHoloPyramidWidget();

private:
    Q_DISABLE_COPY(QxHoloPyramidWidget);

    // Callback that reports whenever the camera changes.
    static void cameraChangedCB(void *data, SoSensor *);

    QxHoloViewer* viewer;

    McHandle<SoSeparator> root;
    McHandle<SoPerspectiveCamera> camera;

    SoNodeSensor* sensor;
};
