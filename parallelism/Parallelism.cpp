#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
    struct ParallelismPass : public FunctionPass {
        static char ID;
        ParallelismPass() : FunctionPass(ID){}

        virtual bool runOnFunction(Function &F){
            errs() << "Function called: " << F.getName() << "!\n";
            return false;
        }
    }; 
}
char ParallelismPass::ID = 0;
static void RegisterParallelismPass(const PassManagerBuilder &, legacy::PassManagerBase &PM){
        PM.add(new ParallelismPass());
}
static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, RegisterParallelismPass);