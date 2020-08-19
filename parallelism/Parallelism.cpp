#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace{
  struct ParallelismPass : public FunctionPass{
    static char ID;
    ParallelismPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F){
      errs() << "I saw a function called " << F.getName() << "!\n";
      return false;
    }
  };
} 

char ParallelismPass::ID = 0;

static RegisterPass<ParallelismPass> X("parallelism", "Parallelism Pass");
