#ifndef __LARCVUTIL_RANGETEST_H__
#define __LARCVUTIL_RANGETEST_H__

#include "UniqueRangeSet.h"

namespace larcv {

  /// namespace to contain functions that involve util::Range and util::UniqueRangeSet
  namespace test {

    /// bench-mark func: for util::UniqueRangeSet::Insert function
    double bmark_insert(const size_t nentries);
    
    /// bench-mark func: for util::Range::Exclude
    double bmark_exclude(const size_t nentries);

    /// Error type for inspecting merging
    enum MergeErrorType_t {
      kMergeNoError,     ///< no error
      kMergeInclusive,   ///< failed to merge a large range w/ small 
      kMergeHalfOverlap, ///< failed to merge two overlapping range (but not inclusive)
      kMergeExclusive    ///< failed to perform util::UniqueRangeSet::Exclude properly
    };
    /// testing overlap
    MergeErrorType_t test_overlap();
  }
}
#endif
