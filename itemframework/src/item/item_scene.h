#ifndef ITEM_SCENE_H
#define ITEM_SCENE_H

#include <QGraphicsScene>
#include <QDomDocument>

class ProjectGui;
class ItemOutput;
class ItemInput;

class ItemScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit ItemScene(QSharedPointer<ProjectGui> projectGui, QObject* parent = 0);
    ~ItemScene();

    /**
     * @brief Load the current scene from the passed xml element
     * @param dom the element to load the scene from
     * @return true on success
     */
    bool loadFromXml(QDomElement& dom);

    /**
     * @brief Save the current scene with all items and their config to xml
     * @param document the document which can be used to create elements on it
     * @param xml the DomElement to save the scene to
     * @return true on success
     */
    bool saveToXml(QDomDocument& document, QDomElement& xml) const;

    /**
     * @brief Resets the bounding rect. Call this method after resizing the view. The sceneRect will be shrinked if possible
     */
    void resetBounding();

    /**
     * @brief pastes the items in the clipboard to the mouse cursor position on the scene
     */
    void paste();

    /**
     * @brief Returns all currently selected and copyable items
     * @return A list containing all the copyable items
     */
    QList<QGraphicsItem*> copyableItems() const;

    /**
     * @brief Returns the items which are copyable (removing connectors where only one end is in the list)
     * @param itms The items to search for copyable items
     * @return A list containing all the copyable items
     */
    static QList<QGraphicsItem*> copyableItems(QList<QGraphicsItem*> itms);

    /**
     * @brief copies the selected items & connectors to clipboard and removes them from the scene
     */
    void cut();

    /**
     * @brief copies the selected items & connections to clipboard
     */
    void copy() const;

    /**
     * @brief Creates a template using the currently selected items.
     */
    void createTemplate();

    /**
     * @brief Copy the passed items to clipboard
     * @param itms the items, connectors & notes to copy to clipboard
     */
    void copy(QList<QGraphicsItem*> itms) const;

    /**
     * @return Returns the associated project gui, which renders/owns this view/scene
     */
    QSharedPointer<ProjectGui> projectGui() const;

    /**
     * @param itemsDocument A XML-representation of the items to render
     * @return A pixmap representation of the given items
     */
    QPixmap createPixmap(QDomDocument const& itemsDocument);

    /**
     * @param items A list of items to render
     * @param bounding The bounding box of the given items
     * @return A pixmap representation of the given items
     */
    QPixmap createPixmap(QList<QGraphicsItem*> items, const QRectF& bounding) const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
    void dropEvent(QGraphicsSceneDragDropEvent* event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event);

signals:
    void sceneRealChanged();
    void loadingProgress(const int progress, const QString& loadcomment = "", const QString& loadinfo = "Loading Project");

private:
    bool saveToXml(QDomDocument& document, QDomElement& xml, QList<QGraphicsItem const*> itms) const;
    bool loadFromXml(QDomElement& dom, QList<QGraphicsItem*>* itemsOut,
                     bool shouldReportProgress, bool shouldConnectIO = true);

    bool notesInInsertMode();
    void deleteItems(QList<QGraphicsItem*> items);
    void insertItem(QString const& name, QPointF const& position);

    void updateConnectionLine();
    void updateBoundingRect();

    bool startMove(QGraphicsSceneMouseEvent* event);
    bool startConnection(QGraphicsSceneMouseEvent* mouseEvent);
    qreal distance(const QPointF& a, const QPointF& b) const;

    void paintItem(QGraphicsItem* item, QPainter& painter, QStyleOptionGraphicsItem& options) const;

    QRectF calculateBoundingBox(QList<QGraphicsItem const*> items, bool ignoreConnectors = false) const;
    QList<QGraphicsItem*> readItems(QDomDocument const& document, bool shouldConnectIO = true);
    QList<QGraphicsItem*> readItems(class QMimeData const& mimeData, char const* const mimeType, char const* const docType);

    void insertItems(QList<QGraphicsItem*> items, const QPointF& scenePos, const QRectF& boundingBox);
    void insertTemplate(class QMimeData const* const mimeData, QPointF const& position = {});

    QHash<QString, int> _copyCountHash;
    QSharedPointer<ProjectGui> _projectGui;
    QPointF _startPoint;
    QPointF _endPoint;
    bool _inConnectionMode = false;
    bool _startItemIsInput;
    bool _recalculateBoundingRect = true;
    bool _inMovingMode = false;

    bool _reversed = false;
    ItemInput* _input = nullptr;
    ItemOutput* _output = nullptr;

    QGraphicsLineItem* _connectionLine = nullptr;
    int _type;
    QRectF _sceneRect;
    QPointF _mouseItemDiff;
};

#endif // ITEM_SCENE_H
