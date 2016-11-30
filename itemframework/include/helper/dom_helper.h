#ifndef DOM_HELPER_H_
#define DOM_HELPER_H_

#include "appcore.h"

#include <qdom.h>
#include <qvariant.h>
#include <qdebug.h>
#include <functional>

#include <QMetaProperty>
/**
 * @brief Class DomHelper acts as a helper class for storing and retrieving data to or from
 * a DOM document.
 *
 * At the moment it helps you with storing and retrieving QVariant data and QObject properties.
 */
class ITEMFRAMEWORK_EXPORT DomHelper
{
public:
    // Global Tag definitions
    static constexpr const char* NameTag = "name";
    static constexpr const char* TypeTag = "type";
    static constexpr const char* ValueTag = "value";
    static constexpr const char* KeyTag = "key";
    static constexpr const char* PropertyTag = "property";
    static constexpr const char* ListEntryTag = "listEntry";
    static constexpr const char* MapEntryTag = "mapEntry";
    static constexpr const char* QObjectTag = "qobject";
    static constexpr const char* QGadgetTag = "qgadget";

    using SerializerWrapperType   = std::function<bool(QDomDocument&, QDomElement&, QVariant const&)>;
    using DeserializerWrapperType = std::function<bool(QDomElement&, QVariant&)>;

    /**
     * @brief Creates a new DOM element \a tagName, saves \a variant to it and returns the created
     * element.
     *
     * The created element will be tagged \a tagName and has at least one attribute named
     * DomHelper::TypeTag with the variant's type name.
     *
     * If \a name is not empty, another attribute named DomHelper::NameTag with value \a name will
     * be created and added to the created DOM element.
     *
     * If \a variant has converters to and from string, i.e.
     *
     *     variant.canConvert(QMetaType::QString) && QVariant(QString()).canConvert(variant.userType())
     *
     * is \c true, an attribute named DomHelper::ValueTag with the variant's string value will be
     * added to the created DOM element.
     *
     * Some variant type's (e.g. QVariant::List, QVariant::StringList) value will be added as human
     * readable children to the created element.
     *
     * If the \a variant value cannot be handled in these ways, the variant will be serialized,
     * converted to base64 and added as text node to the created DOM element.
     *
     * @param variant The variant to be saved.
     * @param tagName The tag name of the created DOM element.
     * @param doc Used to create child nodes.
     * @param name An optional name. If not empty, an attribute with name DomHelper::NameTag and
     * value \a name will be added to the created DOM element.
     *
     * @return A DOM element with tag name \a tagName which has \a variant stored to it or a
     * \c null element if \a variant cannot be saved.
     */
    static QDomElement saveVariant(const QVariant& variant, const QString tagName, QDomDocument& doc,
                                   const QString& name = QString::null);
    /**
     * @brief Returns \c true, if \a variant was successfully saved to \a element;
     * \c false otherwise.
     *
     * At least one attribute named DomHelper::TypeTag with the variant's type name will be
     * added to \a element.
     *
     * If \a name is not empty, another attribute named DomHelper::NameTag with value \a name will
     * be created and added to \a element.
     *
     * If \a variant has converters to and from string, i.e.
     *
     *     variant.canConvert(QMetaType::QString) && QVariant(QString()).canConvert(variant.userType())
     *
     * is \c true, an attribute named DomHelper::ValueTag with the variant's string value will be
     * added to \a element.
     *
     * Some variant type's (e.g. QVariant::List, QVariant::StringList) value will be added as human
     * readable children to \a element.
     *
     * If the \a variant value cannot be handled in these ways, the variant will be serialized,
     * converted to base64 and added as text node to \a element.
     *
     * @param variant The variant to be saved.
     * @param element The element to save the variant to.
     * @param doc Used to create child nodes.
     * @param name An optional name. If not empty, an attribute with name DomHelper::NameTag and
     * value \a name will be added to \a element.

     * @return \c true, if \a variant was successfully saved to \a element; \c false otherwise.
     */
    static bool saveVariant(const QVariant& variant, QDomElement element, QDomDocument& doc,
                            const QString& name = QString::null);
    /**
     * @brief Loads \a variant from \a element and returns \c true on success, \c false otherwise.
     *
     * If \a element has a string attribute named DomHelper::NameTag, the attribute's value will
     * be returned in \a name.
     *
     * Note that the variant should have been stored with
     * DomHelper::saveVariant(const QVariant&, QDomElement, QDomDocument&,const QString&) or
     * DomHelper::saveVariant(const QVariant&, const QString, QDomDocument&,const QString&)
     * in order to be loaded successfully.
     *
     * @param variant The variant to be loaded.
     * @param name If \a element has a string attribute named DomHelper::NameTag, the
     * attribute's value will be returned in \a name.
     * @param element The DOM element to retrieve the variant from.
     *
     * @return \c true, if \a variant could be loaded successfully, \c false otherwise.
     */
    static bool loadVariant(QVariant& variant, QString& name, const QDomElement& element);
    /**
     * @brief Loads \a variant from \a element and returns \c true on success, \c false otherwise.
     *
     * Note that the variant should have been stored with
     * DomHelper::saveVariant(const QVariant&, QDomElement, QDomDocument&,const QString&) or
     * DomHelper::saveVariant(const QVariant&, const QString, QDomDocument&,const QString&)
     * in order to be loaded successfully.
     *
     * @param variant The variant to be loaded.
     * @param name If \a element has a string attribute named DomHelper::NameTag, the
     * attribute's value will be returned in \a name.
     * @param element The DOM element to retrieve the variant from.
     *
     * @return \c true, if \a variant could be loaded successfully, \c false otherwise.
     */
    static bool loadVariant(QVariant& variant, const QDomElement& container);

