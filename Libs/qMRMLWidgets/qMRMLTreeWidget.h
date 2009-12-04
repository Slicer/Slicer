#ifndef __qMRMLTreeWidget_h
#define __qMRMLTreeWidget_h

#include <QTreeView>
#include "qCTKPimpl.h"
#include "qMRMLWidgetsWin32Header.h"

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

 protected slots:
  virtual void rowsInserted ( const QModelIndex & parent, int start, int end );
  virtual void rowsRemoved ( const QModelIndex & parent, int start, int end );

private:
  QCTK_DECLARE_PRIVATE(qMRMLTreeWidget);
};

#endif
