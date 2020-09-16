#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <map>

using namespace llvm;

namespace {

	// struct Node{
	// 	llvm::Value* value;
	// 	std::string op_name;
	// };

	//define llvm pass
	struct ParallelismPass : public FunctionPass {

		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		ParallelismPass() : FunctionPass(ID) {}

		//std::multimap<Node, bool> list_bb_values;
		std::map<Instruction*, int> map_instr_cycle;
		std::map<Instruction*, int>::iterator it_map_instr_cycle;
		llvm::Value::use_iterator it_use_value;

		//run on each file function
		virtual bool runOnFunction(Function &llvm_function) {
			//print funcion name
			errs() << "Function: " << llvm_function.getName() << "\n";
			//run on each function basic block 
			runOnBasicBlocks(llvm_function);
			
			return false;
		}

		//run on each funcion basic block
		void runOnBasicBlocks(Function &llvm_function) {
			//foreach basic block
			for (auto &bb_llvm : llvm_function.getBasicBlockList()){
				//run on basick block instructions
				runOnInstructions(llvm_function, bb_llvm);
			}
		}

		//Cycles As Soon As Possible 
		int asap(Instruction &llvm_instruction, BasicBlock &llvm_bb){
			//if the instruction parent is not this same basic block, then it has no dependencies on this basic block. 
			//Its parent comes from another basic block above. So its cycle is -1.
			if (llvm_instruction.getParent() != &llvm_bb)
				return -1;

			//define var cycle
			int cycle = 0;

			//check if this instruction cycle is already in the map
			it_map_instr_cycle = map_instr_cycle.find(&llvm_instruction);
			//if it is just return the cycle value that was calculated
			if(it_map_instr_cycle != map_instr_cycle.end())
				return it_map_instr_cycle->second;

			//foreach instruction operands
			for(unsigned i = 0; i < llvm_instruction.getNumOperands(); ++i){
				//cast operand value as a instruction
				Instruction *inst = llvm::dyn_cast<llvm::Instruction>(llvm_instruction.getOperand(i));
				//if it is a instruction
				if (inst){
					//cycle is always the max between the current value and the cycle value returned from recursion
					cycle=std::max(cycle,1+asap(*inst,llvm_bb)); 
				}
			}
			//insert instruction and cycle value in the map
			map_instr_cycle.insert({&llvm_instruction, cycle});

			//return the cycle value
			return cycle;
		}

		void copyMap(std::map<Instruction*, int> *map_instr_cycle, int cycle){
			for(it_map_instr_cycle = map_instr_cycle->begin(); it_map_instr_cycle != map_instr_cycle->end(); ++it_map_instr_cycle){
				if(it_map_instr_cycle->second == cycle)
					map_instr_cycle->insert({it_map_instr_cycle->first, it_map_instr_cycle->second});
			}
		}

		//run on instructions
		void runOnInstructions(Function &llvm_function, BasicBlock &bb_llvm) {
			int cycle = 0;

			//print ASAP Cycles
			errs() << "\n\n--- ASAP ---\n";
			//foreach instruction on basick block
			for (auto &llvm_instruction : bb_llvm.getInstList()) {
				//cast instruction to debug instruction
				auto debug_value_instruction = llvm::dyn_cast<llvm::DbgValueInst>(&llvm_instruction);
				//if its not a debug instruction
				if(!debug_value_instruction){
					cycle = asap(llvm_instruction,bb_llvm);
					//print the instruction asap cycle
     				errs() << " Cycle "<< cycle <<  ": "<< llvm_instruction.getOpcodeName() << " (" << llvm_instruction << ")\n";
				}
			}

			std::map<Instruction*, int> map_instr_cycle2;
			copyMap(&map_instr_cycle2, cycle);

			//ALAP Cycles
			errs() << "\n\n--- ALAP ---\n";
			for (it_map_instr_cycle = map_instr_cycle.begin(); it_map_instr_cycle != map_instr_cycle.end(); ++it_map_instr_cycle) {
				if(map_instr_cycle2.find(it_map_instr_cycle->first) == map_instr_cycle2.end()){
					cycle = it_map_instr_cycle->first->getNumUses();
					map_instr_cycle2.insert({it_map_instr_cycle->first, cycle - it_map_instr_cycle->second});	
				}				
			}
			//print ALAP Cycles
			for (it_map_instr_cycle = map_instr_cycle2.begin(); it_map_instr_cycle != map_instr_cycle2.end(); ++it_map_instr_cycle) {
				errs() << " Cycle "<< it_map_instr_cycle->second <<  ": "<< it_map_instr_cycle->first->getOpcodeName() << " (" << it_map_instr_cycle->first << ")\n";
			}			
		}
	
		// void runOnBinaryInst(Instruction &llvm_instruction, BasicBlock &bb_llvm){
		// 	auto binary_instruction = llvm::dyn_cast<llvm::BinaryOperator>(&llvm_instruction);

		// 	std::string op_name = getBinaryOperatorName(binary_instruction->getOpcode()).append(next_id());

		// 	Node node;
		// 	node.value = binary_instruction;
		// 	node.op_name = op_name;

		// 	list_bb_values.insert({node, false});

		// 	for(unsigned i = 0; i < binary_instruction->getNumOperands(); ++i){
		// 		node.value = binary_instruction->getOperand(i);
		// 		list_bb_values.insert({node, false});
		// 	}
		// }
	};
} // namespace llvm

char ParallelismPass::ID = 0;

static RegisterPass<ParallelismPass> X("parallelism", "Parallelism Pass");
