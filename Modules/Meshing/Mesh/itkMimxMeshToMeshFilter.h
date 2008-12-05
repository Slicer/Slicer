/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxMeshToMeshFilter.h,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.3 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkMeshToMeshFilter_h
#define __itkMeshToMeshFilter_h

#include "itkMeshSource.h"

namespace itk
{

        /** \class MeshToMeshFilter
        * \brief 
        *
        * MeshToMeshFilter is the base class for all process objects that output
        * Mesh data and require mesh data as input. Specifically, this class
        * defines the SetInput() method for defining the input to a filter.
        **/
        template <class TInputMesh, class TOutputMesh>
        class ITK_EXPORT MeshToMeshFilter : public MeshSource<TOutputMesh>
        {
        public:
                /** Standard class typedefs. */
                typedef MeshToMeshFilter  Self;
                typedef MeshSource<TOutputMesh>  Superclass;
                typedef SmartPointer<Self>  Pointer;
                typedef SmartPointer<const Self>  ConstPointer;


                /** Run-time type information (and related methods). */
                itkTypeMacro(MeshToMeshFilter, MeshSource);

                /** Create a valid output. */
                DataObject::Pointer  MakeOutput(unsigned int idx);

                /** Some Mesh related typedefs. */
                typedef   TInputMesh                             InputMeshType;
                typedef   typename InputMeshType::Pointer        InputMeshPointer;
                typedef   typename InputMeshType::ConstPointer   InputMeshConstPointer;

                /** Some Mesh related typedefs. */
                typedef   TOutputMesh                             OutputMeshType;
                typedef   typename OutputMeshType::Pointer        OutputMeshPointer;

                /** Set the input Mesh of this process object.  */
                void SetInput(unsigned int idx, const InputMeshType *input);

                /** Get the input Mesh of this process object.  */
                const InputMeshType * GetInput(unsigned int idx);

                /** Get the output Mesh of this process object.  */
                OutputMeshType * GetOutput(void);

                /** Prepare the output */
                void GenerateOutputInformation(void);

        protected:
                MeshToMeshFilter();
                ~MeshToMeshFilter();
                void PrintSelf(std::ostream& os, Indent indent) const;

        private:
                MeshToMeshFilter(const MeshToMeshFilter&); //purposely not implemented
                void operator=(const MeshToMeshFilter&); //purposely not implemented

        };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMeshToMeshFilter.txx"
#endif

#endif
