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

#include "qSlicerAbstractModule.h"

// STD includes

class AModule: public qSlicerAbstractModule
{
public:
  virtual QString title()const { return "A Title";}
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation()
  {
    return 0;
  }

  virtual vtkMRMLAbstractLogic* createLogic()
  {
    return 0;
  }
protected:
  virtual void setup () {}
};

int qSlicerAbstractModuleTest1(int, char * [] )
{
  AModule module;

  return EXIT_SUCCESS;
}

