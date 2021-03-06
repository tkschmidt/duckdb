#include "duckdb/execution/operator/helper/physical_limit.hpp"
#include "duckdb/execution/physical_plan_generator.hpp"
#include "duckdb/planner/operator/logical_limit.hpp"

namespace duckdb {
using namespace std;

unique_ptr<PhysicalOperator> PhysicalPlanGenerator::CreatePlan(LogicalLimit &op) {
	D_ASSERT(op.children.size() == 1);

	auto plan = CreatePlan(*op.children[0]);

	auto limit = make_unique<PhysicalLimit>(op.types, op.limit, op.offset);
	limit->children.push_back(move(plan));
	return move(limit);
}

} // namespace duckdb