    /**
     * @brief Creates a new DOM element with tag name DomHelper::PropertyTag, saves
     * the \a object's property named \a name to it and returns the created element
     *
     * As the property value is a variant, the property will be saved in the format described
     * in DomHelper::saveVariant(const QVariant&, const QString, QDomDocument&, const QString&)
     * with DomHelper::PropertyTag as tag name.
     *
     * @param object The object to retrieve the property from.
     * @param name The name of the property
     * @param doc Used to create the element and child nodes.
     *
     * @return A DOM element with tag name DomHelper::PropertyTag which has the \a object's
     * property named \a name stored to it or a \c null element if the property cannot be saved.
     */
    static QDomElement saveProperty(const QObject* object, const char* name, QDomDocument& doc);
    /**
     * @brief Creates a new DOM element with tag name DomHelper::PropertyTag, saves
     * the \a object's \a property to it and returns the created element
     *
     * As the property value is a variant, the property will be saved in the format described
     * in DomHelper::saveVariant(const QVariant&, const QString, QDomDocument&, const QString&)
     * with DomHelper::PropertyTag as tag name.
     *
     * @param object The object to retrieve the property from.
     * @param property The property to be saved
     * @param doc Used to create the element and child nodes.
     *
     * @return A DOM element with tag name DomHelper::PropertyTag which has the \a object's
     * \a property stored to it or a \c null element if the property cannot be saved.
     */
    static QDomElement saveProperty(const QObject* object, const QMetaProperty& property,
                                    QDomDocument& doc);
    /**
     * @brief Loads a property from \a element, sets the related property for \a object
     * and returns \c true on success, \c false otherwise.
     *
     * The property's name is retrieved from \a element's attribute named DomHelper::Name.
     *
     * Note that the property should have been stored with
     * DomHelper::saveProperty(const QObject*, const char*, QDomDocument&,) or
     * DomHelper::saveProperty(const QObject*, const QMetaProperty&, QDomDocument&)
     * in order to be loaded successfully.
     *
     * @param object The QObject to set the loaded property for.
     * @param element The element to retrieve the property from.
     *
     * @return \c true, if successful, \c false otherwise.
     */
    static bool loadProperty(QObject* object, const QDomElement& element);
    /**
     * @brief Saves all properties of \a object that are marked as USER property to \a container
     * and returns \c true if successful, \c false otherwise.
     *
     * @param object The QObject to read the properties from.
     * @param container The container to save the properties to.
     * @param doc Used to create the element and child nodes.
     *
     * @return \c true if successful, \c false otherwise.
     */
    static bool saveUserProperties(const QObject* object, QDomElement container, QDomDocument& doc);
    /**
     * @brief Loads all property elements from \a container and sets them in \a object. Returns
     * \c true if successful, \c false otherwise.
     *
     * The method will read all child elements of \a container that are tagged
     * DomHelper::PropertyTag.
     *
     * Note that the properties should have been stored with
     * DomHelper::saveProperty(const QObject*, const char*, QDomDocument&,) or
     * DomHelper::saveProperty(const QObject*, const QMetaProperty&, QDomDocument&) or
     * DomHelper::saveUserProperties(const QObject*, QDomElement, QDomDocument&)
     * in order to be loaded successfully.
     *
     * @param object The QObject to store the properties to.
     * @param container The DOM element to load the properties from.
     *
     * @return \c true if successful, \c false otherwise.
     */
    static bool loadUserProperties(QObject* object, const QDomElement& container);


