# Projects & Workspaces

Projects and Workspaces are a powerful feature to save and restore the state of your application. Right now the application state is saved to a file, but you could easily extend the application to support remote projects & workspaces (e.g. stored on a sql server). Project are grouped into workspaces and a project can also be part of multiple workspaces, but only one workspace can be open at the time. The user can switch workspaces at program start by picking the desired workspace from the list of recent used workspaces.

Apart from the position, name and the connections of every item in the project, the item's state will also be saved if you implement it. Read more in the [Storage Doc](./storage.md).

The Projects & Workspaces can be saved, loaded, imported, exported, moved, deleted and more by the user in the context menu of the project list. Projects will also be autosaved and monitored for external changes.


## Example Workspace File

Here's an example workspace xml which contains a single project:

```
<?xml version="1.0"?>
<!DOCTYPE TravizpWorkspaceXML>
<TravizWorkspace description="" version="1.0" name="test">
 <TravizProject fastLoad="true">
  <ProjectConnection path="./test.tpro"/>
 </TravizProject>
</TravizWorkspace>

```

## Example Project File

In this example project 3 items of a different type have been placed on the item-scene. The filter item has a gui to configure it and it's the only item that saves/loads some settings.

![Screenshot of the example project](./testproject.png)

The number filter item has a single property of type `NumberFilterConfig` (class definition [here](./storage.md)) in it's item class.

Here you see the resulting xml:

```
<?xml version="1.0"?>
<!DOCTYPE TravizpProjectXML>
<TravizProject version="2.0" description="No description set." name="test">
 <GraphicsItemNote x="-82" y="-99">
  <GraphicsItemNoteData content="A simple project with three items and one note" width="180" color="fff9ee7a"/>
 </GraphicsItemNote>
 <GraphicsItem x="200" y="50" type="NumberViewerItem" id="0" name="number viewer1"/>
 <GraphicsItem x="-100" y="50" type="NumberEmitterItem" id="1" name="number emitter1"/>
 <GraphicsItem x="50" y="50" type="NumberFilterItem" id="2" name="number filter1">
  <GraphicsItemData>
   <property type="NumberFilterConfig" name="config">
    <qgadget>
     <property value="3" type="int" name="modulus"/>
    </qgadget>
   </property>
  </GraphicsItemData>
 </GraphicsItem>
 <GraphicsItemConnector fromIndex="0" toIndex="0" transportType="NumberTransporter*" fromItem="2" toItem="0"/>
 <GraphicsItemConnector fromIndex="0" toIndex="0" transportType="NumberTransporter*" fromItem="1" toItem="2"/>
</TravizProject>

```