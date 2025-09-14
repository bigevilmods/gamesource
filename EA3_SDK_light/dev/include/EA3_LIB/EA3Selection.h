// (c) 1998-2010 Electronic Arts Inc.

#ifndef EA3Selection_H
#define EA3Selection_H
#include "ea3mem.h"
class EA3Base;

class EA3_EXPORT EA3Selection : public EA3Mem {
public:
  // Constructors, deconstructors
  EA3Selection();
  virtual ~EA3Selection();
  virtual void select(const EA3Base *object);
  virtual void unselect(const EA3Base *object);
  virtual bool isSelected(const EA3Base *object);
  virtual void getSelected(XbPArray<EA3Base *> &sel);
  virtual void getSelected(XbPArray<EA3Base *> &sel, int &maxToGet);
  virtual unsigned int numSelected() const;
  virtual void clear();
private:
  void *internals;
};

#endif
