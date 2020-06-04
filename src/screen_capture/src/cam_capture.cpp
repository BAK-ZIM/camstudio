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
#include "screen_capture/cam_capture.h"
#include "screen_capture/cam_point.h"
#include "screen_capture/cam_size.h"
#include "screen_capture/cam_draw_data.h"
#include "screen_capture/cam_stop_watch.h"
#include "screen_capture/cam_gdiplus.h"
#include "screen_capture/annotations/cam_annotation_cursor.h"
#include "screen_capture/annotations/cam_annotation_systemtime.h"
#include <cam_hook/cam_hook.h>
#include <memory>
#include <cassert>
#include <ctime>

constexpr auto CAPTURE_BPP = 32;

cam_capture_source::cam_capture_source(HWND hwnd, const cam::rect<int> & /*view*/)
    : bitmap_info_{}
    , bitmap_frame_{nullptr}
    , hwnd_{hwnd}
    , desktop_dc_{::GetDC(hwnd_)}
    , memory_dc_{::CreateCompatibleDC(desktop_dc_)}
    , src_rect_()
    , annotations_()
    , stopwatch_(std::make_unique<cam::stop_watch>())
    , virtual_screen_info_(cam::get_virtual_screen_info())
{
    if (hwnd == nullptr)
    {
        src_rect_ = virtual_screen_info_.size;
    }
    else
    {
        RECT window_rect = {};
        BOOL ret = ::GetWindowRect(hwnd, &window_rect);
        assert(ret != 0 && "Failed to get window rect.");

        src_rect_.left_ = window_rect.left;
        src_rect_.top_ = window_rect.top;
        src_rect_.right_ = window_rect.right;
        src_rect_.bottom_ = window_rect.bottom;
    }

    bitmap_info_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info_.bmiHeader.biWidth = src_rect_.width();
    bitmap_info_.bmiHeader.biHeight = -src_rect_.height();
    bitmap_info_.bmiHeader.biPlanes = 1;
    bitmap_info_.bmiHeader.biBitCount = CAPTURE_BPP;
    bitmap_info_.bmiHeader.biCompression = BI_RGB;

    bitmap_frame_ = ::CreateDIBSection(desktop_dc_, &bitmap_info_, DIB_RGB_COLORS,
        reinterpret_cast<void **>(&bitmap_data_), nullptr, 0);

    /* \todo handle CreateDIBSection failure */
    assert(bitmap_frame_);

    frame_.bitmap_info = &bitmap_info_;
    frame_.bitmap_data = bitmap_data_;

    stopwatch_->time_start();
}

cam_capture_source::~cam_capture_source()
{
    ::DeleteDC(memory_dc_);
    ::ReleaseDC(hwnd_, desktop_dc_);
}

bool cam_capture_source::capture_frame(const cam::rect<int> &capture_rect)
{
    old_selected_bitmap_ = ::SelectObject(memory_dc_, bitmap_frame_);
    const auto ret = ::BitBlt(memory_dc_, 0, 0,
        capture_rect.width(), capture_rect.height(),
        desktop_dc_,
        capture_rect.left(), capture_rect.top(),
        SRCCOPY | CAPTUREBLT);

    assert(ret);
    if (!ret)
    {
        ::SelectObject(memory_dc_, old_selected_bitmap_);
        return false;
    }

    captured_rect_ = capture_rect;

    _draw_annotations(capture_rect);

    ::SelectObject(memory_dc_, old_selected_bitmap_);

    return true;
}

const cam_frame *cam_capture_source::get_frame()
{
    frame_.width = captured_rect_.width();
    frame_.height = captured_rect_.height();
    frame_.stride = src_rect_.width() * (CAPTURE_BPP / 8);
    return &frame_;
}

void cam_capture_source::enable_annotations()
{
    enable_annotations_ = true;
}

void cam_capture_source::add_annotation(std::unique_ptr<cam_iannotation> annotation)
{
    annotations_.emplace_back(std::move(annotation));
}

void cam_capture_source::_draw_annotations(const cam::rect<int> &capture_rect)
{
    if (!enable_annotations_)
        return;
    if (annotations_.empty())
        return;

    {
        POINT pt;
        ::GetCursorPos(&pt);
        const auto mouse_point = _translate_from_virtual(pt);

        Gdiplus::Graphics canvas(memory_dc_);
        canvas.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

        const auto mouse_event_count = mouse_hook::get().get_mouse_events_count();
        if (mouse_event_count > 0)
        {
            mouse_events_.resize(mouse_event_count);
            mouse_hook::get().get_mouse_events(&mouse_events_[0], mouse_event_count);
        }

        unsigned int mouse_status = 0;
        if (!mouse_events_.empty())
        {
            for (const auto &mouse_event : mouse_events_)
            {
                switch (mouse_event.dwExtraInfo)
                {
                case WM_LBUTTONDOWN: mouse_status |= cam_mouse_button::left_button_down; break;
                case WM_LBUTTONUP:   mouse_status |= cam_mouse_button::left_button_up; break;
                case WM_RBUTTONDOWN: mouse_status |= cam_mouse_button::right_button_down; break;
                case WM_RBUTTONUP:   mouse_status |= cam_mouse_button::right_button_up; break;
                case WM_MBUTTONDOWN: mouse_status |= cam_mouse_button::middle_button_down; break;
                case WM_MBUTTONUP:   mouse_status |= cam_mouse_button::middle_button_up; break;
                }
            }
        }
        mouse_events_.clear();

        double dt = stopwatch_->time_since();
        stopwatch_->time_start();
        cam_draw_data draw_data(dt, capture_rect, mouse_point, static_cast<cam_mouse_button::type>(mouse_status));

        for (const auto &annotation : annotations_)
            annotation->draw(canvas, draw_data);
    }
}

auto cam_capture_source::_translate_from_virtual(const POINT &mouse_position) -> point<int>
{
    // virtual dataspace can be negative, but the target dataspace is always positive.
    const auto screen_rect = virtual_screen_info_.size;
    auto result = point<int>(mouse_position.x, mouse_position.y);
    result.x(result.x() - screen_rect.left());
    result.y(result.y() - screen_rect.top());

    return result;
}
