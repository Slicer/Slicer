/*
 * vtkMRMLAnnotationDisplayableManager.cpp
 *
 *  Created on: Jul 6, 2010
 */

#include <vtkMRMLAnnotationDisplayableManager.h>
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>
#include <qMRMLThreeDRenderView.h>
#include <vtkMRMLDisplayableManagerFactory.h>

vtkMRMLAnnotationDisplayableManager
    * vtkMRMLAnnotationDisplayableManager::instanceOfManager = NULL;

vtkMRMLAnnotationDisplayableManager::~vtkMRMLAnnotationDisplayableManager()
{
  // TODO Auto-generated destructor stub
}

/**
 * Get an instance of the manager. If no instance exists, a new one will be created.
 */
vtkMRMLAnnotationDisplayableManager*
vtkMRMLAnnotationDisplayableManager::GetInstance()
{

  if (instanceOfManager == NULL)
    {

      // create new instance
      instanceOfManager = new vtkMRMLAnnotationDisplayableManager();

      // register this displayable manager with the factory
      if (!instanceOfManager->RegisterManager())
        {

          // registration failed
          // something went wrong
          fprintf(stderr,
              "Could not create a new instance of the manager or could not register it!\r\n");
          instanceOfManager = NULL;

        }

      return instanceOfManager;
    }
  else
    {

      // return old instance, should be already registered at the factory
      return instanceOfManager;

    }

}

/**
 * Register the manager with the factory.
 */
bool
vtkMRMLAnnotationDisplayableManager::RegisterManager()
{

  int
      cnt =
          qSlicerApplication::application()->layoutManager()->threeDRenderViewCount();

  if (cnt > 0 && (this != NULL))
    {
      printf(
          "Starting registration of a new displayable manager instance...\r\n");

      // at least one render view exists, get the first

      qSlicerApplication * app = qSlicerApplication::application();
      qSlicerLayoutManager * lm = NULL;
      qMRMLThreeDRenderView * ddd = NULL;
      vtkMRMLDisplayableManagerFactory * factory = NULL;
      if (app != NULL)
        {
          lm = app->layoutManager();
        }
      else
        {
          fprintf(stderr, "Could not get the application!\r\n");
          return false;
        }
      if (lm != NULL)
        {

          ddd = lm->threeDRenderView(0);

        }
      else
        {
          fprintf(stderr, "Could not get the layout manager!\r\n");
          return false;
        }

      if (ddd != NULL)
        {
          factory = ddd->displayableManagerFactory();
        }
      else
        {

          fprintf(stderr, "Could not get the 3D Render View!\r\n");
          return false;
        }

      // make sure we got the factory
      Q_ASSERT(factory);

      if (factory != NULL)
        {

          factory->RegisterDisplayableManager(this);
          factory->SetMRMLViewNode(factory->GetMRMLViewNode());

        }
      else
        {
          fprintf(stderr, "Could not get displayable manager factory!\r\n");
          return false;
        }

      return true;
    }

  // something went wrong
  return false;

}
