#ifndef ABSTRACT_ITEM_P_H
#define ABSTRACT_ITEM_P_H

#include <QString>
#include <QImage>
#include <QRectF>
#include <QHash>
#include <QDomElement>
#include <QGraphicsSimpleTextItem>
#include "helper/settings_scope.h"

class AbstractItemInputOutputBase;
class ItemInput;
class ItemOutput;
class AbstractItem;

class AbstractItemPrivate
{
public:
    explicit AbstractItemPrivate(AbstractItem* parent);

    AbstractItem* const q_ptr;
    Q_DECLARE_PUBLIC(AbstractItem)

    void realignInputs();
    void realignOutputs();
    void showRenameDialog();
    void realignInputsOutputs(QList<AbstractItemInputOutputBase*>& elements, float offsetX = 0);

    float inputX(float x = 0) const;
    float inputY(float y = 0) const;
    float outputX(float x = 0) const;
    float outputY(float y = 0) const;
    float inputOutputY(float y) const;

    QString _type;
    QString _name;
    QString _description;
    QImage _image;
    int _progress;
    int _autohide_timer;
    bool _is_autohide_active;

    QList<ItemInput*> _inputs;
    QList<ItemOutput*> _outputs;

    QScopedPointer<SettingsScope> _settingsScope;

    QRectF _shape;
    QRectF _boundingRect;

    QGraphicsSimpleTextItem _typeLabel;
    QGraphicsSimpleTextItem _nameLabel;

    static int const _connectorHeight = 5;
    static int const _connectorWidth = 15;

    static QHash<QString, int> _itemTypesCount;

};

#endif // ITEM_P_H
