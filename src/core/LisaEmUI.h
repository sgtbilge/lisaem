#ifndef LISAEMUI_H
#define LISAEMUI_H

#include <string>
#include <cstdint>

/**
 * @brief LisaEmUI abstract interface.
 * 
 * This interface decouples the core emulation from the UI toolkit.
 */
class LisaEmUI {
public:
    virtual ~LisaEmUI() {}

    // Logging and status
    virtual void Log(int level, const char* message) = 0;
    virtual void SetStatus(const std::string& message) = 0;
    virtual void Error(const std::string& message, const std::string& title) = 0;
    virtual bool Confirm(const std::string& message, const std::string& title) = 0;

    // Display
    virtual void RefreshDisplay(uint32_t now) = 0;
    virtual void SetVideoMode(int mode) = 0;
    virtual void FlushScreen() = 0;
    virtual void BlackOut() = 0;

    // System actions
    virtual void OnPowerOff() = 0;
    virtual void OnReboot() = 0;

    // Audio/Visual effects
    virtual void PlaySound(const std::string& name) = 0;
    virtual void UpdateFloppyAnimation(int state) = 0;
    virtual void UpdatePowerButtonAnimation(int state) = 0;
};

#endif // LISAEMUI_H
