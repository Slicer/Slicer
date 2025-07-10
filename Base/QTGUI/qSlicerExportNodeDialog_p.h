#ifndef __qSlicerExportNodeDialog_p_h
#define __qSlicerExportNodeDialog_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QDialog>
#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QStackedWidget>
#include <QVBoxLayout>

// Slicer includes
#include "qSlicerExportNodeDialog.h"
#include "ui_qSlicerExportNodeDialog.h"

class vtkMRMLNode;
class vtkMRMLStorableNode;
class qSlicerFileWriterOptionsWidget;

//-----------------------------------------------------------------------------
// Helper class to represent widgets that need to be displayed separately for each node type
// To add NodeTypeWidgetSets to the dialog: use NodeTypeWidgetSets::insertWidgetsAtRow
// To remove them from the dialog: do manually using QFormLayout::takeRow, and call
// NodeTypeWidgetSets::notifyRemovedFromDialog
class NodeTypeWidgetSet : public QObject
{
  Q_OBJECT
public:
  // Node types will be represented by class name QStrings.
  using NodeType = QString;

  // Create a set of widgets that are node-type-specific. They will be parented to the given parent, which is expected
  // to be the dialog that contains the QFormLayout the widgets are going to be inserted into.
  // storableNode is a prototype node used to determine node type and used to help initialize some parameters
  NodeTypeWidgetSet(QWidget* parent, vtkMRMLStorableNode* storableNode, vtkMRMLScene* scene);

  // Set text on the QLabel member widget to show the given node type display name
  // Passing an empty string results sets appropriate label text that doesn't mention node type
  void setLabelText(QString nodeTypeDisplayName);

  // Return extension corresponding to format selected in dropdown, e.g. ".nrrd"
  QString extension() const;

  // Return the actual text of the currently selected option in the format dropdown, e.g. "NRRD (.nrrd)"
  QString formatText() const;

  // Set given options widget to be the current one for this NodeTypeWidgetSet, i.e. the one displayed in the options
  // stacked widget Passing nullptr is okay: that means there is no options widget, and it blanks out the stacked widget
  void changeCurrentOptionsWidget(qSlicerFileWriterOptionsWidget*);

  // Set the options stacked widget to a blank widget
  void makeOptionsStackedWidgetBlank();

  // Set visibility of all member widgets.
  void setMemberWidgetVisibility(bool visible);

  // Insert member widgets into the given form layout starting from the given row.
  void insertWidgetsAtRow(int row, QFormLayout* formLayout);

  // Tell the NodeTypeWidgetSet instance that its member widgets have been removed from the form layout they were in
  void notifyRemovedFromDialog();

  // Set the current options widget based on the format text in exportFormatComboBox.
  // Finds the correct options widget in the map formatToOptionsWidget, creating one if there isn't one.
  // prototypeNode must be valid to use this.
  bool updateOptionsWidget();

  // Get display name associated to the node type (e.g. if node type is "vtkMRMLScalarVolumeNode", display name is
  // "Volume") prototypeNode must be valid to use this.
  QString getTypeDisplayName() const;

  // A node used to represent its node type; helps with initializing some export dialog parameters.
  // WARNING: This pointer is not always reliable. User of this class is responsible for updating this pointer.
  vtkMRMLStorableNode* prototypeNode;

  // Set different display style and margin for the frame that contains the widgets
  enum class FrameStyle
  {
    NoFrame,
    Frame
  };
  void setFrameStyle(FrameStyle);

  // Node type that the member widgets will be configured for.
  NodeType nodeType;

  // Member widgets
  QLabel* label = nullptr;                   // The label in front of our frame in the form layout
  QFrame* frame = nullptr;                   // The widget that will go into the form layout
  QVBoxLayout* frameLayout = nullptr;        // The layout that frame will have
  QComboBox* exportFormatComboBox = nullptr; // Goes into frameLayout
  QStackedWidget* optionsStackedWidget;      // Goes into frameLayout
  qSlicerFileWriterOptionsWidget* optionsWidget =
    nullptr; // The current options widget, if any, that should be on display by optionsStackedWidget

