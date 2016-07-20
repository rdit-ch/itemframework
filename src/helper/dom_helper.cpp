#include "helper/dom_helper.h"

#include <qbuffer.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <QMetaProperty>
#include <QPair>

static QHash<int, QPair<DomHelper::SerializerWrapperType, DomHelper::DeserializerWrapperType>> _serializables;

bool DomHelper::saveVariant(const QVariant& variant, QDomElement element,
                            QDomDocument& doc, const QString& name,
                            const QString& typeTagName,
                            const QString& valueTagName)
{
    // Evaluate variant's type to find out the format in which the variant is stored.

    // The attribute value will be set, if the variant can be stored in a human readable way. The
    // element will have an attribute named valueTagName!
    QString attributeValue = QString::null;
    // The byte array will be set when the variant must be serialized because it has a type that
    // is unknown to this method. Note that this implies that the variant has the related stream
    // operator defined! The serialized data will be added to element as a base64 decoded text node.
    QByteArray byteArray;
    // The child element will be set, if the variant's type can be stored in a human readable way
    // but needs its own structure, i.e. complicated variant types.
    QDomElement child;

    bool isNull = false;

    // Evaluate variant, some types can be stored human readable!
    if (!variant.isValid()) {
        // Variant is invalid, setting the attribute value to empty will still create an empty
        // value attribute!
        qWarning() << "cannot serialize invalid variant named" << name;
        return false;
    } else if (variant.type() == QVariant::String) { //variant is a QString
        QString str = variant.toString();

        if (str.isNull()) {
            isNull = true;
        } else {
            attributeValue = str;
        }
    } else if (variant.canConvert(QMetaType::QString) &&
               QVariant(QString()).canConvert(variant.userType())) {
        // As this variant type can be converted to string and vice versa, the string value is
        // stored
        attributeValue = variant.toString();
    } else {
        // Reaching here means it is not a trivial variant type, evaluate type.
        if (variant.type() == QVariant::StringList) {
            // String lists can be saved in a list structure per entry

            // Create a DOM element as container for the list entries, tag it with the variant
            // type's name, i.e. QStringList
            QDomElement listElem = doc.createElement(variant.typeName());
            // Get the list from variant
            const QStringList list = variant.value<QStringList>();

            // Append a child element for each string in list
            for (const QString& str : list) {
                // Create the child element which is saved as string variant with ListEntryTag
                QDomElement entry = DomHelper::saveVariant(str, DomHelper::ListEntryTag, doc);

                // If successful, append the child node to the container
                if (!entry.isNull()) {
                    listElem.appendChild(entry);
                }
            }

            // Set the created container as child element of the variant to be saved
            child = listElem;
        } else if (variant.type() == QVariant::List ||
                   (variant.canConvert(QMetaType::QVariantList) &&
                    QVariant(QVariantList()).canConvert(variant.userType()))
                  ) {
            // Variant lists can be saved in a list structure per entry
            // QList<T> can be converted to QVariantList (we'll only do this if we can convert it back later)

            // Create a DOM element as container for the list entries, tag it with the variant
            // type's name, i.e. QVariantList
            QDomElement listElem = doc.createElement(QMetaType::typeName(QMetaType::QVariantList));
            // Get the list from variant
            const QVariantList list = variant.value<QVariantList>();

            // Append a child element for each list entry
            for (const QVariant& var : list) {
                // Create the child element which is saved as variant with ListEntryTag
                QDomElement entry = DomHelper::saveVariant(var, DomHelper::ListEntryTag, doc);

                // If successful, append the child node to the container
                if (!entry.isNull()) {
                    listElem.appendChild(entry);
                }
            }

            // Set the created container as child element of the variant to be saved
            child = listElem;

        } else if (variant.type() == QVariant::Map ||
                   (variant.canConvert(QMetaType::QVariantMap) &&
                    QVariant(QVariantMap()).canConvert(variant.userType()))
                  ) {
            // Variant map's can be saved in a list structure per entry
            // QMap<QString,T> can be converted to QVariantMap (we'll only do this if we can convert it back later)

            // Create a DOM element as container for the list entries, tag it with the variant
            // type's name, i.e. QVariantList
            QDomElement mapElement = doc.createElement(QMetaType::typeName(QMetaType::QVariantMap));
            // Get the list from variant
            const QVariantMap map = variant.value<QVariantMap>();
            QMapIterator<QString, QVariant> it(map);

            // Append a child element for each map entry
            while (it.hasNext()) {
                it.next();
                // Create the child element which is saved as variant with ListEntryTag
                QDomElement entry = DomHelper::saveVariant(it.value(), DomHelper::MapEntryTag, doc);
                entry.setAttribute(DomHelper::KeyTag, it.key());

                // If successful, append the child node to the container
                if (!entry.isNull()) {
                    mapElement.appendChild(entry);
                }
            }

            // Set the created container as child element of the variant to be saved
            child = mapElement;
        } else if (variant.userType() >= static_cast<int>(QVariant::UserType)) {

            // Check for user-defined serializer.
            int const typeId = QMetaType::type(variant.typeName());
            auto it = _serializables.find(typeId);

            if (it != _serializables.end()) {
                auto serializer = (*it).first;
                QDomElement element = doc.createElement(variant.typeName());
                bool const success = serializer(doc, element, variant);

                if (success) {
                    child = element;
                } else {
                    qCritical() << "Failed to save user-defined type " << variant.typeName();
                }
            } else if (variant.canConvert<QObject*>()) {
                //Variant is QObject derived class or QSharedPointer<T> where T is QObject derived
                const QObject* obj = variant.value<QObject*>();
                QDomElement qobjectElement = doc.createElement(DomHelper::QObjectTag);

                if (DomHelper::saveUserProperties(obj, qobjectElement, doc)) {
                    child  = qobjectElement;
                } else {
                    qWarning() << "Error saving variant of QObject type" << variant.typeName()
                               << ": empty variant! (variant.data == null)";
                }
            } else if (QMetaType(typeId).flags().testFlag(QMetaType::IsGadget)) {
                // The QVariant contains an object which has the Q_GADGET macro
                // in its class definition. This means it has a meta object.
                void const* const object = variant.data();

                if (object != nullptr) {
                    QDomElement objectElement = doc.createElement(DomHelper::QGadgetTag);

                    QMetaType metaType(typeId);

                    if (DomHelper::doSaveUserProperties(metaType.metaObject(), makePropertyReader(object), objectElement, doc)) {
                        child = objectElement;
                    } else {
                        qWarning() << "Error saving user properties of "
                                   << QMetaType::typeName(typeId) << "object!";
                    }
                } else {
                    qWarning() << "Error saving variant of QGadget type" << variant.typeName()
                               << ": empty variant! (variant.data == null)";
                }

            }

        }

        // If we reach here and child is null, we could not find a method to store the variant in
        // a human readable way. Only thing left is serialize the variant and store it as base64
        // decoded byte array.
        // NOTE: this implies that the type has registered QDataStream stream operators! Otherwise
        // it will fail, as there is no known way to serialize the variant.
        if (child.isNull()) {

            // Create a buffer backed up by the byte array and open the buffer for write
            QBuffer writeBuffer(&byteArray);
            writeBuffer.open(QIODevice::WriteOnly);
            // Create a data stream backed up with the buffer
            QDataStream out(&writeBuffer);

            //Try serializing
            if (!QMetaType::save(out, variant.userType(), variant.constData())) {
                qCritical() << "Cannot serialize Variant named" << name << " of Type" << variant.typeName() << "using QDataStream!";
                qDebug() << "Forgot to register DataStream operator?";
                return false;
            }

            //Seek back to start
            Q_ASSERT(writeBuffer.reset());

            // Serialize the variant into the data stream
            out << variant; //will cause a SIGABRT if we try to serialize a type where QMetaType::save fails

            // Close Buffer
            writeBuffer.close();

            // Check for error
            if (out.status() != QDataStream::Ok || byteArray.size() == 0) {
                qCritical() << "Cannot serialize Variant named" << name << " of Type" << variant.typeName() << "using QDataStream!";
            }

            // The byte array is now filled correctly with the serialized variant!
        }
    }

    // If no attribute value nor child nor byte array is set, the variant could not be saved!
    if (!isNull && attributeValue.isNull() && child.isNull() && byteArray.size() == 0) {
        qCritical() << "Cannot save Variant named" << name << " of Type" << variant.typeName();
        return false;
    }

    // If a name is set, create an attribute for the name
    if (!name.isEmpty()) {
        element.setAttribute(DomHelper::NameTag, name);
    }

    // Add the variant type as attribute
    element.setAttribute(typeTagName, variant.typeName());

    // Add the variant's value depending on how it was saved.
    if (child.isNull()) {
        // If no child is set, the variant value is either serialized in byte array or set in
        // attribute value
        if (byteArray.size() == 0) {
            // Human readable simple form, add the value as string attribute
            element.setAttribute(valueTagName, attributeValue);
        } else {
            // Serialized, add the byte array as base64 decoded text node
            element.appendChild(doc.createTextNode(QString(byteArray.toBase64())));
        }
    } else {
        // Add complicated type as child element
        element.appendChild(child);
    }

    // Success
    return true;
}

