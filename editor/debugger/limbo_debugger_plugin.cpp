/**
 * limbo_debugger_plugin.cpp
 * =============================================================================
 * Copyright 2021-2023 Serhii Snitsaruk
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */

#ifdef TOOLS_ENABLED

#include "limbo_debugger_plugin.h"

#include "../../bt/behavior_tree.h"
#include "../../editor/debugger/behavior_tree_data.h"
#include "../../editor/debugger/behavior_tree_view.h"
#include "../../util/limbo_utility.h"
#include "limbo_debugger.h"

#ifdef LIMBOAI_MODULE
#include "core/debugger/engine_debugger.h"
#include "core/error/error_macros.h"
#include "core/math/math_defs.h"
#include "core/object/callable_method_pointer.h"
#include "core/os/memory.h"
#include "core/string/print_string.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "editor/editor_interface.h"
#include "editor/editor_scale.h"
#include "editor/filesystem_dock.h"
#include "editor/plugins/editor_debugger_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/texture_rect.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/file_system_dock.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/tab_container.hpp>
#endif // LIMBOAI_GDEXTENSION

//**** LimboDebuggerTab

void LimboDebuggerTab::_reset_controls() {
	bt_player_list->clear();
	bt_view->clear();
	alert_box->hide();
	info_message->set_text(TTR("Run project to start debugging."));
	info_message->show();
	resource_header->set_disabled(true);
	resource_header->set_text(TTR("Inactive"));
}

void LimboDebuggerTab::start_session() {
	bt_player_list->clear();
	bt_view->clear();
	alert_box->hide();
	info_message->set_text(TTR("Pick a player from the list to display behavior tree."));
	info_message->show();
	session->send_message("limboai:start_session", Array());
}

void LimboDebuggerTab::stop_session() {
	_reset_controls();
	session->send_message("limboai:stop_session", Array());
}

void LimboDebuggerTab::update_active_bt_players(const Array &p_node_paths) {
	active_bt_players.clear();
	for (int i = 0; i < p_node_paths.size(); i++) {
		active_bt_players.push_back(p_node_paths[i]);
	}
	_update_bt_player_list(active_bt_players, filter_players->get_text());
}

String LimboDebuggerTab::get_selected_bt_player() {
	if (!bt_player_list->is_anything_selected()) {
		return "";
	}
	return bt_player_list->get_item_text(bt_player_list->get_selected_items()[0]);
}

void LimboDebuggerTab::update_behavior_tree(const BehaviorTreeData &p_data) {
	resource_header->set_text(p_data.bt_resource_path);
	resource_header->set_disabled(false);
	bt_view->update_tree(p_data);
	info_message->hide();
}

void LimboDebuggerTab::_show_alert(const String &p_message) {
	alert_message->set_text(p_message);
	alert_box->set_visible(!p_message.is_empty());
}

void LimboDebuggerTab::_update_bt_player_list(const List<String> &p_node_paths, const String &p_filter) {
	// Remember selected item.
	String selected_player = "";
	if (bt_player_list->is_anything_selected()) {
		selected_player = bt_player_list->get_item_text(bt_player_list->get_selected_items()[0]);
	}

	bt_player_list->clear();
	int select_idx = -1;
	bool selection_filtered_out = false;
	for (const String &p : p_node_paths) {
		if (p_filter.is_empty() || p.contains(p_filter)) {
			int idx = bt_player_list->add_item(p);
			// Make item text shortened from the left, e.g ".../Agent/BTPlayer".
			bt_player_list->set_item_text_direction(idx, TEXT_DIRECTION_RTL);
			if (p == selected_player) {
				select_idx = idx;
			}
		} else if (p == selected_player) {
			selection_filtered_out = true;
		}
	}

	// Restore selected item.
	if (select_idx > -1) {
		bt_player_list->select(select_idx);
	} else if (!selected_player.is_empty()) {
		if (selection_filtered_out) {
			session->send_message("limboai:untrack_bt_player", Array());
			bt_view->clear();
			_show_alert("");
		} else {
			_show_alert("BehaviorTree player is no longer present.");
		}
	}
}

