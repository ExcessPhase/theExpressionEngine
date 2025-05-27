# theExpressionEngine -- an expression engine leveraging LLVM for just-in-time-compilation [theExpressionEngine](https://github.com/ExcessPhase/theExpressionEngine)

**Author**: Peter Foelsche |
**Date**: February..April 2025 |
**Location**: Austin, TX, USA |
**Email**: [peter_foelsche@outlook.com](mailto:peter_foelsche@outlook.com)

A expression engine prototype leveraging LLVM as an Just-In-Time-Compiler.
Pass an expression on the commandline. Understood are
- addition
- subtraction
- real numbers
- sqrt
- exp
- sin
- cos
- tan
- asin
- acos
- atan
- sinh
- cosh
- tanh
- asinh
- acosh
- atanh
- abs
- log
- log10
- erf
- erfc
- tgamma
- lgamma
- cbrt
- x
- pow
- max
- min
- atan2
- parentheses to support to overwrite precedence
- fmod
- hypot

It reads lines from stdin and interprets them as x values.
It reads one commandline argument,  which is enterpreted as a function and should use x.
You might want to use double quotes around this commandline argument.
e.g.
	`echo -e "0\n1"|./theExpressionEngine.exe "sin(x)"`
which prints the values of sin(0) and sin(1).

## News
Removed flex/bison in favor of boost::spirit::x3.

The switch to single threaded code makes sense, due to performance reasons!
Since changing from multithreaded to single threaded code requires changing the source code,
I tried to make expression and factory a template class. But this requires a different parser/lexer
as bison/flex are still living in the stone age and don't allow for generation of templated code.

The code is organized in a way so that it can be either multithreaded or single thread.
This can be switched using the BTHREADED template argument for unique in unique.h.

I made the current implementation thread-safe (not really tested in a multithreaded environment).

I made the registration of existing objects threading safe.
I don't know how to implement this, other than protecting increment/decrement reference counting by a mutex.

Implemented collapsing of constant values. So it is no longer possible to create an expression object sin(0.0) as it is immediately collapsed to 0.0.
Implemented a name2ptr object passed to factory::parse() to parse predefined parameters and constants.
All the LLVM code is now hidden behind the expression engine and multiple instances might exist and executed in parallel.
The parser/lexer isn't anymore implemented by hand but using flex&bison.
The versions being used are:
- flex 2.6.4
- bison (GNU Bison) 3.8.2
## ToBeDone
Avoid recalculation of the same object!
Implement missing binary functions.
Allow for multiple parameters but not only x.
