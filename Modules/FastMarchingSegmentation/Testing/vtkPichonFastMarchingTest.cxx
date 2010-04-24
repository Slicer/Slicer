#include "vtkPichonFastMarching.h"
#include "TestingMacros.h"

int vtkPichonFastMarchingTest(int, char * [] ){
  vtkSmartPointer<vtkPichonFastMarching> filter = 
    vtkSmartPointer<vtkPichonFastMarching>::New();

  EXERCISE_BASIC_OBJECT_METHODS(filter);

  return EXIT_SUCCESS;
}
