/*
***********************************************************************
  CODEGEN.C : IMPLEMENT CODE GENERATION HERE
************************************************************************
*/
#include "codegen.h"

int argCounter;
int lastUsedOffset;
char lastOffsetUsed[100];
FILE *fptr;
regInfo *regList, *regHead, *regLast;
varStoreInfo *varList, *varHead, *varLast;

/*
*************************************************************************************
   YOUR CODE IS TO BE FILLED IN THE GIVEN TODO BLANKS. YOU CAN CHOOSE TO USE ALL
   UTILITY FUNCTIONS OR NONE. YOU CAN ADD NEW FUNCTIONS. BUT DO NOT FORGET TO
   DECLARE THEM IN THE HEADER FILE.
**************************************************************************************
*/

/*
***********************************************************************
  FUNCTION TO INITIALIZE THE ASSEMBLY FILE WITH FUNCTION DETAILS
************************************************************************
*/
void InitAsm(char* funcName) {
    fprintf(fptr, "\n.globl %s", funcName);
    fprintf(fptr, "\n%s:", funcName); 

    // Init stack and base ptr
    fprintf(fptr, "\npushq %%rbp");  
    fprintf(fptr, "\nmovq %%rsp, %%rbp"); 
}

/*
***************************************************************************
   FUNCTION TO WRITE THE RETURNING CODE OF A FUNCTION IN THE ASSEMBLY FILE
****************************************************************************
*/
void RetAsm() {
    fprintf(fptr,"\npopq  %%rbp");
    fprintf(fptr, "\nretq\n");
} 

/*
***************************************************************************
  FUNCTION TO CONVERT OFFSET FROM LONG TO CHAR STRING 
****************************************************************************
*/
void LongToCharOffset() {
     lastUsedOffset = lastUsedOffset - 8;
     snprintf(lastOffsetUsed, 100,"%d", lastUsedOffset);
     strcat(lastOffsetUsed,"(%rbp)");
}

/*
***************************************************************************
  FUNCTION TO CONVERT CONSTANT VALUE TO CHAR STRING
****************************************************************************
*/
void ProcessConstant(Node* opNode) {
     char value[10];
     LongToCharOffset();
     snprintf(value, 10,"%ld", opNode->value);
     char str[100];
     snprintf(str, 100,"%d", lastUsedOffset);
     strcat(str,"(%rbp)");
     AddVarInfo("", str, opNode->value, true);
     fprintf(fptr, "\nmovq  $%s, %s", value, str);
}

/*
***************************************************************************
  FUNCTION TO SAVE VALUE IN ACCUMULATOR (RAX)
****************************************************************************
*/
void SaveValInRax(char* name) {
    char *tempReg;
    tempReg = GetNextAvailReg(true);
    if (!(strcmp(tempReg, "NoReg"))) {
        LongToCharOffset();
        fprintf(fptr, "\n movq %%rax, %s", lastOffsetUsed);
        UpdateVarInfo(name, lastOffsetUsed, INVAL, false);
        UpdateRegInfo("%rax", 1);
    }
    else {
        fprintf(fptr, "\nmovq %%rax, %s", tempReg);
        UpdateRegInfo(tempReg, 0);
        UpdateVarInfo(name, tempReg, INVAL, false);
        UpdateRegInfo("%rax", 1);
    }
}



/*
***********************************************************************
  FUNCTION TO ADD VARIABLE INFORMATION TO THE VARIABLE INFO LIST
************************************************************************
*/
void AddVarInfo(char* varName, char* location, long val, bool isConst) {
   varStoreInfo* node = malloc(sizeof(varStoreInfo));
   node->varName = varName;
   node->value = val;
   strcpy(node->location,location);
   node->isConst = isConst;
   node->next = NULL;
   node->prev = varLast;
   if (varHead == NULL) {
       varHead = node;
       varLast = node;;
       varList = node;
   } else {
       //node->prev = varLast;
       varLast->next = node;
       varLast = varLast->next;
   }
   varList = varHead;
}

/*
***********************************************************************
  FUNCTION TO FREE THE VARIABLE INFORMATION LIST
************************************************************************
*/
void FreeVarList()
{  
   varStoreInfo* tmp;
   while (varHead != NULL)
    {  
       tmp = varHead;
       varHead = varHead->next;
       free(tmp);
    }
   varLast = NULL;
}

