#include "BatchMakeUtilities.h"

#include "ModuleParameterGroup.h"
#include "ModuleParameter.h"
#include <cstdlib>
#include <sstream>
#include <map>
#include <vector>

std::string GenerateBatchMakeWrapper(const ModuleDescription& module)
{
  std::ostringstream wrapper;
  std::string indent1("  ");
  std::string indent2 = indent1 + indent1;
  std::string indent3 = indent2 + indent1;
  std::string indent4 = indent3 + indent1;
  
  // Can only generate BatchMakeWrappers for executables.  
  if (module.GetType() != "CommandLineModule")
    {
    return "";
    }

  // BatchMake only understands parameter types:
  //     file=0, bool=1, int=2, float=3, string=4, enum=5
  //
  // Map what we can into these.
  //
  typedef std::map<std::string, std::string> ModuleParameterToBatchMakeTypeMap;
  ModuleParameterToBatchMakeTypeMap mp2bm;
  mp2bm["integer"] = "2";  // int
  mp2bm["float"] = "3";  // float
  mp2bm["double"] = "3";  // float
  mp2bm["boolean"] = "1"; // bool
  mp2bm["string"] = "4";  // string
  mp2bm["integer-vector"] = "4";  // string
  mp2bm["float-vector"] = "4";  // string
  mp2bm["double-vector"] = "4";  // string
  mp2bm["string-vector"] = "4";  // string
  mp2bm["integer-enumeration"] = "5"; // enum
  mp2bm["float-enumeration"] = "3";  // float ???
  mp2bm["double-enumeration"] = "3";  // float ???
  mp2bm["string-enumeration"] = "4";  // string ???
  mp2bm["file"] = "0";  // file
  mp2bm["directory"] = "0";  // file ???
  mp2bm["image"] = "0";  // file
  mp2bm["geometry"] = "0";  // file
  mp2bm["point"] = "4";  // string ???
  
  // BatchMake format preamble
  wrapper << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
  wrapper << "<BatchMakeApplicationWrapper>" << std::endl;
  wrapper << indent1 << "<BatchMakeApplicationWrapperVersion>1.0</BatchMakeApplicationWrapper>" << std::endl;
  wrapper << indent1 << "<Module>" << std::endl;
  wrapper << indent2 << "<Name>" << module.GetTitle() << "</Name>"
          << std::endl;
  wrapper << indent2 << "<Version>" << module.GetVersion()
          << "</Version>" << std::endl;
  wrapper << indent2 << "<Path>" << module.GetLocation() << "</Path>"
          << std::endl;
  wrapper << indent2 << "<Parameters>" << std::endl;

  
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = module.GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = module.GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;

  ModuleParameterToBatchMakeTypeMap::iterator mp2bmIt;


  // Loop over all parameters that have a flag then loop over the
  // parameters that are index parameters
  //
  
  // Loop over executables with flags
  int pcount = 0;
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    for (pit = pbeginit; pit != pendit; ++pit)
      {
      std::string prefix;
      std::string flag;
      bool hasFlag = false;
      
      if ((*pit).GetLongFlag() != "")
        {
        prefix = "--";
        flag = (*pit).GetLongFlag();
        hasFlag = true;
        }
      else if ((*pit).GetFlag() != "")
        {
        prefix = "-";
        flag = (*pit).GetFlag();
        hasFlag = true;
        }

      if (hasFlag)
        {
        pcount++;
        
        // Batchmake uses two parameters to represent non-boolean
        // options.  THe first of the parameters is the "flag", the
        // second is the value.  THe Parent tag is used to ling flags
        // to values.
        wrapper << indent3 << "<Param>" << std::endl;

        // first parameter in the pair is ALWAYS a bool. If there is
        // not a pair, then the parameter is bool anyway.
        wrapper << indent4 << "<Type>1</Type>" << std::endl;

        // if we are going to be creating a paired set of parameters,
        // then name the first one the <parameter name>.flag and the
        // second one just <parameter name> for the value.
        if ((*pit).GetTag() != "boolean")
          {
          wrapper << indent4 << "<Name>";
          wrapper << (*pit).GetName() + ".flag";
          wrapper << "</Name>" << std::endl;
          }
        else
          {
          // just use the name
          wrapper << indent4 << "<Name>";
          wrapper << (*pit).GetName();
          wrapper << "</Name>" << std::endl;
          }

        // Value is the flag
        wrapper << indent4 << "<Value>";
        wrapper << prefix + flag;
        wrapper << "</Value>" << std::endl;

        // The flag itself never has a parent
        wrapper << indent4 << "<Parent>0</Parent>" << std::endl;

        // BatchMake external flag is:
        //   0=nothing, 1=input, 2=output
        //
        // THe flag itself is always external = 0
        wrapper << indent4 << "<External>0</External>" << std::endl;
        
        // Any module parameter that has a flag is optional.
        wrapper << indent4 << "<Optional>1</Optional>" << std::endl;
        
        wrapper << indent3 << "</Param>" << std::endl;

        // Is a child BatchMake parameter needed for the parameter?
        if ((*pit).GetTag() != "boolean")
          {
          pcount++;

          wrapper << indent3 << "<Param>" << std::endl;
          
          // find the module parameter type in the map to BatchMake types
          mp2bmIt = mp2bm.find((*pit).GetTag());
          if (mp2bmIt != mp2bm.end())
            {
            wrapper << indent4 << "<Type>";
            wrapper << (*mp2bmIt).second;
            wrapper << "</Type>" << std::endl;
            }
          else
            {
            // unsupported type. map to string
            wrapper << indent4 << "<Type>4</Type>" << std::endl;
            }

          // THe name of the BatchMake paired parameter is just the
          // original parameter name
          wrapper << indent4 << "<Name>";
          wrapper << (*pit).GetName();
          wrapper << "</Name>" << std::endl;
          
          wrapper << indent4 << "<Value>";
          wrapper << (*pit).GetDefault();
          wrapper << "</Value>" << std::endl;

          // The parent is the previous parameter (the parameter for
          // the flag)
          wrapper << indent4 << "<Parent>" << pcount - 1
                  << "</Parent>" << std::endl;
          
          // BatchMake external flag is:
          //   0=nothing, 1=input, 2=output
          if ((*pit).GetTag() == "image" || (*pit).GetTag() == "geometry"
              || (*pit).GetTag() == "file")
            {
            if ((*pit).GetChannel() == "input")
              {
              wrapper << indent4 << "<External>1</External>" << std::endl;
              }
            else if ((*pit).GetChannel() == "output")
              {
              wrapper << indent4 << "<External>2</External>" << std::endl;
              }
            else
              {
              wrapper << indent4 << "<External>0</External>" << std::endl;
              }
            }
          else
            {
            wrapper << indent4 << "<External>0</External>" << std::endl;
            }
          
          // If the flag was specified, then the parameter for the
          // value cannot be option
          wrapper << indent4 << "<Optional>0</Optional>" << std::endl;
          
          wrapper << indent3 << "</Param>" << std::endl;
          }
        }
      }
    }  

  // now tack on any parameters that are based on indices
  //
  // build a list of indices to traverse in order
  std::map<int, ModuleParameter> indexmap;
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;
  
    for (pit = pbeginit; pit != pendit; ++pit)
      {
      if ((*pit).GetIndex() != "")
        {
        indexmap[atoi((*pit).GetIndex().c_str())] = (*pit);
        }
      }
    }

  // walk the index parameters in order
  std::map<int, ModuleParameter>::const_iterator iit;
  for (iit = indexmap.begin(); iit != indexmap.end(); ++iit)
    {
    wrapper << indent3 << "<Param>" << std::endl;
    
    // find the module parameter type in the map to BatchMake types
    mp2bmIt = mp2bm.find((*iit).second.GetTag());
    if (mp2bmIt != mp2bm.end())
      {
      wrapper << indent4 << "<Type>";
      wrapper << (*mp2bmIt).second;
      wrapper << "</Type>" << std::endl;
      }
    else
      {
      // unsupported type. map to string
      wrapper << indent4 << "<Type>4</Type>" << std::endl;;
      }

    wrapper << indent4 << "<Name>";
    wrapper << (*iit).second.GetName();
    wrapper << "</Name>" << std::endl;
    
    wrapper << indent4 << "<Value>";
    wrapper << (*iit).second.GetDefault();
    wrapper << "</Value>" << std::endl;
    
    // index parameters have no parents
    wrapper << indent4 << "<Parent>0</Parent>" << std::endl;
    
    // BatchMake external flag is:
    //   0=nothing, 1=input, 2=output
    if ((*iit).second.GetTag() == "image"
        || (*iit).second.GetTag() == "geometry"
        || (*iit).second.GetTag() == "file")
      {
      if ((*iit).second.GetChannel() == "input")
        {
        wrapper << indent4 << "<External>1</External>" << std::endl;
        }
      else if ((*iit).second.GetChannel() == "output")
        {
        wrapper << indent4 << "<External>2</External>" << std::endl;
        }
      else
        {
        wrapper << indent4 << "<External>0</External>" << std::endl;
        }
      }
    else
      {
      wrapper << indent4 << "<External>0</External>" << std::endl;
      }
    
    // Any index parameter is not optional
    wrapper << indent4 << "<Optional>0</Optional>" << std::endl;
    
    wrapper << indent3 << "</Param>" << std::endl;
    }

  wrapper << indent2 << "</Parameters>" << std::endl;
  wrapper << indent1 << "</Module>" << std::endl;
  wrapper << "</BatchMakeApplicationWrapper>" << std::endl;

  return wrapper.str();
}
