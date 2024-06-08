#pragma once
#include "misc.h"
// custom operator
// https://stackoverflow.com/questions/8425077/can-i-create-a-new-operator-in-c-and-how
namespace vka
{
// generic LHSlt holder
template <typename LHS, typename OP>
struct LHSlt
{
	LHS lhs_;
};

// declare myop as an operator-like construct
enum
{
	hashCombine_OP
};

// parse 'lhs <myop' into LHSlt
template <typename LHS>
LHSlt<LHS, decltype(hashCombine_OP)> operator<(const LHS &lhs, decltype(hashCombine_OP))
{
	return {lhs};
}

template <class A, class B>
int operator>(LHSlt<A, decltype(hashCombine_OP)> lhsof, B rhs)
{
	A &lhs = lhsof.lhs_;
	// actual implementation
	return hashCombine(lhs, rhs);
}

#define HASHC < hashCombine_OP >
}        // namespace vka