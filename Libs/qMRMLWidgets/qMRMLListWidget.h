#ifndef __qMRMLListWidget_h
#define __qMRMLListWidget_h

// Qt includes
#include <QListView>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class qMRMLListWidgetPrivate;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLListWidget : public QListView
{
  Q_OBJECT
public:
  qMRMLListWidget(QWidget *parent=0);
  virtual ~qMRMLListWidget();
                           
  vtkMRMLScene* mrmlScene()const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);

private:
  CTK_DECLARE_PRIVATE(qMRMLListWidget);
};

#endif
