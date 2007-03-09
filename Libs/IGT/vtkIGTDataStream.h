
#ifndef IGTDATASTREAM_H
#define IGTDATASTREAM_H


#include <string>
#include <vector>

#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"
#include "vtkIGTMatrixState.h"





#define IGT_MATRIX_STREAM 0
#define IGT_IMAGE_STREAM 1

class VTK_IGT_EXPORT vtkIGTDataStream : public vtkObject
{
public:

  // Constructors/Destructors
  //  Magic lines for vtk and Slicer
    static vtkIGTDataStream *New();
    vtkTypeRevisionMacro(vtkIGTDataStream,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);


  /**
   * Constructor
   @ param buffersize: size of buufer (
   */
  vtkIGTDataStream ();

  vtkSetObjectMacro(MatrixState,vtkIGTMatrixState); 
  vtkGetObjectMacro(MatrixState,vtkIGTMatrixState); 

  
/**
   * Empty Destructor
   */
  virtual ~vtkIGTDataStream ( );

  
protected:
 
  
private:

    vtkIGTMatrixState* MatrixState;
  
 };

#endif // IGTDATASTREAM_H
