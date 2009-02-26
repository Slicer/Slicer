#ifndef __vtkFetchMIWriter_h
#define __vtkFetchMIWriter_h

#include "vtkObject.h"
#include "vtkFetchMIWin32Header.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorableNode.h"
#include "vtkTagTable.h"

class VTK_FETCHMI_EXPORT vtkFetchMIWriter : public vtkObject
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
  virtual void WriteXMLHeader ( const char *dataFilename) { };

  // Description:
  // Method that writes the metadata for a node
  // Derived classes must implement this if they need to use it..
  virtual void WriteMetadataForNode ( const char *nodeID, vtkMRMLScene *scene ) {};

  // Description:
  // Method that writes the metadata for the scene.
  // Derived classes must implement this if they need to use it.
  virtual void WriteMetadataForScene ( vtkMRMLScene *scene) {};
    
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

