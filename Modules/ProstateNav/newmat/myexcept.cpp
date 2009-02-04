//$$myexcept.cpp                        Exception handler

// Copyright (C) 1993,4,6: R B Davies


#define WANT_STREAM                    // include.h will get stream fns
#define WANT_STRING

#include "include.h"                   // include standard files


#include "myexcept.h"                  // for exception handling

#ifdef use_namespace
namespace RBD_COMMON {
#endif


//#define REG_DEREG                    // for print out uses of new/delete
//#define CLEAN_LIST                   // to print entries being added to
                                       // or deleted from cleanup list

#ifdef SimulateExceptions

void Throw()
{
   for (Janitor* jan = JumpBase::jl->janitor; jan; jan = jan->NextJanitor)
      jan->CleanUp();
   JumpItem* jx = JumpBase::jl->ji;    // previous jumpbase;
   if ( !jx ) { Terminate(); }         // jl was initial JumpItem
   JumpBase::jl = jx;                  // drop down a level; cannot be in front
                                       // of previous line
   Tracer::last = JumpBase::jl->trace;
   longjmp(JumpBase::jl->env, 1);
}

#endif                                 // end of simulate exceptions


unsigned long BaseException::Select;
char* BaseException::what_error;
int BaseException::SoFar;
int BaseException::LastOne;

BaseException::BaseException(const char* a_what)
{
   Select++; SoFar = 0;
   if (!what_error)                   // make space for exception message
   {
      LastOne = 511;
      what_error = new char[512];
      if (!what_error)                // fail to make space
      {
         LastOne = 0;
         what_error = (char *)"No heap space for exception message\n";
      }
   }
   AddMessage("\n\nAn exception has been thrown\n");
   AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}

void BaseException::AddMessage(const char* a_what)
{
   if (a_what)
   {
      int l = strlen(a_what); int r = LastOne - SoFar;
      if (l < r) { strcpy(what_error+SoFar, a_what); SoFar += l; }
      else if (r > 0)
      {
         strncpy(what_error+SoFar, a_what, r);
         what_error[LastOne] = 0;
         SoFar = LastOne;
      }
   }
}

void BaseException::AddInt(int value)
{
   bool negative;
   if (value == 0) { AddMessage("0"); return; }
   else if (value < 0) { value = -value; negative = true; }
   else negative = false;
   int n = 0; int v = value;        // how many digits will we need?
   while (v > 0) { v /= 10; n++; }
   if (negative) n++;
   if (LastOne-SoFar < n) { AddMessage("***"); return; }

   SoFar += n; n = SoFar; what_error[n] = 0;
   while (value > 0)
   {
      int nv = value / 10; int rm = value - nv * 10;  value = nv;
      what_error[--n] = (char)(rm + '0');
   }
   if (negative) what_error[--n] = '-';
   return;
}

void Tracer::PrintTrace()
{
   cout << "\n";
   for (Tracer* et = last; et; et=et->previous)
      cout << "  * " << et->entry << "\n";
}

void Tracer::AddTrace()
{
   if (last)
   {
      BaseException::AddMessage("Trace: ");
      BaseException::AddMessage(last->entry);
      for (Tracer* et = last->previous; et; et=et->previous)
      {
         BaseException::AddMessage("; ");
         BaseException::AddMessage(et->entry);
      }
      BaseException::AddMessage(".\n");
   }
}

#ifdef SimulateExceptions


Janitor::Janitor()
{
   if (do_not_link)
   {
      do_not_link = false; NextJanitor = 0; OnStack = false;
#ifdef CLEAN_LIST
      cout << "Not added to clean-list " << (unsigned long)this << "\n";
#endif
   }
   else
   {
      OnStack = true;
#ifdef CLEAN_LIST
      cout << "Add to       clean-list " << (unsigned long)this << "\n";
#endif
      NextJanitor = JumpBase::jl->janitor; JumpBase::jl->janitor=this;
   }
}

Janitor::~Janitor()
{
   // expect the item to be deleted to be first on list
   // but must be prepared to search list
   if (OnStack)
   {
#ifdef CLEAN_LIST
      cout << "Delete from  clean-list " << (unsigned long)this << "\n";
#endif
      Janitor* lastjan = JumpBase::jl->janitor;
      if (this == lastjan) JumpBase::jl->janitor = NextJanitor;
      else
      {
         for (Janitor* jan = lastjan->NextJanitor; jan;
            jan = lastjan->NextJanitor)
         {
            if (jan==this)
               { lastjan->NextJanitor = jan->NextJanitor; return; }
            lastjan=jan;
         }

         Throw(BaseException(
"Cannot resolve memory linked list\nSee notes in myexcept.cpp for details\n"
         ));


// This message occurs when a call to ~Janitor() occurs, apparently
// without a corresponding call to Janitor(). This could happen if my
// way of deciding whether a constructor is being called by new
// fails.

// It may happen if you are using my simulated exceptions and also have
// your compiler s exceptions turned on.

// It can also happen if you have a class derived from Janitor
// which does not include a copy constructor [ eg X(const &X) ].
// Possibly also if delete is applied an object on the stack (ie not
// called by new). Otherwise, it is a bug in myexcept or your compiler.
// If you do not #define TEMPS_DESTROYED_QUICKLY you will get this
// error with Microsoft C 7.0. There are probably situations where
// you will get this when you do define TEMPS_DESTROYED_QUICKLY. This
// is a bug in MSC. Beware of "operator" statements for defining
// conversions; particularly for converting from a Base class to a
// Derived class.

// You may get away with simply deleting this error message and Throw
// statement if you can not find a better way of overcoming the
// problem. In any case please tell me if you get this error message,
// particularly for compilers apart from Microsoft C 7.0.


      }
   }
}

JumpItem* JumpBase::jl;              // will be set to zero
jmp_buf JumpBase::env;
bool Janitor::do_not_link;           // will be set to false


int JanitorInitializer::ref_count;

JanitorInitializer::JanitorInitializer()
{
   if (ref_count++ == 0) new JumpItem;
                                    // need JumpItem at head of list
}

#endif                              // end of SimulateExceptions

Tracer* Tracer::last;               // will be set to zero


void Terminate()
{
   cout << "\n\nThere has been an exception with no handler - exiting";
   const char* what = BaseException::what();
   if (what) cout << what << "\n";
   exit(1);
}



#ifdef DO_FREE_CHECK
// Routines for tracing whether new and delete calls are balanced

FreeCheckLink::FreeCheckLink() : next(FreeCheck::next)
   { FreeCheck::next = this; }

FCLClass::FCLClass(void* t, char* name) : ClassName(name) { ClassStore=t; }

FCLRealArray::FCLRealArray(void* t, char* o, int s)
  : Operation(o), size(s) { ClassStore=t; }

FCLIntArray::FCLIntArray(void* t, char* o, int s)
  : Operation(o), size(s) { ClassStore=t; }

FreeCheckLink* FreeCheck::next;
int FreeCheck::BadDelete;

void FCLClass::Report()
{ cout << "   " << ClassName << "   " << (unsigned long)ClassStore << "\n"; }

void FCLRealArray::Report()
{
   cout << "   " << Operation << "   " << (unsigned long)ClassStore <<
      "   " << size << "\n";
}

void FCLIntArray::Report()
{
   cout << "   " << Operation << "   " << (unsigned long)ClassStore <<
      "   " << size << "\n";
}

void FreeCheck::Register(void* t, char* name)
{
   FCLClass* f = new FCLClass(t,name);
   if (!f) { cout << "Out of memory in FreeCheck\n"; exit(1); }
#ifdef REG_DEREG
   cout << "Registering   " << name << "   " << (unsigned long)t << "\n";
#endif
}

void FreeCheck::RegisterR(void* t, char* o, int s)
{
   FCLRealArray* f = new FCLRealArray(t,o,s);
   if (!f) { cout << "Out of memory in FreeCheck\n"; exit(1); }
#ifdef REG_DEREG
   cout << o << "   " << s << "   " << (unsigned long)t << "\n";
#endif
}

void FreeCheck::RegisterI(void* t, char* o, int s)
{
   FCLIntArray* f = new FCLIntArray(t,o,s);
   if (!f) { cout << "Out of memory in FreeCheck\n"; exit(1); }
#ifdef REG_DEREG
   cout << o << "   " << s << "   " << (unsigned long)t << "\n";
#endif
}

void FreeCheck::DeRegister(void* t, char* name)
{
   FreeCheckLink* last = 0;
#ifdef REG_DEREG
   cout << "Deregistering " << name << "   " << (unsigned long)t << "\n";
#endif
   for (FreeCheckLink* fcl = next; fcl; fcl = fcl->next)
   {
      if (fcl->ClassStore==t)
      {
         if (last) last->next = fcl->next; else next = fcl->next;
         delete fcl; return;
      }
      last = fcl;
   }
   cout << "\nRequest to delete non-existent object of class and location:\n";
   cout << "   " << name << "   " << (unsigned long)t << "\n";
   BadDelete++;
   Tracer::PrintTrace();
   cout << "\n";
}

void FreeCheck::DeRegisterR(void* t, char* o, int s)
{
   FreeCheckLink* last = 0;
#ifdef REG_DEREG
   cout << o << "   " << s << "   " << (unsigned long)t << "\n";
#endif
   for (FreeCheckLink* fcl = next; fcl; fcl = fcl->next)
   {
      if (fcl->ClassStore==t)
      {
         if (last) last->next = fcl->next; else next = fcl->next;
         if (s >= 0 && ((FCLRealArray*)fcl)->size != s)
         {
            cout << "\nArray sizes do not agree:\n";
            cout << "   " << o << "   " << (unsigned long)t
               << "   " << ((FCLRealArray*)fcl)->size << "   " << s << "\n";
            Tracer::PrintTrace();
            cout << "\n";
         }
         delete fcl; return;
      }
      last = fcl;
   }
   cout << "\nRequest to delete non-existent real array:\n";
   cout << "   " << o << "   " << (unsigned long)t << "   " << s << "\n";
   BadDelete++;
   Tracer::PrintTrace();
   cout << "\n";
}

void FreeCheck::DeRegisterI(void* t, char* o, int s)
{
   FreeCheckLink* last = 0;
#ifdef REG_DEREG
   cout << o << "   " << s << "   " << (unsigned long)t << "\n";
#endif
   for (FreeCheckLink* fcl = next; fcl; fcl = fcl->next)
   {
      if (fcl->ClassStore==t)
      {
         if (last) last->next = fcl->next; else next = fcl->next;
         if (s >= 0 && ((FCLIntArray*)fcl)->size != s)
         {
            cout << "\nArray sizes do not agree:\n";
            cout << "   " << o << "   " << (unsigned long)t
               << "   " << ((FCLIntArray*)fcl)->size << "   " << s << "\n";
            Tracer::PrintTrace();
            cout << "\n";
         }
         delete fcl; return;
      }
      last = fcl;
   }
   cout << "\nRequest to delete non-existent int array:\n";
   cout << "   " << o << "   " << (unsigned long)t << "   " << s << "\n";
   BadDelete++;
   Tracer::PrintTrace();
   cout << "\n";
}

void FreeCheck::Status()
{
   if (next)
   {
      cout << "\nObjects of the following classes remain undeleted:\n";
      for (FreeCheckLink* fcl = next; fcl; fcl = fcl->next) fcl->Report();
      cout << "\n";
   }
   else cout << "\nNo objects remain undeleted\n\n";
   if (BadDelete)
   {
      cout << "\nThere were " << BadDelete << 
         " requests to delete non-existent items\n\n";
   }
}

#endif                            // end of DO_FREE_CHECK

// derived exception bodies

Logic_error::Logic_error(const char* a_what) : BaseException()
{
   Select = BaseException::Select;
   AddMessage("Logic error:- "); AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}

Runtime_error::Runtime_error(const char* a_what)
   : BaseException()
{
   Select = BaseException::Select;
   AddMessage("Runtime error:- "); AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}

Domain_error::Domain_error(const char* a_what) : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("domain error\n"); AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}