/*
***********************************************************************
  FUNCTION TO LOOKUP VARIABLE INFORMATION FROM THE VARINFO LIST
************************************************************************
*/
char* LookUpVarInfo(char* name, long val) {
    varList = varLast;
    if (varList == NULL) printf("NULL varlist");
    while (varList != NULL) {
        if (varList->isConst == true) {
            if (varList->value == val) return varList->location;
        }
        else {
            if (!strcmp(name,varList->varName)) return varList->location;
        }
        varList = varList->prev;
    }
    varList = varHead;
    return "";
}

/*
***********************************************************************
  FUNCTION TO UPDATE VARIABLE INFORMATION 
************************************************************************
*/
void UpdateVarInfo(char* varName, char* location, long val, bool isConst) {
  
   if (!(strcmp(LookUpVarInfo(varName, val), ""))) {
       AddVarInfo(varName, location, val, isConst);
   }
   else {
       varList = varHead;
       if (varList == NULL) printf("NULL varlist");
       while (varList != NULL) {
           if (!strcmp(varList->varName,varName)) {
               varList->value = val;
               strcpy(varList->location,location);
               varList->isConst = isConst;
               break;
        }
        varList = varList->next;
       }
    }
    varList = varHead;
}

/*
***********************************************************************
  FUNCTION TO PRINT THE VARIABLE INFORMATION LIST
************************************************************************
*/
void PrintVarListInfo() {
    varList = varHead;
    if (varList == NULL) printf("NULL varlist");
    while (varList != NULL) {
        if (!varList->isConst) {
            printf("\t %s : %s", varList->varName, varList->location);
        }
        else {
            printf("\t %ld : %s", varList->value, varList->location);
        }
        varList = varList->next;
    }
    varList = varHead;
}

/*
***********************************************************************
  FUNCTION TO ADD NEW REGISTER INFORMATION TO THE REGISTER INFO LIST
************************************************************************
*/
void AddRegInfo(char* name, int avail) {

   regInfo* node = malloc(sizeof(regInfo));
   node->regName = name;
   node->avail = avail;
   node->next = NULL; 

   if (regHead == NULL) {
       regHead = node;
       regList = node;
       regLast = node;
   } else {
       regLast->next = node;
       regLast = node;
   }
   regList = regHead;
}

/*
***********************************************************************
  FUNCTION TO FREE REGISTER INFORMATION LIST
************************************************************************
*/
void FreeRegList()
{  
   regInfo* tmp;
   while (regHead != NULL)
    {  
       tmp = regHead;
       regHead = regHead->next;
       free(tmp);
    }

}

/*
***********************************************************************
  FUNCTION TO UPDATE THE AVAILIBILITY OF REGISTERS IN THE REG INFO LIST
************************************************************************
*/
void UpdateRegInfo(char* regName, int avail) {
    while (regList != NULL) {
        if (regName == regList->regName) {
            regList->avail = avail;
        }
        regList = regList->next;
    }
    regList = regHead;
}

/*
***********************************************************************
  FUNCTION TO RETURN THE NEXT AVAILABLE REGISTER
************************************************************************
*/
char* GetNextAvailReg(bool noAcc) {
    regList = regHead;
    if (regList == NULL) printf("NULL reglist");
    while (regList != NULL) {
        if (regList->avail == 1) {
            if (!noAcc) return regList->regName;
            // if not rax and dont return accumulator set to true, return the other reg
            // if rax and noAcc == true, skip to next avail
            if (noAcc && strcmp(regList->regName, "%rax")) { 
                return regList->regName;
            }
        }
        regList = regList->next;
    }
    regList = regHead;
    return "NoReg";
}

/*
***********************************************************************
  FUNCTION TO DETERMINE IF ANY REGISTER APART FROM OR INCLUDING 
  THE ACCUMULATOR(RAX) IS AVAILABLE
************************************************************************
*/
int IfAvailReg(bool noAcc) {
    regList = regHead;
    if (regList == NULL) printf("NULL reglist");
    while (regList != NULL) {
        if (regList->avail == 1) {
            // registers available
            if (!noAcc) return 1;
            if (noAcc && strcmp(regList->regName, "%rax")) {
                return 1;
            }
        }
        regList = regList->next;
    }
    regList = regHead;
    return 0;
}

