#include <bfd.h>

#include "BinaryFileDescriptor.h"

// Implementation for Linux, Mac, etc. to allow us to peek into an
// executable file and determine whether it contains global symbols.

BinaryFileDescriptor::~BinaryFileDescriptor() 
{
  // clean up - need to delete the blocks of memory that we
  // allocated to hold each section
  MemorySectionContainer::iterator sit;
  for (sit = this->Sections.begin(); sit != this->Sections.end(); ++sit)
    {
      free((*sit).second);
    }
}

bool
BinaryFileDescriptor
::Open(const char *filename)
{
  bfd *abfd;
  
  bfd_init();
  abfd = bfd_openr(filename, NULL);
  if (!abfd)
    {
    return false;
    }
  
  /* make sure it's an object file */
  if (!bfd_check_format (abfd, bfd_object)) 
    {
    bfd_close(abfd);
    return false;
    }
  
  this->BFD = abfd;
  return true;
}

void
BinaryFileDescriptor
::Close()
{
  if (this->BFD)
    {
    bfd_close(this->BFD);
    }
}

void *
BinaryFileDescriptor
::GetSymbolAddress(const char *name)
{
  long storageNeeded, numberOfSymbols;
  asymbol **symbolTable;
  void *addr = 0;
  
  if (!this->BFD)
    {
    return 0;
    }

  // Get the symbol table
  storageNeeded = bfd_get_symtab_upper_bound(this->BFD);
  symbolTable = (asymbol **) malloc(storageNeeded);
  
  numberOfSymbols =
    bfd_canonicalize_symtab(this->BFD, symbolTable);
    
  // Run through the symbol table, looking for the requested symbol
  for (int i = 0; i < numberOfSymbols; i++) 
    {
    if (strcmp(name, symbolTable[i]->name) == 0)
      { 
      // Found the symbol, get the section pointer
      asection *p = bfd_get_section(symbolTable[i]);
          
      // Do we have this section already?
      MemorySectionContainer::iterator sit;
      for (sit = this->Sections.begin(); 
           sit != this->Sections.end(); ++sit)
        {
        if ((*sit).first == p)
          {
          break;
          }  
        }
            
      PTR mem;
      if (sit == this->Sections.end())
        {
        // Get the contents of the section
        bfd_size_type sz = bfd_get_section_size (p);
        mem = malloc (sz);
        if (bfd_get_section_contents(this->BFD, p, mem, (file_ptr) 0,sz))
          {
          this->Sections.push_back( MemorySectionType(p, mem) );
          }
        else
          {
          // Error reading section
          free(mem);
          break;
          }
        }
      else
        {
        // pull the start of the section block from the cache
        mem = const_cast<void*>((*sit).second);
        }
            
      // determine the address of this section
      addr = (char *)mem
        + (bfd_asymbol_value(symbolTable[i])
           - bfd_asymbol_base(symbolTable[i]));
      break;
      }
    }

  // cleanup. just delete the outer vector for the symbol table
  free(symbolTable);
  
  return addr;
}

