#ifndef STARTUP_HELPER_TEMPLATES
#define STARTUP_HELPER_TEMPLATES

namespace internal
{
typedef void (*VoidFnPtr)();

//Template to get a pointer to the init() function (static) of class T.
template<class T, class EN = void> struct GetInitFnPtr {
    //Fallback for the case when T has no init function
    static constexpr const VoidFnPtr value = nullptr;
};
template<class T> struct GetInitFnPtr<T, decltype(void(T::init()))> {
    //Specialization for the case when T has indeed a init() function
    static constexpr const VoidFnPtr value = &T::init;
};

//Template to get a pointer to the deinit() function (static) of class T.
template<class T, class EN = void> struct GetDeinitFnPtr {
    //Fallback for the case when T has no deinit function
    static constexpr const VoidFnPtr value = nullptr;
};
template<class T> struct GetDeinitFnPtr<T, decltype(void(T::deinit()))> {
    //Specialization for the case when T has indeed a deinit() function
    static constexpr const VoidFnPtr value = &T::deinit;
};

//Helper to check whether or not class T has a staticMetaObject (qt)
template<class T, class EN = void> struct HasMetaObject : std::false_type {
    //Specialization for the case when T has no staticMetaObject
};

template<class T> struct HasMetaObject<T,
         typename std::enable_if<std::is_same<const QMetaObject, decltype(T::staticMetaObject)>::value>::type> : std::true_type {
             //Specialization for the case when T has indeed staticMetaObject
             // using is_same to compare the type of staticMetaObject to QMetaObject. If theres no such member => SFINAE
         };
}

#endif // STARTUP_HELPER_TEMPLATES

