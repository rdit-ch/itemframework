#include "item_connector.h"
#include "item/item_input.h"
#include "item/item_output.h"
#include "item/abstract_item.h"
#include <QGraphicsScene>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <QPainter>
#include "res/resource.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QCursor>

Item_Connector::Item_Connector(ItemOutput* output, ItemInput* input)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    this->input = input;
    this->output = output;
    b_was_pressed = false;
    b_user_modified = false;
    b_coords_start_hor = true;
    i_routing_mode = 0;
    connect(input, SIGNAL(positionChanged()), this, SLOT(do_update()));
    connect(output, SIGNAL(positionChanged()), this, SLOT(do_update()));
    connect(input, SIGNAL(outputDisconnected()), this, SLOT(checkConnection()));
    connect(output, SIGNAL(inputDisconnected()), this, SLOT(checkConnection()));
    connect(output, SIGNAL(dataChanged()), this, SLOT(repaint()));
    pen = AbstractItem::connectorStyle(input->transportType());
}

Item_Connector::~Item_Connector()
{
    disconnect(input, 0, this, 0);
    disconnect(output, 0, this, 0);

    if (input->output() == output) {
        input->disconnectOutput();
    }

    prepareGeometryChange(); //this and the following lines seem to be necessary, otherwise the scene will redraw the item after deletion..
    ppa_selpoints = ppa_line =  QPainterPath();

    scene()->removeItem(this);
    emit changed();
}


void Item_Connector::checkConnection()
{
    if (! (input->output() == output && output->inputs().contains(input))) {
        delete this;
    }
}

void Item_Connector::repaint()
{
    update();
}


#define MIN_LIN_LEN 50
#define SEL_POINTS_MUL 2
#define RECTLEN (pen.width()*SEL_POINTS_MUL)
#define MAKE_SEL_POINT_RECT(x,y) QRectF((x)-RECTLEN/2,(y)-RECTLEN/2,RECTLEN,RECTLEN)
#define ROUTE_AROUND_DISTANCE 10

void Item_Connector::do_update()
{
    QPointF start = output->scenePosition();
    QPointF end = input->scenePosition();
    QPointF diff = end - start;

    //How the new calculation works:
    //Since a line is either horizontal or vertical, two consecutive points of the line share either a common x or a common y coordinate.
    //So you only need to add the changing coordinate, the rest is automatically calculated

    QList<qreal> coords;

    if (diff.y() == 0 && diff.x() > MIN_LIN_LEN) { //straight line
        if (i_routing_mode == 1 && b_user_modified && !lis_cur_coords.isEmpty()) {
            extend_user_routing();
        } else {
            lis_cur_coords = coords; //No points needed

            i_routing_mode = 1;
            b_coords_start_hor = true;
            b_user_modified = false;
        }
    } else if (diff.x() > MIN_LIN_LEN) { //Normal case, with 3 line segments
        if (i_routing_mode == 2 && b_user_modified) {
            extend_user_routing();
        } else {
            coords.append(start.x() + diff.x() / 2); //Horizontal
            coords.append(start.y() + diff.y()); //Vertical
            lis_cur_coords = coords;

            i_routing_mode = 2;
            b_coords_start_hor = true;
            b_user_modified = false;
        }
    } else { // other case with 5 line segments
        //the following code calculates wheater there is enough for a line between the two boundings
        AbstractItem* input_owner = input->owner();
        AbstractItem* output_owner = output->owner();

        bool to_small = false;

        if (diff.y() < 0) { //input is above the output
            qreal space_above_used = (input_owner->boundingRect().bottom() - input->localPosition().y()) + (output->localPosition().y() - output_owner->boundingRect().top());
            to_small = ((-diff.y() - ROUTE_AROUND_DISTANCE) < space_above_used);
        } else {
            qreal space_below_used = (output_owner->boundingRect().bottom() - output->localPosition().y()) + (input->localPosition().y() - input_owner->boundingRect().top());
            to_small = ((diff.y() - ROUTE_AROUND_DISTANCE) < space_below_used);

        }

        if (to_small && (diff.x() < MIN_LIN_LEN / 2)) { //there isn't enogh space between the boundings
            if (i_routing_mode == 3 && b_user_modified) {
                extend_user_routing();
            } else {
                qreal len1 = start.x() + MIN_LIN_LEN / 2;
                qreal len1a = input_owner->boundingRect().right() + input_owner->pos().x() + ROUTE_AROUND_DISTANCE;

                if (len1a > len1) {
                    len1 = len1a;
                }

                coords.append(len1); //Horizontal
                qreal len2;

                if (diff.y() < 0) { //input is above the output
                    len2 = (input_owner->boundingRect().top() + input_owner->pos().y() - ROUTE_AROUND_DISTANCE);
                } else { //input is below the ouput
                    len2 = (input_owner->boundingRect().bottom() + input_owner->pos().y() + ROUTE_AROUND_DISTANCE);
                }

                coords.append(len2); //Vertical
                coords.append(end.x() - MIN_LIN_LEN / 2); //Horizontal
                coords.append(end.y()); //Vertical
                lis_cur_coords = coords;

                i_routing_mode = 3;
                b_coords_start_hor = true;
                b_user_modified = false;
            }
        } else { //there is enough space for a line between the boundings
            if (i_routing_mode == 4 && b_user_modified) {
                extend_user_routing();
            } else {
                coords.append(start.x() + MIN_LIN_LEN / 2); //Horizontal
                coords.append(start.y() + diff.y() / 2); //Vertical
                coords.append(end.x() - MIN_LIN_LEN / 2); //Horizontal
                coords.append(end.y()); //Vertical
                lis_cur_coords = coords;

                i_routing_mode = 4;
                b_coords_start_hor = true;
                b_user_modified = false;
            }
        }
    }

    poi_end = end;
    recalc_pathes();
}

