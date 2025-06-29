/*
********************************************************************************
  CONSTPROP.C : IMPLEMENT THE DOWNSTREAM CONSTANT PROPOGATION OPTIMIZATION HERE
*********************************************************************************
*/

#include "constprop.h"

refConst *lastNode, *headNode;
/*
*************************************************************************************
   YOUR CODE IS TO BE FILLED IN THE GIVEN TODO BLANKS. YOU CAN CHOOSE TO USE ALL
   UTILITY FUNCTIONS OR NONE. YOU CAN ADD NEW FUNCTIONS. BUT DO NOT FORGET TO
   DECLARE THEM IN THE HEADER FILE.
**************************************************************************************
*/

/*
***********************************************************************
  FUNCTION TO FREE THE CONSTANTS-ASSOCIATED VARIABLES LIST
************************************************************************
*/
void FreeConstList()
{
   refConst* tmp;
   while (headNode != NULL)
    {
       tmp = headNode;
       headNode = headNode->next;
       free(tmp);
    }

}

/*
*************************************************************************
  FUNCTION TO ADD A CONSTANT VALUE AND THE ASSOCIATED VARIABLE TO THE LIST
**************************************************************************
*/
void UpdateConstList(char* name, long val) {
    refConst* node = malloc(sizeof(refConst));
    if (node == NULL) return;
    node->name = name;
    node->val = val;
    node->next = NULL;
    if (headNode == NULL) {
        lastNode = node;
        headNode = node;
    }
    else {
        lastNode->next = node;
        lastNode = node;
    }
}

/*
*****************************************************************************
  FUNCTION TO LOOKUP IF A CONSTANT ASSOCIATED VARIABLE IS ALREADY IN THE LIST
******************************************************************************
*/
refConst* LookupConstList(char* name) {
    refConst *node;
    node = headNode; 
    while (node != NULL){
        if (!strcmp(name, node->name))
            return node;
        node = node->next;
    }
    return NULL;
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE CHANGES/ADD AUXILLIARY FUNCTIONS BELOW THIS LINE
**********************************************************************************************************************************
*/


/*
************************************************************************************
  THIS FUNCTION IS MEANT TO UPDATE THE CONSTANT LIST WITH THE ASSOCIATED VARIABLE
  AND CONSTANT VALUE WHEN ONE IS SEEN. IT SHOULD ALSO PROPOGATE THE CONSTANTS WHEN 
  WHEN APPLICABLE. YOU CAN ADD A NEW FUNCTION IF YOU WISH TO MODULARIZE BETTER.
*************************************************************************************
*/
void TrackConst(NodeList* statements) {
  Node* node;
  while (statements != NULL) 
  {
    node = statements->node;
    /*
      ****************************************
              TODO : YOUR CODE HERE
      ****************************************
    */
    // part 1: update the constant table
    // ONLY case: long a = 1
    // ASSIGN, CONSTANT on right, name is the name of the variable
    if(node->stmtCode == ASSIGN && node->right->exprCode == CONSTANT)
    {
      UpdateConstList(node->name, node->right->value);
    }

    // part 2: propogate constants
    // see variable, look up variable in the constant table
    // if variable exists, replace with constant

    // RETURN is the simple case, only 1 value allowed on the right, replace if it is a variable
    if(node->stmtCode == RETURN && node->left->exprCode == VARIABLE)
    {
      if(LookupConstList(node->left->name) != NULL)
      {
        long num = LookupConstList(node->left->name)->val;
        FreeVariable(node->left);
        node->left = CreateNumber(num);
        madeChange = true;
      }
    }
    // ASSIGN is the other statement can be propogated
    else if(node->stmtCode == ASSIGN)
    {
      // can have a VARIABLE on the right, replace if in list
      if(node->right->exprCode == VARIABLE && LookupConstList(node->right->name) != NULL)
      {
        long num = LookupConstList(node->right->name)->val;
        FreeVariable(node->right);
        node->right = CreateNumber(num);
        madeChange = true;
      }

      // or it can have an EXPRESSION, which could be an OPERATION, which could be A FUNCTIONCALL or a normal operation
      if(node->right->exprCode == OPERATION)
      {
        // for FUNCTIONCALL, propagate the args if we can
        if(node->right->opCode == FUNCTIONCALL)
        {
          NodeList* args = node->right->arguments;
          while(args != NULL)
          {
            if(args->node->exprCode == VARIABLE && LookupConstList(args->node->name) != NULL)
            {
              long num = LookupConstList(args->node->name)->val;
              FreeVariable(args->node);
              args->node = CreateNumber(num);
              madeChange = true;
            }
            args = args->next;
          }
        }
        // for everything else, just check left and right
        else
        {
          if(node->right->left->exprCode == VARIABLE && LookupConstList(node->right->left->name) != NULL)
          {
            long num = LookupConstList(node->right->left->name)->val;
            FreeVariable(node->right->left);
            node->right->left = CreateNumber(num);
            madeChange = true;
          }
          // NEGATE doesnt have a right
          if(node->right->right != NULL && node->right->right->exprCode == VARIABLE && LookupConstList(node->right->right->name) != NULL)
          {
            long num = LookupConstList(node->right->right->name)->val;
            FreeVariable(node->right->right);
            node->right->right = CreateNumber(num);
            madeChange = true;
          } 
        }
      }    
    }

    statements = statements->next;
  }

  FreeConstList();
}


bool ConstProp(NodeList* funcdecls) {
  while (funcdecls != NULL){
    /*
    ****************************************
            TODO : YOUR CODE HERE
    ****************************************
    */
    TrackConst(funcdecls->node->statements);
    funcdecls = funcdecls->next;
  }
  return madeChange;
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE CHANGES/ADD AUXILLIARY FUNCTIONS ABOVE THIS LINE
**********************************************************************************************************************************
*/