Invalid_argument::Invalid_argument(const char* a_what) : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("invalid argument\n"); AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}

Length_error::Length_error(const char* a_what) : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("length error\n"); AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}

Out_of_range::Out_of_range(const char* a_what) : Logic_error()
{
   Select = BaseException::Select;
   AddMessage("out of range\n"); AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}

//Bad_cast::Bad_cast(const char* a_what) : Logic_error()
//{
//   Select = BaseException::Select;
//   AddMessage("bad cast\n"); AddMessage(a_what);
//   if (a_what) Tracer::AddTrace();
//}

//Bad_typeid::Bad_typeid(const char* a_what) : Logic_error()
//{
//   Select = BaseException::Select;
//   AddMessage("bad type id.\n"); AddMessage(a_what);
//   if (a_what) Tracer::AddTrace();
//}

Range_error::Range_error(const char* a_what) : Runtime_error()
{
   Select = BaseException::Select;
   AddMessage("range error\n"); AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}

Overflow_error::Overflow_error(const char* a_what) : Runtime_error()
{
   Select = BaseException::Select;
   AddMessage("overflow error\n"); AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}

Bad_alloc::Bad_alloc(const char* a_what) : BaseException()
{
   Select = BaseException::Select;
   AddMessage("bad allocation\n"); AddMessage(a_what);
   if (a_what) Tracer::AddTrace();
}




unsigned long Logic_error::Select;
unsigned long Runtime_error::Select;
unsigned long Domain_error::Select;
unsigned long Invalid_argument::Select;
unsigned long Length_error::Select;
unsigned long Out_of_range::Select;
//unsigned long Bad_cast::Select;
//unsigned long Bad_typeid::Select;
unsigned long Range_error::Select;
unsigned long Overflow_error::Select;
unsigned long Bad_alloc::Select;

#ifdef use_namespace
}
#endif