void Item_Connector::recalc_pathes()
{
    prepareGeometryChange();
    //Calculate the line Segments and construct the path
    QPointF start = output->scenePosition();
    QPointF end = input->scenePosition();

    ppa_line = QPainterPath();
    lis_cur_selpoints.clear();

    QPointF lastpoint = start;
    bool horizontal = b_coords_start_hor;

    ppa_line.moveTo(start);
    lis_cur_selpoints.append(qMakePair(MAKE_SEL_POINT_RECT(start.x(), start.y()),
                                       horizontal ? Item_Connector::vertical : Item_Connector::horizontal));


    for (int i = 0; i < lis_cur_coords.length(); i++) {
        qreal len = lis_cur_coords[i];
        QPointF cur; //current point we have to construct
        QPointF half; //point in the half from previous to current
        QPointF next; //next point we'll have to construct

        if (horizontal) {
            cur = QPointF(len, lastpoint.y());
            half = QPointF((lastpoint.x() + len) / 2, lastpoint.y());
            next = (i == lis_cur_coords.length() - 1) ? end : QPointF(len, lis_cur_coords.at(i + 1));
        } else {
            cur = QPointF(lastpoint.x(), len);
            half = QPointF(lastpoint.x(), (lastpoint.y() + len) / 2);
            next = (i == lis_cur_coords.length() - 1) ? end : QPointF(lis_cur_coords.at(i + 1), len);
        }

        ppa_line.lineTo(cur);
        lis_cur_selpoints.append(qMakePair(MAKE_SEL_POINT_RECT(half.x(), half.y()),
                                           horizontal ? Item_Connector::vertical : Item_Connector::horizontal));


        //in which diagonal direction the cursor for the selpoint should look (topright or topleft)
        //draw all 4 possible edges on a paper and you will find the same formula:
        bool b_topright_cursor = ((lastpoint.x() < next.x()) != (lastpoint.y() > next.y()));
        lis_cur_selpoints.append(qMakePair(MAKE_SEL_POINT_RECT(cur.x(), cur.y()),
                                           b_topright_cursor ? Item_Connector::topright : Item_Connector::topleft));
        lastpoint = cur;
        horizontal = !horizontal;
    }

    ppa_line.lineTo(end);
    lis_cur_selpoints.append(qMakePair(MAKE_SEL_POINT_RECT((lastpoint.x() + end.x()) / 2, (lastpoint.y() + end.y()) / 2),
                                       horizontal ? Item_Connector::vertical : Item_Connector::horizontal));
    lis_cur_selpoints.append(qMakePair(MAKE_SEL_POINT_RECT(end.x(), end.y()),
                                       horizontal ? Item_Connector::vertical : Item_Connector::horizontal));

    ppa_line.connectPath(ppa_line.toReversed()); // add reverse path
    ppa_line.closeSubpath();

    //Make line thicker
    QPainterPathStroker ps;
    ps.setCapStyle(pen.capStyle());
    ps.setWidth(pen.widthF());
    ps.setJoinStyle(pen.joinStyle());
    ps.setMiterLimit(pen.miterLimit());
    QPainterPath p2 = ps.createStroke(ppa_line);
    p2.addPath(ppa_line);
    ppa_line = p2;

    ppa_selpoints = QPainterPath();

    foreach (Selpoint selpoint, lis_cur_selpoints) {
        ppa_selpoints.addRect(selpoint.first);
    }

}

