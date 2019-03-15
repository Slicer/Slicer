// Qt includes
#include <QApplication>
#include <QTimer>

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <qMRMLLayoutManager.h>

namespace
{

// --------------------------------------------------------------------------
bool checkViewArrangement(int line, qMRMLLayoutManager* layoutManager,
                          vtkMRMLLayoutNode * layoutNode, int expectedViewArrangement)
{
  // Ignore deprecated arrangements
  if (expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView
      || expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayoutLightboxView
      || expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView
      || expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayoutSingleLightboxView
      || expectedViewArrangement == vtkMRMLLayoutNode::SlicerLayout3DPlusLightboxView)
    {
    return true;
    }

  if (layoutManager->layout() != expectedViewArrangement ||
      layoutNode->GetViewArrangement() != expectedViewArrangement)
    {
    std::cerr << "Line " << line << " - Add scene failed:\n"
              << " expected ViewArrangement: " << expectedViewArrangement << "\n"
              << " current ViewArrangement: " << layoutNode->GetViewArrangement() << "\n"
              << " current layout: " << layoutManager->layout() << std::endl;
    return false;
    }
  return true;
}

// --------------------------------------------------------------------------

// Note:
// (1) Because of Qt5 issue #50160, we need to explicitly call the quit function.
//     This ensures that the workaround associated with qSlicerWebWidget,
//     qMRMLChartWidget, ... is applied.
//     See https://bugreports.qt.io/browse/QTBUG-50160#comment-305211

int safeApplicationQuit(QApplication* app)
{
  QTimer autoExit;
  QObject::connect(&autoExit, SIGNAL(timeout()), app, SLOT(quit()));
  autoExit.start(1000);
  return app->exec();
}

} // end of anonymous namespace
