# Storage & Settings System

The Itemframework provides a flexible, recursive xml serializer/deserializer.

Here's what we can serialize/deserialize:

* QStrings, Integers, Booleans and other basic types
* Every type that can be converted into a string and can be converted back (via QVariant).
* QStringList
* `QList<T>` and `QVector<T>` if you register `T` using `DomHelper::registerVariantListConverter<T>()` (before using it).
* `QHash<QString,T>` and `QMap<QString,T>` if you register `T` using `DomHelper::registerVariantMapConverter<T>()` (before using it).
* Classes which contain the `Q_GADGET` macro or classes that are QObject derived, have the `Q_OBJECT` macro and a constructor with a `Q_INVOKABLE` macro.
* Classes which have a custom serializer/deserializer registred using `DomHelper::registerSerializable`.

Classes which contain the `Q_GADGET` or the `Q_OBJECT` macro are scanned for `Q_PROPERTIES` with `USER true`. If the type of the property can be serialized (according to the list above) the property will be serialized/deserialized. Otherwise a warning will be printed. 


```

class NumberFilterConfig
{
    Q_GADGET
    Q_PROPERTY(int modulus READ modulus WRITE setModulus USER true)
    //... more properties could be here 

public:
    int modulus() const;
    void setModulus(int modulus);

private:
    int _modulus = 1;
};

Q_DECLARE_METATYPE(NumberFilterConfig)

```

## Invoke the serializer

The serializer/deserialized is part of the (static) `DomHelper` class, which:

* can be used standalone. Checkout out the doxygen documentation of the `DomHelper` class.
* will be used to automatically serialize the `Q_PROPERTIES` of `AbstractItem` subclasses.
* can be used through the `SettingsScope` class (see below).


## Custom `load` and `save` behaviour for Items

By default all `AbstractItem` subclasses will get their `Q_PROPERTIES` serialized/deserialized automatically. If you wan't a different behaviour you can override `AbstractItem::load` and `AbstractItem::save`. This will give you full power about how the xml of that item is going to look.

```
bool MyItem::save(QDomDocument& doc, QDomElement& element) const {
    // Save the item's state into the passed element by either writing to it's attribues or by adding childs using doc.createElement();

    // Return true if saving was sucessfull
}

bool MyItem::load(QDomElement& element) {
    // Restore the item's state from the passed element and return true on success
}

```

## Global, hierarchical/overridable Settings (SettingScope)

Sometimes you want to have a default setting and later override it with a custom value. E.g a default color palette for the application, which can be overridden on project level or item level. For that usecase we developed the `SettingsScope` class.

Per default there is a scope per:

* application (exactly one, saved under $HOME/.config/... ),
* workspace (one per workspace, saved in the workspace file),
* project (one per project, saved in the project file),
* and item (one per item instance, saved in the project file).

A Scope holds a list of settings, whereas each setting has a key (string) and a value (any type that can be serialized using DomHelper). Each scope has a parent scope from which it inherits it's settings unless the settings are overriden in the current scope. A Scope can be accessed using the `value`/`setValue` methods. If a setting in the current scopes changes (or in a parent scope without having a local override for it) the `valueChanged` signal will be invoked.


