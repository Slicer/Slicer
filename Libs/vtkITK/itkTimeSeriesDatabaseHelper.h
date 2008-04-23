#ifndef __itkTimeSeriesDatabaseHelper_h
#define __itkTimeSeriesDatabaseHelper_h
#include <list>
#include <iostream>
#include <map>
#include <string>
#include <cstdarg>
#include <cassert>

namespace itk {
  namespace TimeSeriesDatabaseHelper {
    // Some useful classes
    /*
     * counted_ptr - simple reference counted pointer.
     *
     * The is a non-intrusive implementation that allocates an additional
     * int and pointer for every counted object.
     */
    /* For ANSI-challenged compilers, you may want to #define
     * NO_MEMBER_TEMPLATES or explicit */
#if 0
#define NO_MEMBER_TEMPLATES
    template <class X> class counted_ptr
      {
      public:
        typedef X element_type;
        
        explicit counted_ptr(X* p = 0) // allocate a new counter
          : itsCounter(0) {if (p) itsCounter = new counter(p);}
        ~counted_ptr()
          {release();}
        counted_ptr(const counted_ptr& r) throw()
          {acquire(r.itsCounter);}
        counted_ptr& operator=(const counted_ptr& r)
          {
            if (this != &r) {
              release();
              acquire(r.itsCounter);
            }
            return *this;
          }
        
#ifndef NO_MEMBER_TEMPLATES
        template <class Y> friend class counted_ptr<Y>;
        template <class Y> counted_ptr(const counted_ptr<Y>& r) throw()
          {acquire(r.itsCounter);}
        template <class Y> counted_ptr& operator=(const counted_ptr<Y>& r)
          {
            if (this != &r) {
              release();
              acquire(r.itsCounter);
            }
            return *this;
          }
#endif // NO_MEMBER_TEMPLATES
        
        X& operator*()  const throw()   {return *itsCounter->ptr;}
        X* operator->() const throw()   {return itsCounter->ptr;}
        X* get()        const throw()   {return itsCounter ? itsCounter->ptr : 0;}
        bool unique()   const throw()
        {return (itsCounter ? itsCounter->count == 1 : true);}
        
      private:
        
        struct counter {
        counter(X* p = 0, unsigned c = 1) : ptr(p), count(c) {}
          X*          ptr;
          unsigned    count;
        }* itsCounter;
        
        void acquire(counter* c) throw()
        { // increment the count
          itsCounter = c;
          if (c) ++c->count;
        }

        void release()
        { // decrement the count, delete if it is 0
          if (itsCounter) {
            if (--itsCounter->count == 0) {
              delete itsCounter->ptr;
              delete itsCounter;
            }
            itsCounter = 0;
          }
        }
      };

#endif
    // LRU Cache

    using namespace std;


#ifdef NDEBUG
#define IF_DEBUG(x)
#else
#define IF_DEBUG(x) x
#endif 


    /// A cache class. 
    ///
    /// Stores cached values for keys. Yseful when the value is
    /// a result of some complex calculation on key that we want
    /// to avoid doing repeatedly.
    ///
    /// The basic interface is insert() for insertions and find() 
    /// to look up items. key_type must have operator < defined.
    ///
    /// Has a maximal size (amount of elements - key/value pairs) 
    /// and employs a LRU (Least Recently Used) removal policy when 
    /// that maximum is exceeded, meaning that the item which was 
    /// accesses least recently is removed.
    ///
    /// The class also keeps statistics, cache hit/miss
    /// rate that may be useful while debugging. The statistics
    /// counting works only if NDEBUG is not defined.
    ///
    template <typename key_type, typename value_type>
      class LRUCache
    {
    public:
      /// Create a new cache.
      ///
      /// \param maxsize_ maximal size of the cache
      ///
    LRUCache(unsigned maxsize_ = 100) 
      : maxsize(maxsize_) 
      {
      }

      void set_maxsize ( unsigned maxsize_ ) {
        maxsize = maxsize_;
      }

      unsigned get_maxsize () {
        return maxsize;
      }

      ~LRUCache() 
        {
          clear();
        }

      /// How many elements are currently stored in the cache ?
      ///
      size_t size()
      {
        return lru_list.size();
      }

      /// Is the cache empty ?
      ///
      bool empty()
      {
        return lru_list.empty();
      }

      /// Clear the cache.
      ///
      void clear()
      {
        lru_list.clear();
        table.clear();
        IF_DEBUG(stats.clear());
      }

