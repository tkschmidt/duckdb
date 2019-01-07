//===----------------------------------------------------------------------===//
//                         DuckDB
//
// parser/expression/default_expression.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "parser/expression.hpp"
#include "parser/sql_node_visitor.hpp"

namespace duckdb {
//! Represents the default value of a column
class DefaultExpression : public Expression {
public:
	DefaultExpression() : Expression(ExpressionType::VALUE_DEFAULT) {
	}

	unique_ptr<Expression> Accept(SQLNodeVisitor *v) override {
		return v->Visit(*this);
	}
	ExpressionClass GetExpressionClass() override {
		return ExpressionClass::DEFAULT;
	}

	unique_ptr<Expression> Copy() override;

	void EnumerateChildren(std::function<unique_ptr<Expression>(unique_ptr<Expression> expression)> callback) override {
	}
	void EnumerateChildren(std::function<void(Expression *expression)> callback) const override {
	}
	//! Deserializes a blob back into an DefaultExpression
	static unique_ptr<Expression> Deserialize(ExpressionType type, TypeId return_type, Deserializer &source);

	string ToString() const override {
		return "DEFAULT";
	}
};
} // namespace duckdb