QDomElement DomHelper::saveVariant(const QVariant& variant, const QString tagName, QDomDocument& doc,
                                   const QString& name)
{
    // Create a DOM element to hold the variant, name it as specified
    QDomElement element = doc.createElement(tagName);
    // Save the variant in the created DOM element. If successful, return it. Otherwise return
    // a null element.
    return saveVariant(variant, element, doc, name) ? element : QDomElement();
}

bool DomHelper::saveVariant(const QVariant& variant, QDomElement element, QDomDocument& doc,
                            const QString& name)
{
    // Save the variant using default type and value tags
    return saveVariant(variant, element, doc, name, TypeTag, ValueTag);
}

bool DomHelper::loadVariant(QVariant& variant, QString& name, const QDomElement& element)
{
    // Try to load the name from name attribute
    name = element.attribute(NameTag, QString::null);
    // Load the variant from element
    return loadVariant(variant, element);
}

bool DomHelper::loadVariant(QVariant& variant, const QDomElement& element)
{
    // Invalidate the output variant.
    variant = QVariant();

    // Get type name
    QString typeName = element.attribute(TypeTag);
    // Get related meta type id
    int typeId = QMetaType::type(typeName.toStdString().c_str());

    // If the meta type is not known, we cannot load the variant. Note that this might indicate
    // a missing meta type registration
    if (typeId == QMetaType::UnknownType) {
        qCritical() << QString("cannot deserialize variant: unknown meta type %1!").arg(typeName);
        return false;
    }

    // If element to load from has child nodes, it is a complicated type to be loaded
    if (element.hasChildNodes()) {
        // Complicated types should have a child node that is tagged with the type name or a
        // text node with base64 decoded serialized data.
        // Exception: QList<T> types have a child node tagged with QVariantList (see below)
        // Try to read child element tagged with type name.
        QDomElement child = element.firstChildElement(typeName);

        // If successful, read the variant from child element
        if (!child.isNull()) {
            // The types should be somehow one of the special handled type in ::saveVariant() above.
            // Any special save handling implemented there, must be reflected for load here!

            // Check for user-defined serializer.
            auto it = _serializables.find(typeId);

            if (it != _serializables.end()) {
                auto deserializer = (*it).second;
                QVariant valueVariant;
                bool const success = deserializer(child, valueVariant);

                if (success) {
                    variant.setValue(valueVariant);
                } else {
                    qCritical() << "Failed to load user-defined type " << typeName;
                }

            } else if (typeId == QVariant::List) {
                // Handle variant lists. Read all child nodes tagged ListEntryTag from list
                // element.
                QVariantList list;
                // Get first entry's element
                QDomElement entryElem = child.firstChildElement(ListEntryTag);

                // For all list entry child nodes, load the variant entries and store them in list
                while (!entryElem.isNull()) {
                    QVariant variant;

                    // Read the variant and store it in list if successful
                    if (loadVariant(variant, entryElem)) {
                        list << variant;
                    }

                    // Get next list entry child node from list element
                    entryElem = entryElem.nextSiblingElement(ListEntryTag);
                }

                // Store the loaded list in output variant
                variant = list;

            } else if (typeId == QVariant::StringList) {
                // Handle string lists. Read all child nodes tagged ListEntryTag from list
                // element.
                QStringList list;
                // Get first entry's element
                QDomElement entryElem = child.firstChildElement(ListEntryTag);

                // For all list entry child nodes, load the string entries and store them in list
                while (!entryElem.isNull()) {
                    QVariant variant;

                    // Read the string as string variant and store it in list if successful
                    if (loadVariant(variant, entryElem)) {
                        list << variant.toString();
                    }

                    // Get next list entry child node from list element
                    entryElem = entryElem.nextSiblingElement(ListEntryTag);
                }

                // Store the loaded list in output variant
                variant = list;

            } else {
                // The child type is unknown
                qCritical() << QString("unknown type node %1").arg(typeName);
                return false;
            }

            // Successfully loaded a variant from a typed child node.
            return true;
        } else if (QVariant(QVariantList()).canConvert(typeId)
                   && !(child = element.firstChildElement(QMetaType::typeName(QMetaType::QVariantList))).isNull()) {
            // The found element has a child element of with the tag QVariantList
            //  and the QVariantList can be converted into the target type (QList<T>)

            // Handle QLists. Read all child nodes tagged ListEntryTag from list element.
            QVariantList list;
            // Get first entry's element
            QDomElement entryElem = child.firstChildElement(ListEntryTag);

            // For all list entry child nodes, load the variant entries and store them in list
            while (!entryElem.isNull()) {
                QVariant variant;

                // Read the variant and store it in list if successful
                if (loadVariant(variant, entryElem)) {
                    list << variant;
                }

                // Get next list entry child node from list element
                entryElem = entryElem.nextSiblingElement(ListEntryTag);
            }

            // Store the loaded list in output variant
            variant = list;

            if (!variant.convert(typeId)) { //QVariantList to QList<T> conversion failed
                qWarning() << "Could convert QVariantList to" << typeName;
                return false;
            }

            // Successfully loaded a QList<T> via QVariantList from Child Node
            return true;

        } else if (QVariant(QVariantMap()).canConvert(typeId)
                   && !(child = element.firstChildElement(QMetaType::typeName(QMetaType::QVariantMap))).isNull()) {
            // The found element has a child element of with the tag QVariantMap
            //  and the QVariantMap can be converted into the target type (QMap<String,T>)

            // Handle QMaps. Read all child nodes tagged MapEntryTag from list element.
            QVariantMap map;
            // Get first entry's element
            QDomElement entryElem = child.firstChildElement(MapEntryTag);

            // For all list entry child nodes, load the variant entries and store them in list
            while (!entryElem.isNull()) {
                if (entryElem.hasAttribute(KeyTag)) {
                    QString key = entryElem.attribute(KeyTag);
                    QVariant value;

                    // Read the variant and store it in list if successful
                    if (loadVariant(value, entryElem)) {
                        map.insert(key, value);
                    }
                }

                // Get next map entry child node from list element
                entryElem = entryElem.nextSiblingElement(MapEntryTag);
            }

            // Store the loaded map in output variant
            variant = map;

            if (!variant.convert(typeId)) { //QVariantMap to QMap<QString,T> conversion failed
                qWarning() << "Could convert QVariantMap to" << typeName;
                return false;
            }

            // Successfully loaded a QMap<QString, T> via QVariantMap from Child Node
            return true;
        } else if (!(child = element.firstChildElement(QObjectTag)).isNull()) {
            QMetaType mt(typeId);

            if (!mt.isValid() || (!mt.flags().testFlag(QMetaType::PointerToQObject) &&
                                  !mt.flags().testFlag(QMetaType::SharedPointerToQObject))) {
                qWarning() << "Could not load Object" << typeName << "because it's no longer a QObject subclass";
                return false;
            }

            QObject* object = nullptr;

            if (mt.flags().testFlag(QMetaType::SharedPointerToQObject)) { //QSharedPointer<T*>
                Q_ASSERT(typeName.startsWith("QSharedPointer<") && typeName.endsWith('>'));
                QString innerTypeName = typeName.mid(QString("QSharedPointer<").length());
                innerTypeName.chop(1);
                innerTypeName += "*";
                int innerMetaTypeId = QMetaType::type(innerTypeName.toStdString().c_str());

                if (innerMetaTypeId == QMetaType::UnknownType) {
                    qWarning() << "Could not load Object" << typeName << "because" << innerTypeName << "is no longer a QObject subclass";
                    return false;
                }

                QMetaType innerMetaType(innerMetaTypeId);
                object = innerMetaType.metaObject()->newInstance();
                variant = QVariant::fromValue(object);

                if (!variant.convert(innerMetaTypeId)) {
                    delete object;
                    qWarning() << "Could load QObject" << innerTypeName << "because cast failed";
                    return false;
                }
            } else { //Normal T*
                object = mt.metaObject()->newInstance();
                variant = QVariant::fromValue(object);
            }

            if (object == nullptr) {
                qWarning() << "Couldn't create a new instance of " << typeName << ". Q_INVOKABLE missing on ctor?";
                return false;
            }

            if (!DomHelper::loadUserProperties(object, child)) {
                delete object;
                qWarning() << "Error while loading properties of QObject" << typeName;
                return false;
            }

            return true;

        } else if (!(child = element.firstChildElement(QGadgetTag)).isNull()) {
            QMetaType metaType(typeId);

            if (!metaType.isValid() || !metaType.flags().testFlag(QMetaType::IsGadget)) {
                qWarning() << "Could not load Object" << typeName << "because it is not a QGadget.";
            }

            void* object = QMetaType::create(typeId);

            if (!DomHelper::doLoadUserProperties(metaType.metaObject(), makePropertyWriter(object), child)) {
                QMetaType::destroy(typeId, object);
                qWarning() << "Error while loading properties of QGadget" << typeName;
                return false;
            }

            QVariant objectVariant(typeId, object);
            variant.swap(objectVariant);

            return true;
        }

        // Reaching here means we have no child node that is tagged with the type name. Check for
        // a text node with base64 decoded serialized data
        QDomNode node = element.firstChild();

        if (node.isText()) {
            // OK, we have a text node. Read the text, decode it and store it in a byte array
            QByteArray byteArray = QByteArray::fromBase64(node.toText().data().toLatin1());
            // Open a read buffer to read the array stream
            QBuffer readBuffer(&byteArray);
            readBuffer.open(QIODevice::ReadOnly);
            // Create a data stream backed up with the read buffer
            QDataStream in(&readBuffer);

            // Deserialize and load the variant from stream
            variant.load(in);

            // Close read buffer
            readBuffer.close();

            // Check for error
            if (in.status() != QDataStream::Ok) {
                qCritical() << "cannot deserialize variant!";
                // Be sure the output variant is invalid
                variant = QVariant();
                return false;
            }

            // Successfully loaded a deserialized variant
            return true;
        }

        // We have a child node but could not load/decode it
        return false;

    } else {
        // Simple variant types are stored as value attribute
        QString value = element.attribute(ValueTag);

        // Validate value
        if (value.isEmpty()) {
            // None/Empty: nullstring
            variant = QVariant(QString::null);
        } else {
            variant = value;
        }

        if (variant.canConvert(typeId)) {
            // The value string can be converted to the related variant type
            variant.convert(typeId);
        } else {
            // No way to convert the string value to the variant type
            qCritical() << QString("cannot convert value to type %1").arg(typeName);
            return false;
        }
    }

    // Successful
    return true;
}

