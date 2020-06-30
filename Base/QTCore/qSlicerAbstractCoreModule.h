/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerAbstractCoreModule_h
#define __qSlicerAbstractCoreModule_h

// Qt includes
#include <QObject>
#include <QStringList>

// CTK includes
#include <ctkPimpl.h>

// QTBase includes
#include "qSlicerBaseQTCoreExport.h"

class qSlicerAbstractModuleRepresentation;
class vtkMRMLAbstractLogic;
class vtkSlicerApplicationLogic;
class vtkMRMLScene;
class qSlicerAbstractCoreModulePrivate;


#define qSlicerGetTitleMacro(_TITLE)               \
  static QString staticTitle() { return _TITLE; }  \
  QString title() const override { return _TITLE; }

/// \brief qSlicerAbstractCoreModule is the base class of any module in Slicer.
//
/// Core modules, Loadable modules, CLI modules derive from it.
/// It is responsible to create the UI and the Logic:
/// createWidgetRepresentation() and createLogic() must be reimplemented in
/// derived classes.
/// A Slicer module has a name and a title: The name is its UID, the title
/// displayed to the user.
/// When a MRML scene is set to the module, the module set the scene to the
/// UI widget and the logic.
class Q_SLICER_BASE_QTCORE_EXPORT qSlicerAbstractCoreModule : public QObject
{
  /// Any object deriving from QObject must have the Q_OBJECT macro in
  /// order to have the signal/slots working and the meta-class name valid.
  Q_OBJECT

  /// The following properties are added to the meta-class
  /// and though are available through PythonQt.

  /// This property contains the name of the module.
  /// e.g. "Volumes", "VolumeRendering", "SampleData"...
  /// The name identifies a module and must be unique.
  /// In comparison to \a title, \a name contains only letter characters,
  /// no space/hyphen/apostrophe.
  /// The module name is set by the module factory (the registered item key
  /// string).
  /// Because \a name is unique, slots and functions that take a module as
  /// argument should use \name and not \a title.
  Q_PROPERTY(QString name READ name)

  /// This property contains the title of the module.
  /// e.g. "Volumes", "Volume Rendering", "Welcome to Slicer"...
  /// \a title (not \a name) is displayed to the user in the GUI (but internally
  /// \a name is used to uniquely identify the module).
  /// The module title can contain any characters.
  /// \a title() must be overwritten for each module.
  Q_PROPERTY(QString title READ title)

  /// This property holds the categories a module belongs to.
  /// It is used to organize modules in the modules menu.
  /// Sub-categories are supported by using '.' as a a separator to specify a
  /// subcategory (no depth limit), e.g.: "Filtering.Arithmetic".
  /// A category doesn't have to exist, it will be created if needed.
  /// The method \a categories() must be reimplemented for each module.
  /// An empty category means the module shows up at the top level. It is
  /// possible to define more than 1 category, the module will then be visible
  /// in all the listed categories
  Q_PROPERTY(QStringList categories READ categories)

  /// This property controls the index used to sort modules in the module
  ///  selector's menu in the module's category.
  /// An index of 0 means the module should be first in the category,
  /// a value of 1 if the module should be second and so on.
  /// -1 means that the module should be added at the end. In case modules
  /// indexes have the same index, the alphabetical order will be used.
  /// -1 by default.
  Q_PROPERTY(int index READ index)

  /// This property holds whether the module is visible to the user.
  /// If the module is hidden, it doesn't appear in the list of modules menu.
  /// However, the module is programmatically accessible.
  /// By default, modules are visible (hidden == false).
  /// \sa isHidden
  Q_PROPERTY(bool hidden READ isHidden)

  /// This property holds whether the module should be able to create new
  /// widget representations or not.
  /// By default, modules can create new widget representations.
  /// \sa isWidgetRepresentationCreationEnabled
  /// \sa setWidgetRepresentationCreationEnabled
  Q_PROPERTY(bool widgetRepresentationCreationEnabled
             READ isWidgetRepresentationCreationEnabled
             WRITE setWidgetRepresentationCreationEnabled)

  /// This property holds the help of the module.
  /// The help is displayed inside the module as a tab.
  /// \a helpText must be reimplemented for each module.
  /// \sa acknowledgement, contributors
  Q_PROPERTY(QString helpText READ helpText)

