#ifndef __qSlicerSaveDataDialog_p_h
#define __qSlicerSaveDataDialog_p_h

// Qt includes
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QStyledItemDelegate>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerSaveDataDialog.h"
#include "ui_qSlicerSaveDataDialog.h"

class vtkMRMLStorableNode;

//-----------------------------------------------------------------------------
class qSlicerSaveDataDialogPrivate
  : public QDialog
  , public Ui_qSlicerSaveDataDialog
{
  Q_OBJECT
public:
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
  QFileInfo sceneFile()const;

protected:
  enum ColumnType
  {
    NodeNameColumn = 0,
    NodeTypeColumn = 1,
    NodeStatusColumn = 2,
    FileFormatColumn = 3,
    FileNameColumn = 4,
    FileDirectoryColumn = 5
  };

  bool              prepareForSaving();
  void              populateScene();
  void              populateNode(vtkMRMLStorableNode* node);
  QFileInfo         nodeFileInfo(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeNameItem(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeTypeItem(vtkMRMLStorableNode* node);
  QTableWidgetItem* createNodeStatusItem(vtkMRMLStorableNode* node, const QFileInfo& fileInfo);
  QWidget*          createFileFormatsWidget(vtkMRMLStorableNode* node, const QFileInfo& fileInfo);
  QTableWidgetItem* createFileNameItem(const QFileInfo& fileInfo, const QString& extension = QString());
  QWidget*          createFileDirectoryWidget(const QFileInfo& fileInfo);
  bool              mustSceneBeSaved()const;

  vtkMRMLScene* MRMLScene;
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
  static QString fixupFileName(const QString& fileName, const QString& extension = QString());
  static QRegExp fileNameRegExp(const QString& extension = QString());
};

#endif
