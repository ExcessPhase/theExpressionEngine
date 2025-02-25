#include <iostream>
#include "factory.h"
#include "expression.h"
#include "type.h"
#include "environment.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/TargetSelect.h"


int main()
{
        using namespace llvm;
        using namespace theExpessionEngine;
	// Initialize the native target
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();

        LLVMContext Context;
        Module* M = new Module("top", Context);
        IRBuilder<> Builder(Context);
    
        environment sEnv;
        const auto pFactory = factory::getFactory(sEnv);

        const auto pE = pFactory->plus(
		pFactory->realConstant(1.1),
		pFactory->realConstant(2.1)
	);

        // Create a constant double value
        Value* const ConstantVal = pE->generateCodeW(Context, Builder);//ConstantFP::get(Context, APFloat(3.14));
    
        // Create the function type: double()
        FunctionType* FT = FunctionType::get(Type::getDoubleTy(Context), false);
    
        // Create the function: double getValue()
        Function* GetValueFunc = Function::Create(FT, Function::ExternalLinkage, "getValue", M);
    
        // Create a basic block and add it to the function
        BasicBlock* BB = BasicBlock::Create(Context, "EntryBlock", GetValueFunc);
        Builder.SetInsertPoint(BB);
    
        // Return the constant value
        Builder.CreateRet(ConstantVal);
    
        // Verify the function
        verifyFunction(*GetValueFunc);
    
        // Print the LLVM IR
        M->print(errs(), nullptr);
    
        // Create the execution engine
        std::string ErrStr;
        ExecutionEngine* EE = EngineBuilder(std::unique_ptr<Module>(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create();
        if (!EE) {
            errs() << "Failed to create ExecutionEngine: " << ErrStr << "\n";
            return 1;
        }
    
        // Add the module and compile the function
        EE->finalizeObject();
    
        // Call the function and get the result
        std::vector<GenericValue> NoArgs;
        GenericValue GV = EE->runFunction(GetValueFunc, NoArgs);
    
        // Print the result
        std::cout << "Result: " << GV.DoubleVal << std::endl; // Output: 3.14
    
        // Clean up
        delete EE;
    
        return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
