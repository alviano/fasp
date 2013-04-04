#include "Tnorm.h"

#include <cassert>

Tnorm::Tnorm()
{
}

Tnorm::~Tnorm()
{
}

double Tnorm::disjunction(double x, double y) const {
    assert(0 <= x && x <= 1);
    return negation(
        conjunction(
            negation(x),
            negation(y)
        )
    );
}

double Tnorm::negation(double x) const {
    assert(0 <= x && x <= 1);
    return 1 - x;
}
