#ifndef LISAEMCORE_H
#define LISAEMCORE_H

#include <string>
#include <cstdint>
#include "LisaEmUI.h"

/**
 * @brief Configuration data structure, decoupled from UI toolkits.
 */
struct LisaEmConfig {
    std::string slot1, slot2, slot3;
    std::string s1l, s1h, s2l, s2h, s3l, s3h;
    std::string s1lp, s1hp, s2lp, s2hp, s3lp, s3hp;
    std::string parallel, parallelp;

    int mymaxlisaram = 1024;
    uint32_t kbid = 0;
    uint32_t iorom = 0x88;

    std::string rompath;
    std::string dualrom;
    std::string myserial;
    std::string serial1_setting;
    std::string serial1_param;
    std::string serial2_setting;
    std::string serial2_param;

    int iw_png_on = 0;
    std::string iw_png_path;
    int iw_dipsw_1 = 210;
};

/**
 * @brief LisaEmCore manages the machine emulation state and logic.
 */
class LisaEmCore {
public:
    LisaEmCore(LisaEmUI* ui);
    ~LisaEmCore();

    // Lifecycle
    int Initialize(const LisaEmConfig& config);
    void Shutdown();

    // Execution
    int EmulateLoop(uint32_t now);
    
    // Control
    void PowerOn();
    void PowerOff();
    void Reset();
    void Pause(bool paused);

    // State query
    bool IsRunning() const { return m_running; }
    bool IsPaused() const { return m_paused; }

private:
    LisaEmUI* m_ui;
    LisaEmConfig m_config;
    bool m_running;
    bool m_paused;

    // We keep legacy state pointers as private members for now
    // until we fully migrate them out of vars.h
    uint8_t* m_lisaram_ptr;
};

#endif // LISAEMCORE_H
