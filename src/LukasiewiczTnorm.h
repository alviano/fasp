#ifndef LUKASIEWICZTNORM_H_
#define LUKASIEWICZTNORM_H_

#include "Tnorm.h"

class LukasiewiczTnorm : public Tnorm
{
public:
	LukasiewiczTnorm();
	virtual ~LukasiewiczTnorm();
	
	virtual Tnorm* clone() const;
	virtual double conjunction(double x, double y) const;
};

#endif /*LUKASIEWICZTNORM_H_*/
