#ifndef GRAPHICS_VIEW_H
#define GRAPHICS_VIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include "item_scene.h"

class Gui_QProgress_Dialog;
class QDomDocument;
class QDomElement;

class ItemView : public QGraphicsView
{
    Q_OBJECT
public:
    ItemView(QSharedPointer<ProjectGui> projectGui);
    ItemScene* itemScene();
    bool load(QDomElement& domElement);
    bool reload(QDomElement& domElement);
    bool save(QDomDocument& domDocument, QDomElement& domElement);

private:
    QSharedPointer<ProjectGui> _projectGui;
    ItemScene* _scene;
protected:
    void wheelEvent(QWheelEvent* event);
    void resizeEvent(QResizeEvent* event);
};

#endif // GRAPHICS_VIEW_H
