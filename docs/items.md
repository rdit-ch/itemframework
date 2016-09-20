# Items & Connectors

Per default the item toolbox in your application will be empty. You have to provide item's that the user can use afterwards.

An Item is basically just an entry point for the user to launch into your code. A item has a type and an arbiarty number of inputs & outputs. Each input and output has a type. An output can only be connected to (multiple) inputs of the same type. An input cannot be connected to multiple outputs. The appearance of the item (icon, context-menu, window?) can be fully customized by you. Also: What you transport over the connections is enterily up to you. You'll receive a signal when an input has changed on your item.

Usually Item's are shipped as plugins. Read More in the [Plugin System](./plugin.md) doc.  
The Item's state can be saved/restored automatically by using our [Storage System](./storage.md).

To get started you need to subclass one of the following classes:

* AbstractItem: Baseclass for all Items without default behaviour
* AbstractWindowItem: Baseclass for Items that use a window, which can be opened by doubleclicking the item.

In the constructor of your item, you should usually:

* Set your item's icon
* Add a number of inputs and/or outputs to the item
* Connect to the `dataChanged` signal of the inputs

In order to automatically save/restore the state of the item and also make it copy and pasteable you should add some `Q_PROPERTIES` to the class. A frequently used trick for items that do most of their work in their window class is to create a class which holds the item's configuration, add that class as the only property to the item, and forward the class instance to the widget in the setter of the property. That way you don't have to synchonize changes in the widget manually back to the item for saving.




To get started you should checkout the example project and inspect the items there.



