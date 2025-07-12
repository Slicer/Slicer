/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital), based on qMRMLMarkupsOptionsWidgetsFactory.h by
  Csaba Pinter (Perklab, Queen's University), and was supported by The Research
  Council of Norway through the ALive project (grant nr. 311393).

==============================================================================*/

// Qt includes
#include <QScopedPointer>
#include <QSignalSpy>
#include <QTest>

// CTK includes
#include "ctkTest.h"

// VTK includes
#include <vtkNew.h>

// qMRML includes
#include "qMRMLCollapsibleButton.h"
#include "qMRMLWidget.h"

// vtkMRML includes
#include "vtkMRMLScene.h"

// ----------------------------------------------------------------------------
class qMRMLCollapsibleButtonTester : public QObject
{
  Q_OBJECT
private slots:
  void testSetMRMLScene(); // Test setMRMLScene and propagation of qt signal
};

// ----------------------------------------------------------------------------
void qMRMLCollapsibleButtonTester::testSetMRMLScene()
{
  QScopedPointer<qMRMLCollapsibleButton> collapsibleButton(new qMRMLCollapsibleButton);
  QSignalSpy registeredSignalSpy(collapsibleButton.data(), SIGNAL(mrmlSceneChanged(vtkMRMLScene*)));
  vtkNew<vtkMRMLScene> scene;

  // Trigger the mrmlsceneChanged signal
  collapsibleButton->setMRMLScene(scene.GetPointer());

  // Make sure the signal triggers only 1 time.
  QCOMPARE(registeredSignalSpy.count(), 1);
  // Verify that the original pointer and the stored in the collapsible button are the same
  QVERIFY(collapsibleButton->mrmlScene() == scene.GetPointer());
}

CTK_TEST_MAIN(qMRMLCollapsibleButtonTest)
#include "moc_qMRMLCollapsibleButtonTest.cxx"