QDomElement DomHelper::saveProperty(const QObject* object, const char* name, QDomDocument& doc)
{
    // Retrieve the property from object and save it as variant tagged as property
    return saveVariant(object->property(name), PropertyTag, doc, name);
}

QDomElement DomHelper::saveProperty(const QObject* object,
                                    const QMetaProperty& property,
                                    QDomDocument& doc)
{
    char const* const name = property.name();

    // Retrieve the property from object and save it as variant tagged as property
    return saveVariant(object->property(name), PropertyTag, doc, name);
}

bool DomHelper::loadProperty(QObject* object, const QDomElement& element)
{
    const QMetaObject* metaObject = object->metaObject();

    auto loader = makePropertyWriter(object);

    return doLoadProperty(metaObject, loader, element);
}

bool DomHelper::doLoadProperty(QMetaObject const* metaObject, std::function<bool(QMetaProperty const&, QVariant const&)> reader, const QDomElement& element)
{
    // The property should have been saved as variant element tagged as property with
    // related name
    QVariant propValue;
    QString name;

    // Load the variant and related property name
    if (!loadVariant(propValue, name, element)) {
        qWarning() << "failed to load variant" << name;
        return false;
    }

    // Convert name to char array for meta methods
    QByteArray propName = name.toUtf8();
    // Get object's meta object
    // And the property index of the related property
    int propIndex = metaObject->indexOfProperty(propName);

    // Check if property is known to object. If not, quit
    if (propIndex == -1) {
        qWarning("unknown property \"%s\"", (const char*)propName);
        // Note: this is not an error, the property which was persisted is just ignored, as the
        // object has the property not anymore defined. If the property should be save as dynamic
        // property to the object, just comment the following return statement.
        return true;
    }

    // Get property
    QMetaProperty property = metaObject->property(propIndex);

    // Get property type id
    int propType = property.userType();

    // If property value is valid, check if the type of the loaded variant can be converted
    // to the property type!
    if (propValue.isValid() && !propValue.canConvert(propType)) {
        qWarning("property \"%s\": type mismatch: have \"%s\", expected \"%s\", no conversion available",
                 (const char*)propName, (const char*)propValue.typeName(),
                 QMetaType::typeName(propType));
        return false;
    }


    // Convert the variant
    if (propValue.convert(propType)) {
        // Set the property in the object and return result
        //if (object->setProperty(propName, propValue)) {
        if (reader(property, propValue)) {
//        qDebug("property \"%s\" loaded successfully", (const char*)propName);
            return true;
        }
    }

    qWarning() << "Couldn't set property " << propName << "to value" << propValue;
    return false;
}

