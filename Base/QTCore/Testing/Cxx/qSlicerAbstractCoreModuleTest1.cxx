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

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerAbstractCoreModule.h"
#include "qSlicerAbstractModuleRepresentation.h"

// Qt includes
#include <QScopedPointer>

// STD includes
#include <cstdlib>
#include <iostream>

//-----------------------------------------------------------------------------
class AModuleWidgetRepresentation : public qSlicerAbstractModuleRepresentation
{
public:
  AModuleWidgetRepresentation()
  {
    ++Count;
  }
  ~AModuleWidgetRepresentation() override
  {
    --Count;
  }

  static int Count;

protected:
  void setup () override {}

};

int AModuleWidgetRepresentation::Count = 0;

//-----------------------------------------------------------------------------
class AModule: public qSlicerAbstractCoreModule
{
public:
  QString title()const override { return "A Title";}
  qSlicerAbstractModuleRepresentation* createWidgetRepresentation() override
  {
    return new AModuleWidgetRepresentation();
  }

  vtkMRMLAbstractLogic* createLogic() override
  {
    return nullptr;
  }
protected:
  void setup () override {}
};

//-----------------------------------------------------------------------------
int qSlicerAbstractCoreModuleTest1(int, char * [] )
{
  AModule module;

  //
  // Test WidgetRepresentationCreationEnabled
  //

  {
    bool current = module.isWidgetRepresentationCreationEnabled();
    bool expected = true;
    if (current != expected)
      {
      std::cerr << "Line " << __LINE__
                << " - Problem with is/setWidgetRepresentationCreationEnabled methods !\n"
                << " current:" << current << "\n"
                << " expected:" << expected << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    QScopedPointer<qSlicerAbstractModuleRepresentation> repr(module.widgetRepresentation());
    if (!repr)
      {
      std::cerr << "Line " << __LINE__
                << " - Problem with is/setWidgetRepresentationCreationEnabled methods:"
                << " widgetRepresentation is expected to be non-null." << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    module.setWidgetRepresentationCreationEnabled(false);
    bool current = module.isWidgetRepresentationCreationEnabled();
    bool expected = false;
    if (current != expected)
      {
      std::cerr << "Line " << __LINE__
                << " - Problem with is/setWidgetRepresentationCreationEnabled methods !\n"
                << " current:" << current << "\n"
                << " expected:" << expected << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    QScopedPointer<qSlicerAbstractModuleRepresentation> repr(module.widgetRepresentation());
    if (repr)
      {
      std::cerr << "Line " << __LINE__
                << " - Problem with is/setWidgetRepresentationCreationEnabled methods:"
                << " widgetRepresentation is expected to be null." << std::endl;
      return EXIT_FAILURE;
      }
  }

  module.setWidgetRepresentationCreationEnabled(true);

  {
    QScopedPointer<qSlicerAbstractModuleRepresentation> repr(module.widgetRepresentation());
    if (!repr)
      {
      std::cerr << "Line " << __LINE__
                << " - Problem with is/setWidgetRepresentationCreationEnabled methods:"
                << " widgetRepresentation is expected to be non-null." << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    int current = AModuleWidgetRepresentation::Count;
    int expected = 0;
    if (current != expected)
      {
      std::cerr << "Line " << __LINE__
                << " - Problem with representation destructor !\n"
                << " current count:" << current << "\n"
                << " expected count:" << expected << std::endl;
      return EXIT_FAILURE;
      }
  }

  return EXIT_SUCCESS;
}

