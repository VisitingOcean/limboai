/**
 * bt_task.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#include "bt_task.h"

#include "../../blackboard/blackboard.h"
#include "../../util/limbo_string_names.h"
#include "../../util/limbo_utility.h"
#include "bt_comment.h"

#ifdef LIMBOAI_MODULE
#include "core/error/error_macros.h"
#include "core/io/resource.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/object/script_language.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/variant/variant.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/variant/variant.hpp"
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/script.hpp>
#endif // LIMBOAI_GDEXTENSION

void BT::_bind_methods() {
	BIND_ENUM_CONSTANT(FRESH);
	BIND_ENUM_CONSTANT(RUNNING);
	BIND_ENUM_CONSTANT(FAILURE);
	BIND_ENUM_CONSTANT(SUCCESS);
}

String BTTask::_generate_name() {
	String ret;

	// Generate name based on script path.
	Ref<Script> sc = GET_SCRIPT(this);
	if (sc.is_valid() && sc->get_path().is_absolute_path()) {
		ret = sc->get_path().get_basename().get_file().to_pascal_case();
	}

	// Generate name based on core class name.
	if (ret.is_empty()) {
		ret = get_class();
	}

	return ret.trim_prefix("BT");
}

Array BTTask::_get_children() const {
	Array arr;
	int num_children = get_child_count();
	arr.resize(num_children);
	for (int i = 0; i < num_children; i++) {
		arr[i] = get_child(i).ptr();
	}

	return arr;
}

void BTTask::_set_children(Array p_children) {
	data.children.clear();
	const int num_children = p_children.size();
	data.children.resize(num_children);
	for (int i = 0; i < num_children; i++) {
		Variant task_var = p_children[i];
		Ref<BTTask> task_ref = task_var;
		task_ref->data.parent = this;
		task_ref->data.index = i;
		data.children.set(i, task_var);
	}
}

String BTTask::get_task_name() {
	if (data.custom_name.is_empty()) {
#ifdef LIMBOAI_MODULE
		if (get_script_instance() && get_script_instance()->has_method(LW_NAME(_generate_name))) {
			if (unlikely(!get_script_instance()->get_script()->is_tool())) {
				ERR_PRINT(vformat("BTTask: Task script should be a \"tool\" script!"));
			} else {
				return get_script_instance()->call(LimboStringNames::get_singleton()->_generate_name);
			}
		}
		return _generate_name();
#elif LIMBOAI_GDEXTENSION
		String s;
		VCALL_OR_NATIVE_V(_generate_name, String, s);
		return s;
#endif
	}
	return data.custom_name;
}

Ref<BTTask> BTTask::get_root() const {
	const BTTask *task = this;
	while (!task->is_root()) {
		task = task->data.parent;
	}
	return Ref<BTTask>(task);
}

void BTTask::set_custom_name(const String &p_name) {
	if (data.custom_name != p_name) {
		data.custom_name = p_name;
		emit_changed();
	}
};

void BTTask::initialize(Node *p_agent, const Ref<Blackboard> &p_blackboard) {
	ERR_FAIL_COND(p_agent == nullptr);
	ERR_FAIL_COND(p_blackboard == nullptr);
	data.agent = p_agent;
	data.blackboard = p_blackboard;
	for (int i = 0; i < data.children.size(); i++) {
		get_child(i)->initialize(p_agent, p_blackboard);
	}

	VCALL_OR_NATIVE(_setup);
}

Ref<BTTask> BTTask::clone() const {
	Ref<BTTask> inst = duplicate(false);
	inst->data.parent = nullptr;
	inst->data.agent = nullptr;
	inst->data.blackboard.unref();
	int num_null = 0;
	for (int i = 0; i < data.children.size(); i++) {
		Ref<BTTask> c = get_child(i)->clone();
		if (c.is_valid()) {
			c->data.parent = inst.ptr();
			c->data.index = i;
			inst->data.children.set(i - num_null, c);
		} else {
			num_null += 1;
		}
	}
	if (num_null > 0) {
		// * BTComment tasks return nullptr at runtime - we remove those.
		inst->data.children.resize(data.children.size() - num_null);
	}

#ifdef LIMBOAI_MODULE
	// Make BBParam properties unique.
	List<PropertyInfo> props;
	inst->get_property_list(&props);
	HashMap<Ref<Resource>, Ref<Resource>> duplicates;
	for (List<PropertyInfo>::Element *E = props.front(); E; E = E->next()) {
		if (!(E->get().usage & PROPERTY_USAGE_STORAGE)) {
			continue;
		}

		Variant v = inst->get(E->get().name);

		if (v.is_ref_counted()) {
			Ref<RefCounted> ref = v;
			if (ref.is_valid()) {
				Ref<Resource> res = ref;
				if (res.is_valid() && res->is_class("BBParam")) {
					if (!duplicates.has(res)) {
						duplicates[res] = res->duplicate();
					}
					res = duplicates[res];
					inst->set(E->get().name, res);
				}
			}
		}
	}
#elif LIMBOAI_GDEXTENSION
	// Make BBParam properties unique.
	TypedArray<Dictionary> props = inst->get_property_list();
	HashMap<Ref<Resource>, Ref<Resource>> duplicates;
	for (int i = 0; i < props.size(); i++) {
		Dictionary prop = props[i];
		if (!(int(prop["usage"]) & PROPERTY_USAGE_STORAGE)) {
			continue;
		}

		StringName prop_name = prop["name"];
		Variant v = inst->get(prop_name);

		if (v.get_type() == Variant::OBJECT && int(prop["hint"]) == PROPERTY_HINT_RESOURCE_TYPE) {
			Ref<RefCounted> ref = v;
			if (ref.is_valid()) {
				Ref<Resource> res = ref;
				if (res.is_valid() && res->is_class("BBParam")) {
					if (!duplicates.has(res)) {
						duplicates[res] = res->duplicate();
					}
					res = duplicates[res];
					inst->set(prop_name, res);
				}
			}
		}
	}
#endif // LIMBOAI_MODULE & LIMBOAI_GDEXTENSION

	return inst;
}

BT::Status BTTask::execute(double p_delta) {
	if (data.status != RUNNING) {
		// Reset children status.
		if (data.status != FRESH) {
			for (int i = 0; i < get_child_count(); i++) {
				data.children.get(i)->abort();
			}
		}

		VCALL_OR_NATIVE(_enter);
	} else {
		data.elapsed += p_delta;
	}

	VCALL_OR_NATIVE_ARGS_V(_tick, Status, data.status, p_delta);

	if (data.status != RUNNING) {
		VCALL_OR_NATIVE(_exit);
		data.elapsed = 0.0;
	}
	return data.status;
}

void BTTask::abort() {
	for (int i = 0; i < data.children.size(); i++) {
		get_child(i)->abort();
	}
	if (data.status == RUNNING) {
		VCALL_OR_NATIVE(_exit);
	}
	data.status = FRESH;
	data.elapsed = 0.0;
}

int BTTask::get_child_count_excluding_comments() const {
	int count = 0;
	for (int i = 0; i < data.children.size(); i++) {
		if (!IS_CLASS(data.children[i], BTComment)) {
			count += 1;
		}
	}
	return count;
}

void BTTask::add_child(Ref<BTTask> p_child) {
	ERR_FAIL_COND_MSG(p_child->get_parent().is_valid(), "p_child already has a parent!");
	p_child->data.parent = this;
	p_child->data.index = data.children.size();
	data.children.push_back(p_child);
	emit_changed();
}

void BTTask::add_child_at_index(Ref<BTTask> p_child, int p_idx) {
	ERR_FAIL_COND_MSG(p_child->get_parent().is_valid(), "p_child already has a parent!");
	if (p_idx < 0 || p_idx > data.children.size()) {
		p_idx = data.children.size();
	}
	data.children.insert(p_idx, p_child);
	p_child->data.parent = this;
	p_child->data.index = p_idx;
	for (int i = p_idx + 1; i < data.children.size(); i++) {
		get_child(i)->data.index = i;
	}
	emit_changed();
}

void BTTask::remove_child(Ref<BTTask> p_child) {
	int idx = data.children.find(p_child);
	ERR_FAIL_COND_MSG(idx == -1, "p_child not found!");
	data.children.remove_at(idx);
	p_child->data.parent = nullptr;
	p_child->data.index = -1;
	for (int i = idx; i < data.children.size(); i++) {
		get_child(i)->data.index = i;
	}
	emit_changed();
}

void BTTask::remove_child_at_index(int p_idx) {
	ERR_FAIL_INDEX(p_idx, get_child_count());
	data.children[p_idx]->data.parent = nullptr;
	data.children[p_idx]->data.index = -1;
	data.children.remove_at(p_idx);
	for (int i = p_idx; i < data.children.size(); i++) {
		get_child(i)->data.index = i;
	}
	emit_changed();
}

bool BTTask::is_descendant_of(const Ref<BTTask> &p_task) const {
	const BTTask *task = this;
	while (task != nullptr) {
		task = task->data.parent;
		if (task == p_task.ptr()) {
			return true;
		}
	}
	return false;
}

Ref<BTTask> BTTask::next_sibling() const {
	if (data.parent != nullptr) {
		if (get_index() != -1 && data.parent->get_child_count() > (get_index() + 1)) {
			return data.parent->get_child(get_index() + 1);
		}
	}
	return Ref<BTTask>();
}

PackedStringArray BTTask::_get_configuration_warnings() {
	return PackedStringArray();
}

PackedStringArray BTTask::get_configuration_warnings() {
	PackedStringArray ret;

	PackedStringArray warnings;
	VCALL_V(_get_configuration_warnings, warnings); // Get script warnings.
	ret.append_array(warnings);
	ret.append_array(_get_configuration_warnings());

	return ret;
}

void BTTask::print_tree(int p_initial_tabs) {
	String tabs = "--";
	for (int i = 0; i < p_initial_tabs; i++) {
		tabs += "--";
	}

	PRINT_LINE(vformat("%s Name: %s Instance: %s", tabs, get_task_name(), Ref<BTTask>(this)));

	for (int i = 0; i < get_child_count(); i++) {
		get_child(i)->print_tree(p_initial_tabs + 1);
	}
}

void BTTask::_bind_methods() {
	// Public Methods.
	ClassDB::bind_method(D_METHOD("is_root"), &BTTask::is_root);
	ClassDB::bind_method(D_METHOD("get_root"), &BTTask::get_root);
	ClassDB::bind_method(D_METHOD("initialize", "p_agent", "p_blackboard"), &BTTask::initialize);
	ClassDB::bind_method(D_METHOD("clone"), &BTTask::clone);
	ClassDB::bind_method(D_METHOD("execute", "p_delta"), &BTTask::execute);
	ClassDB::bind_method(D_METHOD("get_child", "p_idx"), &BTTask::get_child);
	ClassDB::bind_method(D_METHOD("get_child_count"), &BTTask::get_child_count);
	ClassDB::bind_method(D_METHOD("get_child_count_excluding_comments"), &BTTask::get_child_count_excluding_comments);
	ClassDB::bind_method(D_METHOD("add_child", "p_child"), &BTTask::add_child);
	ClassDB::bind_method(D_METHOD("add_child_at_index", "p_child", "p_idx"), &BTTask::add_child_at_index);
	ClassDB::bind_method(D_METHOD("remove_child", "p_child"), &BTTask::remove_child);
	ClassDB::bind_method(D_METHOD("remove_child_at_index", "p_idx"), &BTTask::remove_child_at_index);
	ClassDB::bind_method(D_METHOD("has_child", "p_child"), &BTTask::has_child);
	ClassDB::bind_method(D_METHOD("is_descendant_of", "p_task"), &BTTask::is_descendant_of);
	ClassDB::bind_method(D_METHOD("get_index"), &BTTask::get_index);
	ClassDB::bind_method(D_METHOD("next_sibling"), &BTTask::next_sibling);
	ClassDB::bind_method(D_METHOD("print_tree", "p_initial_tabs"), &BTTask::print_tree, Variant(0));
	ClassDB::bind_method(D_METHOD("get_task_name"), &BTTask::get_task_name);
	ClassDB::bind_method(D_METHOD("abort"), &BTTask::abort);

	// Properties, setters and getters.
	ClassDB::bind_method(D_METHOD("get_agent"), &BTTask::get_agent);
	ClassDB::bind_method(D_METHOD("set_agent", "p_agent"), &BTTask::set_agent);
	ClassDB::bind_method(D_METHOD("_get_children"), &BTTask::_get_children);
	ClassDB::bind_method(D_METHOD("_set_children", "p_children"), &BTTask::_set_children);
	ClassDB::bind_method(D_METHOD("get_blackboard"), &BTTask::get_blackboard);
	ClassDB::bind_method(D_METHOD("get_parent"), &BTTask::get_parent);
	ClassDB::bind_method(D_METHOD("get_status"), &BTTask::get_status);
	ClassDB::bind_method(D_METHOD("get_elapsed_time"), &BTTask::get_elapsed_time);
	ClassDB::bind_method(D_METHOD("get_custom_name"), &BTTask::get_custom_name);
	ClassDB::bind_method(D_METHOD("set_custom_name", "p_name"), &BTTask::set_custom_name);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "custom_name"), "set_custom_name", "get_custom_name");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "agent", PROPERTY_HINT_RESOURCE_TYPE, "Node", PROPERTY_USAGE_NONE), "set_agent", "get_agent");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "blackboard", PROPERTY_HINT_RESOURCE_TYPE, "Blackboard", PROPERTY_USAGE_NONE), "", "get_blackboard");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL), "_set_children", "_get_children");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "status", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "", "get_status");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "elapsed_time", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "", "get_elapsed_time");

#ifdef LIMBOAI_MODULE
	GDVIRTUAL_BIND(_setup);
	GDVIRTUAL_BIND(_enter);
	GDVIRTUAL_BIND(_exit);
	GDVIRTUAL_BIND(_tick, "p_delta");
	GDVIRTUAL_BIND(_generate_name);
	GDVIRTUAL_BIND(_get_configuration_warnings);
#elif LIMBOAI_GDEXTENSION
	// TODO: Registering virtual functions is not available in godot-cpp...
#endif
}

BTTask::BTTask() {
}

BTTask::~BTTask() {
	for (int i = 0; i < get_child_count(); i++) {
		ERR_FAIL_COND(!get_child(i).is_valid());
		get_child(i)->data.parent = nullptr;
		get_child(i).unref();
	}
}