  /// This property holds the acknowledgments for the module
  /// Credits (organization, grants, thanks...) can be given and are displayed
  /// in the acknowledgment tab in the module panel. The contributors of the module
  /// will automatically be appended when needed, don't duplicate the
  /// information in the acknowledgements.
  /// \a acknowledgementText() must be reimplemented for each module.
  /// \sa helpText, contributors
  Q_PROPERTY(QString acknowledgementText READ acknowledgementText)

  /// This property holds the authors of the module
  /// It is shown in the Acknowledgement page.
  /// If there are multiple contributors, they must be listed in the order
  /// of contribution. For each contributor, the name of the institution
  /// can succeed the author: e.g. John Doe, CompanyName Inc.
  /// \a contributors() must be reimplemented for each module.
  /// \sa helpText, acknowledgementText
  Q_PROPERTY(QStringList contributors READ contributors)

  /// This property holds the URL of the module for the Slicer wiki.
  /// It can be used in the help/acknowledgement.
  /// This method is being phased out, as module documentations is moving to ReadTheDocs.
  /// Modules that already documented on ReadTheDocs can use defaultDocumentationLink property instead.
  Q_PROPERTY(QString slicerWikiUrl READ slicerWikiUrl)

  /// Auto-generated string that contains a paragraph that links to the
  /// default module documentation location on ReadTheDocs.
  /// It can be used in the help/acknowledgement.
  Q_PROPERTY(QString defaultDocumentationLink READ defaultDocumentationLink);

  /// This property holds the module name list of the module dependencies.
  /// It is used to order the loading of the modules. When setup() is called
  /// it is ensured that all the dependencies have already been setup.
  /// There is no dependency cycle check, so special care must be taken to
  /// avoid infinite loop.
  /// By default, there is no dependencies.
  Q_PROPERTY(QStringList dependencies READ dependencies)

  /// This property holds the path of the module if any.
  /// The path is the library (dll, so...) location on disk.
  /// \a path is set by the module factory and shouldn't be reimplemented
  /// in each module.
  /// \todo Ideally this function should be added within the
  /// qSlicerLoadableModule.
  Q_PROPERTY(QString path READ path)

  /// This property holds whether the module is loaded from an installed
  /// directory.
  /// \a isInstalled is set by the module factory and shouldn't be
  /// reimplemented in each module.
  Q_PROPERTY(bool isInstalled READ isInstalled)

  /// This property holds whether module is a built-in Slicer module or
  /// one from an extension or any user-specified folder.
  /// \a isBuiltIn is set by the module factory and shouldn't be
  /// reimplemented in each module.
  Q_PROPERTY(bool isBuiltIn READ isBuiltIn)

  /// This property specifies node classes this module is associated with.
  /// All children node classes are also associated with the module.
  /// Currently association is only used for editing node properties.
  /// If a node class is specified in this property and setEditedNode() and
  /// nodeEditable() methods are implemented in the module's widget then
  /// "Edit properties" function in node selector widgets will activate this
  /// module and select the chosen node.
  /// Any module can associate nodes with any module, by calling
  /// qSlicerApplication::application()->registerNodeModule() method.
  Q_PROPERTY(QStringList associatedNodeTypes READ associatedNodeTypes)

public:

  typedef QObject Superclass;
  /// Constructor
  /// Warning: If there is no parent given, make sure you delete the object.
  /// The modules can typically be instantiated before the application
  /// is initialized (module manager, iomanager...). Most of the
  /// initialization must be done in qSlicerAbstractCoreModule::setup()
  qSlicerAbstractCoreModule(QObject *parent=nullptr);
  ~qSlicerAbstractCoreModule() override;

  virtual void printAdditionalInfo();

  /// Convenience method to return slicer wiki URL
  QString slicerWikiUrl()const{ return "http://www.slicer.org/slicerWiki/index.php"; }

  /// Convenience method that returns a string that can be inserted into the application help text that contains
  /// link to the module's documentation in current Slicer version's documentation on ReadTheDocs.
  /// The text is "For more information, see the online documentation."
  /// and it points to https://slicer.readthedocs.io/en/(version)/user_guide/modules/(modulename).html
  QString defaultDocumentationLink()const;

  /// Initialize the module, an appLogic must be given to
  /// initialize the module
  void initialize(vtkSlicerApplicationLogic* appLogic);

