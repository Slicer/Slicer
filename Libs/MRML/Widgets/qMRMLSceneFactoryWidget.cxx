/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QRandomGenerator>

// qMRML includes
#include "qMRMLSceneFactoryWidget.h"
#include "qMRMLNodeFactory.h"
#include "ui_qMRMLSceneFactoryWidget.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

class qMRMLSceneFactoryWidgetPrivate: public Ui_qMRMLSceneFactoryWidget
{
  Q_DECLARE_PUBLIC(qMRMLSceneFactoryWidget);
protected:
  qMRMLSceneFactoryWidget* const q_ptr;
public:
  qMRMLSceneFactoryWidgetPrivate(qMRMLSceneFactoryWidget& object);
  void init();
  void setNodeActionsEnabled(bool enable);

  vtkMRMLScene*  MRMLScene;
  QRandomGenerator RandomGenerator;
};

// --------------------------------------------------------------------------
qMRMLSceneFactoryWidgetPrivate::qMRMLSceneFactoryWidgetPrivate(qMRMLSceneFactoryWidget& object)
  : q_ptr(&object)
{
  this->MRMLScene = nullptr;
  // RandomGenerator is not seeded with random number to make behavior reproducible
}

// --------------------------------------------------------------------------
void qMRMLSceneFactoryWidgetPrivate::init()
{
  Q_Q(qMRMLSceneFactoryWidget);
  this->setupUi(q);
  QObject::connect(this->NewSceneButton, SIGNAL(clicked()), q, SLOT(generateScene()));
  QObject::connect(this->DeleteSceneButton, SIGNAL(clicked()), q, SLOT(deleteScene()));
  QObject::connect(this->NewNodeButton, SIGNAL(clicked()), q, SLOT(generateNode()));
  QObject::connect(this->DeleteNodeButton, SIGNAL(clicked()), q, SLOT(deleteNode()));
}

// --------------------------------------------------------------------------
void qMRMLSceneFactoryWidgetPrivate::setNodeActionsEnabled(bool enable)
{
  this->NewNodeButton->setEnabled(enable);
  this->NewNodeLineEdit->setEnabled(enable);
  this->DeleteNodeButton->setEnabled(enable);
  this->DeleteNodeLineEdit->setEnabled(enable);
}

// --------------------------------------------------------------------------
// qMRMLSceneFactoryWidget methods

// --------------------------------------------------------------------------
qMRMLSceneFactoryWidget::qMRMLSceneFactoryWidget(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLSceneFactoryWidgetPrivate(*this))
{
  Q_D(qMRMLSceneFactoryWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLSceneFactoryWidget::~qMRMLSceneFactoryWidget()
{
  this->deleteScene();
}

// --------------------------------------------------------------------------
void qMRMLSceneFactoryWidget::generateScene()
{
  Q_D(qMRMLSceneFactoryWidget);

  if (d->MRMLScene)
    {
    d->MRMLScene->Delete();
    }
  d->MRMLScene = vtkMRMLScene::New();
  d->setNodeActionsEnabled(true);
  d->DeleteSceneButton->setEnabled(true);
  emit mrmlSceneChanged(d->MRMLScene);
}

// --------------------------------------------------------------------------
void qMRMLSceneFactoryWidget::deleteScene()
{
  Q_D(qMRMLSceneFactoryWidget);
  if (!d->MRMLScene)
    {
    return;
    }
  d->setNodeActionsEnabled(false);
  d->DeleteSceneButton->setEnabled(false);
  emit this->mrmlSceneChanged(nullptr);
  d->MRMLScene->Delete();
  d->MRMLScene = nullptr;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLSceneFactoryWidget::mrmlScene()const
{
  Q_D(const qMRMLSceneFactoryWidget);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneFactoryWidget::generateNode()
{
  Q_D(qMRMLSceneFactoryWidget);
  Q_ASSERT(d->MRMLScene != nullptr);
  QString nodeClassName = d->NewNodeLineEdit->text();
  if (nodeClassName.isEmpty())
    {
    int numClasses = d->MRMLScene->GetNumberOfRegisteredNodeClasses();
    int classNumber = d->RandomGenerator.generate() % numClasses;
    vtkMRMLNode* node = d->MRMLScene->GetNthRegisteredNodeClass(classNumber);
    Q_ASSERT(node);
    while (node->GetSingletonTag())
      {
      classNumber = d->RandomGenerator.generate() % numClasses;
      node = d->MRMLScene->GetNthRegisteredNodeClass(classNumber);
      Q_ASSERT(node);
      }
    nodeClassName = QString::fromUtf8(node->GetClassName());
    if (nodeClassName.isEmpty())
      {
      qWarning() << "Class registered (#" << classNumber << "):"
                 << node << " has an empty classname";
      }
    }
  return this->generateNode(nodeClassName);
}

// --------------------------------------------------------------------------
vtkMRMLNode* qMRMLSceneFactoryWidget::generateNode(const QString& className)
{
  Q_D(qMRMLSceneFactoryWidget);
  Q_ASSERT(!className.isEmpty());
  Q_ASSERT(d->MRMLScene != nullptr);
  vtkMRMLNode* node = qMRMLNodeFactory::createNode(d->MRMLScene, className);
  Q_ASSERT(node);
  if (node)
    {
    emit mrmlNodeAdded(node);
    }
  return node;
}

// --------------------------------------------------------------------------
void qMRMLSceneFactoryWidget::deleteNode()
{
  Q_D(qMRMLSceneFactoryWidget);
  Q_ASSERT(d->MRMLScene != nullptr);
  QString nodeClassName = d->DeleteNodeLineEdit->text();
  if (!nodeClassName.isEmpty())
    {
    this->deleteNode(nodeClassName);
    return;
    }
  int numNodes = d->MRMLScene->GetNumberOfNodes();
  if (numNodes == 0)
    {
    return;
    }
  vtkMRMLNode* node = d->MRMLScene->GetNthNode(d->RandomGenerator.generate() % numNodes);
  d->MRMLScene->RemoveNode(node);
  // FIXME: disable delete button when there is no more nodes in the scene to delete
  emit mrmlNodeRemoved(node);
}

// --------------------------------------------------------------------------
void qMRMLSceneFactoryWidget::deleteNode(const QString& className)
{
  Q_D(qMRMLSceneFactoryWidget);
  Q_ASSERT(!className.isEmpty());
  Q_ASSERT(d->MRMLScene != nullptr);
  int numNodes = d->MRMLScene->GetNumberOfNodesByClass(className.toUtf8());
  if (numNodes == 0)
    {
    qDebug() << "qMRMLSceneFactoryWidget::deleteNode(" <<className <<") no node";
    return;
    }
  vtkMRMLNode* node = d->MRMLScene->GetNthNodeByClass(d->RandomGenerator.generate() % numNodes, className.toUtf8());
  qDebug() << "qMRMLSceneFactoryWidget::deleteNode(" <<className <<") ==" << node->GetClassName();
  d->MRMLScene->RemoveNode(node);
  // FIXME: disable delete button when there is no more nodes in the scene to delete
  emit mrmlNodeRemoved(node);
}
