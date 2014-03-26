/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRMLAtlasCreator includes
#include "vtkMRMLAtlasCreatorNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAtlasCreatorNode);

//----------------------------------------------------------------------------
vtkMRMLAtlasCreatorNode::vtkMRMLAtlasCreatorNode()
{
  this->OriginalImagesFilePathList = 0;
  this->SegmentationsFilePathList = 0;
  this->OutputDirectory = 0;

  this->Toolkit = 0;

  this->TemplateType = 0;
  this->DynamicTemplateIterations = 0;
  this->FixedTemplateDefaultCaseFilePath = 0;
  this->IgnoreTemplateSegmentation = 0;

  this->LabelsList = 0;

  this->RegistrationType = 0;

  this->SaveTransforms = 0;
  this->DeleteAlignedImages = 0;
  this->DeleteAlignedSegmentations = 0;
  this->NormalizeAtlases = 0;
  this->NormalizeTo = 0;

  this->OutputCast = 0;

  this->PCAAnalysis = 0;
  this->PCAMaxEigenVectors = 0;
  this->PCACombine = 0;

  this->UseCluster = 0;
  this->SchedulerCommand = 0;

  this->NumberOfThreads = 0;

  this->SkipRegistration = 0;
  this->ExistingTemplate = 0;
  this->TransformsDirectory = 0;

  this->UseDRAMMS = 0;

  this->DebugMode = 0;
  this->DryrunMode = 0;

  this->TestMode = 0;
}

