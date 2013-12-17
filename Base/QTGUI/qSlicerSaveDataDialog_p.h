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
#include <QStyledItemDelegate>

// SlicerQt includes
#include "qSlicerIOOptions.h"
#include "qSlicerSaveDataDialog.h"
#include "ui_qSlicerSaveDataDialog.h"

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
  explicit qSlicerSaveDataDialogPrivate(QWidget* _parent=0);
  virtual ~qSlicerSaveDataDialogPrivate();

  void populateItems();

  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene* mrmlScene()const;

public slots:
  void setDirectory(const QString& newDirectory);
  void selectModifiedSceneData();
  void selectModifiedData();
  bool save();
  /// Reimplemented from QDialog::accept(), only accept the dialog if
  /// save() is successful.
  virtual void accept();

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
  bool              prepareForSaving();
  void              restoreAfterSaving();
  void              setSceneRootDirectory(const QString& rootDirectory);
  void              updateOptionsWidget(int row);

  QString           sceneFileFormat()const;

  void              populateScene();
  void              populateNode(vtkMRMLNode* node);

  QFileInfo         nodeFileInfo(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeNameItem(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeTypeItem(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeStatusItem(vtkMRMLStorableNode* node, const QFileInfo& fileInfo);
  QWidget*          createFileFormatsWidget(vtkMRMLStorableNode* node, QFileInfo& fileInfo);
  QTableWidgetItem* createFileNameItem(const QFileInfo& fileInfo, const QString& extension, const QString& nodeID);
  ctkDirectoryButton* createFileDirectoryWidget(const QFileInfo& fileInfo);

  static QString extractKnownExtension(const QString& fileName, vtkObject* object);
  static QString stripKnownExtension(const QString& fileName, vtkObject* object);

  QFileInfo         file(int row)const;
  vtkObject*        object(int row)const;
  QString           format(int row)const;
  QString           type(int row)const;
  qSlicerIOOptions* options(int row)const;
  vtkMRMLNode*      getNodeByID(char *id)const;

  vtkMRMLScene* MRMLScene;
  QString MRMLSceneRootDirectoryBeforeSaving;
  QString LastMRMLSceneFileFormat;

  friend class qSlicerFileNameItemDelegate;
};

//-----------------------------------------------------------------------------
class qSlicerFileNameItemDelegate : public QStyledItemDelegate
{
public:
  typedef QStyledItemDelegate Superclass;
  qSlicerFileNameItemDelegate( QObject * parent = 0 );
  virtual QWidget* createEditor( QWidget * parent,
                                 const QStyleOptionViewItem & option,
                                 const QModelIndex & index ) const;
  virtual void setModelData(QWidget *editor,
                            QAbstractItemModel *model,
                            const QModelIndex &index) const;
  static QString fixupFileName(const QString& fileName, const QString& extension,
                               vtkMRMLScene *mrmlScene, const QString &nodeID);
  /// Generate a regular expression that can ensure a filename has a valid
  /// extension.
  /// Example of supported extensions:
  /// "", "*", ".*", ".jpg", ".png" ".tar.gz"...
  /// An empty extension or "*" means any filename (or directory) is valid
  static QRegExp fileNameRegExp(const QString& extension = QString());

  vtkMRMLScene* MRMLScene;
};

#endif
