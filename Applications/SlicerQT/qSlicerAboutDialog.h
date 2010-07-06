#ifndef __qSlicerAboutDialog_h
#define __qSlicerAboutDialog_h

// Qt includes
#include <QDialog>

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerQTExport.h"

class qSlicerAboutDialogPrivate;

class Q_SLICERQT_EXPORT qSlicerAboutDialog :
  public QDialog
{
  Q_OBJECT
public:
  qSlicerAboutDialog(QWidget *parentWidget = 0);

private:
  CTK_DECLARE_PRIVATE(qSlicerAboutDialog);
};

#endif