  /// Set/Get the name of the module. The name is used to uniquely describe
  /// a module: name must be unique.
  /// The name is set by the module factory (the registered item key string).
  virtual QString name()const;
  virtual void setName(const QString& name);

  /// Title of the module, (displayed to the user)
  /// \a title() must be reimplemented in derived classes.
  virtual QString title()const = 0;

  /// Categories the module belongs to. Categories support subcategories. Use the
  /// '.' separator to specify a subcategory (no depth limit), e.g.:
  /// "Filtering.Arithmetic".
  /// The function must be reimplemented in derived classes.
  /// Note: If a category doesn't exist, it will be created.
  virtual QStringList categories()const;

  /// Return the category index of the module.
  virtual int index()const;

  /// Returns \a true if the module should be hidden to the user.
  /// By default, interactive modules are visible and non-interactive
  /// modules are hidden.
  /// \sa hidden, isWidgetRepresentationCreationEnabled
  virtual bool isHidden()const;

  /// Return the contributors of the module
  virtual QStringList contributors()const;

  /// Return help text of the module
  /// Must be reimplemented in the derived classes
  virtual QString helpText()const;

  /// Return acknowledgement text for the module
  /// Must be reimplemented in the derived classes
  virtual QString acknowledgementText()const;

  /// Set/Get if the module should be able to create new widget
  /// representations or not.
  /// \sa widgetRepresentation()
  bool isWidgetRepresentationCreationEnabled()const;
  void setWidgetRepresentationCreationEnabled(bool value);

  /// This method allows to get a pointer to the WidgetRepresentation.
  /// If no WidgetRepresentation already exists, one will be created calling
  /// 'createWidgetRepresentation' method.
  /// \sa createNewWidgetRepresentation(), createWidgetRepresentation()
  qSlicerAbstractModuleRepresentation* widgetRepresentation();

  /// Force the creation of a new widget representation.
  /// It does not return the widget of the module, but a new instance instead.
  /// It can be useful when embedding a module widget into another module.
  /// \sa widgetRepresentation(), createWidgetRepresentation()
  /// \sa isWidgetRepresentationCreationEnabled()
  qSlicerAbstractModuleRepresentation* createNewWidgetRepresentation();

  /// Get/Set the application logic.
  /// It must be set.
  void setAppLogic(vtkSlicerApplicationLogic* appLogic);
  vtkSlicerApplicationLogic* appLogic() const;

  /// This method allows to get a pointer to the module logic.
  /// If no moduleLogic already exists, one will be created calling
  /// the 'createLogic' method.
  /// A module logic is typically a vtkSlicerModuleLogic but not necessarily.
  Q_INVOKABLE vtkMRMLAbstractLogic* logic();

  /// Return a pointer on the MRML scene
  Q_INVOKABLE vtkMRMLScene* mrmlScene() const;

  virtual QStringList dependencies()const;

  /// Returns path if any
  /// \todo Ideally this function should be added within the qSlicerLoadableModule.
  QString path()const;
  void setPath(const QString& newPath);

  /// Return true is this instance of the module is loaded from an installed directory
  /// \todo Ideally this function should be added within the qSlicerLoadableModule.
  bool isInstalled()const;
  void setInstalled(bool value);

  /// Determine if this module is a built-in Slicer module or one from an extension
  /// or any user-specified folder.
  bool isBuiltIn()const;
  void setBuiltIn(bool value);

  /// Return node types associated with this module (e.g., node types this module can edit)
  virtual QStringList associatedNodeTypes()const;

public slots:

  /// Set the current MRML scene to the module, it is propagated to the logic
  /// and representations if any
  virtual void setMRMLScene(vtkMRMLScene*);

protected:
  /// All initialization code should be done in the setup
  virtual void setup() = 0;

  /// Create and return a widget representation for the module.
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation() = 0;

  /// Create and return the module logic
  /// Note: Only one instance of the logic will exist per module
  virtual vtkMRMLAbstractLogic* createLogic() = 0;

protected:
  QScopedPointer<qSlicerAbstractCoreModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAbstractCoreModule);
  Q_DISABLE_COPY(qSlicerAbstractCoreModule);
  friend class qSlicerAbstractModuleRepresentation;

  /// Internal method called by the destructor of qSlicerAbstractModuleRepresentation
  /// to remove the representation from the list.
  void representationDeleted(qSlicerAbstractModuleRepresentation *representation);
};

#endif
