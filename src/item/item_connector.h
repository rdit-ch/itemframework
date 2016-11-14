#ifndef GRAPHICSITEMCONNECTOR_H
#define GRAPHICSITEMCONNECTOR_H

#include <QGraphicsLineItem>
#include <QObject>
#include <QPen>

class ItemInput;
class ItemOutput;

class Item_Connector : public QGraphicsObject
{
    Q_OBJECT

public:
    Item_Connector(ItemOutput* output, ItemInput* input);
    virtual ~Item_Connector();
    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF& point) const;
    bool collidesWithPath(const QPainterPath& path, Qt::ItemSelectionMode mode) const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    ItemOutput* get_output() const;
    ItemInput* get_input() const;
    bool load_additional(class QDomElement& de);
    bool save_additional(class QDomDocument& doc, class QDomElement& de) const;
    void mousePressEvent(QGraphicsSceneMouseEvent* e);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);
    void hoverMoveEvent(QGraphicsSceneHoverEvent* e);

private:
    ItemOutput* output;
    ItemInput* input;
    QPen pen;
    QPainterPath ppa_line;
    QPainterPath ppa_selpoints;
    bool b_was_pressed; //whether or not a selection Point was selected before the mouse-move event occoured
    int i_selected_selpoint; //the index of the selected selpoint in lis_cur_selpoints
    int i_routing_mode; //the currently applied routing mode
    bool b_user_modified; //whether or not the user modified the path
    QPointF poi_end; //last end point
    void recalc_pathes(); //recalculates all painter pathes and selpoints based on lis_current_coords.
    void extend_user_routing(); //tries to merge the movement into the user modified path
    void move_line_segment(int seg, qreal len); //moves a line segment to the specified x or y cordinate
    bool b_coords_start_hor; //whether the first line segment branchs out horiontally or vertically of the start item.
    enum Selpoint_Type {vertical, horizontal, topright, topleft};
    QList<qreal> lis_cur_coords; //all current line segment coordinates
    typedef QPair<QRectF, Selpoint_Type> Selpoint;
    QList<Selpoint> lis_cur_selpoints; //all current selpoints

private slots:
    void checkConnection();
    void repaint();

public slots:
    void do_update();

signals:
    void changed();
};

#endif // GRAPHICSITEMCONNECTOR_H