bool DomHelper::saveUserProperties(const QObject* object, QDomElement container, QDomDocument& doc)
{
    // Get the properties from object using the meta object system
    const QMetaObject* metaObject = object->metaObject();

    auto saver = makePropertyReader(object);

    return doSaveUserProperties(metaObject, saver, container, doc);
}

bool DomHelper::doSaveUserProperties(QMetaObject const* metaObject, std::function<bool(QMetaProperty const&, QVariant&)> saver, QDomElement container, QDomDocument& doc)
{
    if (metaObject == nullptr) {
        qWarning() << "Error saving user properties: metaObject is null!";
        return false;
    }

    // Get property count
    const int nProps = metaObject->propertyCount();
    // Indicate success
    bool success = true;

    // Cycle through the properties and save all USER properties
    for (int i = 0; i < nProps; ++i) {
        // Get property meta description
        QMetaProperty prop = metaObject->property(i);

        // If it is a user property, save it
        if (prop.isUser()) {
            // Create element for property and save property to it
            QVariant variant;
            bool const saveSuccess = saver(prop, variant);

            if (saveSuccess) {
                QDomElement propElem = saveVariant(variant, PropertyTag, doc, prop.name());

                // If successful, append it to the specified container, log error and indicate problem
                // otherwise
                if (!propElem.isNull()) {
                    container.appendChild(propElem);
                } else {
                    qCritical() << QString("failed to save property '%1'!").arg(prop.name());
                    success = false;
                }
            } else {
                qCritical() << QString("failed to read property '%1'!").arg(prop.name());
                success = false;
            }

        }
    }

    // Return success result
    return success;
}