/*
***********************************************************************
  FUNCTION TO DETERMINE IF A SPECIFIC REGISTER IS AVAILABLE
************************************************************************
*/
bool IsAvailReg(char* name) {
    regList = regHead;
    if (regList == NULL) printf("NULL reglist");
    while (regList != NULL) {
        if (!strcmp(regList->regName, name)) {
           if (regList->avail == 1) {
               return true;
           } 
        }
        regList = regList->next;
    }
    regList = regHead;
    return false;
}

/*
***********************************************************************
  FUNCTION TO PRINT THE REGISTER INFORMATION
************************************************************************
*/
void PrintRegListInfo() {
    regList = regHead;
    if (regList == NULL) printf("NULL reglist");
    while (regList != NULL) {
        printf("\t %s : %d", regList->regName, regList->avail);
        regList = regList->next;
    }
    regList = regHead;
}

/*
***********************************************************************
  FUNCTION TO CREATE THE REGISTER LIST
************************************************************************
*/
void CreateRegList() {
    // Create the initial reglist which can be used to store variables.
    // 4 general purpose registers : AX, BX, CX, DX
    // 4 special purpose : SP, BP, SI , DI. 
    // Other registers: r8, r9
    // You need to decide which registers you will add in the register list 
    // use. Can you use all of the above registers?
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ***************************************
    */

    // add all registers and set availiabliity to true

}



/*
***********************************************************************
  THIS FUNCTION IS MEANT TO PUT THE FUNCTION ARGUMENTS ON STACK
************************************************************************
*/
int PushArgOnStack(NodeList* arguments) {
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ****************************************
    */
    int counter = 0;
    while (arguments != NULL) {
        /*
        ***********************************************************************
                TODO : YOUR CODE HERE
        THINK ABOUT WHERE EACH ARGUMENT COMES FROM. EXAMPLE WHERE IS THE 
        FIRST ARGUMENT OF A FUNCTION STORED.
        ************************************************************************
        */ 
        // REGISTER ORDER rdi, rsi, rdx, rcx, r8, r9

        LongToCharOffset();
        if(arguments->node->exprCode == CONSTANT)
        {
           AddVarInfo("", lastOffsetUsed, arguments->node->value, true);
        }
        else
        {
            AddVarInfo(arguments->node->name, lastOffsetUsed, INVAL, false);
        }
        // movq register lastoffsetused
        if(counter == 0)
        {
            fprintf(fptr, "\nmovq %%rdi, %s", lastOffsetUsed);
        }
        else if(counter == 1)
        {
            fprintf(fptr, "\nmovq %%rsi, %s", lastOffsetUsed);
        }
        else if(counter == 2)
        {
            fprintf(fptr, "\nmovq %%rdx, %s", lastOffsetUsed);
        }
        else if(counter == 3)
        {
            fprintf(fptr, "\nmovq %%rcx, %s", lastOffsetUsed);
        }
        else if(counter == 4)
        {
            fprintf(fptr, "\nmovq %%r8, %s", lastOffsetUsed);
        }
        else if(counter == 5)
        {
            fprintf(fptr, "\nmovq %%r9, %s", lastOffsetUsed);
        }

        counter += 1;

        arguments = arguments->next;
    }

    return argCounter;
}


/*
*************************************************************************
  THIS FUNCTION IS MEANT TO GET THE FUNCTION ARGUMENTS FROM THE  STACK
**************************************************************************
*/
void PopArgFromStack(NodeList* arguments) {
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ****************************************
    */

    while (arguments != NULL) {
        /*
        ***********************************************************************
                TODO : YOUR CODE HERE
        THINK ABOUT WHERE EACH ARGUMENT COMES FROM. EXAMPLE WHERE IS THE
        FIRST ARGUMENT OF A FUNCTION STORED AND WHERE SHOULD IT BE EXTRACTED
        FROM AND STORED TO..
        ************************************************************************
        */

        arguments = arguments->next;
    }
    
}

/*
 ***********************************************************************
  THIS FUNCTION IS MEANT TO PROCESS EACH CODE STATEMENT AND GENERATE 
  ASSEMBLY FOR IT. 
  TIP: YOU CAN MODULARIZE BETTER AND ADD NEW SMALLER FUNCTIONS IF YOU 
  WANT THAT CAN BE CALLED FROM HERE.
 ************************************************************************
 */  
