/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerApplicationLogic - the main logic to manage the application
// .SECTION Description
// The Main entry point for the slicer3 application.
// -- manages the connection to the mrml scene
// -- manages the creation of Views and Slices (logic only)
// -- serves as central point for dispatching events
// There is a corresponding vtkSlicerApplicationGUI class that provides
// a user interface to this class by observing this class.
//

#ifndef __vtkSlicerApplicationLogic_h
#define __vtkSlicerApplicationLogic_h

#include "vtkSlicerLogic.h"
#include "vtkSlicerSliceLogic.h"

#include "vtkMRMLScene.h"

#include "vtkCollection.h"


class VTK_SLICER_BASE_EXPORT vtkSlicerApplicationLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerApplicationLogic *New();
  vtkTypeRevisionMacro(vtkSlicerApplicationLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
    

  // Description:
  // Get the current MRML Scene object
  vtkGetObjectMacro(MRMLScene,vtkMRMLScene);

  // Description:
  // Connect to the given URL.  Disconnect any currently active 
  // connection to switch to a new connection.  A NULL pointer means
  // to disconnect current and not have a current connection
  // (creates a blank scene for manipulation).
  // Return code tells if connection was completed successfully.
  void Connect (const char *URL) {
    if (this->MRMLScene)
      {
      this->MRMLScene->SetURL(URL);
      this->MRMLScene->Connect();
      }
  };
  // Description:
  // Commit your current scene modifications to the connected URL
  // Return code tells result of commit.
  int Commit ();
  // Description:
  // Commit your current scene modifications to specified URL
  // Return code tells result of commit.
  int Commit (const char *URL);


  // Description:
  // Additional methods here to manipulate the application:
 
  // Info needed: 
  //   SlicerVersion 
  //   others?


  //
  // Views, Slices, Modules
  // -- these are the fundamental slicer elements
  // -- these are dynamic (discovered, create, deleted at run time)
  //

  // Description:
  // Views are the 3D viewports into the mrml scene
  // SlicerLogic maintains the list of currently active views
  vtkSetObjectMacro (Views,vtkCollection);
  vtkGetObjectMacro (Views,vtkCollection);

  // Description:
  // the ActiveView is the default destination of UI events
  //vtkSetObjectMacro (ActiveView,vtkSlicerViewLogic);
  //vtkGetObjectMacro (ActiveView,vtkSlicerViewLogic);

  ///// Slices
  // Description:
  // Slices are the 2D viewports that show composited layers
  // of volume data from a particular slice definition.
  vtkSetObjectMacro (Slices,vtkCollection);
  vtkGetObjectMacro (Slices,vtkCollection);

  // Description:
  // the ActiveSlice is the default destination of UI events
  vtkSetObjectMacro (ActiveSlice,vtkSlicerSliceLogic);
  vtkGetObjectMacro (ActiveSlice,vtkSlicerSliceLogic);

  ///// Modules
  // Description:
  // Modules are additional pieces of Slicer functionality
  // that are loaded and managed at run time
  vtkSetObjectMacro (Modules,vtkCollection);
  vtkGetObjectMacro (Modules,vtkCollection);

  // Description:
  // the ActiveModule is the default destination of UI events
  //vtkSetObjectMacro (ActiveModule,vtkSlicerModule);
  //vtkGetObjectMacro (ActiveModule,vtkSlicerModule);

  //
  // Transient Application State
  // -- these are elements that are inherently part of the
  //    currently running application and are not stored in 
  //    the mrml tree
  // -- any state that is expected to be saved and restored
  //    must be either part of the mrml scene or
  //    stored in the registry part of the GUI layer
  //


protected:

  vtkSlicerApplicationLogic();
  ~vtkSlicerApplicationLogic();
  vtkSlicerApplicationLogic(const vtkSlicerApplicationLogic&);
  void operator=(const vtkSlicerApplicationLogic&);

private:
  
  vtkMRMLScene *MRMLScene;
  
  // for now, make these generic collections
  // - maybe they should be subclassed to be type-specific?
  vtkCollection *Views;
  vtkCollection *Slices;
  vtkCollection *Modules;

  //vtkSlicerViewLogic *ActiveView;
  vtkSlicerSliceLogic *ActiveSlice;
  //vtkSlicerModuleLogic *ActiveModule;


  // Transient Application State
  

};

#endif

