#include "CudappOpenGLBufferObject.h"

#include "cuda_runtime_api.h"
#include "cuda_gl_interop.h"

#include "CudappBase.h"

OpenGLBufferObject::OpenGLBufferObject()
{
  this->BufferObject = 0;
  this->DevPointer = NULL;
}

OpenGLBufferObject::~OpenGLBufferObject()
{
  this->Unmap();
  this->Unregister();
}

void OpenGLBufferObject::Register(GLuint bufferObject)
{
  cudaError_t error = 
    cudaGLRegisterBufferObject(bufferObject);
  if (error != cudaSuccess)
        Base::PrintError(error);
  else    
    this->BufferObject = bufferObject;
}

void OpenGLBufferObject::Unregister()
{
  cudaError_t error = 
    cudaGLUnregisterBufferObject(this->BufferObject);
  if (error != cudaSuccess)
        Base::PrintError(error);
}

/**
 * @brief maps a GLBufferObject to a memory space. 
 * @returns a pointer to the mapped area
 * 
 * @note Any prior mappings of this Bufferobject will be removed.
 * If the BufferObject has not yet been registered NULL will be returned.
 */
void* OpenGLBufferObject::Map()
{
  this->Unmap();
  if (this->BufferObject == 0)
    return NULL;
  cudaError_t error = cudaGLMapBufferObject(&this->DevPointer, this->BufferObject);
  if (error != cudaSuccess) 
    Base::PrintError(error);
  return this->DevPointer;
}

/**
 * @brief unmaps a BufferObject's memory point. sets DevPointer to NULL.
 */
void OpenGLBufferObject::Unmap()
{
  if (this->DevPointer != NULL)
  {
    cudaError_t error = 
      cudaGLUnregisterBufferObject(this->BufferObject);
      if (error != cudaSuccess)
        Base::PrintError(error);
        
    this->DevPointer = NULL;
  }
}

void OpenGLBufferObject::PrintSelf(std::ostream &os) const
{
  this->Superclass::PrintSelf(os, indent);
  if (this->GetBufferObject() == 0)
    os << "Not yet initialized Buffer Object";
}

