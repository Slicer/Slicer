//$$ sort.cpp                            Sorting

// Copyright (C) 1991,2,3,4: R B Davies

#define WANT_MATH

#include "include.h"

#include "newmatap.h"

#ifdef use_namespace
namespace NEWMAT {
#endif

#ifdef DO_REPORT
#define REPORT { static ExeCounter ExeCount(__LINE__,13); ++ExeCount; }
#else
#define REPORT {}
#endif

/******************************** Quick sort ********************************/

// Quicksort.
// Essentially the method described in Sedgewick s algorithms in C++
// My version is still partially recursive, unlike Segewick s, but the
// smallest segment of each split is used in the recursion, so it should
// not overlead the stack.

// If the process does not seems to be converging an exception is thrown.


#define DoSimpleSort 17            // when to switch to insert sort
#define MaxDepth 50                // maximum recursion depth

static void MyQuickSortDescending(Real* first, Real* last, int depth);
static void InsertionSortDescending(Real* first, const int length,
   int guard);
static Real SortThreeDescending(Real* a, Real* b, Real* c);

static void MyQuickSortAscending(Real* first, Real* last, int depth);
static void InsertionSortAscending(Real* first, const int length,
   int guard);


void sort_descending(GeneralMatrix& GM)
{
   REPORT
   Tracer et("sort_descending");

   Real* data = GM.Store(); int max = GM.Storage();

   if (max > DoSimpleSort) MyQuickSortDescending(data, data + max - 1, 0);
   InsertionSortDescending(data, max, DoSimpleSort);

}

static Real SortThreeDescending(Real* a, Real* b, Real* c)
{
   // sort *a, *b, *c; return *b; optimise for already sorted
   if (*a >= *b)
   {
      if (*b >= *c) { REPORT return *b; }
      else if (*a >= *c) { REPORT Real x = *c; *c = *b; *b = x; return x; }
      else { REPORT Real x = *a; *a = *c; *c = *b; *b = x; return x; }
   }
   else if (*c >= *b) { REPORT Real x = *c; *c = *a; *a = x; return *b; }
   else if (*a >= *c) { REPORT Real x = *a; *a = *b; *b = x; return x; }
   else { REPORT Real x = *c; *c = *a; *a = *b; *b = x; return x; }
}

static void InsertionSortDescending(Real* first, const int length,
   int guard)
// guard gives the length of the sequence to scan to find first
// element (eg = length)
{
   REPORT
   if (length <= 1) return;

   // scan for first element
   Real* f = first; Real v = *f; Real* h = f;
   if (guard > length) { REPORT guard = length; }
   int i = guard - 1;
   while (i--) if (v < *(++f)) { v = *f; h = f; }
   *h = *first; *first = v;

   // do the sort
   i = length - 1; f = first;
   while (i--)
   {
      Real* g = f++; h = f; v = *h;
      while (*g < v) *h-- = *g--;
      *h = v;
   }
}

static void MyQuickSortDescending(Real* first, Real* last, int depth)
{
   REPORT
   for (;;)
   {
      const int length = last - first + 1;
      if (length < DoSimpleSort) { REPORT return; }
      if (depth++ > MaxDepth)
         Throw(ConvergenceException("QuickSortDescending fails: "));
      Real* centre = first + length/2;
      const Real test = SortThreeDescending(first, centre, last);
      Real* f = first; Real* l = last;
      for (;;)
      {
         while (*(++f) > test) {}
         while (*(--l) < test) {}
         if (l <= f) break;
         const Real temp = *f; *f = *l; *l = temp;
      }
      if (f > centre)
         { REPORT MyQuickSortDescending(l+1, last, depth); last = f-1; }
      else { REPORT MyQuickSortDescending(first, f-1, depth); first = l+1; }
   }
}

void sort_ascending(GeneralMatrix& GM)
{
   REPORT
   Tracer et("sort_ascending");

   Real* data = GM.Store(); int max = GM.Storage();

   if (max > DoSimpleSort) MyQuickSortAscending(data, data + max - 1, 0);
   InsertionSortAscending(data, max, DoSimpleSort);

}

static void InsertionSortAscending(Real* first, const int length,
   int guard)
// guard gives the length of the sequence to scan to find first
// element (eg guard = length)
{
   REPORT
   if (length <= 1) return;

   // scan for first element
   Real* f = first; Real v = *f; Real* h = f;
   if (guard > length) { REPORT guard = length; }
   int i = guard - 1;
   while (i--) if (v > *(++f)) { v = *f; h = f; }
   *h = *first; *first = v;

   // do the sort
   i = length - 1; f = first;
   while (i--)
   {
      Real* g = f++; h = f; v = *h;
      while (*g > v) *h-- = *g--;
      *h = v;
   }
}
static void MyQuickSortAscending(Real* first, Real* last, int depth)
{
   REPORT
   for (;;)
   {
      const int length = last - first + 1;
      if (length < DoSimpleSort) { REPORT return; }
      if (depth++ > MaxDepth)
         Throw(ConvergenceException("QuickSortAscending fails: "));
      Real* centre = first + length/2;
      const Real test = SortThreeDescending(last, centre, first);
      Real* f = first; Real* l = last;
      for (;;)
      {
         while (*(++f) < test) {}
         while (*(--l) > test) {}
         if (l <= f) break;
         const Real temp = *f; *f = *l; *l = temp;
      }
      if (f > centre)
         { REPORT MyQuickSortAscending(l+1, last, depth); last = f-1; }
      else { REPORT MyQuickSortAscending(first, f-1, depth); first = l+1; }
   }
}

//********* sort diagonal matrix & rearrange matrix columns ****************

// used by SVD

// these are for sorting singular values - should be updated with faster
// sorts that handle exchange of columns better
// however time is probably not significant compared with SVD time

void SortSV(DiagonalMatrix& D, Matrix& U, bool ascending)
{
   REPORT
   Tracer trace("SortSV_DU");
   int m = U.Nrows(); int n = U.Ncols();
   if (n != D.Nrows()) Throw(IncompatibleDimensionsException(D,U));
   Real* u = U.Store();
   for (int i=0; i<n; i++)
   {
      int k = i; Real p = D.element(i);
      if (ascending)
      {
         for (int j=i+1; j<n; j++)
            { if (D.element(j) < p) { k = j; p = D.element(j); } }
      }
      else
      {
         for (int j=i+1; j<n; j++)
         { if (D.element(j) > p) { k = j; p = D.element(j); } }
      }
      if (k != i)
      {
         D.element(k) = D.element(i); D.element(i) = p; int j = m;
         Real* uji = u + i; Real* ujk = u + k;
         if (j) for(;;)
         {
            p = *uji; *uji = *ujk; *ujk = p;
            if (!(--j)) break;
            uji += n; ujk += n;
         }
      }
   }
}

void SortSV(DiagonalMatrix& D, Matrix& U, Matrix& V, bool ascending)
{
   REPORT
   Tracer trace("SortSV_DUV");
   int mu = U.Nrows(); int mv = V.Nrows(); int n = D.Nrows();
   if (n != U.Ncols()) Throw(IncompatibleDimensionsException(D,U));
   if (n != V.Ncols()) Throw(IncompatibleDimensionsException(D,V));
   Real* u = U.Store(); Real* v = V.Store();
   for (int i=0; i<n; i++)
   {
      int k = i; Real p = D.element(i);
      if (ascending)
      {
         for (int j=i+1; j<n; j++)
            { if (D.element(j) < p) { k = j; p = D.element(j); } }
      }
      else
      {
         for (int j=i+1; j<n; j++)
         { if (D.element(j) > p) { k = j; p = D.element(j); } }
      }
      if (k != i)
      {
         D.element(k) = D.element(i); D.element(i) = p;
         Real* uji = u + i; Real* ujk = u + k;
         Real* vji = v + i; Real* vjk = v + k;
         int j = mu;
         if (j) for(;;)
         {
            p = *uji; *uji = *ujk; *ujk = p; if (!(--j)) break;
            uji += n; ujk += n;
         }
         j = mv;
         if (j) for(;;)
         {
            p = *vji; *vji = *vjk; *vjk = p; if (!(--j)) break;
            vji += n; vjk += n;
         }
      }
   }
}




#ifdef use_namespace
}
#endif