    QDomElement saveProperty(const QVariant& 	property,
                             char const* const 	name,
                             QDomDocument& 		doc);


    /**
     * Registry for serializers and deserializers of user-defined types.
     */
    template <typename T>
    static void registerSerializable(
        std::function<bool(QDomDocument&, QDomElement&, T const&)> serializer,
        std::function<T(QDomElement const&, bool&)>                deserializer)
    {
        int const typeId = qMetaTypeId<T>();

        SerializerWrapperType serializerWrapper = [=](QDomDocument& doc, QDomElement& element,
                                  QVariant const& variant) {
            T value = variant.value<T>();
            bool const success = serializer(doc, element, value);
            return success;
        };

        DeserializerWrapperType deserializerWrapper = [=](QDomElement& element, QVariant& variant) {
            bool success = false;
            T value = deserializer(element, success);
            if (success) {
                variant.setValue(value);
            }
            return success;
        };

        if (!doRegisterSerializable(typeId, serializerWrapper, deserializerWrapper)) {
            qDebug() << "Failed to register serializable " << QMetaType::typeName(typeId);
        }
    }

    template <typename T>
    static typename std::enable_if<!std::is_base_of<QObject, T>::value, bool>::type
    saveUserProperties(T const* object, QDomElement container, QDomDocument& doc)
    {
        QMetaType metaType(qMetaTypeId<T>());
        if (metaType.flags().testFlag(QMetaType::IsGadget)) {
            return doSaveUserProperties(metaType.metaObject(), makePropertyReader(object), container, doc);
        }
        return false;
    }

    template <typename T>
    static typename std::enable_if<!std::is_base_of<QObject, T>::value, bool>::type
    loadUserProperties(T* object, QDomElement container)
    {
        QMetaType metaType(qMetaTypeId<T>());
        if (metaType.flags().testFlag(QMetaType::IsGadget)) {
            return doLoadUserProperties(metaType.metaObject(), makePropertyWriter(object), container);
        }
        return false;
    }

private:
    static bool doSaveUserProperties(QMetaObject const* metaObject, std::function<bool(QMetaProperty const&, QVariant&)> saver, QDomElement container, QDomDocument& doc);

    static bool doLoadUserProperties(QMetaObject const* metaObject, std::function<bool(QMetaProperty const&, QVariant const&)> loader, const QDomElement& container);

    static bool doLoadProperty(QMetaObject const* object, std::function<bool(QMetaProperty const&, QVariant const&)> loader, const QDomElement& element);

    static std::function<bool(QMetaProperty const&, QVariant&)> makePropertyReader(void const* object);
    static std::function<bool(QMetaProperty const&, QVariant&)> makePropertyReader(QObject const* object);
    static std::function<bool(QMetaProperty const&, QVariant const&)> makePropertyWriter(void* object);
    static std::function<bool(QMetaProperty const&, QVariant const&)> makePropertyWriter(QObject* object);

    static bool doRegisterSerializable(int typeId, SerializerWrapperType serializer,
                                                   DeserializerWrapperType deserializer);

    static bool saveVariant(const QVariant& variant, QDomElement element,
                            QDomDocument& doc, const QString& name,
                            const QString& typeTagName,
                            const QString& valueTagName);
};

/**
 * Helper to convert from T* to QSharedPointer<T>
 */
template <typename T> static QSharedPointer<T> createSharedPointer(T* obj) {
    return QSharedPointer<T>(obj);
}

/**
 * Helper to register converter from T* to QSharedPointer<T>
 * Will also register T as Metatype using qRegisterMetaType
 */
