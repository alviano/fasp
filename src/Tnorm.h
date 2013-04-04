#ifndef TNORM_H_
#define TNORM_H_

class Tnorm
{
public:
	Tnorm();
	virtual ~Tnorm();
	
	virtual Tnorm* clone() const = 0;
	
	virtual double conjunction(double x, double y) const = 0;
	virtual double disjunction(double x, double y) const;
	virtual double negation(double x) const;
};

#endif /*TNORM_H_*/
