#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <map>

using namespace llvm;

namespace {
	//define llvm pass
	struct ParallelismPass : public FunctionPass {

		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		ParallelismPass() : FunctionPass(ID) {}

		//define maps and their iterators
		std::map<Instruction*, int> map_instr_cycle_asap;
		std::map<Instruction*, int> map_instr_cycle_alap;
		std::map<Instruction*, int>::iterator it_map_instr_cycle;
		std::map<Instruction*, int>::reverse_iterator r_it_map_instr_cycle;

		//define vars to cycle
		int greatest_cycle = 0;
		int cycle = 0;

		//number of basic blocks
		int cont_bb = 0;

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

				map_instr_cycle_asap.clear();
				map_instr_cycle_alap.clear();
				greatest_cycle = 0;
				cycle = 0;

				errs() << "\n\n-- Basic Block: " << ++cont_bb << " --\n";

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
			it_map_instr_cycle = map_instr_cycle_asap.find(&llvm_instruction);
			//if it is just return the cycle value that was calculated
			if(it_map_instr_cycle != map_instr_cycle_asap.end())
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
			map_instr_cycle_asap.insert({&llvm_instruction, cycle});

			//get greatest number of cycles
			if(cycle > greatest_cycle) greatest_cycle = cycle;
			
			//if it is a terminator instruction, it belongs to the last cycle
			if(llvm_instruction.isTerminator())
				cycle = greatest_cycle;

			//return the cycle value
			return cycle;
		}

		//Cycles As Late As Possible
		int alap(Instruction &llvm_instruction, BasicBlock &llvm_bb, int greatest_cycle, int cycle){
			//if the instruction parent is not this same basic block, then it has no dependencies on this basic block. 
			//Its parent comes from another basic block above. So it belongs to the last cycle.
			if (llvm_instruction.getParent() != &llvm_bb)
				return greatest_cycle+1;

			//check if this instruction cycle is already in the map
			it_map_instr_cycle = map_instr_cycle_alap.find(&llvm_instruction);
			//if it is just return the cycle value that was calculated
			if(it_map_instr_cycle != map_instr_cycle_alap.end())
				return it_map_instr_cycle->second;

			//foreach instruction "use values"
			for(auto it_user_value = llvm_instruction.user_begin(); it_user_value != llvm_instruction.user_end(); ++it_user_value){
				//cast "use value" as a instruction
				Instruction *inst = llvm::dyn_cast<llvm::Instruction>(*it_user_value);
				//if it is a instruction
				if (inst){
					//cycle is always the min between the current value and the cycle value returned from recursion
					cycle=std::min(cycle,alap(*inst,llvm_bb, greatest_cycle, cycle)-1); 
				}
			}
			//insert instruction and cycle value in the map
			map_instr_cycle_alap.insert({&llvm_instruction, cycle});

			//if it is a terminator instruction, it belongs to the last cycle
			if(llvm_instruction.isTerminator())
				cycle = greatest_cycle+1;

			//return the cycle value
			return cycle;
		}
		
		//run on instructions
		void runOnInstructions(Function &llvm_function, BasicBlock &bb_llvm) {
			//print ASAP Cycles
			errs() << "\n--- ASAP ---\n";
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

			//ALAP Cycles
			errs() << "\n\n--- ALAP ---\n";
			//use reverse iterator to go through instructions
			for (r_it_map_instr_cycle = map_instr_cycle_asap.rbegin(); r_it_map_instr_cycle != map_instr_cycle_asap.rend(); ++r_it_map_instr_cycle) {
				alap(*r_it_map_instr_cycle->first,bb_llvm, greatest_cycle, greatest_cycle);
			}
			//print ALAP Cycles
			for (it_map_instr_cycle = map_instr_cycle_alap.begin(); it_map_instr_cycle != map_instr_cycle_alap.end(); ++it_map_instr_cycle) {
				errs() << " Cycle "<< it_map_instr_cycle->second <<  ": "<< it_map_instr_cycle->first->getOpcodeName() << " (" << *it_map_instr_cycle->first << ")\n";
			}	
		}
	};
} // namespace llvm

char ParallelismPass::ID = 0;

static RegisterPass<ParallelismPass> X("parallelism", "Parallelism Pass");
