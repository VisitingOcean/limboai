/**
 * bt_new_scope.h
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifndef BT_NEW_SCOPE_H
#define BT_NEW_SCOPE_H

#include "../bt_decorator.h"

#include "../../../blackboard/blackboard_plan.h"

class BTNewScope : public BTDecorator {
	GDCLASS(BTNewScope, BTDecorator);
	TASK_CATEGORY(Decorators);

private:
	Ref<BlackboardPlan> blackboard_plan;

protected:
	static void _bind_methods();

	void set_blackboard_plan(const Ref<BlackboardPlan> &p_plan) { blackboard_plan = p_plan; }
	Ref<BlackboardPlan> get_blackboard_plan() const { return blackboard_plan; }

	virtual Status _tick(double p_delta) override;

public:
	virtual void initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard) override;
};

#endif // BT_NEW_SCOPE_H
