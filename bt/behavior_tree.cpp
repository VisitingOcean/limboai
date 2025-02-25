/**
 * behavior_tree.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "behavior_tree.h"

#include "../util/limbo_string_names.h"

#ifdef LIMBOAI_MODULE
#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/templates/list.h"
#include "core/variant/variant.h"
#endif // ! LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include "godot_cpp/core/error_macros.hpp"
#endif // ! LIMBOAI_GDEXTENSION

void BehaviorTree::set_description(const String &p_value) {
	description = p_value;
	emit_changed();
}

void BehaviorTree::set_blackboard_plan(const Ref<BlackboardPlan> &p_plan) {
	if (blackboard_plan == p_plan) {
		return;
	}

	if (Engine::get_singleton()->is_editor_hint() && blackboard_plan.is_valid() &&
			blackboard_plan->is_connected(LW_NAME(changed), callable_mp(this, &BehaviorTree::_plan_changed))) {
		blackboard_plan->disconnect(LW_NAME(changed), callable_mp(this, &BehaviorTree::_plan_changed));
	}

	blackboard_plan = p_plan;
	if (blackboard_plan.is_null()) {
		blackboard_plan = Ref<BlackboardPlan>(memnew(BlackboardPlan));
	}

	if (Engine::get_singleton()->is_editor_hint()) {
		blackboard_plan->connect(LW_NAME(changed), callable_mp(this, &BehaviorTree::_plan_changed));
	}

	emit_changed();
}

void BehaviorTree::set_root_task(const Ref<BTTask> &p_value) {
	root_task = p_value;
	emit_changed();
}

Ref<BehaviorTree> BehaviorTree::clone() const {
	Ref<BehaviorTree> copy = duplicate(false);
	copy->set_path("");
	if (root_task.is_valid()) {
		copy->root_task = root_task->clone();
	}
	return copy;
}

void BehaviorTree::copy_other(const Ref<BehaviorTree> &p_other) {
	ERR_FAIL_COND(p_other.is_null());
	description = p_other->get_description();
	root_task = p_other->get_root_task();
}

Ref<BTTask> BehaviorTree::instantiate(Node *p_agent, const Ref<Blackboard> &p_blackboard) const {
	ERR_FAIL_COND_V_MSG(root_task == nullptr, memnew(BTTask), "Trying to instance a behavior tree with no valid root task.");
	Ref<BTTask> inst = root_task->clone();
	inst->initialize(p_agent, p_blackboard);
	return inst;
}

void BehaviorTree::_plan_changed() {
	emit_changed();
}

void BehaviorTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_description", "p_value"), &BehaviorTree::set_description);
	ClassDB::bind_method(D_METHOD("get_description"), &BehaviorTree::get_description);
	ClassDB::bind_method(D_METHOD("set_blackboard_plan", "p_plan"), &BehaviorTree::set_blackboard_plan);
	ClassDB::bind_method(D_METHOD("get_blackboard_plan"), &BehaviorTree::get_blackboard_plan);
	ClassDB::bind_method(D_METHOD("set_root_task", "p_value"), &BehaviorTree::set_root_task);
	ClassDB::bind_method(D_METHOD("get_root_task"), &BehaviorTree::get_root_task);
	ClassDB::bind_method(D_METHOD("clone"), &BehaviorTree::clone);
	ClassDB::bind_method(D_METHOD("copy_other", "p_other"), &BehaviorTree::copy_other);
	ClassDB::bind_method(D_METHOD("instantiate", "p_agent", "p_blackboard"), &BehaviorTree::instantiate);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description", PROPERTY_HINT_MULTILINE_TEXT), "set_description", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "blackboard_plan", PROPERTY_HINT_RESOURCE_TYPE, "BlackboardPlan", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT), "set_blackboard_plan", "get_blackboard_plan");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root_task", PROPERTY_HINT_RESOURCE_TYPE, "BTTask", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL), "set_root_task", "get_root_task");
}

BehaviorTree::BehaviorTree() {
}

BehaviorTree::~BehaviorTree() {
	if (Engine::get_singleton()->is_editor_hint() && blackboard_plan.is_valid() &&
			blackboard_plan->is_connected(LW_NAME(changed), callable_mp(this, &BehaviorTree::_plan_changed))) {
		blackboard_plan->disconnect(LW_NAME(changed), callable_mp(this, &BehaviorTree::_plan_changed));
	}
}
