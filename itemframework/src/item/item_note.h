#ifndef GRAPHICS_ITEM_NOTE_H
#define GRAPHICS_ITEM_NOTE_H

#include <QGraphicsObject>

class ItemNote : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit ItemNote();
    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool inEditMode() const;
    bool load(class QDomElement&);
    bool save(class QDomDocument&, class QDomElement&) const;
    void paint(QPainter* painter, QStyleOptionGraphicsItem const* option, QWidget* widget);
protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    QVariant itemChange(GraphicsItemChange change, QVariant const& value);
private:
    QColor _color;
    bool _resizeMode;
    bool _editMode;
    QRectF _shape;
    QRectF _boundingRect;
    QGraphicsTextItem* _textItem;
    qreal width() const;
    void setWidth(qreal r);
private slots:
    void updateSize();
    void selectColor();
    void startEdit();
    void stopEdit();
public slots:
    void remove();
signals:
    void changed();

public slots:

};

#endif // GRAPHICS_ITEM_NOTE_H
