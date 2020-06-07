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

#include "cam_hook/cam_hook.h"
#include <fmt/printf.h>
#include <limits>
#include <vector>
#include <mutex>
#include <thread>
#include <cstdint>
#include <atomic>

using namespace std::chrono_literals;

struct hook_pimpl
{
    std::atomic<bool> paused_{false};
    HINSTANCE instance_{ nullptr };
    HHOOK hook_{ nullptr };

    // the mouse hook tracking part
    std::mutex mouse_events_lock_;
    std::vector<MSLLHOOKSTRUCT> mouse_events_;
    std::thread debug_unhook_;
};


auto mouse_hook::get() -> mouse_hook &
{
    static mouse_hook hook;
    return hook;
}

auto mouse_hook::global_message_proc(int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    return mouse_hook::get().message_proc(nCode, wParam, lParam);
}

mouse_hook::mouse_hook()
    : pimpl_(std::make_unique<hook_pimpl>())
{
}

mouse_hook::~mouse_hook()
{
    detach();
}

void mouse_hook::set_instance(HINSTANCE instance)
{
    pimpl_->instance_ = instance;
}

void mouse_hook::attach()
{
    clear_mouse_events();
    pimpl_->hook_ = ::SetWindowsHookEx(WH_MOUSE_LL, mouse_hook::global_message_proc, pimpl_->instance_, 0);

#if DEPLOY_BUILD == 0
    pimpl_->debug_unhook_ = std::thread([this]()
    {
        while (pimpl_->hook_ != nullptr && !IsDebuggerPresent())
            std::this_thread::sleep_for(0ms);
        fmt::print("shutting down or debugger attached\n");

        // only detach when we are not shutting down.
        if (pimpl_->hook_)
            _detach_impl();
    });
#endif
}

void mouse_hook::detach()
{
    _detach_impl();

    if (pimpl_->debug_unhook_.joinable())
        pimpl_->debug_unhook_.join();
}

void mouse_hook::pause()
{
    pimpl_->paused_ = true;
}

void mouse_hook::unpause()
{
    pimpl_->paused_ = false;
}

void mouse_hook::_detach_impl()
{
    ::UnhookWindowsHookEx(pimpl_->hook_);
    pimpl_->hook_ = nullptr;

    clear_mouse_events();
}

auto mouse_hook::message_proc(int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT
{
#if DEPLOY_BUILD == 0
    // for debugging..
    if (IsDebuggerPresent())
        _detach_impl();
#endif

    if (!pimpl_->paused_ && nCode >= HC_ACTION)
    {
        switch (wParam)
        {
        case WM_MOUSEWHEEL:
            [[fallthrough]];
        case WM_MOUSEHWHEEL:
            [[fallthrough]];
        case WM_LBUTTONDOWN:
            [[fallthrough]];
        case WM_LBUTTONUP:
            [[fallthrough]];
        case WM_RBUTTONDOWN:
            [[fallthrough]];
        case WM_RBUTTONUP:
        {
            MSLLHOOKSTRUCT mouse_event = *reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
            mouse_event.dwExtraInfo = static_cast<ULONG_PTR>(wParam);
            {
                std::lock_guard<std::mutex> slock(pimpl_->mouse_events_lock_);
                pimpl_->mouse_events_.emplace_back(mouse_event);
            }
        } break;

        // ignore
        case WM_MOUSEMOVE:
            break;
        default:
            break;
        }
    }

    return CallNextHookEx(pimpl_->hook_, nCode, wParam, lParam);
}

auto mouse_hook::get_mouse_events_count() -> int32_t
{
    std::lock_guard<std::mutex> slock(pimpl_->mouse_events_lock_);
    // \todo make lowering cast safe
    return static_cast<int32_t>(pimpl_->mouse_events_.size());
}

// We must assume that the user has actually allocated enough memory. Please don't lie to us.
auto mouse_hook::get_mouse_events(MSLLHOOKSTRUCT *dst, int32_t count) -> bool
{
    if (dst == nullptr)
        return false;

    {
        std::lock_guard<std::mutex> slock(pimpl_->mouse_events_lock_);
        const auto copy_count = std::min<int32_t>(count, static_cast<int32_t>(pimpl_->mouse_events_.size()));
        memcpy(dst, pimpl_->mouse_events_.data(), copy_count * sizeof(MSLLHOOKSTRUCT));

        // \todo replace std::vector with circular object buffer.
        const auto itr = pimpl_->mouse_events_.begin() + copy_count;
        pimpl_->mouse_events_.erase(pimpl_->mouse_events_.begin(), itr);

        if (pimpl_->mouse_events_.capacity() > 10000)
            pimpl_->mouse_events_.shrink_to_fit();

        return true;
    }
}

void mouse_hook::clear_mouse_events()
{
    std::lock_guard<std::mutex> slock(pimpl_->mouse_events_lock_);
    pimpl_->mouse_events_.clear();
    pimpl_->mouse_events_.shrink_to_fit();
}
