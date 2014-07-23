#include "common.h"

/** Comparator necessary to use set<MemRange> */
struct MemRangeComparator
{
    bool
    operator()
    (
        const MemRange& a,
        const MemRange& b
    ) const
    {
        return !a.Intersects(b) && (a.Base() < b.Base());
    }



};

typedef set<MemRange, MemRangeComparator> MemRangeSet;

MemRangeSet WhitelistedAddressRanges;

//-------------------------- IS ADDRESS WHITE LISTED ---------------------------
bool
IsAddressWhitelisted
(
    ADDRINT Address
)
{
    MemRange r(Address, 1);
    WhitelistedAddressRanges.count(r);
    //    return binary_search(WhitelistedAddressRanges.begin(), WhitelistedAddressRanges.end(), r);
    return WhitelistedAddressRanges.end() != WhitelistedAddressRanges.find(r);
}



//--------------------------- IS IMAGE WHITE LISTED ----------------------------
bool
IsImageWhitelisted
(
    IMG img
)
{
    MemRange r(IMG_StartAddress(img), IMG_SizeMapped(img));
    //    return binary_search(WhitelistedAddressRanges.begin(), WhitelistedAddressRanges.end(), r);
    return WhitelistedAddressRanges.end() != WhitelistedAddressRanges.find(r);
}



//---------------------------- ADD WHITE LIST IMAGE ----------------------------
void
AddWhitelistImage
(
    IMG img
)
{
    MemRange r(IMG_StartAddress(img), IMG_SizeMapped(img));
    WhitelistedAddressRanges.insert(r);
}
