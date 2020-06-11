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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Common test driver includes
#include "qMRMLWidgetCxxTests.h"
#include "qMRMLLayoutManagerTestHelper.cxx"

// Qt includes
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

// Slicer includes
#include "qMRMLLayoutManager.h"
#include "qMRMLLayoutManager_p.h"
#include "qMRMLSliceControllerWidget.h"
#include "qMRMLSliceWidget.h"
#include "vtkSlicerConfigure.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkWeakPointer.h>
#include "qMRMLWidget.h"

//------------------------------------------------------------------------------
class qSlicerLayoutCustomSliceViewFactory
 : public qMRMLLayoutSliceViewFactory
{
  Q_OBJECT
public:
  typedef qMRMLLayoutSliceViewFactory Superclass;
  qSlicerLayoutCustomSliceViewFactory(QObject* parent):Superclass(parent)
  {
    this->LastNode = nullptr;
  }

  ~qSlicerLayoutCustomSliceViewFactory() override = default;

  vtkWeakPointer<vtkMRMLSliceNode> LastNode;

protected:
  QWidget* createViewFromNode(vtkMRMLAbstractViewNode* viewNode) override
  {
    if (!this->layoutManager() || !viewNode)
      {// can't create a slice widget if there is no parent widget
      Q_ASSERT(viewNode);
      return nullptr;
      }

    // there is a unique slice widget per node
    Q_ASSERT(!this->viewWidget(viewNode));

    this->LastNode = viewNode;

    qMRMLSliceWidget * sliceWidget = new qMRMLSliceWidget(this->layoutManager()->viewport());
    sliceWidget->sliceController()->setControllerButtonGroup(this->SliceControllerButtonGroup);
    QString sliceLayoutName(viewNode->GetLayoutName());
    QString sliceViewLabel(viewNode->GetLayoutLabel());
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(viewNode);
    QColor sliceLayoutColor = QColor::fromRgbF(sliceNode->GetLayoutColor()[0],
                                               sliceNode->GetLayoutColor()[1],
                                               sliceNode->GetLayoutColor()[2]);
    sliceWidget->setSliceViewName(sliceLayoutName);
    sliceWidget->setObjectName(QString("qMRMLSliceWidget" + sliceLayoutName));
    sliceWidget->setSliceViewLabel(sliceViewLabel);
    sliceWidget->setSliceViewColor(sliceLayoutColor);
    sliceWidget->setMRMLScene(this->mrmlScene());
    sliceWidget->setMRMLSliceNode(sliceNode);
    sliceWidget->setSliceLogics(this->sliceLogics());

    this->sliceLogics()->AddItem(sliceWidget->sliceLogic());

    QLabel* label = new QLabel();
    label->setText("This is a SliceView customization");

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(sliceWidget);
    layout->addWidget(label);

    QWidget * widget = new QWidget(this->layoutManager()->viewport());
    widget->setObjectName("CustomSliceWidget");
    widget->setLayout(layout);

    return widget;
  }
};

//------------------------------------------------------------------------------
class vtkMRMLCustomViewNode
  : public vtkMRMLAbstractViewNode
{
public:
  static vtkMRMLCustomViewNode *New();
  vtkTypeMacro(vtkMRMLCustomViewNode, vtkMRMLAbstractViewNode);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override
  {
    return "CustomView";
  }

protected:
  vtkMRMLCustomViewNode() = default;
  ~vtkMRMLCustomViewNode() override = default;
  vtkMRMLCustomViewNode(const vtkMRMLCustomViewNode&);
  void operator=(const vtkMRMLCustomViewNode&);
};

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCustomViewNode);

//------------------------------------------------------------------------------
class qMRMLLayoutCustomViewFactory
 : public qMRMLLayoutViewFactory
{
  Q_OBJECT
public:
  typedef qMRMLLayoutViewFactory Superclass;
  qMRMLLayoutCustomViewFactory(QObject* parent) : Superclass(parent)
  {
    this->LastNode = nullptr;
  }
  ~qMRMLLayoutCustomViewFactory() override = default;

  QString viewClassName()const override
  {
    return "vtkMRMLCustomViewNode";
  }

  vtkWeakPointer<vtkMRMLSliceNode> LastNode;

protected:

  QWidget* createViewFromNode(vtkMRMLAbstractViewNode* viewNode) override
  {
    if (!viewNode || !this->layoutManager() || !this->layoutManager()->viewport())
      {
      Q_ASSERT(viewNode);
      return nullptr;
      }

    // There must be a unique Custom widget per node
    Q_ASSERT(!this->viewWidget(viewNode));

    this->LastNode = viewNode;

    QLabel* label = new QLabel();
    label->setText("This is a custom view");

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(label);

    QWidget * widget = new QWidget(this->layoutManager()->viewport());
    widget->setObjectName("CustomWidget");
    widget->setLayout(layout);

    return widget;
  }
};

