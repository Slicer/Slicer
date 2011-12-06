#ifndef __vtkFetchMIWriter_h
#define __vtkFetchMIWriter_h

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLStorableNode.h"
#include "vtkTagTable.h"

// VTK includes
#include "vtkObject.h"

#include "vtkSlicerFetchMIModuleLogicExport.h"

class VTK_SLICER_FETCHMI_MODULE_LOGIC_EXPORT vtkFetchMIWriter : public vtkObject
{
  
public:
  static vtkFetchMIWriter* New();
  vtkTypeRevisionMacro(vtkFetchMIWriter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set on the name of the filename containing the Document Declaration
  vtkGetStringMacro ( DocumentDeclarationFilename);
  vtkSetStringMacro ( DocumentDeclarationFilename);

  // Description:
  // Get/Set on the name of a filename containing header information
  vtkGetStringMacro ( HeaderFilename );
  vtkSetStringMacro ( HeaderFilename );

  // Description:
  // Get/Set on the name of a filename containing formatted metadata
  vtkGetStringMacro ( MetadataFilename );
  vtkSetStringMacro ( MetadataFilename );

  // Description:
  // Get/Set on the name of this writer.
  vtkGetStringMacro ( Name );
  vtkSetStringMacro ( Name );

  // Description:
  // Method that writes the document declaration for a resource post.
  // Derived classes must implement this if they need to use it.
  virtual void WriteDocumentDeclaration() { };

  // Description:
  // Method that writes the XML header document 
  // Derived classes must implement this if they need to use it.
  virtual void WriteXMLHeader ( const char *vtkNotUsed(dataFilename)) { };

  // Description:
  // Method that writes the metadata for a node
  // Derived classes must implement this if they need to use it..
  virtual void WriteMetadataForNode ( const char * vtkNotUsed(nodeID),
                                      vtkMRMLScene *vtkNotUsed(scene) ) {};

  // Description:
  // Method that writes the metadata for the scene.
  // Derived classes must implement this if they need to use it.
  virtual void WriteMetadataForScene ( vtkMRMLScene *vtkNotUsed(scene)) {};
    
 protected:
  vtkFetchMIWriter();
  virtual ~vtkFetchMIWriter();
  char *DocumentDeclarationFilename;
  char *HeaderFilename;
  char *MetadataFilename;
  char *Name;

  vtkFetchMIWriter(const vtkFetchMIWriter&); // Not implemented
  void operator=(const vtkFetchMIWriter&); // Not Implemented
};

#endif

