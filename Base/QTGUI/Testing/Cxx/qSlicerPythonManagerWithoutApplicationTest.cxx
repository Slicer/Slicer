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

// CTK includes
#include <ctkTest.h>

// Slicer includes
#include "qSlicerCorePythonManager.h"

// ----------------------------------------------------------------------------
class qSlicerPythonManagerWithoutApplicationTester: public QObject
{
  Q_OBJECT

private:
  qSlicerCorePythonManager PythonManager;

private slots:
  void testInitialize();
};

// ----------------------------------------------------------------------------
void qSlicerPythonManagerWithoutApplicationTester::testInitialize()
{
  this->PythonManager.initialize();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerPythonManagerWithoutApplicationTest)
#include "moc_qSlicerPythonManagerWithoutApplicationTest.cxx"

