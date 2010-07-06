/*
 * vtkMRMLAnnotationDisplayableManager.h
 *
 *  Created on: Jul 6, 2010
 */

#ifndef VTKMRMLANNOTATIONDISPLAYABLEMANAGER_H_
#define VTKMRMLANNOTATIONDISPLAYABLEMANAGER_H_

#include <vtkMRMLAbstractDisplayableManager.h>

class vtkMRMLAnnotationDisplayableManager : public vtkMRMLAbstractDisplayableManager
{
public:

  static vtkMRMLAnnotationDisplayableManager*
  GetInstance();

  virtual
  ~vtkMRMLAnnotationDisplayableManager();
protected:
  vtkMRMLAnnotationDisplayableManager()
  {
    // TODO Auto-generated constructor stub

  }

private:

  static vtkMRMLAnnotationDisplayableManager * instanceOfManager;

   bool RegisterManager();


};

#endif /* VTKMRMLANNOTATIONDISPLAYABLEMANAGER_H_ */
