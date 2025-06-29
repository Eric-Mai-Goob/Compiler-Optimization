#include "optimizer.h"


void Optimizer(NodeList *funcdecls) {
/*
*************************************
     TODO: YOUR CODE HERE
*************************************
*/
     bool x1 = true;
     bool x2 = true;
     bool x3 = true;
     while(x1 || x2 || x3)
     {
          x1 = ConstantFolding(funcdecls);
          x2 = ConstProp(funcdecls);
          x3 = DeadAssign(funcdecls);
     }
}
