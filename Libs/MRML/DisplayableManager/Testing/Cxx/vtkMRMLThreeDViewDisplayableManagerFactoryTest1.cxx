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

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

// Initialize object factory
#define MRMLDisplayableManagerCxxTests_AUTOINIT 1(MRMLDisplayableManagerCxxTests)
#include <vtkAutoInit.h>
VTK_AUTOINIT(MRMLDisplayableManagerCxxTests)

//----------------------------------------------------------------------------
int vtkMRMLThreeDViewDisplayableManagerFactoryTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkMRMLThreeDViewDisplayableManagerFactory * factory = vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance();
  if (!factory)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with GetInstance() method" << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Since the factory is a singleton, object returned using either New() or GetInstance()
  // should be the same
  vtkNew<vtkMRMLThreeDViewDisplayableManagerFactory> factoryUsingSmartPointer;
  if (!factoryUsingSmartPointer.GetPointer())
    {
    std::cerr << "Line " << __LINE__ << " - Problem with New() method" << std::endl;
    return EXIT_FAILURE;
    }
  if (factory != factoryUsingSmartPointer.GetPointer())
    {
    std::cerr << "Line " << __LINE__
        << " - Factory obtained using either GetInstance() or New() should be the same."
        << std::endl;
    return EXIT_FAILURE;
    }

  if (factory->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->GetRegisteredDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 0" << std::endl;
    std::cerr << "\tCurrent: " << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Check if IsDisplayableManagerRegistered works as expected
  bool isRegistrered = factory->IsDisplayableManagerRegistered("vtkMRMLINVALIDManager");
  if (isRegistrered)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->IsDisplayableManagerRegistered()" << std::endl;
    std::cerr << "\tExpected: false" << std::endl;
    std::cerr << "\tCurrent: " << isRegistrered << std::endl;
    return EXIT_FAILURE;
    }

  isRegistrered = factory->IsDisplayableManagerRegistered("vtkMRMLCameraDisplayableManager");
  if (isRegistrered)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->IsDisplayableManagerRegistered()" << std::endl;
    std::cerr << "\tExpected: false" << std::endl;
    std::cerr << "\tCurrent: " << isRegistrered << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Register invalid DisplayableManager
  bool success = factory->RegisterDisplayableManager("vtkMRMLINVALIDManager");
  if (success)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->RegisterDisplayableManager()" << std::endl;
    std::cerr << "\tExpected: 0" << std::endl;
    std::cerr << "\tCurrent: " << success << std::endl;
    return EXIT_FAILURE;
    }

  if (factory->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->GetRegisteredDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 0" << std::endl;
    std::cerr << "\tCurrent: " << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Register CameraDisplayableManager
  success = factory->RegisterDisplayableManager("vtkMRMLCameraDisplayableManager");
  if (!success)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->RegisterDisplayableManager()" << std::endl;
    std::cerr << "\tExpected: 1" << std::endl;
    std::cerr << "\tCurrent: " << success << std::endl;
    return EXIT_FAILURE;
    }

  if (factory->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->GetRegisteredDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 1" << std::endl;
    std::cerr << "\tCurrent: " << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  // Register again CameraDisplayableManager - The count should remain the same
  success = factory->RegisterDisplayableManager("vtkMRMLCameraDisplayableManager");
  if (success)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->RegisterDisplayableManager()" << std::endl;
    std::cerr << "\tExpected: 0" << std::endl;
    std::cerr << "\tCurrent: " << success << std::endl;
    return EXIT_FAILURE;
    }

  if (factory->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->GetRegisteredDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 1" << std::endl;
    std::cerr << "\tCurrent: " << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Check if IsDisplayableManagerRegistered works as expected
  isRegistrered = factory->IsDisplayableManagerRegistered("vtkMRMLINVALIDManager");
  if (isRegistrered)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->IsDisplayableManagerRegistered()" << std::endl;
    std::cerr << "\tExpected: false" << std::endl;
    std::cerr << "\tCurrent: " << isRegistrered << std::endl;
    return EXIT_FAILURE;
    }

  isRegistrered = factory->IsDisplayableManagerRegistered("vtkMRMLCameraDisplayableManager");
  if (!isRegistrered)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->IsDisplayableManagerRegistered()" << std::endl;
    std::cerr << "\tExpected: true" << std::endl;
    std::cerr << "\tCurrent: " << isRegistrered << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Renderer, RenderWindow and Interactor
  vtkNew<vtkRenderer> rr;
  vtkNew<vtkRenderWindow> rw;
  vtkNew<vtkRenderWindowInteractor> ri;
  rw->SetSize(600, 600);
  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere
  rw->AddRenderer(rr.GetPointer());
  rw->SetInteractor(ri.GetPointer());

  // Set Interactor Style
  vtkNew<vtkMRMLThreeDViewInteractorStyle> iStyle;
  ri->SetInteractorStyle(iStyle.GetPointer());

  // MRML scene and ViewNode
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLViewNode> viewNode;
  vtkMRMLNode * nodeAdded = scene->AddNode(viewNode.GetPointer());
  if (!nodeAdded)
    {
    std::cerr << "Line " << __LINE__ << " - Failed to add vtkMRMLViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Instantiate DisplayableManagerGroup
  vtkMRMLDisplayableManagerGroup * group =
      factory->InstantiateDisplayableManagers(rr.GetPointer());
  if (!group)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with factory->InstantiateDisplayableManagers() method"
        << std::endl;
    std::cerr << "\tgroup should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
    }

  if (group->GetDisplayableManagerCount() != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method group->GetDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 1" << std::endl;
    std::cerr << "\tCurrent: " << group->GetDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  // Delete the group
  group->Delete();

  // Try to instantiate again the DisplayableManagerGroup
  group = factory->InstantiateDisplayableManagers(rr.GetPointer());
  if (!group)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with factory->InstantiateDisplayableManagers() method" << std::endl;
    std::cerr << "\tgroup should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
    }

  if (group->GetDisplayableManagerCount() != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method group->GetDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 1" << std::endl;
    std::cerr << "\tCurrent: " << group->GetDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Register a DisplayableManager after the DisplayableManagerGroup has been instantiated
  factory->RegisterDisplayableManager("vtkMRMLViewDisplayableManager");

  if (factory->GetRegisteredDisplayableManagerCount() != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->GetRegisteredDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 2" << std::endl;
    std::cerr << "\tCurrent: " << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  // Check if the group contains the registered DisplayableManager
  if (group->GetDisplayableManagerCount() != 2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method group->GetDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 2" << std::endl;
    std::cerr << "\tCurrent: " << group->GetDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Attempt to get the Registered DisplayableManagers
  vtkMRMLAbstractDisplayableManager * dm1 =
      group->GetDisplayableManagerByClassName("vtkMRMLViewDisplayableManager");
  if (!dm1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with group->GetDisplayableManagerByClassName() method" << std::endl;
    std::cerr << "\tdm1 should NOT be NULL" << std::endl;
    return EXIT_FAILURE;
    }
  if (!dm1->IsA("vtkMRMLViewDisplayableManager"))
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with group->GetDisplayableManagerByClassName() method" << std::endl;
    std::cerr << "\tExpected: vtkMRMLViewDisplayableManager" << std::endl;
    std::cerr << "\tCurrent: " << dm1->GetClassName() << std::endl;
    }

  vtkMRMLAbstractDisplayableManager * dm2 =
      group->GetDisplayableManagerByClassName("vtkMRMLCameraDisplayableManager");
  if (!dm2)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with group->GetDisplayableManagerByClassName() method" << std::endl;
    std::cerr << "\tdm2 should NOT be NULL" << std::endl;
    }
  else if (!dm2->IsA("vtkMRMLCameraDisplayableManager"))
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with group->GetDisplayableManagerByClassName() method" << std::endl;
    std::cerr << "\tExpected: vtkMRMLCameraDisplayableManager" << std::endl;
    std::cerr << "\tCurrent: " << dm2->GetClassName() << std::endl;
    }

  //----------------------------------------------------------------------------
  // Attempt to UnRegister a DisplayableManager already instantiated in a Group
  success = factory->UnRegisterDisplayableManager("vtkMRMLCameraDisplayableManager");

  if (factory->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->GetRegisteredDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 1" << std::endl;
    std::cerr << "\tCurrent: " << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  // Check if the group has been updated
  if (group->GetDisplayableManagerCount() != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method group->GetDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 1" << std::endl;
    std::cerr << "\tCurrent: " << group->GetDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  group->Delete();

  return EXIT_SUCCESS;
}
