#ifndef __qSlicerActionsDialog_h
#define __qSlicerActionsDialog_h

// Qt includes
#include <QDialog>

// SlicerQt includes
#include "ui_qSlicerActionsDialog.h"
#include "qSlicerBaseQTGUIExport.h"

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerActionsDialog
  : public QDialog, protected Ui_qSlicerActionsDialog
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  qSlicerActionsDialog(QWidget* parentWidget =0);
  virtual ~qSlicerActionsDialog();

  void addAction(QAction* action, const QString& group=QString());
  void addActions(const QList<QAction*>& actions, const QString& group=QString());
  void clear();

  void setActionsWithNoShortcutVisible(bool visible);
  void setMenuActionsVisible(bool visible);
};

#endif
