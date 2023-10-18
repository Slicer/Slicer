/**
 * @class   vtkMRMLOpenGLLineStippleMapper2D
 * @brief   2D stippled line support for OpenGL
 *
 * vtkMRMLOpenGLLineStippleMapper2D provides support for 2D stippled line for
 * Slicer under OpenGL.
 *
 */

#ifndef vtkMRMLOpenGLLineStippleMapper2D_h
#define vtkMRMLOpenGLLineStippleMapper2D_h

#include "vtkMRMLDisplayableManagerExport.h"

#include "vtkOpenGLPolyDataMapper2D.h"
#include "vtkShader.h"
#include <map>

class vtkActor2D;
class vtkViewport;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLOpenGLLineStippleMapper2D : public vtkOpenGLPolyDataMapper2D
{
public:
  vtkTypeMacro(vtkMRMLOpenGLLineStippleMapper2D, vtkOpenGLPolyDataMapper2D);
  static vtkMRMLOpenGLLineStippleMapper2D* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkMRMLOpenGLLineStippleMapper2D() = default;
  ~vtkMRMLOpenGLLineStippleMapper2D() override = default;

  /**
   * Perform string replacements on the shader templates, called from
   * ReplaceShaderValues
   */
  void ReplaceShaderTCoord(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor2D* act) override;

  /**
   * Perform string replacements on the shader templates
   */
  void ReplaceShaderValues(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkViewport* viewport, vtkActor2D* act) override;

  /**
   * Set the shader parameters related to the mapper/input data, called by UpdateShader
   */
  void SetMapperShaderParameters(
    vtkOpenGLHelper& cellBO, vtkViewport* viewport, vtkActor2D* act) override;

private:
  vtkMRMLOpenGLLineStippleMapper2D(const vtkMRMLOpenGLLineStippleMapper2D&) = delete;
  void operator=(const vtkMRMLOpenGLLineStippleMapper2D&) = delete;
};

#endif
