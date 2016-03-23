#ifndef __LARCV_RANGETEST_CXX__
#define __LARCV_RANGETEST_CXX__

#include "range_test.h"
#include <time.h>
namespace larcv {
  namespace test {

    //
    // Bench-marking function
    //
    double bmark_insert(const size_t nentries)
    {
      UniqueRangeSet<size_t> range_set;
      clock_t time_start = clock();
      for(size_t i=0; i<nentries; ++i)
	range_set.emplace(4*i,4*i+3);
      clock_t time_sum = clock() - time_start;
      return (range_set.size() == nentries ? ((double)time_sum)/CLOCKS_PER_SEC : -1);
    }
    
    double bmark_exclude(const size_t nentries)
    {
      UniqueRangeSet<double> sample;
      for(size_t i=0; i<nentries; ++i)
	sample.emplace((double)(4*i),(double)(4*i+1));

      auto const& start = (*sample.begin()).Start();
      auto const& end   = (*sample.rbegin()).End();

      clock_t time_start = clock();
      auto res = sample.Exclusive( (start - (end - start)*0.1),
				   (end + (end - start)*0.1) );
      clock_t time_end = clock();
      if( res.size() != (nentries + 1) ) return -1;
      return ((double)(time_end - time_start))/CLOCKS_PER_SEC;
    }

    MergeErrorType_t test_overlap()
    {
      // Test inclusive overlap
      Range<size_t> small(10,20);
      Range<size_t> large(0,30);

      small += large;
      if(small != large) return kMergeInclusive;

      // Test half-overlap
      Range<size_t> head(10,20);
      Range<size_t> tail(15,25);

      head += tail;
      if( head.Start() != 10 || head.End() !=25 ) return kMergeHalfOverlap;

      // Test UniqueRangeSet::Exclusive
      UniqueRangeSet<size_t> range_set;
      for(size_t i=0; i<10; ++i) 
	range_set.emplace(4*i, (4*i+4));

      auto start = (*range_set.begin()).Start();
      auto end   = (*range_set.rbegin()).End();

      auto ex_range_set = range_set.Exclusive(start,end);
      if(ex_range_set.size()) return kMergeExclusive;

      range_set.clear();
      ex_range_set.clear();

      for(size_t i=0; i<10; ++i)
	range_set.emplace(4*i, (4*i+3));

      start = (*range_set.begin()).Start();
      end   = (*range_set.rbegin()).End();

      ex_range_set = range_set.Exclusive(start,end);
      if( range_set.size() != (ex_range_set.size()+1) ) return kMergeExclusive;

      ex_range_set += range_set;
      if( ex_range_set.size() != 1 ) return kMergeExclusive;

      return kMergeNoError;
    }
  }  
}
#endif
