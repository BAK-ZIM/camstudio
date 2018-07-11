/**
 * Copyright(C) 2018  Steven Hoving
 *
 * This program is free software : you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <https://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "application_settings_ui.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(application_settings_ui, CDialogEx)

application_settings_ui::application_settings_ui(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_APPLICATION_SETTINGS_UI, pParent)
{

}

application_settings_ui::application_settings_ui(CWnd* pParent, settings_model &model)
    : CDialogEx(IDD_APPLICATION_SETTINGS_UI, pParent)
    , settings_(&model)
{
}

application_settings_ui::~application_settings_ui()
{
}

BOOL application_settings_ui::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    /* minimize on record */
    minimize_on_record_checkbox_.SetCheck(settings_->get_application_minimize_on_capture_start());
    /* auto generate filename */
    auto_filename_generation_checkbox_.SetCheck(settings_->get_application_auto_filename());

    /* source fps */
    //const auto fps_text = std::to_wstring(model_->video_source_fps_);
    //video_source_fps_.SetWindowText(fps_text.c_str());
#if 0
    /* source name */
    for (const auto video_source_name : video_source::names())
        video_source_combo_.AddString(video_source_name);
    video_source_combo_.SetCurSel(model_->video_source_.get_index());

    /* video container */
    for (const auto video_container_name : video_container::names())
        video_container_combo_.AddString(video_container_name);
    video_container_combo_.SetCurSel(model_->video_container_.get_index());

    /* codec name */
    for (const auto video_codec_name : video_codec::names())
        video_codec_combo_.AddString(video_codec_name);
    video_codec_combo_.SetCurSel(model_->video_codec_.get_index());
#endif

    return TRUE;
}

void application_settings_ui::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_APPLICATION_MINIMIZE_ON_RECORD, minimize_on_record_checkbox_);
    DDX_Control(pDX, IDC_AUTO_FILENAME_GENERATION, auto_filename_generation_checkbox_);
    DDX_Control(pDX, IDC_OUTPUT_DIRECTORY_CHECKBOX, output_directory_combobox_);
    DDX_Control(pDX, IDC_OUTPUT_DIRECTORY_USER_SPECIFIED_EDIT, output_directory_user_specified_edit_);
    DDX_Control(pDX, IDC_OUTPUT_DIRECTORY_USER_SPECIFIED_BROWSE_BUTTON, output_directory_user_specified_browse_button_);
    DDX_Control(pDX, IDC_TEMP_DIRECTORY_COMBOBOX, temp_directory_combobox_);
    DDX_Control(pDX, IDC_TEMP_DIRECTORY_USER_SPECIFIED_EDIT, temp_directory_user_specified_edit_);
    DDX_Control(pDX, IDC_TEMP_DIRECTORY_USER_SPECIFIED_BROWSE_BUTTON, temp_directory_user_specified_browse_button_);
}

BEGIN_MESSAGE_MAP(application_settings_ui, CDialogEx)
    ON_BN_CLICKED(IDC_APPLICATION_MINIMIZE_ON_RECORD, &application_settings_ui::OnBnClickedApplicationMinimizeOnRecord)
    ON_BN_CLICKED(IDC_AUTO_FILENAME_GENERATION, &application_settings_ui::OnBnClickedAutoFilenameGeneration)
END_MESSAGE_MAP()


void application_settings_ui::OnBnClickedApplicationMinimizeOnRecord()
{
    settings_->set_application_minimize_on_capture_start(minimize_on_record_checkbox_.GetCheck() != 0);
}

void application_settings_ui::OnBnClickedAutoFilenameGeneration()
{
    settings_->set_application_auto_filename(auto_filename_generation_checkbox_.GetCheck() != 0);
}