void ProcessStatements(NodeList* statements) {
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ****************************************
    */
    while (statements != NULL) {
        /*
        ****************************************
                TODO : YOUR CODE HERE
        ****************************************
        */          
        if(statements->node->stmtCode == RETURN)
        {
            // move value in %rax to return
            if(statements->node->left->exprCode == CONSTANT)
            {
                fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->left->value);
            }
            else if(statements->node->left->exprCode == VARIABLE)
            {
                fprintf(fptr, "\nmovq %s, %%rax", LookUpVarInfo(statements->node->left->name, INVAL));
            }
            RetAsm();
        }
        // every other statement is an ASSIGN
        else
        {
            if(statements->node->right->opCode == O_NONE)
            {
                if(statements->node->right->exprCode == CONSTANT)
                {
                    ProcessConstant(statements->node->right);
                }
                else
                {
                    char* location = LookUpVarInfo(statements->node->right->name, INVAL);

                    LongToCharOffset();
                    UpdateVarInfo(statements->node->name, lastOffsetUsed, INVAL, false);

                    fprintf(fptr, "\nmovq %s, %s", location, lastOffsetUsed);
                }
            }
            else if(statements->node->right->opCode == FUNCTIONCALL)
            {
                LongToCharOffset();
                UpdateVarInfo(statements->node->name, lastOffsetUsed, INVAL, false);

                NodeList* args = statements->node->right->arguments;
                int counter = 0;

                while(args != NULL)
                {
                    if(args->node->exprCode == CONSTANT)
                    {
                        if(counter == 0)
                            {
                                fprintf(fptr, "\nmovq $%ld, %%rdi", args->node->value);
                            }
                            else if(counter == 1)
                            {
                                fprintf(fptr, "\nmovq $%ld, %%rsi", args->node->value);
                            }
                            else if(counter == 2)
                            {
                                fprintf(fptr, "\nmovq $%ld, %%rdx", args->node->value);
                            }
                            else if(counter == 3)
                            {
                                fprintf(fptr, "\nmovq $%ld, %%rcx", args->node->value);
                            }
                            else if(counter == 4)
                            {
                                fprintf(fptr, "\nmovq $%ld, %%r8", args->node->value);
                            }
                            else if(counter == 5)
                            {
                                fprintf(fptr, "\nmovq $%ld, %%r9", args->node->value);
                            }
                    }
                    else
                    {
                        char* location = LookUpVarInfo(args->node->name, INVAL);
                        if(counter == 0)
                            {
                                fprintf(fptr, "\nmovq %s, %%rdi", location);
                            }
                            else if(counter == 1)
                            {
                                fprintf(fptr, "\nmovq %s, %%rsi", location);
                            }
                            else if(counter == 2)
                            {
                                fprintf(fptr, "\nmovq %s, %%rdx", location);
                            }
                            else if(counter == 3)
                            {
                                fprintf(fptr, "\nmovq %s, %%rcx", location);
                            }
                            else if(counter == 4)
                            {
                                fprintf(fptr, "\nmovq %s, %%r8", location);
                            }
                            else if(counter == 5)
                            {
                                fprintf(fptr, "\nmovq %s, %%r9", location);
                            }
                    }

                    counter += 1;
                    args = args->next;
                }

                fprintf(fptr, "\ncall %s", statements->node->right->left->name);
                fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));
                fprintf(fptr, "\nmovq %s, %%rax", LookUpVarInfo(statements->node->name, INVAL));
                //fprintf(fptr, "\nleave");
            }
            else
            {
                LongToCharOffset();
                UpdateVarInfo(statements->node->name, lastOffsetUsed, INVAL, false);

                switch(statements->node->right->opCode)
                {
                    case MULTIPLY:
                        // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        // arg2
                        if(statements->node->right->right->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->right->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rbx", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rbx", statements->node->right->right->value);
                        }

                        fprintf(fptr, "\nimulq %%rbx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));

                        break;
                    case DIVIDE:
                        // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        // arg2
                        if(statements->node->right->right->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->right->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rbx", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rbx", statements->node->right->right->value);
                        }
                        
                        fprintf(fptr, "\ncqto");
                        fprintf(fptr, "\nidivq %%rbx");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));

                        break;
                    case ADD:
                        // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        // arg2
                        if(statements->node->right->right->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->right->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rbx", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rbx", statements->node->right->right->value);
                        }

                        fprintf(fptr, "\naddq %%rbx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));

                        break;
                    case SUBTRACT:
                        // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        // arg2
                        if(statements->node->right->right->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->right->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rbx", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rbx", statements->node->right->right->value);
                        }

                        fprintf(fptr, "\nsubq %%rbx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));
                        
                        break;
                    case NEGATE:
                    // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        fprintf(fptr, "\nnegq %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));
                       
                        break;
                    case BOR:
                    // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        // arg2
                        if(statements->node->right->right->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->right->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rbx", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rbx", statements->node->right->right->value);
                        }

                        fprintf(fptr, "\norq %%rbx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));
                        
                        break;
                    case BAND:
                    // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        // arg2
                        if(statements->node->right->right->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->right->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rbx", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rbx", statements->node->right->right->value);
                        }

                        fprintf(fptr, "\nandq %%rbx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));

                        break;
                    case BXOR:
                    // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        // arg2
                        if(statements->node->right->right->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->right->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rbx", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rbx", statements->node->right->right->value);
                        }

                        fprintf(fptr, "\nxorq %%rbx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));
                        
                        break;
                    case BSHR:
                    // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        // arg2
                        if(statements->node->right->right->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->right->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rbx", location);
                            fprintf(fptr, "\nsarq %%rbx, %%rax");
                        }
                        else
                        {
                            fprintf(fptr, "\nsarq $%ld, %%rax", statements->node->right->right->value);
                            //fprintf(fptr, "\nmovq $%ld, %%rbx", statements->node->right->right->value);
                        }

                        //fprintf(fptr, "\nsarq %%rbx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));
                        
                        break;
                    case BSHL:
                        // arg1
                        if(statements->node->right->left->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->left->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rax", location);
                        }
                        else
                        {
                            fprintf(fptr, "\nmovq $%ld, %%rax", statements->node->right->left->value);
                        }

                        // arg2
                        if(statements->node->right->right->exprCode == VARIABLE)
                        {
                            char* location = LookUpVarInfo(statements->node->right->right->name, INVAL);

                            fprintf(fptr, "\nmovq %s, %%rbx", location);
                            fprintf(fptr, "\nsalq %%rbx, %%rax");
                        }
                        else
                        {
                            fprintf(fptr, "\nsalq $%ld, %%rax", statements->node->right->right->value);
                            //fprintf(fptr, "\nmovq $%ld, %%rbx", statements->node->right->right->value);
                        }

                        //fprintf(fptr, "\nsalq %%rbx, %%rax");
                        fprintf(fptr, "\nmovq %%rax, %s", LookUpVarInfo(statements->node->name, INVAL));

                        break;
                    default:
                        break;
                }
            }
        }

        statements = statements->next;
    }
}

