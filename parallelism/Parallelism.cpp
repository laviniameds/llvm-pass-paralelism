#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <unordered_set>

using namespace llvm;

namespace {

	struct ParallelismPass : public FunctionPass {
		static char ID;
		ParallelismPass() : FunctionPass(ID) {}
		std::unordered_set<llvm::Value*> list_values;

		virtual bool runOnFunction(Function &llvm_function) {
			errs() << "I saw a function called " << llvm_function.getName() << "!\n";
			runOnBasicBlocks(llvm_function);
			printList();
			return false;
		}

		void runOnBasicBlocks(Function &llvm_function) {
			for (auto &bb_llvm : llvm_function.getBasicBlockList())
				runOnInstructions(llvm_function, bb_llvm);
		}

		int getOperatorType(int llvm_opCpde) {
			if (llvm_opCpde >= llvm::Instruction::BinaryOps::BinaryOpsBegin && llvm_opCpde < llvm::Instruction::BinaryOpsEnd) {
				return 1;
			}
			else if (llvm_opCpde >= llvm::Instruction::TermOpsBegin && llvm_opCpde < llvm::Instruction::TermOpsEnd) {
				return 2;
			}
			else if (llvm_opCpde >= llvm::Instruction::OtherOpsBegin && llvm_opCpde < llvm::Instruction::OtherOpsEnd) {
				return 3;
			}
			else
				return -1;
		}

		void runOnBinaryInstruction(llvm::Instruction &llvm_instruction) {
			auto binary_instruction = llvm::dyn_cast<llvm::BinaryOperator>(&llvm_instruction);

			list_values.insert(binary_instruction);

			for(unsigned i = 0; i < binary_instruction->getNumOperands(); ++i){
				list_values.insert(binary_instruction->getOperand(i));
			}
		}

		void printList(){
			int i = 0;
			for(auto const& it: list_values)
				errs() << ++i << " : " << it << "\n";
		}

		void runOnInstructions(Function &llvm_function, BasicBlock &bb_llvm) {
			for (auto &llvm_instruction : bb_llvm.getInstList()) {
				int llvm_opCpde = llvm_instruction.getOpcode();
				int opType = getOperatorType(llvm_opCpde);

				switch (opType) {
				case 1:
					runOnBinaryInstruction(llvm_instruction);
					break;
				default:
					//errs() << "Other Instruction" << "\n";
					break;
				}
			}
		}
	};
} // namespace llvm

char ParallelismPass::ID = 0;

static RegisterPass<ParallelismPass> X("parallelism", "Parallelism Pass");
