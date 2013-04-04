#include "GodelTnorm.h"

#include <cmath>
#include <cassert>

GodelTnorm::GodelTnorm()
{
}

GodelTnorm::~GodelTnorm()
{
}

double GodelTnorm::conjunction(double x, double y) const {
    assert(0 <= x && x <= 1);
    assert(0 <= y && y <= 1);
    return fmin(x, y);
}
