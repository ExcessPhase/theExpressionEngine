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
#if 0
llvm::FunctionType* funcType = llvm::FunctionType::get(
    llvm::Type::getVoidTy(context),
    { llvm::Type::getDoublePtrTy(context) }, // `double*` as a parameter
    false);

llvm::Function* function = llvm::Function::Create(
    funcType, llvm::Function::ExternalLinkage, "evaluateExpression", module);

#endif 
	FunctionType* FT = FunctionType::get(
		Type::getDoubleTy(Context),
		{	Type::getDoublePtrTy(Context)
		},
		false
	);
	Function* GetValueFunc = Function::Create(FT, Function::ExternalLinkage, "getValue", M.get());
	BasicBlock* BB = BasicBlock::Create(Context, "EntryBlock", GetValueFunc);
	Builder.SetInsertPoint(BB);

	const auto pFactory = factory::getFactory();

	const auto pE = pFactory->parse(argv[1]);

	Function::arg_iterator args = GetValueFunc->arg_begin();
	Value* doublePtrArg = &(*args);
	// Create a constant double value
	Value* const ConstantVal = pE->generateCodeW(Context, Builder, M.get(), doublePtrArg);//ConstantFP::get(Context, APFloat(3.14));

	// Return the constant value
	Builder.CreateRet(ConstantVal);

	if (verifyModule(*M, &errs()))
	{	std::cerr << "Error: module verification failed" << std::endl;
		M->print(errs(), nullptr);
		return 1;
	}

	// Create the execution engine
	std::string ErrStr;
	auto EE = std::unique_ptr<ExecutionEngine>(EngineBuilder(std::move(M)).setErrorStr(&ErrStr).setEngineKind(EngineKind::JIT).create());
	if (!EE)
	{	errs() << "Failed to create ExecutionEngine: " << ErrStr << "\n";
		return 1;
	}

	// Add the module and compile the function
	EE->finalizeObject();

	auto funcAddress = EE->getFunctionAddress("getValue");
	if (!funcAddress) {
	    std::cerr << "Error: Failed to get function address for 'getValue'" << std::endl;
	    return 1;
	}

	// Cast the function address to a function pointer with the correct signature
	using JITFunctionType = double(*)(const double*);
	auto jitFunction = reinterpret_cast<JITFunctionType>(funcAddress);

	// Call the function and get the result
	std::vector<double> sX({1.1});
#if 0
	GenericValue ArgVal;
	ArgVal.PointerVal = sX.data();
	std::vector<GenericValue> Args({ArgVal});
#endif
	std::string sLine;
	while (std::getline(std::cin, sLine))
	{	sX[0] = std::stod(sLine.c_str());
		//GenericValue GV = EE->runFunction(GetValueFunc, Args);
		//std::cout << GV.DoubleVal << std::endl; // Output: 3.14
		std::cout << jitFunction(sX.data()) << " " << pE->evaluate(sX.data()) << std::endl;
	}
	// Clean up
	//delete EE;
	return 0;
}
