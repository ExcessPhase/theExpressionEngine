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
	using namespace theExpressionEngine;
	// Initialize the native target
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
	const auto pFactory = factory::getFactory();
	const auto pE = pFactory->parse(argv[1]);
	std::vector<double> sX(1);
	std::string sLine;
	while (std::getline(std::cin, sLine))
	{	sX[0] = std::stod(sLine.c_str());
		//GenericValue GV = EE->runFunction(GetValueFunc, Args);
		//std::cout << GV.DoubleVal << std::endl; // Output: 3.14
		std::cout << pE->evaluateLLVM(sX.data()) << " " << pE->evaluate(sX.data()) << std::endl;
	}
	return 0;
}