//extends a user modified path with the minimal changes
void Item_Connector::extend_user_routing()
{
    QPointF end = input->scenePosition();
    QPointF diff_end = end - poi_end;
    //we only need to change the last segment. the start point will be fine.

    if (!diff_end.isNull()) {
        if ((b_coords_start_hor + lis_cur_coords.length()) % 2) { //whether or not we go horizontal or vertical into the end item.
            lis_cur_coords.replace(lis_cur_coords.length() - 1, end.y());
        } else {
            lis_cur_coords.replace(lis_cur_coords.length() - 1, end.x());
        }
    }
}


bool Item_Connector::save_additional(QDomDocument&, QDomElement& de) const
{
    if (i_routing_mode != 0 && b_user_modified) {
        de.setAttribute("rmode", i_routing_mode);
        QStringList sli_lens;

        foreach (qreal l, lis_cur_coords) {
            sli_lens.append(QString::number(l));
        }

        de.setAttribute("len", sli_lens.join(';'));
        de.setAttribute("hori", b_coords_start_hor);
    }

    return true;
}

bool Item_Connector::load_additional(QDomElement& de)
{
    if (de.hasAttribute("rmode") && de.hasAttribute("len") && de.hasAttribute("hori")) {
        int r = de.attribute("rmode").toInt();
        bool h = de.attribute("hori", "1").toInt() == 1;
        QStringList sli_lens = de.attribute("len").split(';');
        QList<qreal> lis_coords;
        bool ok = true;

        foreach (QString l, sli_lens) {
            float f = l.toFloat(&ok);

            if (!ok) {
                break;
            }

            lis_coords.append(f);
        }

        if (ok) {
            lis_cur_coords = lis_coords;
            i_routing_mode = r;
            b_coords_start_hor = h;
            b_user_modified = true;
            return true;
        }

        i_routing_mode = 0;
        b_user_modified = false;
        b_coords_start_hor = true;
        return false;
    }

    i_routing_mode = 0;
    b_user_modified = false;
    b_coords_start_hor = true;
    return true;
}

void Item_Connector::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    QPointF p = e->scenePos();
    b_was_pressed = (isSelected() && ppa_selpoints.contains(p)); //Selection point clicked

    if (b_was_pressed) {
        for (int i = 0; i < lis_cur_selpoints.length(); i++) { //find clicked selection point
            if (lis_cur_selpoints.at(i).first.contains(p)) {
                i_selected_selpoint = i;
                break;
            }
        }

        if (i_selected_selpoint == 0) { //clicked start
            i_selected_selpoint = 1; //behave like if user clicked  selpoint in the middle of first segment
        } else if (i_selected_selpoint == lis_cur_selpoints.length() - 1) { //clicked end
            i_selected_selpoint = lis_cur_selpoints.length() - 2; //behave like user clicked selpoint in the middle of the last segment
        }
    }

    QGraphicsObject::mousePressEvent(e);
}



void Item_Connector::move_line_segment(int seg, qreal len)
{
    //moves a segment (starting at segment 0) to the position specified by len
    //creates additional first and last segments when trying to move the first and last segment
    if (seg == lis_cur_coords.length()) {
        QPointF end = input->scenePosition();

        if ((b_coords_start_hor + lis_cur_coords.length()) % 2) {
            lis_cur_coords.append(end.x());
        } else {
            lis_cur_coords.append(end.y());
        }

        move_line_segment(seg, len);
    } else if (seg == 0) {
        QPointF start = output->scenePosition();

        if (b_coords_start_hor) {
            lis_cur_coords.prepend(start.y());
            b_coords_start_hor = false;
        } else {
            lis_cur_coords.prepend(start.x());
            b_coords_start_hor = true;
        }

        i_selected_selpoint += 2;
        move_line_segment(1, len);
    } else {
        lis_cur_coords.replace(seg - 1, len);
    }

}
void Item_Connector::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (b_was_pressed)  { //we clicked a selpoint, lets move some lines around
        b_user_modified = true;
        QPointF p = e->scenePos();
        int i_line_segment = i_selected_selpoint / 2;


        switch (lis_cur_selpoints.at(i_selected_selpoint).second) {
        case Item_Connector::vertical:
            move_line_segment(i_line_segment, p.y());
            break;

        case Item_Connector::horizontal:
            move_line_segment(i_line_segment, p.x());
            break;

        case Item_Connector::topleft:
        case Item_Connector::topright:
            if (lis_cur_selpoints.at(i_selected_selpoint + 1).second == Item_Connector::horizontal) {
                move_line_segment(i_line_segment, p.x());
                move_line_segment(i_line_segment - 1, p.y());
            } else {
                move_line_segment(i_line_segment, p.y());
                move_line_segment(i_line_segment - 1, p.x());
            }

            break;
        }

        recalc_pathes();
        emit changed();

    }

    QGraphicsObject::mouseMoveEvent(e);
}

