# Singletons and Static Initialization

The Itemframework has a baseclass `Singleton<T>` which is used across the itemframework, but can also be used by the usercore developer. Also, we have a pattern for static initialization of additional components, which ensures the correct order of initialization.

In contrast to a normal singleton, our singleton system has the following additional features:

* Singletons can depend on other Singletons, the correct order of initialization is guaranteed.
* Dependency Cycles will be detected and reported
* Singletons have a `postInit` and `preDestroy` method which will be called before the app starts/ends.

Note: `Singleton<T>` can only be used from within libraries which are loaded **before** the apps starts. This means the Singleton functionality can be used in the usercore, starter and itemframework project, but not in the plugins.

The program flow is as follows:

1. main() is invoked and the dynamic linker loads the itemframework and the usercore library
2. The subclasses of `Singleton<T>` and the "Components" add themself to the `StartupHelper` class using static initialization.
3. The application event loop starts (`QCoreApplication::exec`)
4. The dependencies marked by the Singletons and Components are resolved, and the order of initialzation is determined.
5. The Singletons/Components are created in order.
6. `postInit()` is called on all Singletons in order.
7. The application runs untill the user closes it.
8. `preDestroy()` is called on all Singletons in reverse order.
9. The Singletons/Components are destructed/deleted in reverse order.

## Components

If you only need static initialization at the right time, you can add a `static void init()` and/or `static void deinit()` function to your class. Then add a `STARTUP_ADD_COMPONENT(myclass)` to the source file to take care of the registration


## Singletons

If your singleton class is named `A` you should let it inherit from `Singleton<A>`. This will automatically create a `instance` member function, which you can later use to fetch the instance. Also you will have to add the `bool postInit()` and `bool preDestroy()` members. You can return `true` from these members if you have nothing else to initialize/destroy or return `false` if initialization/destruction failed.

Finally add a `STARTUP_ADD_SINGLETON(A)` to the source file to take care of the registration.

## Control the order of initialization

Both, Singletons and Components can control their position in the list of to be initialized singletons/components by adding the following classinfo's:

- `Q_CLASSINFO("dependsOn", "PluginManager")` depend on the PluginManager Singleton/Component unconditionally.
- `Q_CLASSINFO("optionallyDependsOn", "GuiManager")` depend on the GuiManager Singleton/Component only if it's available.
- `Q_CLASSINFO("guiModule","true")` marks the module as Gui Module. This means the Singleton/Components will only be loaded if you are in a Gui Application and not if you are in a Testcase/Console-Application

Singletons/Components must either inherit from `QObject` and use the `Q_OBJECT` macro or use the `Q_GADGET` macro.


