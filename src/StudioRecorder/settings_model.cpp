/**
 * Copyright(C) 2018 - 2020  Steven Hoving
 *
 * This program is free software : you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "settings_model.h"
#include "settings_table.h"
#include "utility/make_array.h"
#include "utility/filesystem.h"

#include <fmt/format.h>
#include <cpptoml.h>
#include <string_view>
#include <optional>
#include <iostream>

using namespace std::string_literals;

constexpr auto SETTINGS_FILENAME = L"settings.toml";

namespace config
{
namespace capture
{
    constexpr auto settings = "capture-settings";
    constexpr auto type = "capture_type";
    constexpr auto region_fixed = "capture_region_fixed";
    constexpr auto region_mouse_drag = "capture_region_mouse_drag";
    constexpr auto rect = "capture_rect";
}

namespace cursor
{
    constexpr auto settings = "cursor-settings";
    constexpr auto enabled = "cursor_enabled";

    constexpr auto halo_enabled = "cursor_halo_enabled";
    constexpr auto halo_type = "cursor_halo_type";
    constexpr auto halo_color = "cursor_halo_color";
    constexpr auto halo_size = "cursor_halo_size";

    constexpr auto click_enabled = "cursor_click_enabled";
    constexpr auto click_left_color = "cursor_click_left_color";
    constexpr auto click_right_color = "cursor_click_right_color";
    constexpr auto click_middle_color = "cursor_click_middle_color";

    constexpr auto ring_enabled = "cursor_ring_enabled";
    constexpr auto ring_threshold = "cursor_ring_threshold";
    constexpr auto ring_size = "cursor_ring_size";
    constexpr auto ring_width = "cursor_ring_width";

    constexpr auto ring_click_left_color = "cursor_ring_click_left_color";
    constexpr auto ring_click_right_color = "cursor_ring_click_right_color";
    constexpr auto ring_click_middle_color = "cursor_ring_click_middle_color";


namespace defaults
{
    constexpr auto enabled = true;

    constexpr auto halo_enabled = false;
    constexpr auto halo_type = cursor_halo_type::circle;
    constexpr auto halo_color = cam::color(0xa0FFFF80);
    constexpr auto halo_size = 100;

    constexpr auto click_enabled = false;
    constexpr auto click_left_color = cam::color(0xa0, 0xff, 0, 0);
    constexpr auto click_right_color = cam::color(0xa0, 0, 0, 0xff);
    constexpr auto click_middle_color = cam::color(0xa0, 0, 0xff, 0);

    constexpr auto ring_enabled = false;
    constexpr auto ring_threshold = 100;
    constexpr auto ring_size = 20;
    constexpr auto ring_width = 1.5;
    constexpr auto ring_click_left_color = cam::color(0xa0, 0xff, 0, 0);
    constexpr auto ring_click_right_color = cam::color(0xa0, 0, 0, 0xff);
    constexpr auto ring_click_middle_color = cam::color(0xa0, 0, 0xff, 0);
} // namespace defaults
} // namespace cursor

namespace application
{
    constexpr auto settings = "application-settings";
    constexpr auto auto_filename = "auto_filename";
    constexpr auto minimize_on_capture_start = "minimize_on_capture_start";
    constexpr auto temp_directory_access = "temp_directory_access";
    constexpr auto temp_directory = "temp_directory";
    constexpr auto output_directory_access = "output_directory_access";
    constexpr auto output_directory = "output_directory";
} // namespace application

namespace shortcut
{
    constexpr auto settings = "shortcut-settings";
} // namespace shortcut

} // namespace config

bool settings_model::get_cursor_enabled() const
{
    return cursor_enabled_;
}

void settings_model::set_cursor_ring_enabled(bool enabled)
{
    cursor_ring_enabled_ = enabled;
}

bool settings_model::get_cursor_ring_enabled() const
{
    return cursor_ring_enabled_;
}

void settings_model::set_cursor_halo_enabled(bool enabled)
{
    cursor_halo_enabled_ = enabled;
}

bool settings_model::get_cursor_halo_enabled() const
{
    return cursor_halo_enabled_;
}

void settings_model::set_cursor_halo_type(cursor_halo_type halo_type)
{
    cursor_halo_type_ = halo_type;
}

cursor_halo_type settings_model::get_cursor_halo_type() const
{
    return cursor_halo_type_;
}

void settings_model::set_cursor_halo_color(cam::color color)
{
    cursor_halo_color_ = color;
}

cam::color settings_model::get_cursor_halo_color() const
{
    return cursor_halo_color_;
}

void settings_model::set_cursor_halo_size(const int size)
{
    cursor_halo_size_ = size;
}

int settings_model::get_cursor_halo_size() const
{
    return cursor_halo_size_;
}

void settings_model::set_cursor_click_enabled(bool enabled)
{
    cursor_click_enabled_ = enabled;
}

bool settings_model::get_cursor_click_enabled() const
{
    return cursor_click_enabled_;
}

void settings_model::set_cursor_click_left_color(cam::color color)
{
    cursor_click_left_color_ = color;
}

auto settings_model::get_cursor_click_left_color() const-> cam::color
{
    return cursor_click_left_color_;
}

void settings_model::set_cursor_click_right_color(cam::color color)
{
    cursor_click_right_color_ = color;
}

auto settings_model::get_cursor_click_right_color() const -> cam::color
{
    return cursor_click_right_color_;
}

void settings_model::set_cursor_click_middle_color(cam::color color)
{
    cursor_click_middle_color_ = color;
}

auto settings_model::get_cursor_click_middle_color() const -> cam::color
{
    return cursor_click_middle_color_;
}

void settings_model::set_cursor_ring_threshold(int threshold)
{
    cursor_ring_threshold_ = threshold;
}

int settings_model::get_cursor_ring_threshold() const
{
    return cursor_ring_threshold_;
}

void settings_model::set_cursor_ring_size(int size)
{
    cursor_ring_size_ = size;
}

int settings_model::get_cursor_ring_size() const
{
    return cursor_ring_size_;
}

void settings_model::set_cursor_ring_width(double width)
{
    cursor_ring_width_ = width;
}

double settings_model::get_cursor_ring_width() const
{
    return cursor_ring_width_;
}

void settings_model::set_cursor_ring_click_left_color(cam::color color)
{
    cursor_ring_click_left_color_ =color;
}

auto settings_model::get_cursor_ring_click_left_color() const -> cam::color
{
    return cursor_ring_click_left_color_;
}

void settings_model::set_cursor_ring_click_right_color(cam::color color)
{
    cursor_ring_click_right_color_ = color;
}

auto settings_model::get_cursor_ring_click_right_color() const -> cam::color
{
    return cursor_ring_click_right_color_;
}

void settings_model::set_cursor_ring_click_middle_color(cam::color color)
{
    cursor_ring_click_middle_color_ = color;
}

auto settings_model::get_cursor_ring_click_middle_color() const -> cam::color
{
    return cursor_ring_click_middle_color_;
}

void settings_model::set_shortcut(shortcut_action::type shortcut_type, std::wstring shortcut)
{
    shortcut_settings_.at(shortcut_type).shortcut = std::move(shortcut);
}

auto settings_model::get_shortcut(shortcut_action::type shortcut_type) -> std::wstring
{
    return shortcut_settings_.at(shortcut_type).shortcut;
}

auto settings_model::get_shortcut_data(shortcut_action::type shortcut_type) -> const shortcut_definition &
{
    return shortcut_settings_.at(shortcut_type);
}

auto settings_model::get_shortcut_map() -> const shortcut_map_type &
{
    return shortcut_settings_;
}

void settings_model::set_application_auto_filename(bool auto_filename) noexcept
{
    application_auto_filename_ = auto_filename;
}

bool settings_model::get_application_auto_filename() const noexcept
{
    return application_auto_filename_;
}

void settings_model::set_application_minimize_on_capture_start(bool minimize_on_capture_start) noexcept
{
    application_minimize_on_capture_start_ = minimize_on_capture_start;
}

bool settings_model::get_application_minimize_on_capture_start() const noexcept
{
    return application_minimize_on_capture_start_;
}

void settings_model::set_application_temp_directory_type(temp_output_directory::type temp_directory_access) noexcept
{
    application_temp_directory_access_ = temp_directory_access;
}

temp_output_directory::type settings_model::get_application_temp_directory_type() const noexcept
{
    return application_temp_directory_access_;
}

void settings_model::set_application_temp_directory(std::string temp_directory)
{
    application_temp_directory_ = std::move(temp_directory);
}

const std::string &settings_model::get_application_temp_directory() const noexcept
{
    return application_temp_directory_;
}

void settings_model::set_application_output_directory_type(application_output_directory::type output_directory_access) noexcept
{
    application_output_directory_access_ = output_directory_access;
}

application_output_directory::type settings_model::get_application_output_directory_type() const noexcept
{
    return application_output_directory_access_;
}

void settings_model::set_application_output_directory(std::string output_directory)
{
    application_output_directory_ = std::move(output_directory);
}

const std::string &settings_model::get_application_output_directory() const noexcept
{
    return application_output_directory_;
}

void settings_model::save()
{
    auto root = cpptoml::make_table();
    // add a config file format version so we can handle conversions in the future.
    root->insert("version", 1);

    _save_capture_settings(*root);
    _save_cursor_settings(*root);
    _save_shotcut_settings(*root);
    _save_application_settings(*root);

    std::ofstream stream(utility::create_config_path(SETTINGS_FILENAME));
    cpptoml::toml_writer writer{stream, ""};
    root->accept(writer);
}

void settings_model::_save_capture_settings(cpptoml::table &root)
{
    table capture = cpptoml::make_table();

    capture.insert(config::capture::type, capture_type_);
    capture.insert(config::capture::region_fixed, capture_fixed_);
    capture.insert(config::capture::region_mouse_drag, capture_mouse_drag_);
    capture.insert(config::capture::rect, capture_rect_);

    root.insert(config::capture::settings, capture.get_table());
}

void settings_model::_save_cursor_settings(cpptoml::table &root)
{
    table cursor = cpptoml::make_table();

    cursor.insert(config::cursor::enabled, cursor_enabled_);
    cursor.insert(config::cursor::ring_enabled, cursor_ring_enabled_);
    cursor.insert(config::cursor::halo_enabled, cursor_halo_enabled_);
    cursor.insert(config::cursor::halo_type, cursor_halo_type_);
    cursor.insert(config::cursor::halo_color, cursor_halo_color_);
    cursor.insert(config::cursor::halo_size, cursor_halo_size_);

    cursor.insert(config::cursor::click_enabled, cursor_click_enabled_);
    cursor.insert(config::cursor::click_left_color, cursor_click_left_color_);
    cursor.insert(config::cursor::click_right_color, cursor_click_right_color_);
    cursor.insert(config::cursor::click_middle_color, cursor_click_middle_color_);

    cursor.insert(config::cursor::ring_threshold, cursor_ring_threshold_);
    cursor.insert(config::cursor::ring_size, cursor_ring_size_);
    cursor.insert(config::cursor::ring_width, cursor_ring_width_);
    cursor.insert(config::cursor::ring_click_left_color, cursor_ring_click_left_color_);
    cursor.insert(config::cursor::ring_click_right_color, cursor_ring_click_right_color_);
    cursor.insert(config::cursor::ring_click_middle_color, cursor_ring_click_middle_color_);

    root.insert(config::cursor::settings, cursor.get_table());
}

void settings_model::_save_application_settings(cpptoml::table &root)
{
    table application = cpptoml::make_table();

    application.insert(config::application::auto_filename, application_auto_filename_);
    application.insert(config::application::minimize_on_capture_start, application_minimize_on_capture_start_);
    application.insert(config::application::temp_directory_access, application_temp_directory_access_);
    application.insert(config::application::temp_directory, application_temp_directory_);
    application.insert(config::application::output_directory_access, application_output_directory_access_);
    application.insert(config::application::output_directory, application_output_directory_);

    root.insert(config::application::settings, application.get_table());
}

void settings_model::_save_shotcut_settings(cpptoml::table &root)
{
    table shortcuts = cpptoml::make_table();
    constexpr auto setting_keys = shortcut_action::setting_keys();

    if (const auto &start = shortcut_settings_.at(shortcut_action::record_start_or_pause); start.is_enabled != shortcut_enabled::unsupported)
        shortcuts.insert(setting_keys.at(shortcut_action::record_start_or_pause), start.is_enabled == shortcut_enabled::yes ? start.shortcut : L""s);

    if (const auto &stop = shortcut_settings_.at(shortcut_action::record_stop); stop.is_enabled != shortcut_enabled::unsupported)
        shortcuts.insert(setting_keys.at(shortcut_action::record_stop), stop.is_enabled == shortcut_enabled::yes ? stop.shortcut : L""s);

    if (const auto &cancel = shortcut_settings_.at(shortcut_action::record_cancel); cancel.is_enabled != shortcut_enabled::unsupported)
        shortcuts.insert(setting_keys.at(shortcut_action::record_cancel), cancel.is_enabled == shortcut_enabled::yes ? cancel.shortcut : L""s);

    // currently not supported.
#if 0
    if (const auto &zoom = shortcut_settings_.at(shortcut_action::zoom); zoom.is_enabled == shortcut_enabled::yes)
        shortcuts.insert(setting_keys.at(shortcut_action::zoom), zoom.shortcut);

    if (const auto &autopan = shortcut_settings_.at(shortcut_action::autopan); autopan.is_enabled == shortcut_enabled::yes)
        shortcuts.insert(setting_keys.at(shortcut_action::autopan), autopan.shortcut);
#endif
    root.insert(config::shortcut::settings, shortcuts.get_table());
}

void settings_model::_load_capture_settings(const cpptoml::table &root)
{
    table capture = root.get_table(config::capture::settings);
    capture_type_ = capture.get_optional<capture_type>(config::capture::type, capture_type::select_screen);
    capture_fixed_ = capture.get_optional<bool>(config::capture::region_fixed, false);
    capture_mouse_drag_ = capture.get_optional<bool>(config::capture::region_mouse_drag, false);
    capture_rect_ = capture.get_optional<cam::rect<int>>(config::capture::rect, {});
}

void settings_model::_load_cursor_settings(const cpptoml::table &root)
{
    const table cursor = root.get_table(config::cursor::settings);

    cursor_enabled_ = cursor.get_optional(config::cursor::enabled,
        config::cursor::defaults::enabled);

    // halo configuration
    cursor_halo_enabled_ = cursor.get_optional(config::cursor::halo_enabled,
        config::cursor::defaults::halo_enabled);

    cursor_halo_type_ = cursor.get_optional(config::cursor::halo_type,
        config::cursor::defaults::halo_type);

    cursor_halo_color_ = cursor.get_optional(config::cursor::halo_color,
        config::cursor::defaults::halo_color);

    cursor_halo_size_ = cursor.get_optional(config::cursor::halo_size,
        config::cursor::defaults::halo_size);

    cursor_click_enabled_ = cursor.get_optional(config::cursor::click_enabled,
        config::cursor::defaults::click_enabled);

    cursor_click_left_color_ = cursor.get_optional(config::cursor::click_left_color,
        config::cursor::defaults::click_left_color);

    cursor_click_right_color_ = cursor.get_optional(config::cursor::click_right_color,
        config::cursor::defaults::click_right_color);

    cursor_click_middle_color_ = cursor.get_optional(config::cursor::click_middle_color,
        config::cursor::defaults::click_middle_color);

    // ring configuration
    cursor_ring_enabled_ = cursor.get_optional(config::cursor::ring_enabled,
        config::cursor::defaults::ring_enabled);

    cursor_ring_threshold_ = cursor.get_optional(config::cursor::ring_threshold,
        config::cursor::defaults::ring_threshold);

    cursor_ring_size_ = cursor.get_optional(config::cursor::ring_size,
        config::cursor::defaults::ring_size);

    cursor_ring_width_ = cursor.get_optional(config::cursor::ring_width,
        config::cursor::defaults::ring_width);

    cursor_ring_click_left_color_ = cursor.get_optional(config::cursor::ring_click_left_color,
        config::cursor::defaults::ring_click_left_color);

    cursor_ring_click_right_color_ = cursor.get_optional(config::cursor::ring_click_right_color,
        config::cursor::defaults::ring_click_right_color);

    cursor_ring_click_middle_color_ = cursor.get_optional(config::cursor::ring_click_middle_color,
        config::cursor::defaults::ring_click_middle_color);
}

void settings_model::_load_application_settings(const cpptoml::table &root)
{
    table application = root.get_table(config::application::settings);
    application_auto_filename_ = application.get_optional<bool>(config::application::auto_filename, false);
    application_minimize_on_capture_start_ = application.get_optional<bool>(config::application::minimize_on_capture_start, false);
    application_temp_directory_access_ = application.get_optional<temp_output_directory::type>(config::application::temp_directory_access, temp_output_directory::user_temp);
    application_temp_directory_ = application.get_optional<std::string>(config::application::temp_directory, "");
    application_output_directory_access_ = application.get_optional<application_output_directory::type>(config::application::output_directory_access, application_output_directory::ask_user);
    application_output_directory_ = application.get_optional<std::string>(config::application::output_directory, "");
}

void settings_model::_load_shortcut_settings(const cpptoml::table &root)
{
    table shortcuts = root.get_table(config::shortcut::settings);
    _load_shortcut(shortcuts, shortcut_action::record_start_or_pause);
    _load_shortcut(shortcuts, shortcut_action::record_stop);
    _load_shortcut(shortcuts, shortcut_action::record_cancel);
    // currently not supported
    // _load_shortcut(shortcuts, shortcut_action::zoom);
    // _load_shortcut(shortcuts, shortcut_action::autopan);
}

void settings_model::_load_shortcut(const table &shortcuts, shortcut_action::type shortcut)
{
    constexpr auto keys = shortcut_action::setting_keys();

    auto &shortcut_setting = shortcut_settings_.at(shortcut);
    if (const auto field_name = keys.at(shortcut); shortcuts.contains(field_name))
    {
        const auto field = shortcuts.get<std::wstring>(field_name);
        const auto field_value = *field;
        shortcut_setting.is_enabled = field_value.empty() ? shortcut_enabled::no : shortcut_enabled::yes;
        shortcut_setting.shortcut = field_value;
    }
    else
    {
        shortcut_setting.is_enabled = shortcut_enabled::unsupported;
    }
}

void settings_model::load()
{
    const auto config_filepath = utility::create_config_path(SETTINGS_FILENAME);

    // non existing config file is not an error.
    if (!std::filesystem::exists(config_filepath))
        return;

    const auto root = cpptoml::parse_file(config_filepath.string());
    const auto version = *root->get_as<int>("version");
    assert(version == 1);

    _load_capture_settings(*root);
    _load_cursor_settings(*root);
    _load_shortcut_settings(*root);
    _load_application_settings(*root);
}

void settings_model::set_capture_mode(capture_type type)
{
    capture_type_ = type;
}

capture_type settings_model::get_capture_mode()
{
    return capture_type_;
}

void settings_model::set_capture_rect(const cam::rect<int> &capture_rect)
{
    capture_rect_ = capture_rect;
}

cam::rect<int> settings_model::get_capture_rect()
{
    return capture_rect_;
}

void settings_model::set_region_mouse_drag(bool capture_mouse_drag)
{
    capture_mouse_drag_ = capture_mouse_drag;
}

bool settings_model::get_region_mouse_drag()
{
    return capture_mouse_drag_;
}

void settings_model::set_region_fixed(bool capture_fixed)
{
    capture_fixed_ = capture_fixed;
}

bool settings_model::get_region_fixed()
{
    return capture_fixed_;
}

void settings_model::set_cursor_enabled(bool enabled)
{
    cursor_enabled_ = enabled;
}
