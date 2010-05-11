#ifndef __qSlicerSaveDataDialog_p_h
#define __qSlicerSaveDataDialog_p_h

// Qt includes
#include <QDialog>
#include <QDir>
#include <QFileInfo>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerSaveDataDialog.h"
#include "ui_qSlicerSaveDataDialog.h"
#include "qSlicerBaseQTGUIExport.h"

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSaveDataDialogPrivate
  : public QDialog
  , public ctkPrivate<qSlicerSaveDataDialog>
  , public Ui_qSlicerSaveDataDialog
{
  Q_OBJECT
  CTK_DECLARE_PUBLIC(qSlicerSaveDataDialog);
public:
  explicit qSlicerSaveDataDialogPrivate(QWidget* _parent=0);
  virtual ~qSlicerSaveDataDialogPrivate();
                                     
  QList<qSlicerIO::IOProperties> selectedFiles();
  void populateItems(vtkMRMLScene* scene);

public slots:
  void setDirectory(const QString& newDirectory);
  void selectModifiedSceneData();
  void selectModifiedData();
  
protected slots:
  void formatChanged();

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
};

#endif
