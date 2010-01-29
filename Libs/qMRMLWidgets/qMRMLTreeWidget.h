#ifndef __qMRMLTreeWidget_h
#define __qMRMLTreeWidget_h

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <QTreeView>

#include "qMRMLWidgetsExport.h"

class qMRMLTreeWidgetPrivate;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLTreeWidget : public QTreeView
{
  Q_OBJECT
public:
  qMRMLTreeWidget(QWidget *parent=0);
  virtual ~qMRMLTreeWidget();
                           
  vtkMRMLScene* mrmlScene()const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);
 
private:
  QCTK_DECLARE_PRIVATE(qMRMLTreeWidget);
};

#endif
