#ifndef CALLBACK_H
#define CALLBACK_H
#include <stdio.h>


//Refactored for C++11. In the git repo there is a c++03 version of it as well.

template <class R = void, class ...P>
class Callback_Base
{
public:
    virtual R Execute(P... params) const = 0;
    virtual ~Callback_Base() {}
};



template <class T, class R = void, class ...P>
class Callback : public Callback_Base<R, P...>
{
public:
    typedef R(T::*Callback_Function)(P... params);
    Callback(): instance(NULL), func(NULL) {}
    Callback(T* defining_instance, Callback_Function cb)
    {
        instance = defining_instance;
        func = cb;
    }
    ~Callback() {}
    R Execute(P... params) const
    {
        if (func == NULL || instance == NULL) {
            return R();
        } else {
            return (instance->*func)(params...);
        }
    }
private:
    T* instance;
    Callback_Function func;
};

//same as Callback but with a fixed state parameter that is always passed as first argument to the callback function
template <class T, class S, class R = void, class ...P>
class StatefulCallback : public Callback_Base<R, P...>
{
public:
    typedef R(T::*Callback_Function)(S state, P... params);
    StatefulCallback(): instance(NULL), func(NULL) {}
    StatefulCallback(T* defining_instance, Callback_Function cb, S state)
    {
        instance = defining_instance;
        func = cb;
        obj_state = state;
    }
    ~StatefulCallback() {}
    R Execute(P... params) const
    {
        if (func == NULL || instance == NULL) {
            return R();
        } else {
            return (instance->*func)(obj_state, params...);
        }
    }
private:
    S obj_state;
    T* instance;
    Callback_Function func;
};

#endif
