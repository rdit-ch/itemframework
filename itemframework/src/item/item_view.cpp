#include "item_view.h"
#include "gui/gui_progress_dialog.h"
#include "gui/gui_manager.h"
#include <QDomDocument>

ItemView::ItemView(QSharedPointer<ProjectGui> projectGui) : QGraphicsView()
{
    _projectGui = projectGui;
    _scene = new ItemScene(_projectGui, this);
    //OpenGl disabled because of wrong rendering of Text-Characters
    //setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    this->setDragMode(QGraphicsView::RubberBandDrag);
    _scene->setSceneRect(QRectF(geometry()));
    setScene(_scene);
}

void ItemView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers()&Qt::ControlModifier) {
        //Adapted from: http://www.qtcentre.org/wiki/index.php?title=QGraphicsView:_Smooth_Panning_and_Zooming
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        qreal currentScale = transform().m11(); //current horizontal scaling
        // Scale the view / do the zoom
        qreal scaleFactor = 1.15;

        if ((event->delta() > 0) && (currentScale < 1.5)) {
            // Zoom in
            scale(scaleFactor, scaleFactor);
        } else if ((event->delta() < 0) && (currentScale > 0.5)) {
            // Zooming out
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void ItemView::resizeEvent(QResizeEvent*)
{
    _scene->resetBounding();
}

ItemScene* ItemView::itemScene()
{
    return _scene;
}

bool ItemView::load(class QDomElement& domElement)
{
    Gui_Progress_Dialog prog_dial(GuiManager::instance()->widgetReference());
    connect(_scene, SIGNAL(loadingProgress(int, QString, QString)), &prog_dial, SLOT(progress(int, QString, QString)));
    return _scene->loadFromXml(domElement);
}

bool ItemView::reload(QDomElement& domElement)
{
    _scene->clear();
    return load(domElement);
}

bool ItemView::save(class QDomDocument& domDocument, class QDomElement& domElement)
{
    return _scene->saveToXml(domDocument, domElement);
}
