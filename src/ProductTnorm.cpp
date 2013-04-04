#include "ProductTnorm.h"

#include <cassert>

ProductTnorm::ProductTnorm()
{
}

ProductTnorm::~ProductTnorm()
{
}

double ProductTnorm::conjunction(double x, double y) const {
    assert(0 <= x && x <= 1);
    assert(0 <= y && y <= 1);
    return x * y;
}
