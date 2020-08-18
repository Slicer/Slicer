#ifndef __qSlicerSaveDataDialog_p_h
#define __qSlicerSaveDataDialog_p_h

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
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStyledItemDelegate>

// Slicer includes
#include "qSlicerIOOptions.h"
#include "qSlicerSaveDataDialog.h"
#include "ui_qSlicerSaveDataDialog.h"

class ctkPathLineEdit;
class vtkMRMLNode;
class vtkMRMLStorableNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qSlicerSaveDataDialogPrivate
  : public QDialog
  , public Ui_qSlicerSaveDataDialog
{
  Q_OBJECT
public:
  typedef qSlicerSaveDataDialogPrivate Self;
  explicit qSlicerSaveDataDialogPrivate(QWidget* _parent=nullptr);
  ~qSlicerSaveDataDialogPrivate() override;

  void populateItems();

  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene* mrmlScene()const;

  /// Helper function for finding a node in the main scene and all scene view scenes.
  /// This method differs from vtkMRMLScene::GetNodeByID in that this method looks for
  /// node IDs in the internal scene view scenes as well.
  static vtkMRMLNode* getNodeByID(char *id, vtkMRMLScene* scene);

  void formatChanged(int row);

public slots:
  void setDirectory(const QString& newDirectory);
  void selectModifiedSceneData();
  void selectModifiedData();
  bool save();
  /// Reimplemented from QDialog::accept(), only accept the dialog if
  /// save() is successful.
  void accept() override;

protected slots:
  void formatChanged();
  bool saveScene();
  bool saveNodes();
  QFileInfo sceneFile()const; // ### Slicer 4.4: Move as protected
  void showMoreColumns(bool);
  void updateSize();
  void onSceneFormatChanged();
  void enableNodes(bool);
  void saveSceneAsDataBundle();
  void onItemChanged(QTableWidgetItem*);

protected:
  enum ColumnType
  {
    SelectColumn = 0,
    FileNameColumn = 0,
    FileFormatColumn = 1,
    FileDirectoryColumn = 2,
    OptionsColumn = 3,
    NodeNameColumn = 4,
    NodeTypeColumn = 5,
    NodeStatusColumn = 6
  };

  enum CustomRole
  {
    SceneTypeRole = Qt::UserRole,
    FileExtensionRole,
    UIDRole
  };

  int               findSceneRow()const;
  bool              mustSceneBeSaved()const;
  void              setSceneRootDirectory(const QString& rootDirectory);
  void              updateOptionsWidget(int row);
  void              updateStatusIconFromStorageNode(int row);
  void              setStatusIcon(int row, const QIcon& icon, const QString& message);

  QString           sceneFileFormat()const;

  void              populateScene();
  void              populateNode(vtkMRMLNode* node);

  QFileInfo         nodeFileInfo(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeNameItem(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeTypeItem(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeStatusItem(vtkMRMLStorableNode* node, const QFileInfo& fileInfo);
  QWidget*          createFileFormatsWidget(vtkMRMLStorableNode* node, QFileInfo& fileInfo);
  QTableWidgetItem* createFileNameItem(const QFileInfo& fileInfo, const QString& extension, const QString& nodeID);
  ctkPathLineEdit*  createFileDirectoryWidget(const QFileInfo& fileInfo);
  void              clearUserMessagesInStorageNodes();

  static QString extractKnownExtension(const QString& fileName, vtkObject* object);
  static QString stripKnownExtension(const QString& fileName, vtkObject* object);

  QFileInfo         file(int row)const;
  vtkObject*        object(int row)const;
  QString           format(int row)const;
  QString           type(int row)const;
  qSlicerIOOptions* options(int row)const;

  bool confirmOverwrite(const QString& filepath);

  /// Helper function for finding a node in the main scene and all scene view scenes
  vtkMRMLNode*      getNodeByID(char *id)const;

  vtkMRMLScene* MRMLScene;
  QString MRMLSceneRootDirectoryBeforeSaving;

  // Items are currently being added to the scene, indicates that no GUI updates should be performed.
  bool PopulatingItems;

  QMessageBox::StandardButton ConfirmOverwriteAnswer;
  bool CancelRequested;
  QIcon WarningIcon;
  QIcon ErrorIcon;

  friend class qSlicerFileNameItemDelegate;
};

//-----------------------------------------------------------------------------
class qSlicerFileNameItemDelegate : public QStyledItemDelegate
{
public:
  typedef QStyledItemDelegate Superclass;
  qSlicerFileNameItemDelegate( QObject * parent = nullptr );
  static QString forceFileNameExtension(const QString& fileName, const QString& extension,
                               vtkMRMLScene *mrmlScene, const QString &nodeID);
  static QString forceFileNameValidCharacters(const QString& filename);

  /// Generate a regular expression that can ensure a filename has a valid
  /// extension.
  /// Example of supported extensions:
  /// "", "*", ".*", ".jpg", ".png" ".tar.gz"...
  /// An empty extension or "*" means any filename (or directory) is valid
  static QRegExp fileNameRegExp(const QString& extension = QString());

  vtkMRMLScene* MRMLScene;
};

#endif