bool DomHelper::loadUserProperties(QObject* object, const QDomElement& container)
{
    const QMetaObject* metaObject = object->metaObject();

    auto loader = makePropertyWriter(object);

    return doLoadUserProperties(metaObject, loader, container);
}

bool DomHelper::doLoadUserProperties(QMetaObject const* metaObject, std::function<bool(QMetaProperty const&, QVariant const&)> loader, const QDomElement& container)
{
    // Indicate success
    bool success = true;

    // Read all child elements tagged as property from container
    QDomElement propElem = container.firstChildElement(PropertyTag);

    while (!propElem.isNull()) {
        // Load each child property and store it in object
        if (!doLoadProperty(metaObject, loader, propElem)) {
            qDebug("problem loading property");
            success = false;
        }

        // Get next property child element
        propElem = propElem.nextSiblingElement(PropertyTag);
    }

    return success;
}

bool DomHelper::doRegisterSerializable(int typeId, DomHelper::SerializerWrapperType serializer,
                                       DomHelper::DeserializerWrapperType deserializer)
{
    if (_serializables.contains(typeId)) {
        return false;
    }

    _serializables.insert(typeId, {serializer, deserializer});

    return true;
}

std::function<bool(QMetaProperty const&, QVariant&)> DomHelper::makePropertyReader(void const* object)
{
    return [ = ](QMetaProperty const & p, QVariant & v) {
        auto variant = p.readOnGadget(object);

        if (variant.isValid()) {
            v.swap(variant);
            return true;
        }

        return false;
    };
}

std::function<bool(QMetaProperty const&, QVariant&)> DomHelper::makePropertyReader(QObject const* object)
{
    return [ = ](QMetaProperty const & p, QVariant & v) {
        auto variant = p.read(object);

        if (variant.isValid()) {
            v.swap(variant);
            return true;
        }

        return false;
    };
}

std::function<bool(QMetaProperty const&, QVariant const&)> DomHelper::makePropertyWriter(void* object)
{
    return [ = ](QMetaProperty const & p, QVariant const & v) {
        return p.writeOnGadget(object, v);
    };
}

std::function<bool(QMetaProperty const&, QVariant const&)> DomHelper::makePropertyWriter(QObject* object)
{
    return [ = ](QMetaProperty const & p, QVariant const & v) {
        return p.write(object, v);
    };
}