      /// Inserts a key/value pair to the cache.
      ///
      void insert(const key_type& key, const value_type& value)
      {
        // Is the key already in the cache ?
        // Note: find() is used intentionally - if
        // an element gets updated, it should be moved
        // to be MRU.
        //
        value_type* valptr = find(key);

        // Found ? 
        //
        if (valptr)
          {
            // Update the value.
            //
            *valptr = value;
          }
        else
          { 
            // Add it to the table and to the front of the
            // list (mark it MRU).
            // 
            lru_list.push_front(key);
            cached_value cv(value, lru_list.begin());
            table.insert(make_pair(key, cv));

            // If the maximal size was exceeded, clean up
            // LRU element.
            //
            if (lru_list.size() > maxsize)
              {
                key_type lru_key = lru_list.back();
                table.erase(lru_key);
                lru_list.pop_back();

                IF_DEBUG(stats.removed++);
              }
          }
      }

      /// Looks for a key in the cache.
      ///
      /// Returns a pointer to the value if found, 0 otherwise.
      ///
      /// Warning: a bare pointer is returned, at the user's
      /// risk. The pointer may become invalid at some time,
      /// so it should be used immediately.
      ///
      value_type* find(const key_type& key)
      {
        table_iter ti = table.find(key);

        IF_DEBUG(stats.finds++);

        if (ti == table.end())
          return 0;

        IF_DEBUG(stats.finds_hit++);

        // An access moves the element to the front of
        // the list (marking it MRU).
        //
        list_iter li = ti->second.cache_i;
        lru_list.splice(lru_list.begin(), lru_list, li);

        return &(ti->second.value);
      }

      /// Dumps the cache to output.
      ///
      /// Useful for debugging. Expects key/value types to have 
      /// an output operator (<<) defined.
      ///
      void debug_dump(ostream& ostr = cerr) 
      {
        ostr << "Debug dump of LRUCache\n";
        ostr << "-------------------\n\n";

        if (lru_list.empty())
          {
            ostr << "The cache is empty\n";
          }

        ostr << "Sorted from MRU to LRU:\n\n";

        for (list_iter i = lru_list.begin(); i != lru_list.end(); ++i)
          {
            ostr << "Key: " << *i << endl;

            table_iter ti = table.find(*i);
            assert(ti != table.end());

            ostr << "Value: " << ti->second.value << "\n|\n";
          }

        ostr << endl;
      }

      /// Prints cache statistics.
      ///
      /// Only in debug mode (NDEBUG not defined).
      ///
      void statistics(ostream& ostr = cerr) const
      {
#ifndef NDEBUG
        ostr << "LRUCache statistics\n";
        ostr << "----------------\n\n";
        ostr << format_str("Max size: %ld, cur size %ld. Cache is %5.2lf%% full\n\n",
                           maxsize, lru_list.size(), 100.0 * lru_list.size() / maxsize);
        ostr << format_str("Lookups:  %7ld\nHits:     %7ld\nHit rate: %7.2lf%%\n\n",
                           stats.finds, stats.finds_hit, 100.0 * stats.finds_hit / (stats.finds+1e-15));
        ostr << format_str("Items removed by LRU: %ld\n\n", stats.removed);
#endif // NDEBUG
      }

 #ifndef NDEBUG
    /// Works like sprintf, but returns the resulting string in a 
    /// memory-safe manner.
    ///
    string format_str(char* format, ...) const
    {
      va_list arglist;
      va_start(arglist, format);
      char* buf = new char[10000];

      vsprintf(buf, format, arglist);
      string ret = buf;
      delete [] buf;
      return ret;
    }
#endif // NDEBUG

    private:
      typedef typename list<key_type>::iterator list_iter;

      struct cached_value
      {
      cached_value(value_type value_, list_iter cache_i_)
      : value(value_), cache_i(cache_i_)
        {
        }

        value_type value;
        list_iter cache_i;
      };

      typedef typename map<key_type, cached_value>::iterator table_iter;

      /// Maximal cache size.
      ///
      unsigned maxsize;

      /// Orders keys by time of last access. MRU (most recently used)
      /// in the front, and LRU (least recently used) in the back.
      ///
      /// Note: the elements in lru_list and table are always
      /// the same.
      ///
      list<key_type> lru_list;

      /// Table storing cache elements for quick access.
      ///
      map<key_type, cached_value> table;

#ifndef NDEBUG

      struct cache_statistics
      {
        cache_statistics()
        {
          clear();
        }

        void clear()
        {
          finds = finds_hit = removed = 0;
        }

        typedef unsigned long ulong;

        ulong finds;
        ulong finds_hit;
        ulong removed;
      } stats;
#endif
    };
  }
}
#endif
