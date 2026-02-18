#ifndef WXLISAEMUI_H
#define WXLISAEMUI_H

#include <wx/wx.h>
#include "../../core/LisaEmUI.h"

class LisaEmFrame;

/**
 * @brief wxWidgets implementation of the LisaEmUI interface.
 */
class wxLisaEmUI : public LisaEmUI {
public:
    wxLisaEmUI(LisaEmFrame* frame);
    virtual ~wxLisaEmUI();

    // Logging and status
    void Log(int level, const char* message) override;
    void SetStatus(const std::string& message) override;
    void Error(const std::string& message, const std::string& title) override;
    bool Confirm(const std::string& message, const std::string& title) override;

    // Display
    void RefreshDisplay(uint32_t now) override;
    void SetVideoMode(int mode) override;
    void FlushScreen() override;
    void BlackOut() override;

    // Audio/Visual effects
    void PlaySound(const std::string& name) override;
    void UpdateFloppyAnimation(int state) override;
    void UpdatePowerButtonAnimation(int state) override;

private:
    LisaEmFrame* m_frame;
};

#endif // WXLISAEMUI_H
