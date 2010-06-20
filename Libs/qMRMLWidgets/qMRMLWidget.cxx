



// qMRML includes
#include "qMRMLWidget.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include "vtkSmartPointer.h"

//-----------------------------------------------------------------------------
class qMRMLWidgetPrivate: public ctkPrivate<qMRMLWidget>
{
public:
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
};

//-----------------------------------------------------------------------------
// qMRMLWidget methods

//-----------------------------------------------------------------------------
qMRMLWidget::qMRMLWidget(QWidget * _parent, Qt::WindowFlags f):Superclass(_parent, f)
{
  CTK_INIT_PRIVATE(qMRMLWidget);
}

//-----------------------------------------------------------------------------
qMRMLWidget::~qMRMLWidget()
{
}

//-----------------------------------------------------------------------------
void qMRMLWidget::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_D(qMRMLWidget);
  if (scene == d->MRMLScene)
    {
    return ;
    }
  d->MRMLScene = scene;
  emit mrmlSceneChanged(scene);
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qMRMLWidget, vtkMRMLScene*, mrmlScene, MRMLScene);
