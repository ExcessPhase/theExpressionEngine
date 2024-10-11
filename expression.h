#pragma once
#include <memory>
#include <vector>
#include <string>
#include <variant>

namespace theExpessionEngine
{
struct type;
struct environment;
/// the base class of all expression types
/// immutable
/// no two with the same content are guaranteed to exist
struct expression:std::enable_shared_from_this<const expression>
{	typedef std::shared_ptr<const expression> ptr;
	typedef std::vector<ptr> children;
	const children m_sChildren;
	const std::shared_ptr<const type> m_sType;
	expression(
		const std::shared_ptr<const type> &_rType,
		children&&_rChildren
	);
		/// this compares everything contained in expression
		/// and calls isSmaller() if there is no difference
	bool operator<(const expression&_r) const;
		/// this is to be implemented for all derived classes containing data
		/// the type of the LHS and RHS is guaranteed to be identical
	virtual bool isSmaller(const expression&) const;
	typedef std::variant<double, int, std::string> value;
		/// should be private
		/// only called by evaluate()
	virtual value evaluateThis(environment&) const = 0;
	const value&evaluate(environment&) const;
};
}