#include <wx/wx.h>
#include <wx/string.h>
#include <wx/msgdlg.h>
#include <cstdarg>

// Include legacy C headers
extern "C" {
#include <vars.h>
}

#include "wxLisaEmUI.h"
#include "lisaemframe.h"

// Forward declare existing functions from lisaem_wx.cpp
extern "C" void x_setstatusbar(const char *text);
void black(void);
void setvideomode(int mode);

wxLisaEmUI::wxLisaEmUI(LisaEmFrame* frame) : m_frame(frame) {}
wxLisaEmUI::~wxLisaEmUI() {}

void wxLisaEmUI::Log(int level, const char* message) {
    (void)level;
    // For now, redirect to stderr if buglog isn't available
    if (buglog) fprintf(buglog, "CORE: %s\n", message);
    else fprintf(stderr, "CORE: %s\n", message);
}

void wxLisaEmUI::SetStatus(const std::string& message) {
    x_setstatusbar(message.c_str());
}

void wxLisaEmUI::Error(const std::string& message, const std::string& title) {
    wxMessageBox(wxString(message.c_str(), wxConvUTF8), 
                 wxString(title.c_str(), wxConvUTF8), 
                 wxOK | wxICON_ERROR);
}

bool wxLisaEmUI::Confirm(const std::string& message, const std::string& title) {
    int result = wxMessageBox(wxString(message.c_str(), wxConvUTF8), 
                              wxString(title.c_str(), wxConvUTF8), 
                              wxYES_NO | wxICON_QUESTION);
    return (result == wxYES);
}

void wxLisaEmUI::RefreshDisplay(uint32_t now) {
    if (m_frame) m_frame->VidRefresh(now);
}

void wxLisaEmUI::SetVideoMode(int mode) {
    setvideomode(mode);
}

void wxLisaEmUI::FlushScreen() {
    if (m_frame) m_frame->Update();
}

void wxLisaEmUI::BlackOut() {
    black();
}

void wxLisaEmUI::PlaySound(const std::string& name) {
    (void)name;
}

void wxLisaEmUI::UpdateFloppyAnimation(int state) {
    (void)state;
    if (m_frame) m_frame->FloppyAnimation();
}

void wxLisaEmUI::UpdatePowerButtonAnimation(int state) {
    (void)state;
}