//----------------------------------------------------------------------------
vtkMRMLAtlasCreatorNode::~vtkMRMLAtlasCreatorNode()
{
  if (this->OriginalImagesFilePathList)
    {
    delete [] this->OriginalImagesFilePathList;
    this->OriginalImagesFilePathList = 0;
    }

  if (this->SegmentationsFilePathList)
    {
    delete [] this->SegmentationsFilePathList;
    this->SegmentationsFilePathList = NULL;
    }

  if (this->OutputDirectory)
    {
    delete [] this->OutputDirectory;
    this->OutputDirectory = 0;
    }

  if (this->Toolkit)
    {
    delete [] this->Toolkit;
    this->Toolkit = 0;
    }

  if (this->TemplateType)
    {
    delete [] this->TemplateType;
    this->TemplateType = 0;
    }

  if (this->FixedTemplateDefaultCaseFilePath)
    {
    delete [] this->FixedTemplateDefaultCaseFilePath;
    this->FixedTemplateDefaultCaseFilePath = 0;
    }

  if (this->LabelsList)
    {
    delete [] this->LabelsList;
    this->LabelsList = 0;
    }

  if (this->RegistrationType)
    {
    delete [] this->RegistrationType;
    this->RegistrationType = 0;
    }

  if (this->OutputCast)
    {
    delete [] this->OutputCast;
    this->OutputCast = 0;
    }

  if (this->SchedulerCommand)
    {
    delete [] this->SchedulerCommand;
    this->SchedulerCommand = 0;
    }

  if (this->ExistingTemplate)
    {
    delete [] this->ExistingTemplate;
    this->ExistingTemplate = 0;
    }

  if (this->TransformsDirectory)
    {
    delete [] this->TransformsDirectory;
    this->TransformsDirectory = 0;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAtlasCreatorNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);

  if (this->OriginalImagesFilePathList != 0)
    {
    of << " OriginalImagesFilePathList =\"" << this->OriginalImagesFilePathList << "\"";
    }

  if (this->SegmentationsFilePathList != 0)
    {
    of << " SegmentationsFilePathList =\"" << this->SegmentationsFilePathList << "\"";
    }

  if (this->OutputDirectory != 0)
    {
    of << " OutputDirectory =\"" << this->OutputDirectory << "\"";
    }


  if (this->Toolkit != 0)
    {
    of << " Toolkit =\"" << this->Toolkit << "\"";
    }


  if (this->TemplateType != 0)
    {
    of << " TemplateType =\"" << this->TemplateType << "\"";
    }

  of << " DynamicTemplateIterations =\"" << this->DynamicTemplateIterations << "\"";

  if (this->FixedTemplateDefaultCaseFilePath != 0)
    {
    of << " FixedTemplateDefaultCaseFilePath =\"" << this->FixedTemplateDefaultCaseFilePath << "\"";
    }

  of << " IgnoreTemplateSegmentation =\"" << this->IgnoreTemplateSegmentation << "\"";

  if (this->LabelsList != 0)
    {
    of << " LabelsList =\"" << this->LabelsList << "\"";
    }

  if (this->RegistrationType != 0)
    {
    of << " RegistrationType =\"" << this->RegistrationType << "\"";
    }

  of << " SaveTransforms =\"" << this->SaveTransforms << "\"";
  of << " DeleteAlignedImages =\"" << this->DeleteAlignedImages << "\"";
  of << " DeleteAlignedSegmentations =\"" << this->DeleteAlignedSegmentations << "\"";
  of << " NormalizeAtlases =\"" << this->NormalizeAtlases << "\"";
  of << " NormalizeTo =\"" << this->NormalizeTo << "\"";

  if (this->OutputCast != 0)
    {
    of << " OutputCast =\"" << this->OutputCast << "\"";
    }

  of << " PCAAnalysis =\"" << this->PCAAnalysis << "\"";
  of << " PCAMaxEigenVectors =\"" << this->PCAMaxEigenVectors << "\"";
  of << " PCACombine =\"" << this->PCACombine << "\"";


  of << " UseCluster =\"" << this->UseCluster << "\"";

  if (this->SchedulerCommand != 0)
    {
    of << " SchedulerCommand =\"" << this->SchedulerCommand << "\"";
    }


  of << " NumberOfThreads =\"" << this->NumberOfThreads << "\"";


  of << " SkipRegistration =\"" << this->SkipRegistration << "\"";

  if (this->ExistingTemplate != 0)
    {
    of << " ExistingTemplate =\"" << this->ExistingTemplate << "\"";
    }

  if (this->TransformsDirectory != 0)
    {
    of << " TransformsDirectory =\"" << this->TransformsDirectory << "\"";
    }

  of << " UseDRAMMS =\"" << this->UseDRAMMS << "\"";

  of << " DebugMode =\"" << this->DebugMode << "\"";
  of << " DryrunMode =\"" << this->DryrunMode << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLAtlasCreatorNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if ( !strcmp(attName, "OriginalImagesFilePathList") )
      {
      this->SetOriginalImagesFilePathList( attValue );
      }

    if ( !strcmp(attName, "SegmentationsFilePathList") )
      {
      this->SetSegmentationsFilePathList( attValue );
      }

    if ( !strcmp(attName, "OutputDirectory") )
      {
      this->SetOutputDirectory( attValue );
      }


    if ( !strcmp(attName, "Toolkit") )
      {
      this->SetToolkit( attValue );
      }


    if ( !strcmp(attName, "TemplateType") )
      {
      this->SetTemplateType( attValue );
      }

    if ( !strcmp(attName, "DynamicTemplateIterations") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetDynamicTemplateIterations(val);
      }

    if ( !strcmp(attName, "FixedTemplateDefaultCaseFilePath") )
      {
      this->SetFixedTemplateDefaultCaseFilePath( attValue );
      }

    if ( !strcmp(attName, "IgnoreTemplateSegmentation") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetIgnoreTemplateSegmentation(val);
      }


    if ( !strcmp(attName, "LabelsList") )
      {
      this->SetLabelsList( attValue );
      }


    if ( !strcmp(attName, "RegistrationType") )
      {
      this->SetRegistrationType( attValue );
      }


    if ( !strcmp(attName, "SaveTransforms") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetSaveTransforms(val);
      }

    if ( !strcmp(attName, "DeleteAlignedImages") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetDeleteAlignedImages(val);
      }

    if ( !strcmp(attName, "DeleteAlignedSegmentations") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetDeleteAlignedSegmentations(val);
      }

    if ( !strcmp(attName, "NormalizeAtlases") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetNormalizeAtlases(val);
      }

    if ( !strcmp(attName, "NormalizeTo") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetNormalizeTo(val);
      }


    if ( !strcmp(attName, "OutputCast") )
      {
      this->SetOutputCast( attValue );
      }


    if ( !strcmp(attName, "PCAAnalysis") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetPCAAnalysis(val);
      }

    if ( !strcmp(attName, "PCAMaxEigenVectors") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetPCAMaxEigenVectors(val);
      }

    if ( !strcmp(attName, "PCACombine") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetPCACombine(val);
      }


    if ( !strcmp(attName, "UseCluster") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetUseCluster(val);
      }

    if ( !strcmp(attName, "SchedulerCommand") )
      {
      this->SetSchedulerCommand( attValue );
      }


    if ( !strcmp(attName, "NumberOfThreads") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetNumberOfThreads(val);
      }


    if ( !strcmp(attName, "SkipRegistration") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetSkipRegistration(val);
      }

    if ( !strcmp(attName, "ExistingTemplate") )
      {
      this->SetExistingTemplate( attValue );
      }

    if ( !strcmp(attName, "TransformsDirectory") )
      {
      this->SetTransformsDirectory( attValue );
      }


    if ( !strcmp(attName, "UseDRAMMS") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetUseDRAMMS(val);
      }


    if ( !strcmp(attName, "DebugMode") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetDebugMode(val);
      }

    if ( !strcmp(attName, "DryrunMode") )
      {
      std::stringstream ss;
      ss << attValue;

      int val;
      ss >> val;

      this->SetDryrunMode(val);
      }

    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLAtlasCreatorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLAtlasCreatorNode *node = (vtkMRMLAtlasCreatorNode *) anode;

  this->SetOriginalImagesFilePathList( node->GetOriginalImagesFilePathList() );
  this->SetSegmentationsFilePathList( node->GetSegmentationsFilePathList() );
  this->SetOutputDirectory( node->GetOutputDirectory() );

  this->SetToolkit( node->GetToolkit() );

  this->SetTemplateType(node->GetTemplateType());
  this->SetDynamicTemplateIterations( node->GetDynamicTemplateIterations() );
  this->SetFixedTemplateDefaultCaseFilePath( node->GetFixedTemplateDefaultCaseFilePath() );
  this->SetIgnoreTemplateSegmentation( node->GetIgnoreTemplateSegmentation() );

  this->SetLabelsList( node->GetLabelsList() );

  this->SetRegistrationType( node->GetRegistrationType());

  this->SetSaveTransforms( node->GetSaveTransforms());
  this->SetDeleteAlignedImages( node->GetDeleteAlignedImages());
  this->SetDeleteAlignedSegmentations( node->GetDeleteAlignedSegmentations());
  this->SetNormalizeAtlases( node->GetNormalizeAtlases());
  this->SetNormalizeTo( node->GetNormalizeTo());

  this->SetOutputCast( node->GetOutputCast());

  this->SetPCAAnalysis( node->GetPCAAnalysis());
  this->SetPCAMaxEigenVectors( node->GetPCAMaxEigenVectors());
  this->SetPCACombine( node->GetPCACombine());

  this->SetUseCluster( node->GetUseCluster());
  this->SetSchedulerCommand( node->GetSchedulerCommand());

  this->SetNumberOfThreads( node->GetNumberOfThreads());

  this->SetSkipRegistration( node->GetSkipRegistration());
  this->SetExistingTemplate( node->GetExistingTemplate());
  this->SetTransformsDirectory( node->GetTransformsDirectory());

  this->SetUseDRAMMS( node->GetUseDRAMMS());

  this->SetDebugMode( node->GetDebugMode());
  this->SetDryrunMode( node->GetDryrunMode());

}

