
#include "parser/transform.hpp"

using namespace duckdb;
using namespace std;

bool TransformGroupBy(List *group,
                      vector<unique_ptr<AbstractExpression>> &result) {
	if (!group) {
		return false;
	}

	for (auto node = group->head; node != nullptr; node = node->next) {
		Node *n = reinterpret_cast<Node *>(node->data.ptr_value);
		result.push_back(TransformExpression(n));
	}
	return true;
}

bool TransformOrderBy(List *order, OrderByDescription &result) {
	if (!order) {
		return false;
	}

	for (auto node = order->head; node != nullptr; node = node->next) {
		Node *temp = reinterpret_cast<Node *>(node->data.ptr_value);
		if (temp->type == T_SortBy) {
			OrderByNode ordernode;
			SortBy *sort = reinterpret_cast<SortBy *>(temp);
			Node *target = sort->node;
			if (sort->sortby_dir == SORTBY_ASC ||
			    sort->sortby_dir == SORTBY_DEFAULT)
				ordernode.type = OrderType::ASCENDING;
			else if (sort->sortby_dir == SORTBY_DESC)
				ordernode.type = OrderType::DESCENDING;
			else
				throw NotImplementedException("Unimplemented order by type");
			ordernode.expression = TransformExpression(target);
			result.orders.push_back(
			    OrderByNode(ordernode.type, move(ordernode.expression)));
		} else {
			throw NotImplementedException("ORDER BY list member type %d\n",
			                              temp->type);
		}
	}
	return true;
}

unique_ptr<SelectStatement> TransformSelect(Node *node) {
	SelectStmt *stmt = reinterpret_cast<SelectStmt *>(node);
	switch (stmt->op) {
	case SETOP_NONE: {
		auto result = make_unique<SelectStatement>();
		result->select_distinct = stmt->distinctClause != NULL ? true : false;
		if (!TransformExpressionList(stmt->targetList, result->select_list)) {
			return nullptr;
		}
		result->from_table = TransformFrom(stmt->fromClause);
		if (!TransformGroupBy(stmt->groupClause, result->groupby.groups)) {
			return nullptr;
		}
		result->groupby.having = TransformExpression(stmt->havingClause);
		if (!TransformOrderBy(stmt->sortClause, result->orderby)) {
			return nullptr;
		}
		result->where_clause = TransformExpression(stmt->whereClause);
		if (stmt->limitCount) {
			result->limit.limit =
			    reinterpret_cast<A_Const *>(stmt->limitCount)->val.val.ival;
			result->limit.offset =
			    !stmt->limitOffset
			        ? 0
			        : reinterpret_cast<A_Const *>(stmt->limitOffset)
			              ->val.val.ival;
		}
		return result;
	}
	case SETOP_UNION: {
		auto result = TransformSelect((Node *)stmt->larg);
		if (!result) {
			return nullptr;
		}
		auto right = TransformSelect((Node *)stmt->rarg);
		if (!right) {
			return nullptr;
		}
		result->union_select = move(right);
		return result;
	}
	case SETOP_INTERSECT:
	case SETOP_EXCEPT:
	default:
		throw NotImplementedException("A_Expr not implemented!");
	}
}