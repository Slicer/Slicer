#include "qMRMLNodeFactoryButton.h"
#include "qMRMLNodeFactory.h"

#include <vtkMRMLScene.h>
#include <QDebug>

//-----------------------------------------------------------------------------
class qMRMLNodeFactoryButtonPrivate: public qCTKPrivate<qMRMLNodeFactoryButton>,
                                     public qMRMLNodeFactory
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLNodeFactoryButton);
  qMRMLNodeFactoryButtonPrivate()
    :qMRMLNodeFactory(0)
    {
    }
  void init();
  void syncTextWithAction();
  void updateEnabled();
  
  qMRMLNodeFactoryButton::ActionType Action;
};

// --------------------------------------------------------------------------
void qMRMLNodeFactoryButtonPrivate::init()
{
  QCTK_P(qMRMLNodeFactoryButton);
  this->Action = qMRMLNodeFactoryButton::GenerateRandom;
  this->syncTextWithAction(); 
  
  this->connect(p, SIGNAL(clicked()), p, SLOT(onClick()));
  this->updateEnabled();
}

// --------------------------------------------------------------------------
void qMRMLNodeFactoryButtonPrivate::syncTextWithAction()
{
  QCTK_P(qMRMLNodeFactoryButton);
  switch(this->Action)
    {
    case qMRMLNodeFactoryButton::GenerateRandom:
      p->setText("Generate random node");
      break;
    case qMRMLNodeFactoryButton::DeleteRandom:
      p->setText("Delete random node");
      break;
    default:
      break; 
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeFactoryButtonPrivate::updateEnabled()
{
  QCTK_P(qMRMLNodeFactoryButton);
  bool enabled = this->mrmlScene();
  if (enabled && this->Action == qMRMLNodeFactoryButton::DeleteRandom)
    {
    enabled = this->mrmlScene()->GetNumberOfNodes() > 0;
    }
  p->setEnabled(enabled);
}

// --------------------------------------------------------------------------
// qMRMLNodeFactoryButton methods
// --------------------------------------------------------------------------
qMRMLNodeFactoryButton::qMRMLNodeFactoryButton(QWidget* _parent):QPushButton(_parent)
{
  QCTK_INIT_PRIVATE(qMRMLNodeFactoryButton);
  qctk_d()->init();
}

// --------------------------------------------------------------------------
void qMRMLNodeFactoryButton::setMRMLScene(vtkMRMLScene* _mrmlScene)
{
  QCTK_D(qMRMLNodeFactoryButton);
  this->qvtkReconnect(d->mrmlScene(), _mrmlScene, vtkMRMLScene::NodeAddedEvent,
                      this, SLOT(onMRMLSceneChanged()));
  this->qvtkReconnect(d->mrmlScene(), _mrmlScene, vtkMRMLScene::NodeRemovedEvent,
                      this, SLOT(onMRMLSceneChanged()));
  d->setMRMLScene(_mrmlScene);
  d->updateEnabled();
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLNodeFactoryButton::mrmlScene()const
{
  QCTK_D(const qMRMLNodeFactoryButton);
  return d->mrmlScene();
}

// --------------------------------------------------------------------------
void qMRMLNodeFactoryButton::onClick()
{
  QCTK_D(qMRMLNodeFactoryButton);
  switch(d->Action)
    {
    case GenerateRandom:
      this->generateRandomNode();
      break;
    case DeleteRandom:
      this->deleteRandomNode();
      break;
    default:
      break;
    }
}

// --------------------------------------------------------------------------
void qMRMLNodeFactoryButton::setAction(ActionType _action)
{
  QCTK_D(qMRMLNodeFactoryButton);
  d->Action = _action;
  d->syncTextWithAction();
  d->updateEnabled();
}

// --------------------------------------------------------------------------
qMRMLNodeFactoryButton::ActionType qMRMLNodeFactoryButton::action()const
{
  QCTK_D(const qMRMLNodeFactoryButton);
  return d->Action;
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeFactoryButton::generateRandomNode()
{
  QCTK_D(qMRMLNodeFactoryButton);
  int numClasses = d->mrmlScene()->GetNumberOfRegisteredNodeClasses();
  int classNumber = rand() % numClasses; 
  vtkMRMLNode* node = d->mrmlScene()->GetNthRegisteredNodeClass(classNumber);
  QString classname = QLatin1String(node->GetClassName()); 
  if (classname.isEmpty())
    {
    qWarning() << "Class registered (#" << classNumber << "):"
               << node << " has an empty classname";
    }
 
  return d->createNode(node->GetClassName());
}

// --------------------------------------------------------------------------
void qMRMLNodeFactoryButton::deleteRandomNode()
{
  QCTK_D(qMRMLNodeFactoryButton);
  int numNodes= d->mrmlScene()->GetNumberOfNodes();
  if (numNodes <= 0)
    {
    return;
    }
  vtkMRMLNode* node = d->mrmlScene()->GetNthNode(rand() % numNodes); 
  d->mrmlScene()->RemoveNode(node);
}

// --------------------------------------------------------------------------
void qMRMLNodeFactoryButton::onMRMLSceneChanged()
{
  qctk_d()->updateEnabled();
}
