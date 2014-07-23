#pragma once
/*
 * sortable-mixin.h
 *
 *  Created on: Dec 25, 2013
 */

/**
 * Mixin for sorting objects and object pointers in sets, etc.
 */
class SortableMixin
{
public:
    SortableMixin
    (
        int Order_ = 0
    ) : Order(Order_) {}



    int Order;

    bool
    operator<
    (
        SortableMixin &rhs
    )
    {
        return (Order < rhs.Order);
    }



    struct PtrComparator
    {
        bool
        operator()
        (
            const SortableMixin * a,
            const SortableMixin * b
        )
        {
            return (a->Order < b->Order);
        }



    };
};
