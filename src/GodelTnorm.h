#ifndef GODELTNORM_H_
#define GODELTNORM_H_

#include "Tnorm.h"

class GodelTnorm : public Tnorm
{
public:
	GodelTnorm();
	virtual ~GodelTnorm();
	
	virtual Tnorm* clone() const { return new GodelTnorm(*this); }
	virtual double conjunction(double x, double y) const;
};

#endif /*GODELTNORM_H_*/
