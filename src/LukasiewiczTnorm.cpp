#include "LukasiewiczTnorm.h"

#include <cassert>
#include <cmath>

LukasiewiczTnorm::LukasiewiczTnorm()
{
}

LukasiewiczTnorm::~LukasiewiczTnorm()
{
}

Tnorm* LukasiewiczTnorm::clone() const {
    return new LukasiewiczTnorm(*this);
}

double LukasiewiczTnorm::conjunction(double x, double y) const {
    assert(0 <= x && x <= 1);
    assert(0 <= y && y <= 1);
    return fmax(x + y - 1, 0);
}