void LimboDebuggerTab::_bt_selected(int p_idx) {
	alert_box->hide();
	bt_view->clear();
	info_message->set_text(TTR("Waiting for behavior tree update."));
	info_message->show();
	resource_header->set_text(TTR("Waiting for data"));
	resource_header->set_disabled(true);
	NodePath path = bt_player_list->get_item_text(p_idx);
	Array msg_data;
	msg_data.push_back(path);
	session->send_message("limboai:track_bt_player", msg_data);
}

void LimboDebuggerTab::_filter_changed(String p_text) {
	_update_bt_player_list(active_bt_players, p_text);
}

void LimboDebuggerTab::_window_visibility_changed(bool p_visible) {
	make_floating->set_visible(!p_visible);
}

void LimboDebuggerTab::_resource_header_pressed() {
	String bt_path = resource_header->get_text();
	if (bt_path.is_empty()) {
		return;
	}
	FS_DOCK_SELECT_FILE(bt_path);
	Ref<BehaviorTree> bt = RESOURCE_LOAD(bt_path, "BehaviorTree");
	ERR_FAIL_COND_MSG(!bt.is_valid(), "Failed to load BehaviorTree. Wrong resource path?");
	EditorInterface::get_singleton()->edit_resource(bt);
}

void LimboDebuggerTab::_bind_methods() {
}

void LimboDebuggerTab::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			resource_header->connect(LW_NAME(pressed), callable_mp(this, &LimboDebuggerTab::_resource_header_pressed));
			filter_players->connect(LW_NAME(text_changed), callable_mp(this, &LimboDebuggerTab::_filter_changed));
			bt_player_list->connect(LW_NAME(item_selected), callable_mp(this, &LimboDebuggerTab::_bt_selected));
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			alert_icon->set_texture(get_theme_icon(LW_NAME(StatusWarning), LW_NAME(EditorIcons)));
			BUTTON_SET_ICON(resource_header, LimboUtility::get_singleton()->get_task_icon("BehaviorTree"));
		} break;
	}
}

void LimboDebuggerTab::setup(Ref<EditorDebuggerSession> p_session, CompatWindowWrapper *p_wrapper) {
	session = p_session;
	window_wrapper = p_wrapper;

	if (p_wrapper->is_window_available()) {
		make_floating = memnew(CompatScreenSelect);
		make_floating->set_flat(true);
		make_floating->set_h_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_END);
		make_floating->set_tooltip_text(TTR("Make the LimboAI Debugger floating."));
		make_floating->connect(LW_NAME(request_open_in_screen), callable_mp(window_wrapper, &CompatWindowWrapper::enable_window_on_screen).bind(true));
		toolbar->add_child(make_floating);
		p_wrapper->connect(LW_NAME(window_visibility_changed), callable_mp(this, &LimboDebuggerTab::_window_visibility_changed));
	}

	_reset_controls();
}

LimboDebuggerTab::LimboDebuggerTab() {
	root_vb = memnew(VBoxContainer);
	add_child(root_vb);

	toolbar = memnew(HBoxContainer);
	root_vb->add_child(toolbar);

	resource_header = memnew(Button);
	toolbar->add_child(resource_header);
	resource_header->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
	resource_header->set_focus_mode(FOCUS_NONE);
	resource_header->add_theme_constant_override("hseparation", 8);
	resource_header->set_text(TTR("Inactive"));
	resource_header->set_tooltip_text(TTR("Debugged BehaviorTree resource.\nClick to open."));
	resource_header->set_disabled(true);

	hsc = memnew(HSplitContainer);
	hsc->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	hsc->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	root_vb->add_child(hsc);

	VBoxContainer *list_box = memnew(VBoxContainer);
	hsc->add_child(list_box);

	filter_players = memnew(LineEdit);
	filter_players->set_placeholder(TTR("Filter Players"));
	list_box->add_child(filter_players);

	bt_player_list = memnew(ItemList);
	bt_player_list->set_custom_minimum_size(Size2(240.0 * EDSCALE, 0.0));
	bt_player_list->set_h_size_flags(SIZE_FILL);
	bt_player_list->set_v_size_flags(SIZE_EXPAND_FILL);
	list_box->add_child(bt_player_list);

	view_box = memnew(VBoxContainer);
	hsc->add_child(view_box);

	bt_view = memnew(BehaviorTreeView);
	bt_view->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	bt_view->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	view_box->add_child(bt_view);

	alert_box = memnew(HBoxContainer);
	alert_box->hide();
	view_box->add_child(alert_box);

	alert_icon = memnew(TextureRect);
	alert_box->add_child(alert_icon);
	alert_icon->set_stretch_mode(TextureRect::STRETCH_KEEP_CENTERED);

	alert_message = memnew(Label);
	alert_box->add_child(alert_message);
	alert_message->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);

	info_message = memnew(Label);
	info_message->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
	info_message->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	info_message->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
	info_message->set_custom_minimum_size(Size2(100 * EDSCALE, 0));
	info_message->set_anchors_and_offsets_preset(PRESET_FULL_RECT, PRESET_MODE_KEEP_SIZE, 8 * EDSCALE);

	bt_view->add_child(info_message);
}