/*
 ***********************************************************************
  THIS FUNCTION IS MEANT TO DO CODEGEN FOR ALL THE FUNCTIONS IN THE FILE
 ************************************************************************
*/
void Codegen(NodeList* worklist) {
    fptr = fopen("assembly.s", "w+");
    /*
     ****************************************
              TODO : YOUR CODE HERE
     ****************************************
    */
    if (fptr == NULL) {
        printf("\n Could not create assembly file");
        return; 
    }
    while (worklist != NULL) {
        /*
        ****************************************
                TODO : YOUR CODE HERE
        ****************************************
        */

        // init asm for func, push args onto stack, process statements, ret pop args off stack
        InitAsm(worklist->node->name);
        PushArgOnStack(worklist->node->arguments);
        ProcessStatements(worklist->node->statements);

        //PrintVarListInfo();
        //printf("\n");

        FreeVarList();

        worklist = worklist->next; 
    }
    fclose(fptr);
}

/*
**********************************************************************************************************************************
 YOU CAN MAKE ADD AUXILLIARY FUNCTIONS BELOW THIS LINE. DO NOT FORGET TO DECLARE THEM IN THE HEADER
**********************************************************************************************************************************
*/

/*
**********************************************************************************************************************************
 YOU CAN MAKE ADD AUXILLIARY FUNCTIONS ABOVE THIS LINE. DO NOT FORGET TO DECLARE THEM IN THE HEADER
**********************************************************************************************************************************
*/


