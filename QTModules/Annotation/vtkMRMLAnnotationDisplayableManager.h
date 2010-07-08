/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer
 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManager.h,v $
 Date:      $Date: 2010/10/06 11:42:53 $
 Version:   $Revision: 1.1 $

 =========================================================================auto=*/

#ifndef VTKMRMLANNOTATIONDISPLAYABLEMANAGER_H_
#define VTKMRMLANNOTATIONDISPLAYABLEMANAGER_H_

#include <vtkMRMLAbstractDisplayableManager.h>

class vtkMRMLAnnotationDisplayableManager: public vtkMRMLAbstractDisplayableManager
{
public:
  static vtkMRMLAnnotationDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLAnnotationDisplayableManager, vtkMRMLAbstractDisplayableManager);

  /// Register the manager with the factory.
  bool RegisterManager();


protected:
  vtkMRMLAnnotationDisplayableManager();
  virtual ~vtkMRMLAnnotationDisplayableManager();

private:


};

#endif /* VTKMRMLANNOTATIONDISPLAYABLEMANAGER_H_ */