//**** LimboDebuggerPlugin

LimboDebuggerPlugin *LimboDebuggerPlugin::singleton = nullptr;

void LimboDebuggerPlugin::_window_visibility_changed(bool p_visible) {
}

#ifdef LIMBOAI_MODULE
void LimboDebuggerPlugin::setup_session(int p_idx) {
#elif LIMBOAI_GDEXTENSION
void LimboDebuggerPlugin::_setup_session(int32_t p_idx) {
#endif
	Ref<EditorDebuggerSession> session = get_session(p_idx);

	if (tab != nullptr) {
		tab->queue_free();
		window_wrapper->queue_free();
	}

	window_wrapper = memnew(CompatWindowWrapper);
	window_wrapper->set_window_title(vformat(TTR("%s - Godot Engine"), TTR("LimboAI Debugger")));
	window_wrapper->set_margins_enabled(true);

	tab = memnew(LimboDebuggerTab());
	tab->setup(session, window_wrapper);
	tab->set_name("LimboAI");
	window_wrapper->set_wrapped_control(tab);
	window_wrapper->set_name("LimboAI");

	window_wrapper->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	window_wrapper->connect(LW_NAME(window_visibility_changed), callable_mp(this, &LimboDebuggerPlugin::_window_visibility_changed));

	session->connect(LW_NAME(started), callable_mp(tab, &LimboDebuggerTab::start_session));
	session->connect(LW_NAME(stopped), callable_mp(tab, &LimboDebuggerTab::stop_session));
	session->add_session_tab(window_wrapper);
}

#ifdef LIMBOAI_MODULE
bool LimboDebuggerPlugin::capture(const String &p_message, const Array &p_data, int p_session) {
#elif LIMBOAI_GDEXTENSION
bool LimboDebuggerPlugin::_capture(const String &p_message, const Array &p_data, int32_t p_session) {
#endif
	bool captured = true;
	if (p_message == "limboai:active_bt_players") {
		tab->update_active_bt_players(p_data);
	} else if (p_message == "limboai:bt_update") {
		BehaviorTreeData data = BehaviorTreeData();
		data.deserialize(p_data);
		if (data.bt_player_path == NodePath(tab->get_selected_bt_player())) {
			tab->update_behavior_tree(data);
		}
	} else {
		captured = false;
	}
	return captured;
}

#ifdef LIMBOAI_MODULE
bool LimboDebuggerPlugin::has_capture(const String &p_capture) const {
#elif LIMBOAI_GDEXTENSION
bool LimboDebuggerPlugin::_has_capture(const String &p_capture) const {
#endif
	return p_capture == "limboai";
}

CompatWindowWrapper *LimboDebuggerPlugin::get_session_tab() const {
	return window_wrapper;
}

int LimboDebuggerPlugin::get_session_tab_index() const {
	TabContainer *c = Object::cast_to<TabContainer>(window_wrapper->get_parent());
	ERR_FAIL_COND_V(c == nullptr, -1);
	return c->get_tab_idx_from_control(window_wrapper);
}

void LimboDebuggerPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_window_visibility_changed"), &LimboDebuggerPlugin::_window_visibility_changed);
}

LimboDebuggerPlugin::LimboDebuggerPlugin() {
	tab = nullptr;
	singleton = this;
}

LimboDebuggerPlugin::~LimboDebuggerPlugin() {
	singleton = nullptr;
}

#endif // ! TOOLS_ENABLED