template <typename T> typename std::enable_if<QtPrivate::IsSharedPointerToTypeDerivedFromQObject<T>::Value>::type registerMetaType()
{
    qRegisterMetaType<typename T::Type*>();
    if (!QMetaType::hasRegisteredConverterFunction(qMetaTypeId<typename T::Type*>(), qMetaTypeId<T>())) {
        QMetaType::registerConverter<typename T::Type*,T>(createSharedPointer<typename T::Type>);
    }
}

/**
 * Helper to register T as Metatype in case T is not a QSharedPointer
 */
template <typename T> typename std::enable_if<!QtPrivate::IsSharedPointerToTypeDerivedFromQObject<T>::Value>::type  registerMetaType()
{
    qRegisterMetaType<T>();
}

/**
 * Helper function to convert a QVariantList to a Qt container of type \a C<T>.
 */
template <template<typename> class C, typename T> static C<T> fromVariantList(QVariantList lis)
{
    C<T> newContainer; //Container of Target Types

    //iterate over all QVariants in the QVariantList
    for (auto i = lis.cbegin(); i != lis.cend(); ++i) {
        if (!i->canConvert(qMetaTypeId<T>())) { //Cannot convert to target type
            qDebug() << "Cannot convert" << i->typeName() << "to" << QMetaType::typeName(qMetaTypeId<T>());
        } else { //Conversion ok
            newContainer.append(i->value<T>()); //Cast QVariant to target type and append to list
         }
     }

    return newContainer;
}

/**
 * Register converter to convert from QVariantList to QList<T> or QVector<T>.
 * \a T will automatically be registered as QMetaType, if it hasn't already been registered.
 */
template <typename T> void registerVariantListConverter()
{
    registerMetaType<T>();

    //Register a converter from QVariantList to QList<T> if it has not already been registered.
    if (!QMetaType::hasRegisteredConverterFunction(QMetaType::QVariantList, qMetaTypeId<QList<T>>())) {
        QMetaType::registerConverter<QVariantList, QList<T>>(fromVariantList<QList, T>);
    }

    //Register a converter from QVariantList to QVector<T> if it has not already been registered.
    if (!QMetaType::hasRegisteredConverterFunction(QMetaType::QVariantList, qMetaTypeId<QVector<T>>())) {
        QMetaType::registerConverter<QVariantList, QVector<T>>(fromVariantList<QVector, T>);
    }
}

/**
 * Helper function to convert a QVariantMap to a Qt container of type \a C<QString,V>.
 */
template <template<typename, typename> class C, typename V> static C<QString, V> fromVariantMap(QVariantMap map)
{
    C<QString, V> newContainer; //Container of Target Types

    QMapIterator<QString, QVariant> it(map);

    while (it.hasNext()) {
        it.next();

        if (!it.value().canConvert(qMetaTypeId<V>())) { //Cannot convert to target type
            qDebug() << "Cannot convert" << it.value().typeName() << "to" << QMetaType::typeName(qMetaTypeId<V>());
        } else { //Conversion ok
            newContainer.insert(it.key(), it.value().value<V>()); //Cast QVariant to target type and append to container
        }

    }

    return newContainer;
}

/**
 * Register converter to convert from QVariantMap to QHash<QString,V> or QMap<QString,V>.
 * \a T will automatically be registered as QMetaType, if it hasn't already been registered.
 */
template <typename V> void registerVariantMapConverter()
{
    registerMetaType<V>();

    //Register a converter from QVariantMap to QMap<K,V> if it has not already been registered.
    if (!QMetaType::hasRegisteredConverterFunction(QMetaType::QVariantMap, qMetaTypeId<QMap<QString, V>>())) {
        QMetaType::registerConverter<QVariantMap, QMap<QString, V>>(fromVariantMap<QMap, V>);
    }

    //Register a converter from QVariantMap to QHash<K,V> if it has not already been registered.
    if (!QMetaType::hasRegisteredConverterFunction(QMetaType::QVariantMap, qMetaTypeId<QHash<QString, V>>())) {
        QMetaType::registerConverter<QVariantMap, QHash<QString, V>>(fromVariantMap<QHash, V>);
    }
}


#endif /* DOM_HELPER_H_ */
