/*=auto=========================================================================

(c) Copyright 2003 Massachusetts Institute of Technology (MIT) All Rights Reserved.

This software ("3D Slicer") is provided by The Brigham and Women's 
Hospital, Inc. on behalf of the copyright holders and contributors.
Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for  
research purposes only, provided that (1) the above copyright notice and 
the following four paragraphs appear on all copies of this software, and 
(2) that source code to any modifications to this software be made 
publicly available under terms no more restrictive than those in this 
License Agreement. Use of this software constitutes acceptance of these 
terms and conditions.

3D Slicer Software has not been reviewed or approved by the Food and 
Drug Administration, and is for non-clinical, IRB-approved Research Use 
Only.  In no event shall data or images generated through the use of 3D 
Slicer Software be used in the provision of patient care.

IN NO EVENT SHALL THE COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE TO 
ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
EVEN IF THE COPYRIGHT HOLDERS AND CONTRIBUTORS HAVE BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.

THE COPYRIGHT HOLDERS AND CONTRIBUTORS SPECIFICALLY DISCLAIM ANY EXPRESS 
OR IMPLIED WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND 
NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS." THE COPYRIGHT HOLDERS AND CONTRIBUTORS HAVE NO OBLIGATION TO 
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================auto=*/
// .NAME vtkMRMLParser - Parse XML scene file
// .SECTION Description


#ifndef __vtkMRMLParser_h
#define __vtkMRMLParser_h

#include <stack> 

#include "vtkObjectFactory.h"
#include "vtkXMLParser.h"

#include "vtkMRML.h"
#include "vtkMRMLScene.h"

class VTK_MRML_EXPORT vtkMRMLParser : public vtkXMLParser
{
public:
  static vtkMRMLParser *New();
  vtkTypeMacro(vtkMRMLParser,vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent){}
  
  vtkMRMLScene* GetMRMLScene() {return this->MRMLScene;};
  void SetMRMLScene(vtkMRMLScene* scene) {this->MRMLScene = scene;};

  vtkCollection* GetNodeCollection() {return this->NodeCollection;};
  void SetNodeCollection(vtkCollection* scene) {this->NodeCollection = scene;};
  
protected:
  vtkMRMLParser() : MRMLScene(NULL),NodeCollection(NULL){};
  ~vtkMRMLParser() {};
  vtkMRMLParser(const vtkMRMLParser&);
  void operator=(const vtkMRMLParser&);
  
  virtual void StartElement(const char* name, const char** atts);
  virtual void EndElement (const char *name);

private:
  vtkMRMLScene* MRMLScene;
  vtkCollection* NodeCollection;
//BTX
  std::stack< vtkMRMLNode *> NodeStack;
//ETX
};

#endif