  // Mapping from formats to options widgets, to keep track of and reuse options widgets as they get created
  QHash<QString, qSlicerFileWriterOptionsWidget*> formatToOptionsWidget;

public slots:
  void formatChangedSlot();
};

//-----------------------------------------------------------------------------
class qSlicerExportNodeDialogPrivate
  : public QDialog
  , public Ui_qSlicerExportNodeDialog
{
  Q_OBJECT
public:
  typedef qSlicerExportNodeDialogPrivate Self;
  explicit qSlicerExportNodeDialogPrivate(QWidget* _parent = nullptr);
  ~qSlicerExportNodeDialogPrivate() override;

  /* Set the nodes being exported and fill out the dialog widgets with a reasonable initial state.
  Two lists of node pointers must be given, one based on recursively including the storable node child items of the
  selected node in the subject hierarchy tree, and one based on non-recursively doing so. selectedNode is the single
  storable node that was selected in the subject hierarchy to trigger this export, if there is one; can be null if there
  isn't. nodeIdToSubjectHierarchyPath is a mapping from node IDs to lists of subject hierarchy item names, where each
  list starts from the parent of the aforementioned node ID and goes up the hierarchy until it reaches selectedNodeID;
  it can be left empty, but the "Preserve hierarchy" functionality will not work.
  The reference parameters of this function are assumed to remain valid references during execution (exec) of this
  dialog.*/
  bool setup(vtkMRMLScene* scene,
             const QList<vtkMRMLStorableNode*>& nodesNonrecursive,
             const QList<vtkMRMLStorableNode*>& nodesRecursive,
             vtkMRMLStorableNode* selectedNode,
             const QHash<QString, QVariant>& nodeIdToSubjectHierarchyPath);

  /// Remove any problem-causing characters from \a fileName, strip off any extension from the end of it if that
  /// extension is known to be associated with \a node, and then put \a extension back on
  static QString forceFileNameExtension(const QString& fileName, const QString& extension, vtkMRMLNode* node);

  /// Return a reasonable default filename choice for \a node with extension \a extension. It is based on the node name.
  static QString defaultFilename(vtkMRMLNode* node, QString extension);

  // Return whether list a has any elements that list b doesn't have.
  template <typename T>
  static bool setDifferenceIsNonempty(const QList<T>& a, const QList<T>& b);

  // Return whether all widgets that are part of the given layout are invisible
  // Visibility is considered relative to the given widget relativeTo
  static bool layoutWidgetsAllInvisible(const QLayout* layout, const QWidget* relativeTo);

public slots:
  void accept() override; // overrides QDialog::accept()

protected slots:
  void formatChangedSlot();
  void onFilenameEditingFinished();

  // This slot is meant to be connected to any checkbox state change that can affect which nodes are included for export
  void onNodeInclusionCheckboxStateChanged(int state);

  // This slot is meant to be connected to "Include children" checkbox state change specifically
  void onIncludeChildrenCheckBoxStateChanged(int state);

protected:
  // Exports nodes based on parameters set in dialog. To be called when the dialog is accepted.
  bool exportNodes();

  // Save the states of some widgets so that they can be restored next time the dialog is activated.
  void saveWidgetStates();

  // Clear any node-type-specific widgets out of the form layout and then repopulate the form layout
  // with widgets that are selected based on the current state of this->nodeList()
  bool populateNodeTypeWidgetSets();

  // Update the visibility and state of the harden transform checkbox based on whether the list of
  // nodes to be exported contains any nodes that have an associated transform
  void updateHardenTransformCheckBox();

  // Update the visibility and state of the preserve hierarchy checkbox based on whether the selected item
  // in the subject hierarchy has children and whether the "Include children" option is checked
  void updatePreserveHierarchyCheckBox();

  // Update the tabbing order to a top-to-bottom order, which can get messed up as we insert and remove widgets.
  // It would normally be better to fix tabbing order in the .ui file, but in this case rows of the layout are
  // dynamically added and removed, and so we set the tabbing order in code instead.
  void adjustTabbingOrder();

  // Return the recommended filename based on current filename and the
  // format selected in the dropdown
  QString recommendedFilename(vtkMRMLStorableNode*) const;

  // Get a path to a suitable target directory for the given node if we wanted to replicate the folder structure of the
  // subject hierarchy. The returned path may or may not exist. In case of any failure, this function logs a warning and
  // falls back to simply returning the top level export target directory, so as to not fail the export completely.
  QDir getSubjectHierarchyBasedDirectory(vtkMRMLStorableNode*) const;

  // Return the list of nodes to be exported, based on the status of RecursiveChildrenCheckBox
  const QList<vtkMRMLStorableNode*>& nodeList() const;

  // Get an entry from the hash map nodeTypeToNodeTypeWidgetSet, but make sure the key
  // is in the table, returning nullptr if not. Enabling logError prints a message into Qt message handling.
  NodeTypeWidgetSet* getNodeTypeWidgetSetSafe(NodeTypeWidgetSet::NodeType nodeType, bool logError = false) const;

  // Validate that there is exactly one node to export, and get that node
  // error if validation fails, returning null result; use this only when it is guaranteed to succeed
  vtkMRMLStorableNode* theOnlyNode() const;

  // Validate that there is exactly one NodeTypeWidgetSet, get it
  // error if validation fails, returning null result; use this only when it is guaranteed to succeed
  NodeTypeWidgetSet* theOnlyNodeTypeWidgetSet() const;

  // Validate that there is exactly one NodeTypeWidgetSet, and get its export format dropdown widget
  // error if validation fails, returning null result; use this only when it is guaranteed to succeed
  QComboBox* theOnlyExportFormatComboBox() const;

  vtkMRMLScene* MRMLScene;

  // The list of nodes to export in each of the two cases: recursively including children of the selected item or not
  QList<vtkMRMLStorableNode*> NodesRecursive;
  QList<vtkMRMLStorableNode*> NodesNonrecursive;

  // Mapping from node IDs to lists of subject hierarchy item names, where
  // each list starts from the parent of the aforementioned node ID and goes up the hierarchy until it reaches
  // selectedNodeID.
  QHash<QString, QVariant> NodeIdToSubjectHierarchyPath;

  // The storable node that was selected in the subject hierarchy to trigger this export, if there is one;
  // can be empty if there isn't. It is conveniently a list so that a uniform interface can be provided by nodeList().
  QList<vtkMRMLStorableNode*> NodesSelectedOnly;

  QString LastUsedDirectory;
  bool LastUsedHardenTransform;
  bool LastUsedPreserveHierarchy;
  bool LastUsedRecursiveChildren;
  bool LastUsedIncludeChildren;

  // Mapping from node type to widget sets that are specific to a node type
  QHash<NodeTypeWidgetSet::NodeType, NodeTypeWidgetSet*> NodeTypeToNodeTypeWidgetSet;

  // The row of the QFormLayout that contains the placeholder label. This is the row where NodeTypeWidgetSets will be
  // inserted.
  int NodeTypeWidgetSetStartRow = -1;

  // List of node types (in the sense of NodeTypeWidgetSet::NodeType) that correspond to node-type-specific widget sets
  // that are currently visible in the dialog. The i^th item in this list has widgets that are meant to go into
  // row number (nodeTypeWidgetSetStartRow + i) of the QFormLayout associated to this dialog. This dialog class manages
  // the insertion and removal of the rows, and while doing so it keeps the following list up to date.
  QList<NodeTypeWidgetSet::NodeType> NodeTypesInDialog;

  // Used to prevent a signal feedback loop coming from the fact that
  // modifying file name can change export format and vice versa.
  bool ProtectFilenameLineEdit;
};

#endif