//------------------------------------------------------------------------------
int qMRMLLayoutManagerWithCustomFactoryTest(int argc, char * argv[] )
{
  (void)checkViewArrangement; // Fix -Wunused-function warning

  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  QWidget w;
  w.show();

  qMRMLLayoutManager layoutManager(&w, &w);

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->SetMRMLApplicationLogic(applicationLogic);

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLLayoutNode> layoutNode;

  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLCustomViewNode>::New());

  scene->AddNode(layoutNode.GetPointer());

  applicationLogic->SetMRMLScene(scene.GetPointer());
  layoutManager.setMRMLScene(scene.GetPointer());

  // Unregister regular SliceView factory and register a custom one
  qMRMLLayoutSliceViewFactory* mrmlSliceViewFactory =
      qobject_cast<qMRMLLayoutSliceViewFactory*>(
        layoutManager.mrmlViewFactory("vtkMRMLSliceNode"));

  qSlicerLayoutCustomSliceViewFactory* customSliceViewFactory =
      new qSlicerLayoutCustomSliceViewFactory(&layoutManager);
    customSliceViewFactory->setSliceLogics(mrmlSliceViewFactory->sliceLogics());

  layoutManager.unregisterViewFactory(mrmlSliceViewFactory);
  layoutManager.registerViewFactory(customSliceViewFactory);

  // Register a factory for vtkMRMLCustomViewNode
  qMRMLLayoutCustomViewFactory* customViewFactory =
      new qMRMLLayoutCustomViewFactory(&layoutManager);
  layoutManager.registerViewFactory(customViewFactory);

  int customLayout = vtkMRMLLayoutNode::SlicerLayoutUserView + 1;
  const char* customLayoutDescription =
      "<layout type=\"horizontal\">"
      "      <item>"
      "        <view class=\"vtkMRMLSliceNode\" singletontag=\"CustomSliceView\">"
      "          <property name=\"HideFromEditors\" action=\"default\">true</property>"
      "        </view>"
      "      </item>"
      "      <item>"
      "        <view class=\"vtkMRMLCustomViewNode\" singletontag=\"CustomView\">"
      "          <property name=\"HideFromEditors\" action=\"default\">true</property>"
      "        </view>"
      "      </item>"
      "</layout>";
  layoutNode->AddLayoutDescription(customLayout, customLayoutDescription);


  layoutNode->SetViewArrangement(vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);

  QWidget* sliceWidget = layoutManager.viewWidget(customSliceViewFactory->LastNode);

  if (!sliceWidget)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qMRMLLayoutManager::viewWidget function: "
              << "Non null sliceWidget is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  if (sliceWidget->objectName() != "CustomSliceWidget")
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qMRMLLayoutManager::viewWidget function: "
              << "Widget with 'CustomSliceWidget' as object name is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  layoutNode->SetViewArrangement(customLayout);

  sliceWidget = layoutManager.viewWidget(customSliceViewFactory->LastNode);

  if (!sliceWidget)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qMRMLLayoutManager::viewWidget function: "
              << "Non null sliceWidget is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  if (sliceWidget->objectName() != "CustomSliceWidget")
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qMRMLLayoutManager::viewWidget function: "
              << "Widget with 'CustomSliceWidget' as object name is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  QWidget* customWidget = layoutManager.viewWidget(customViewFactory->LastNode);

  if (!customWidget)
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qMRMLLayoutManager::viewWidget function: "
              << "Non null customWidget is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  if (customWidget->objectName() != "CustomWidget")
    {
    std::cerr << "Line " << __LINE__
              << " - Problem with qMRMLLayoutManager::viewWidget function: "
              << "Widget with 'CustomWidget' as object name is expected."
              << std::endl;
    return EXIT_FAILURE;
    }

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    return safeApplicationQuit(&app);
    }
  else
    {
    return app.exec();
    }
}

#include "moc_qMRMLLayoutManagerWithCustomFactoryTest.cxx"
