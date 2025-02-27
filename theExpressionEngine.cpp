#if 1
#include <iostream>
#include "factory.h"
#include "expression.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/TargetSelect.h>


int main(int argc, char**argv)
{
	if (argc != 2)
	{	std::cerr << argv[0] << ": Usage : " << argv[0] << " expression" << std::endl;
		return 1;
	}
        using namespace llvm;
        using namespace theExpessionEngine;
	// Initialize the native target
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();

        LLVMContext Context;
        auto M = std::make_unique<Module>("top", Context);
        IRBuilder<> Builder(Context);
    
        FunctionType* FT = FunctionType::get(Type::getDoubleTy(Context), false);
        Function* GetValueFunc = Function::Create(FT, Function::ExternalLinkage, "getValue", M.get());
        BasicBlock* BB = BasicBlock::Create(Context, "EntryBlock", GetValueFunc);
        Builder.SetInsertPoint(BB);
    
        const auto pFactory = factory::getFactory();

        const auto pE = pFactory->parse(argv[1]);

        // Create a constant double value
        Value* const ConstantVal = pE->generateCodeW(Context, Builder, M.get());//ConstantFP::get(Context, APFloat(3.14));
    
        // Return the constant value
        Builder.CreateRet(ConstantVal);
    
	if (verifyModule(*M, &errs()))
	{	std::cerr << "Error: module verification failed" << std::endl;
		M->print(errs(), nullptr);
		return 1;
	}
    
        // Create the execution engine
        std::string ErrStr;
        ExecutionEngine* EE = EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create();
        if (!EE)
        {       errs() << "Failed to create ExecutionEngine: " << ErrStr << "\n";
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
#else
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/TargetSelect.h>

int main()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();

	llvm::LLVMContext context;
	auto module = std::make_unique<llvm::Module>("sqrt_calc", context);
	llvm::IRBuilder<> builder(context);

	// Create a function to hold the computation
	llvm::FunctionType* funcType = llvm::FunctionType::get(builder.getDoubleTy(), false);
	llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "compute", module.get());
	llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", func);
	builder.SetInsertPoint(entry);

	// Calculate 1.1 + 2.1
	llvm::Value* sum = builder.CreateFAdd(
		llvm::ConstantFP::get(context, llvm::APFloat(1.1)), 
		llvm::ConstantFP::get(context, llvm::APFloat(2.1))
	);

	// Calculate the square root of the sum
	llvm::Function* sqrtFunc = llvm::Intrinsic::getDeclaration(module.get(), llvm::Intrinsic::sqrt, builder.getDoubleTy());
	llvm::Value* sqrtResult = builder.CreateCall(sqrtFunc, sum);

	// Return the result
	builder.CreateRet(sqrtResult);

	// Verify the module
	if (llvm::verifyModule(*module, &llvm::errs()))
	{	std::cerr << "Error: module verification failed" << std::endl;
		module->print(llvm::errs(), nullptr);
		return 1;
	}

	// Initialize JIT
	std::string errStr;
	llvm::ExecutionEngine* execEngine = llvm::EngineBuilder(std::move(module)).setErrorStr(&errStr).create();
	if (!execEngine)
	{	std::cerr << "Error: failed to create ExecutionEngine: " << errStr << std::endl;
		return 1;
	}

	// Finalize object
	execEngine->finalizeObject();

	// Run the function
	llvm::GenericValue result = execEngine->runFunction(func, {});

	// Print the result
	std::cout << "Result: " << result.DoubleVal << std::endl;

	delete execEngine;
	return 0;
}
#endif