void Item_Connector::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    if (b_was_pressed) { //we moved some segments around, lets check if we can delete optional segments
        QPointF start = output->scenePosition();
        QPointF end = input->scenePosition();
        QList<qreal> coords_ext; //all lengths with the start selpoints lengths (x,y) and endpoint length (y or x)
        coords_ext.append(start.x());
        coords_ext.insert(b_coords_start_hor, start.y());
        coords_ext.append(lis_cur_coords);
        coords_ext.append((b_coords_start_hor + lis_cur_coords.length()) % 2 ? end.x() : end.y());
        //a line segment can be removed if a segment shares the same coordinate with the segment before the previous (i-2)
        //if the previous segment is not a pseudo-segment (start) then it can be removed as well

        for (int i = 2; i < coords_ext.length(); i++) { //we need to do the if forward, otherwise we would miss two consecutive optional lines
            qreal prev2 = coords_ext.at(i - 2);
            qreal cur = coords_ext.at(i);

            if (cur == prev2) {
                if (i < coords_ext.length() - 1) { //not last segment
                    lis_cur_coords.removeAt(i - 2);
                    coords_ext.removeAt(i);
                }

                if (i > 2) { //not first segment
                    lis_cur_coords.removeAt(i - 3);
                    coords_ext.removeAt(i - 1);
                } else { //first segment
                    b_coords_start_hor ^= 1;
                }

                i -= (i > 2) + (i < coords_ext.length() - 1);

                recalc_pathes();
            }
        }
    }

    QGraphicsObject::mouseReleaseEvent(e);
}


void Item_Connector::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    if (isSelected()) {
        QPointF p = e->scenePos();

        //find selpoint and determine cursor
        foreach (Selpoint selpoint, lis_cur_selpoints) {
            if (selpoint.first.contains(p)) {
                QCursor c;

                switch (selpoint.second) {
                case Item_Connector::vertical:
                    c = Qt::SizeVerCursor;
                    break;

                case Item_Connector::horizontal:
                    c = Qt::SizeHorCursor;
                    break;

                case Item_Connector::topleft:
                    c = Qt::SizeFDiagCursor;
                    break;

                case Item_Connector::topright:
                    c = Qt::SizeBDiagCursor;
                    break;
                }

                setCursor(c);
                return;
            }
        }
    }

    setCursor(QCursor());
}

QRectF Item_Connector::boundingRect() const
{
    return ppa_selpoints.boundingRect().normalized();
}


bool Item_Connector::collidesWithPath(const QPainterPath& path, Qt::ItemSelectionMode mode) const
{

    bool r = false;

    switch (mode) {
    case Qt::ContainsItemShape:
        r = path.contains(shape());
        break;

    case Qt::IntersectsItemShape:
        r = path.intersects(shape());
        break;

    case Qt::ContainsItemBoundingRect:
        r = path.contains(boundingRect());
        break;

    case Qt::IntersectsItemBoundingRect:
        r = path.intersects(boundingRect());
        break;
    }

    return r;
}

bool Item_Connector::contains(const QPointF& point) const
{
    return shape().contains(point);
}

QPainterPath Item_Connector::shape() const
{
    return ppa_line;
}

void Item_Connector::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{

    //painter->fillRect(boundingRect(),Qt::green);
    //painter->fillPath(shape(),Qt::yellow);


    painter->fillPath(ppa_line, pen.color());

    if (output->data() == NULL) {
        painter->fillPath(ppa_line, Resource::get("item_connector_inactive", QBrush(Qt::white, Qt::BDiagPattern)));
    }

    if (isSelected()) {
        painter->fillPath(ppa_selpoints, Resource::get("item_connector_selpoint_color", QColor(Qt::black)));
    }
}

ItemOutput* Item_Connector::get_output() const
{
    return output;
}

ItemInput* Item_Connector::get_input() const
{
    return input;
}
