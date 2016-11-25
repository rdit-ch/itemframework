#ifndef RESOURCE_H
#define RESOURCE_H

#include <QVariant>
#include <QDebug>

namespace core
{

/**
 * @brief The Resource class acts as a key-value store.
 *
 * It allows values of any type to be stored at keys identified by strings.
 *
 * It is currently used to define all style properties in one central place.
 *
 * When requesting the value stored at a certain key, the type of the value needs
 * to be specified by the caller. This is done either by supplying some value
 * of the given type (the default value returned in case of lookup failure) or
 * by simply invoking the template function of the corresponding type. \sa get
 */
class Resource
{
public:
    /**
     * @brief Returns the value associated with the key \a key, if any
     * @param key The key whose value is requested
     * @param default The default value to be returned if no value is associated
     * with the given key \a key. This parameter is used to infer the type of
     * the requested value. If it is omitted, the type must be explicitly specified
     * when invoking this function. In this case, the type must also be default
     * constructible in order to generate a default value.
     * @return The value associated with the key \a key, \a defaultValue if no
     * value is stored at key \a key
     */
    template<typename T> static T get(QString const& key, T const& defaultValue = T())
    {
        init();

#ifdef QT_DEBUG

        if (!map_res.contains(key)) {
            qDebug() << __FILE__ << "No such resource" << key;
        }

#endif

        QVariant v = map_res.value(key, defaultValue);
        return v.value<T>();
    }

    /**
     * @brief Sets the key \a key to the value \a value
     * @param key The key to set to the value \a value
     * @param value The value to set the key \a key to
     * @return true if a previously stored value was overwritten, false otherwise
     */
    template<typename T> static bool set(QString const& key, T const& value)
    {
        init();

        bool overwritten = false;
        QVariant variant = QVariant::fromValue(value);

        if (map_res.contains(key)) {
            overwritten = true;
        }

        map_res.insert(key, variant);

        return overwritten;
    }

private:
    static void init();

    static bool b_init;
    static QMap<QString, QVariant> map_res;
};
} // namespace core

using namespace core;

#endif // RESOURCE_H
