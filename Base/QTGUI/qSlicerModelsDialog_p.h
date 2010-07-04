#ifndef __qSlicerModelsDialog_p_h
#define __qSlicerModelsDialog_p_h

// Qt includes
#include <QDialog>
#include <QStringList>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerModelsDialog.h"
#include "ui_qSlicerModelsDialog.h"
#include "qSlicerBaseQTGUIExport.h"

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModelsDialogPrivate
  : public QDialog
  , public ctkPrivate<qSlicerModelsDialog>
  , public Ui_qSlicerModelsDialog
{
  Q_OBJECT
  CTK_DECLARE_PUBLIC(qSlicerModelsDialog);
public:
  explicit qSlicerModelsDialogPrivate(QWidget* parentWidget = 0);
  virtual ~qSlicerModelsDialogPrivate();

  void init();
  QStringList selectedFiles()const;

public slots:
  void openAddModelFileDialog();
  void openAddModelDirectoryDialog();

protected:
  QStringList SelectedFiles;
};

#endif
