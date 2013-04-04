#ifndef PRODUCTTNORM_H_
#define PRODUCTTNORM_H_

#include "Tnorm.h"

class ProductTnorm : public Tnorm
{
public:
	ProductTnorm();
	virtual ~ProductTnorm();
	
	virtual Tnorm* clone() const { return new ProductTnorm(*this); }
	virtual double conjunction(double x, double y) const;
};

#endif /*PRODUCTTNORM_H_*/