//----------------------------------------------------------------------------
void vtkMRMLAtlasCreatorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "OriginalImagesFilePathList: " << (this->GetOriginalImagesFilePathList() ? this->GetOriginalImagesFilePathList() : "(none)") << "\n";
  os << indent << "SegmentationsFilePathList: " << (this->GetSegmentationsFilePathList() ? this->GetSegmentationsFilePathList() : "(none)") << "\n";
  os << indent << "OutputDirectory: " << (this->GetOutputDirectory() ? this->GetOutputDirectory() : "(none)") << "\n";

  os << indent << "Toolkit: " << (this->GetToolkit() ? this->GetToolkit() : "(none)") << "\n";

  os << indent << "TemplateType: " << (this->GetTemplateType() ? this->GetTemplateType() : "(none)") << "\n";
  os << indent << "DynamicTemplateIterations: " << this->GetDynamicTemplateIterations() << "\n";
  os << indent << "FixedTemplateDefaultCaseFilePath: " << (this->GetFixedTemplateDefaultCaseFilePath() ? this->GetFixedTemplateDefaultCaseFilePath() : "(none)") << "\n";
  os << indent << "IgnoreTemplateSegmentation: " << this->GetIgnoreTemplateSegmentation() << "\n";

  os << indent << "LabelsList: " << (this->GetLabelsList() ? this->GetLabelsList() : "(none)") << "\n";

  os << indent << "RegistrationType: " << (this->GetRegistrationType() ? this->GetRegistrationType() : "(none)") << "\n";

  os << indent << "SaveTransforms: " << this->GetSaveTransforms() << "\n";
  os << indent << "DeleteAlignedImages: " << this->GetDeleteAlignedImages() << "\n";
  os << indent << "DeleteAlignedSegmentations: " << this->GetDeleteAlignedSegmentations() << "\n";
  os << indent << "NormalizeAtlases: " << this->GetNormalizeAtlases() << "\n";
  os << indent << "NormalizeTo: " << this->GetNormalizeTo() << "\n";

  os << indent << "OutputCast: " << (this->GetOutputCast() ? this->GetOutputCast() : "(none)") << "\n";

  os << indent << "PCAAnalysis: " << this->GetPCAAnalysis() << "\n";
  os << indent << "PCAMaxEigenVectors: " << this->GetPCAMaxEigenVectors() << "\n";
  os << indent << "PCACombine: " << this->GetPCACombine() << "\n";

  os << indent << "UseCluster: " << this->GetUseCluster() << "\n";
  os << indent << "SchedulerCommand: " << (this->GetSchedulerCommand() ? this->GetSchedulerCommand() : "(none)") << "\n";

  os << indent << "NumberOfThreads: " << this->GetNumberOfThreads() << "\n";

  os << indent << "SkipRegistration: " << this->GetSkipRegistration() << "\n";
  os << indent << "ExistingTemplate: " << (this->GetExistingTemplate() ? this->GetExistingTemplate() : "(none)") << "\n";
  os << indent << "TransformsDirectory: " << (this->GetTransformsDirectory() ? this->GetTransformsDirectory() : "(none)") << "\n";

  os << indent << "UseDRAMMS: " << this->GetUseDRAMMS() << "\n";

  os << indent << "DebugMode: " << this->GetDebugMode() << "\n";
  os << indent << "DryrunMode: " << this->GetDryrunMode() << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLAtlasCreatorNode::InitializeByDefault()
{
  this->SetOriginalImagesFilePathList("");
  this->SetSegmentationsFilePathList("");
  this->SetOutputDirectory("");

  this->SetToolkit("BRAINSFit");

  this->SetTemplateType("fixed");
  this->DynamicTemplateIterations = 5;
  this->SetFixedTemplateDefaultCaseFilePath("");
  this->IgnoreTemplateSegmentation = 0;

  this->SetLabelsList("");

  this->SetRegistrationType("Affine");

  this->SaveTransforms = 1;
  this->DeleteAlignedImages = 0;
  this->DeleteAlignedSegmentations = 0;
  this->NormalizeAtlases = 0;
  this->NormalizeTo = 1;

  this->SetOutputCast("short");

  this->PCAAnalysis = 0;
  this->PCAMaxEigenVectors = 10;
  this->PCACombine = 0;

  this->UseCluster = 0;
  this->SetSchedulerCommand("");

  this->NumberOfThreads = -1;

  this->SkipRegistration = 0;
  this->SetExistingTemplate("");
  this->SetTransformsDirectory("");

  this->UseDRAMMS = 0;

  this->DebugMode = 0;
  this->DryrunMode = 0;

  this->TestMode = 0;
}

//----------------------------------------------------------------------------
void vtkMRMLAtlasCreatorNode::Launch()
{
  this->InvokeEvent(vtkMRMLAtlasCreatorNode::LaunchComputationEvent);
}
