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
	using namespace llvm;
	using namespace theExpressionEngine;
	// Initialize the native target
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
	if (argc < 2)
	{	std::cerr << argv[0] << ": Usage : " << argv[0] << " expression" << std::endl;
		return 1;
	}
	constexpr const auto BTHREADED = true;
	const auto pFactory = factory<BTHREADED>::getFactory();
	std::vector<expression<BTHREADED>::ptr> sE;
	for (auto p = argv + 1; *p; ++p)
		sE.push_back(pFactory->parse(*p, {{"x", pFactory->parameter(0)}}));
	const auto sES = pFactory->createExpressionSet(sE);
	const auto &rOrder = sES->getOrder();
	std::vector<double> sX(1);
	std::vector<int> sXI;
	std::string sLine;
	std::vector<double> sChildren;
	std::vector<int> sChildrenI;
	while (std::getline(std::cin, sLine))
	{	if (sLine.empty())
			continue;
		sX[0] = std::stod(sLine.c_str());
		sES->evaluateLLVM(sChildren, sChildrenI, sX.data(), sXI.data());
		std::cout << sX[0] << "\t";
		for (std::size_t i = 0, iMax = sE.size(); i < iMax; ++i)
			std::cout << sChildren[rOrder[i]] << "\t";
		std::cout << std::endl;
	}
	return 0;
}
