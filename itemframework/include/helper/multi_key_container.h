#ifndef MULTI_KEY_HASH_H
#define MULTI_KEY_HASH_H
#include <QHash>
#include <QMap>
#include <QVariant>


template <template<typename, typename> class Container, typename TValue, typename... Args>
struct Multi_Key_Container;

//It looks more complicated than it is.
//Multi_Key_Hash<bool,int,int,int> is convenience replacement for QHash<int,QHash<int,QHash<int,bool>>>
//you can use it like mkh[key1][key2][key3][key4] = value;
//if you do use it like this^^ and mkh is not const it will create values on the fly, even if you are not using it as an lvalue.
//Use a const reference if you want to make sure nothing new is inserted

template <typename TValue, typename... KeyArgs>
using Multi_Key_Hash = Multi_Key_Container<QHash, TValue, KeyArgs...>;

template <typename TValue, typename... KeyArgs>
using Multi_Key_Map = Multi_Key_Container<QMap, TValue, KeyArgs...>;

//---------------------------------------------------------------------------------------------------------------------

template <template<typename, typename> class Container, typename TValue, typename TKey1, typename... KeyArgs>
struct Multi_Key_Container<Container, TValue, TKey1, KeyArgs...> : Container<TKey1, Multi_Key_Container<Container, TValue, KeyArgs...>> {
    //Returns the value or a default constructed value
    const TValue value(const TKey1& key1, const KeyArgs& ... keys) const
    {
        return this->operator[](key1).value(keys...);
    }
    //Returns a reference (modifiable) to the value or inserts a default construted value and returns that
    TValue& refvalue(const TKey1& key1, const KeyArgs& ... keys)
    {
        return this->operator[](key1).refvalue(keys...);
    }

    //creates an instance from a QList<QVariantList> structure
    static Multi_Key_Container<Container, TValue, TKey1, KeyArgs...> from_variant_list(const QList<QVariantList>& list)
    {
        Multi_Key_Container<Container, TValue, TKey1, KeyArgs...> cont;
        QList<QVariantList>::const_iterator it;

        for (it = list.constBegin(); it != list.constEnd(); it++) {
            cont.insert_from_variant_list(*it, 0);
        }

        return cont;
    }
private:
    void insert_from_variant_list(const QVariantList& l, int c)
    {
        Q_ASSERT_X(c < l.count(), "Multi_Key_Container::from_variant_list", "not enough columns in variant list");
        QVariant v = l.at(c);
        Q_ASSERT_X(v.canConvert<TKey1>(), "Multi_Key_Container::from_variant_list", "Cannot convert Key to specified Type");
        this->operator[](v.value<TKey1>()).insert_from_variant_list(l, c + 1);
    }

};

template <template<typename, typename> class Container, typename TValue, typename TKeyN>
struct Multi_Key_Container<Container, TValue, TKeyN> : Container<TKeyN, TValue> {
    TValue& refvalue(const TKeyN& keyN)
    {
        return this->operator[](keyN);
    }
private:
    void insert_from_variant_list(const QVariantList& l, int c)
    {
        Q_ASSERT_X(c + 1 < l.count(), "Multi_Key_Container::from_variant_list", "not enough columns in variant list");
        QVariant v = l.at(c);
        Q_ASSERT_X(v.canConvert<TKeyN>(), "Multi_Key_Container::from_variant_list", "Cannot convert Key to specified Type");
        QVariant v2 = l.at(c + 1);
        Q_ASSERT_X(v2.canConvert<TValue>(), "Multi_Key_Container::from_variant_list", "Cannot convert Value to specified Type");
        this->insert(v.value<TKeyN>(), v2.value<TValue>());
    }
};

//If TValue is a bool, we can assume it's true, whenever we have all keys for it
template <template<typename, typename> class Container, typename TKeyN>
struct Multi_Key_Container<Container, bool, TKeyN> : Container<TKeyN, bool> {
    bool& refvalue(const TKeyN& keyN)
    {
        return this->operator[](keyN);
    }
private:
    void insert_from_variant_list(const QVariantList& l, int c)
    {
        Q_ASSERT_X(c < l.count(), "Multi_Key_Container::from_variant_list", "not enough columns in variant list");
        QVariant v = l.at(c);
        Q_ASSERT_X(v.canConvert<TKeyN>(), "Multi_Key_Container::from_variant_list", "Cannot convert Key to specified Type");
        QVariant v2;

        if (c + 1 >= l.count()) {
            v2 = true;
        } else {
            v2 = l.at(c + 1);
            Q_ASSERT_X(v2.canConvert<bool>(), "Multi_Key_Container::from_variant_list", "Cannot convert Key to Bool");
        }

        this->insert(v.value<TKeyN>(), v2.value<bool>());
    }
};




#endif // MULTI_KEY_HASH_H
