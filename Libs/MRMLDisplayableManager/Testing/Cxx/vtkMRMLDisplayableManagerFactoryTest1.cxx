
// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerFactory.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkThreeDViewInteractorStyle.h>
#include <vtkMRMLAbstractDisplayableManager.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

// STD includes
#include <cstdlib>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//----------------------------------------------------------------------------
int vtkMRMLDisplayableManagerFactoryTest1(int argc, char* argv[])
{
  vtkMRMLDisplayableManagerFactory * factory = vtkMRMLDisplayableManagerFactory::GetInstance();
  if (!factory)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with GetInstance() method" << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Since the factory is a singleton, object returned using either New() or GetInstance()
  // should be the same
  VTK_CREATE(vtkMRMLDisplayableManagerFactory, factoryUsingSmartPointer);
  if (!factoryUsingSmartPointer)
    {
    std::cerr << "Line " << __LINE__ << " - Problem with New() method" << std::endl;
    return EXIT_FAILURE;
    }
  if (factory != factoryUsingSmartPointer)
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
  factory->RegisterDisplayableManager("vtkMRMLINVALIDManager");

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
  factory->RegisterDisplayableManager("vtkMRMLCameraDisplayableManager");

  if (factory->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with method factory->GetRegisteredDisplayableManagerCount()" << std::endl;
    std::cerr << "\tExpected: 1" << std::endl;
    std::cerr << "\tCurrent: " << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  // Register again CameraDisplayableManager - The count should remain the same
  factory->RegisterDisplayableManager("vtkMRMLCameraDisplayableManager");

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
  VTK_CREATE(vtkRenderer, rr);
  VTK_CREATE(vtkRenderWindow, rw);
  VTK_CREATE(vtkRenderWindowInteractor, ri);
  rw->SetSize(600, 600);
  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere
  rw->AddRenderer(rr);
  rw->SetInteractor(ri);

  // Set Interactor Style
  VTK_CREATE(vtkThreeDViewInteractorStyle, iStyle);
  ri->SetInteractorStyle(iStyle);

  // MRML scene and ViewNode
  VTK_CREATE(vtkMRMLScene, scene);
  VTK_CREATE(vtkMRMLViewNode, viewNode);
  vtkMRMLNode * nodeAdded = scene->AddNode(viewNode);
  if (!nodeAdded)
    {
    std::cerr << "Line " << __LINE__ << " - Failed to add vtkMRMLViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  //----------------------------------------------------------------------------
  // Instanciate DisplayableManagerGroup
  vtkMRMLDisplayableManagerGroup * group = factory->InstantiateDisplayableManagers(rr);
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
  group = factory->InstantiateDisplayableManagers(rr);
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
  // Register a DisplayableManager after the DisplayableManagerGroup has been instanciated
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
  if (!dm2->IsA("vtkMRMLCameraDisplayableManager"))
    {
    std::cerr << "Line " << __LINE__
        << " - Problem with group->GetDisplayableManagerByClassName() method" << std::endl;
    std::cerr << "\tExpected: vtkMRMLCameraDisplayableManager" << std::endl;
    std::cerr << "\tCurrent: " << dm2->GetClassName() << std::endl;
    }

  //----------------------------------------------------------------------------
  // Attempt to UnRegister a DisplayableManager already instanciated in a Group
  factory->UnRegisterDisplayableManager("vtkMRMLCameraDisplayableManager");

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
