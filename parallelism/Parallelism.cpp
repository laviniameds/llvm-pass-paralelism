#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <map>

using namespace llvm;

namespace {

	//incremental global id
	static long id_ = 1;

	//return id as string
	std::string next_id() {
		return std::to_string(id_++);
	}

	struct ParallelismPass : public FunctionPass {
		static char ID;
		
		ParallelismPass() : FunctionPass(ID) {}
		std::multimap<llvm::Value*, std::string> list_bb_values;

		virtual bool runOnFunction(Function &llvm_function) {
			errs() << "I saw a function called " << llvm_function.getName() << "!\n";
			runOnBasicBlocks(llvm_function);
			printList();
			
			return false;
		}

		void runOnBasicBlocks(Function &llvm_function) {
			for (auto &bb_llvm : llvm_function.getBasicBlockList()){
				runOnInstructions(llvm_function, bb_llvm);
			}
		}

		std::string getBinaryOperatorName(int llvm_opCpde){
			switch (llvm_opCpde) {
				case llvm::Instruction::Add:
				return "ADD";
				break;
				case llvm::Instruction::Sub:
				return "SUB";
				break;
				case llvm::Instruction::Mul:
				return "MUL";
				break;
				case llvm::Instruction::SDiv:
				return "DIV";
				break;
				case llvm::Instruction::SRem:
				return "MOD";
				break;
				case llvm::Instruction::Shl:
				return "SHL";
				break;
				case llvm::Instruction::AShr:
				return "ASHR";
				break;
				default:
				return "BIN_OP_NOT_DEFINED";
			}
		}

		void runOnBinaryInst(Instruction &llvm_instruction, BasicBlock &bb_llvm){
			auto binary_instruction = llvm::dyn_cast<llvm::BinaryOperator>(&llvm_instruction);

			std::string op_name = getBinaryOperatorName(binary_instruction->getOpcode()).append(next_id());

			list_bb_values.insert({binary_instruction, op_name});

			for(unsigned i = 0; i < binary_instruction->getNumOperands(); ++i){
				list_bb_values.insert({binary_instruction->getOperand(i), op_name});
			}
		}

		void runOnTermInst(Instruction &llvm_instruction, BasicBlock &bb_llvm){
			std::string op_name = getBinaryOperatorName(llvm_instruction.getPrevNode()->getOpcode()).append(next_id());

			for(unsigned i = 0; i < llvm_instruction.getNumOperands(); ++i){
				list_bb_values.insert({llvm_instruction.getOperand(i), op_name});
			}
		}

		void runOnInstructions(Function &llvm_function, BasicBlock &bb_llvm) {
			for (auto &llvm_instruction : bb_llvm.getInstList()) {
				auto debug_value_instruction = llvm::dyn_cast<llvm::DbgValueInst>(&llvm_instruction);

				if(!debug_value_instruction){
					if(llvm_instruction.isBinaryOp()){
						runOnBinaryInst(llvm_instruction,bb_llvm);
					}
					else if(llvm_instruction.isTerminator()){
						runOnTermInst(llvm_instruction, bb_llvm);
					}
				}
			}
		}

		void printList(){
			int i = 0;
			for(auto const& it: list_bb_values)
				errs() << ++i << " : " << it.first << " - " << it.second << "\n";
		}
	};
} // namespace llvm

char ParallelismPass::ID = 0;

static RegisterPass<ParallelismPass> X("parallelism", "Parallelism Pass");
