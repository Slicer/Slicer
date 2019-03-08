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
#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QTextBrowser>
#include <QTimer>

// CTK includes
#include "ctkTest.h"
#include <ctkVTKWidgetsUtils.h>
#include <ctkWidgetsUtils.h>

// MRML includes
#include "qSlicerPresetComboBox.h"
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

// ----------------------------------------------------------------------------
class qSlicerPresetComboBoxTester: public QObject
{
  Q_OBJECT

private slots:

  void testSetScene();
  void testAddPresetNode();
  void testPreview();
};

// ----------------------------------------------------------------------------
void qSlicerPresetComboBoxTester::testSetScene()
{
  qSlicerPresetComboBox presetComboBox;
  QCOMPARE(presetComboBox.nodeCount() , 0);

  vtkNew<vtkMRMLScene> scene;
  presetComboBox.setMRMLScene(scene.GetPointer());
  QCOMPARE(presetComboBox.nodeCount() , 0);

  vtkNew<vtkMRMLViewNode> viewNode;
  scene->AddNode(viewNode.GetPointer());
  QCOMPARE(presetComboBox.nodeCount() , 0);

}

// ----------------------------------------------------------------------------
void qSlicerPresetComboBoxTester::testAddPresetNode()
{
  qSlicerPresetComboBox presetComboBox;

  vtkNew<vtkMRMLScene> scene;
  presetComboBox.setMRMLScene(scene.GetPointer());

  QCOMPARE(presetComboBox.nodeCount(), 0);

  vtkNew<vtkMRMLVolumePropertyNode> presetNode;
  presetNode->SetName("CT-AAA");
  vtkNew<vtkPiecewiseFunction> gradientOpacity;
  gradientOpacity->AddPoint(0., 1.);
  gradientOpacity->AddPoint(255., 1.);
  presetNode->SetGradientOpacity(gradientOpacity.GetPointer());
  vtkNew<vtkPiecewiseFunction> scalarOpacity;
  scalarOpacity->AddPoint(-3024., 0.);
  scalarOpacity->AddPoint(3071., 0.8);
  presetNode->SetScalarOpacity(scalarOpacity.GetPointer());
  vtkNew<vtkColorTransferFunction> colorFunction;
  colorFunction->AddRGBPoint(-3024., 0., 0., 0.);
  colorFunction->AddRGBPoint(143.556, 0.615686, 0.356863, 0.184314);
  colorFunction->AddRGBPoint(166.222, 0.882353, 0.603922, 0.290196);
  colorFunction->AddRGBPoint(214.389, 1., 1., 1.);
  colorFunction->AddRGBPoint(419.736, 1., 0.937033, 0.954531);
  colorFunction->AddRGBPoint(3071., 0.827451, 0.658824, 1.);
  presetNode->SetColor(colorFunction.GetPointer());
  scene->AddNode(presetNode.GetPointer());

  presetComboBox.show();
  //qApp->exec();
}
// ----------------------------------------------------------------------------
void qSlicerPresetComboBoxTester::testPreview()
{
  vtkNew<vtkColorTransferFunction> colorFunction;
  colorFunction->AddRGBPoint(0., 0., 0., 0.);
  colorFunction->AddRGBPoint(1., 1., 0., 0.);
  QImage image = ctk::scalarsToColorsImage(colorFunction.GetPointer(), QSize(32, 32));

  QLabel label;
  QPixmap pixmap;

  pixmap.convertFromImage(image);
  label.setText(QString("<img src=\"%1\"/>").arg(ctk::base64HTMLImageTagSrc(image)));
  //label.setPixmap(pixmap); ok !
  label.show();

  QImage expectedImage(32,32, QImage::Format_RGB32);
  QPainter painter;
  painter.begin(&expectedImage);
  QLinearGradient redGradient(0.,0.,1.,0.);
  redGradient.setCoordinateMode(QGradient::StretchToDeviceMode);
  redGradient.setColorAt(0., Qt::black);
  redGradient.setColorAt(1., Qt::red);
  painter.setPen(Qt::NoPen);
  painter.setBrush(QBrush(redGradient));
  painter.drawRect(0,0,32,32);
  painter.end();

  //QVERIFY(image == expectedImage);

  //QPixmap expectedPixmap;
  //expectedPixmap.convertFromImage(expectedImage);
  //QLabel expectedLabel;
  //expectedLabel.setPixmap(expectedPixmap);

  //expectedLabel.show();
  //qApp->exec();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerPresetComboBoxTest)
#include "moc_qSlicerPresetComboBoxTest.cxx"
