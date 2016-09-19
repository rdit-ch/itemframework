# Plugin System

First we need to differentiate between two terms:

* Plugin Container: A shared library (dll,so,dylib) which will be loaded at runtime and can be enabled/disabled by the user.
* Plugin Component: A exported Class which implements a certain interface. Plugin components are bundled in a Plugin Container.

Each Plugin-Container is a own qmake project, and it will link against the itemframework and your usercore.  
Each Plugin-Container must provide a factory (sublass of `InterfaceFactory`), where it registers all to be exported classes.  
Each Plugin-Component must be QObject-derived and should implement a certain Interface.

## Protocol Analyzer Example

Let's say you build a protocol analyzer application. And you want to allow implemention of custom protocols by using the Plugin System.

You have an interface header file (e.g in your usercore):

```
class InterfaceProtocol {
   public:
     virtual const QString& protocolName() const = 0;
     virtual void process(const& QByteArray) = 0;
     //......
}
Q_DECLARE_INTERFACE(InterfaceProtocol,"org.mycompany.myapp.InterfaceProtocol")
```

Lets look at how a Plugin-Container containing a Tcp & Udp Protocol implementation would look like.

Your `tcpprotocol.h` (e.g. inside of the plugins/layer4 folder) would typically look like this:

```
class TcpProtocol: public QObject, public InterfaceProtocol {
    Q_OBJECT
    Q_INTERFACES(InterfaceProtocol)
    public:
      const QString& protocolName() const override;
      void process(const& QByteArray) override;
      //......
}

```

Then your Plugin-Component Factory would look like this:

`layer4factory.h` (e.g. also inside the plugins/layer4 folder)

```
#include "plugin/interface_factory.h"
class Layer4Factory : public InterfaceFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Layer4Factory" FILE "layer4.json")
    Q_INTERFACES(InterfaceFactory)
public:
    Layer4Factory();

public:
    bool init();
};

```

`layer4factory.cpp` (e.g. also inside the plugins/layer4 folder)

```
#include "layer4factory.h"
#include "plugin/plugin_manager.h"
#include "tcpprotocol.h"
#include "udpprotocol.h"

Layer4Factory::Layer4Factory()
{
}

bool Layer4Factory::init()
{
    PluginManager::instance()->addPluginComponent<TcpProtocol, InterfaceProtocol>();
    PluginManager::instance()->addPluginComponent<UdpProtocol, InterfaceProtocol>();
    return true;
}

```

That's it. Build your plugin, make sure it's placed in the correct plugin directory and it should be loaded by the PluginLoader.

Later in your programm, there are two ways of instantiating your plugins:


```
// Create an instance of a specific plugin component:
InterfaceProtocol* tcpProto = PluginManager::instance()->createInstance<InterfaceProtocol>("TcpProtocol");

// Create instances of all plugin components that implement a certain interface:
QVector<InterfaceProtocol*> myProtos = PluginManager::instance()->createInstances<InterfaceProtocol>();

// ...

// Don't forget to destroy the instances when you no longer need them.
delete tcpProto;
qDeleteAll(myProtos);

```

Note: It's also possible to let your plugin components directly intherit from a class which is QObject derived (instead of using `Q_INTERFACES` as shown above). For example: Just create a class which inherits from `AbstractItem` and register it in your factory, and your item will appear in the Item-Toolbox.
