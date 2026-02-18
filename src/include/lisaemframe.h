#ifndef LISAEMFRAME_H
#define LISAEMFRAME_H

#include <wx/wx.h>
#include "../core/LisaEmCore.h"
#include "../host/wxui/wxLisaEmUI.h"

// Event table for LisaEmFrame
enum
{
    ID_SCREENSHOT = 10001, // anti-aliased screenshot
    ID_SCREENSHOT_FULL,        // screenshot with screen
    ID_SCREENSHOT_RAW,        // raw screenshot - no aliasing

    ID_FUSH_PRNT,

#ifdef TRACE
    ID_DEBUG,
    ID_DEBUG2,
    ID_RAMDUMP,
#endif
#ifdef CPU_CORE_TESTER
    ID_CORETEST,
    ID_CORETEST_CLICK,
#endif

    ID_DEBUGGER,
    ID_POWERKEY,
    ID_APPLEPOWERKEY,

    ID_KEY_APL_DOT,
    ID_KEY_APL_S,
    ID_KEY_APL_ENTER,
    ID_KEY_APL_RENTER,
    ID_KEY_APL_1,
    ID_KEY_APL_2,
    ID_KEY_APL_3,
    ID_KEY_NMI,

    ID_KEY_RESET,

    ID_PROFILEPWR,

    ID_ZOOM,

    ID_PROFILE_ALL_ON,
    ID_PROFILE_ALL_OFF,

    ID_PROFILE_S1L,
    ID_PROFILE_S1U,
    ID_PROFILE_S2L,
    ID_PROFILE_S2U,
    ID_PROFILE_S3L,
    ID_PROFILE_S3U,

    ID_PROFILE_NEW,

    ID_FLOPPY,
    ID_NewFLOPPY,

    ID_PAUSE,
#ifdef DEBUG
    ID_SCREENREGION,
#endif
    ID_KEY_OPT_0,
    ID_KEY_OPT_4,
    ID_KEY_OPT_7,
    ID_KEY_WD2501,

    ID_KEYBOARD,
    ID_ASCIIKB,
    ID_RAWKB,
    ID_RAWKBBUF,

    ID_EMULATION_TIMER,

    ID_THROTTLE1,
    ID_THROTTLE5,
    ID_THROTTLE8,
    ID_THROTTLE10,
    ID_THROTTLE12,
    ID_THROTTLE16,
    ID_THROTTLE32,
    ID_THROTTLE48,
    ID_THROTTLE64,
    ID_THROTTLE100,
    ID_THROTTLE128,
    ID_THROTTLE256,
    ID_THROTTLEX,

    ID_ET100_75,
    ID_ET50_30,
    ID_ET40_25,
    ID_ET30_20,

    ID_LISAWEB,
    ID_LISAFAQ,
    ID_LISALIST2,

    ID_VID_HQ35X,
    ID_VID_AA,
    ID_VID_AAG,
    //ID_VID_SCALED,
    ID_VID_DY,
    ID_VID_SY,
    ID_VID_2X3Y,

    // reinstated as per request by Kallikak
    ID_REFRESH_SUB,
    ID_REFRESH_60Hz,
    ID_REFRESH_30Hz,
    ID_REFRESH_24Hz,
    ID_REFRESH_20Hz,
    ID_REFRESH_12Hz,
    ID_REFRESH_8Hz,
    ID_REFRESH_4Hz,

    ID_FORCE_REFRESH,

    ID_HIDE_HOST_MOUSE,
    ID_USE_MOUSE_SCALE,

    ID_VID_SKINS,
    ID_VID_SKINLESSCENTER,
    ID_VID_SKINSELECT,

    ID_VID_SCALED_SUB,

    ID_VID_SCALE_25,
    ID_VID_SCALE_50,
    ID_VID_SCALE_75,

    ID_VID_SCALE_100,
    ID_VID_SCALE_125,
    ID_VID_SCALE_150,
    ID_VID_SCALE_175,
    ID_VID_SCALE_200,
    ID_VID_SCALE_225,
    ID_VID_SCALE_250,
    ID_VID_SCALE_275,
    ID_VID_SCALE_300,

    ID_VID_SCALE_ZOOMIN,
    ID_VID_SCALE_ZOOMOUT,

    ID_VID_FULLSCREEN,

    ID_TOOLBAR_POWER,
    ID_TOOLBAR_FLOPPY,
    ID_TOOLBAR_FLOPPY_NEW,
    ID_TOOLBAR_PROFILE
};

enum
{
    emulation_off=0,
    emulation_running=1,
    emulation_paused=10,
    emuation_paused_for_screen=11
};

class LisaEmFrame : public wxFrame
{
public:

    int running;          // is the Lisa running?  0=off, 1=running, 10=paused/locked.

    // Constructor
    LisaEmFrame(const wxString& title);

    void LoadImages(void);
    void UnloadImages(void);

    // Event handlers
    void OnQuit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void OnAbout(wxCommandEvent& event);

    void OnLisaWeb(wxCommandEvent& event);
    void OnLisaFaq(wxCommandEvent& event);
    void OnLisaList2(wxCommandEvent& event);

    void OnConfig(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);


    void OnRun(wxCommandEvent& event);
    void OnPause(wxCommandEvent& event);

    void OnScreenshot(wxCommandEvent& event);
    void OnScreenRegion(wxCommandEvent& event);
    void OnDebugger(wxCommandEvent& event);
    void OnPOWERKEY(wxCommandEvent& event);
    void OnAPPLEPOWERKEY(wxCommandEvent& event);
    void OnTraceLog(wxCommandEvent& event);
    void OnKEY_APL_DOT(wxCommandEvent& event);
    void OnKEY_APL_S(wxCommandEvent& event);
    void OnKEY_APL_ENTER(wxCommandEvent& event);
    void OnKEY_APL_RENTER(wxCommandEvent& event);
    void OnKEY_APL_1(wxCommandEvent& event);
    void OnKEY_APL_2(wxCommandEvent& event);
    void OnKEY_APL_3(wxCommandEvent& event);
    void OnKEY_NMI(wxCommandEvent& event);
    void OnKEY_RESET(wxCommandEvent& event);
    void OnProFilePower(wxCommandEvent& event);
    void OnProFilePowerX(int bit);
    void OnFLOPPY(wxCommandEvent& event);
    void OnNewFLOPPY(wxCommandEvent& event);

    void OnxFLOPPY(void);
    void OnxNewFLOPPY(void);

    void OnKEY_OPT_0(wxCommandEvent& event);
    void OnKEY_OPT_4(wxCommandEvent& event);
    void OnKEY_OPT_7(wxCommandEvent& event);
    void OnKEYBOARD(wxCommandEvent& event);

    void OnASCIIKB(wxCommandEvent& event);
    void OnRAWKB(wxCommandEvent& event);
    void OnRAWKBBUF(wxCommandEvent& event);
    void reset_throttle_clock(void);

    void OnThrottle5(wxCommandEvent& event);
    void OnThrottle8(wxCommandEvent& event);
    void OnThrottle10(wxCommandEvent& event);
    void OnThrottle12(wxCommandEvent& event);
    void OnThrottle16(wxCommandEvent& event);
    void OnThrottle32(wxCommandEvent& event);
    void OnThrottleX(wxCommandEvent& event);


    void SetStatusBarText(wxString &msg);
    void Update_Status(long elapsed,long idleentry);
    void VidRefresh(long now);
    int EmulateLoop(long idleentry);

    //void OnIdleEvent(wxIdleEvent& event);
    void OnEmulationTimer(wxTimerEvent& event);

    void OnPasteToKeyboard(wxCommandEvent&event);

    void UpdateProfileMenu(void);

    void FloppyAnimation(void);
    void OnVideoAntiAliased(wxCommandEvent& event);
    void OnVideoAAGray(wxCommandEvent& event);
    void OnVideoHQ35X(wxCommandEvent& event); // Added
    void OnVideoDoubleY(wxCommandEvent& event);
    void OnVideoSingleY(wxCommandEvent& event);
    void OnVideo2X3Y(wxCommandEvent& event);

    void OnSkins(wxCommandEvent& event); // Renamed from OnSkinsOn/Off to match table
    void OnSkinlessCenter(wxCommandEvent& event); // Added
    void OnSkinSelect(wxCommandEvent& event); // Added

    // Refresh rates
    void OnRefresh60(wxCommandEvent& event); // Renamed from OnRefresh60Hz
    void OnRefresh30(wxCommandEvent& event); // Added
    void OnRefresh24(wxCommandEvent& event); // Added
    void OnRefresh20(wxCommandEvent& event); // Renamed from OnRefresh20Hz
    void OnRefresh12(wxCommandEvent& event); // Renamed from OnRefresh12Hz
    void OnRefresh8(wxCommandEvent& event);  // Renamed from OnRefresh8Hz
    void OnRefresh4(wxCommandEvent& event);  // Added
    void OnForceRefresh(wxCommandEvent& event);

    void OnFlushPrint(wxCommandEvent& event);
    void OnHideHostMouse(wxCommandEvent& event);
    void OnUseMouseScale(wxCommandEvent& event); // Added

    // Scaling
    void OnScale25(wxCommandEvent& event);
    void OnScale50(wxCommandEvent& event);
    void OnScale75(wxCommandEvent& event);
    void OnScale100(wxCommandEvent& event);
    void OnScale125(wxCommandEvent& event);
    void OnScale150(wxCommandEvent& event);
    void OnScale175(wxCommandEvent& event);
    void OnScale200(wxCommandEvent& event);
    void OnScale225(wxCommandEvent& event);
    void OnScale250(wxCommandEvent& event);
    void OnScale275(wxCommandEvent& event);
    void OnScale300(wxCommandEvent& event);
    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnFullScreen(wxCommandEvent& event);

    // Other missing ones
    void OnProFilePwrOnAll(wxCommandEvent& event);
    void OnProFilePwrOffAll(wxCommandEvent& event);
    void OnProFileS1LPwr(wxCommandEvent& event);
    void OnProFileS1UPwr(wxCommandEvent& event);
    void OnProFileS2LPwr(wxCommandEvent& event);
    void OnProFileS2UPwr(wxCommandEvent& event);
    void OnProFileS3LPwr(wxCommandEvent& event);
    void OnProFileS3UPwr(wxCommandEvent& event);
    void OnNewProFile(wxCommandEvent& event);
    void OnKey_wd02501unix(wxCommandEvent& event);
    
    // Throttles
    void OnThrottle1(wxCommandEvent& event);
    void OnThrottle512(wxCommandEvent& event); // OnThrottleX in header vs OnThrottle512 in table? Table says OnThrottle512 mapped to ID_THROTTLEX
    void OnThrottle48(wxCommandEvent& event);
    void OnThrottle64(wxCommandEvent& event);
    void OnThrottle100(wxCommandEvent& event);
    void OnThrottle128(wxCommandEvent& event);
    void OnThrottle256(wxCommandEvent& event);
    
    void OnET100_75(wxCommandEvent& event);
    void OnET50_30(wxCommandEvent& event);
    void OnET40_25(wxCommandEvent& event);
    void OnET30_20(wxCommandEvent& event);

#ifdef TRACE
    void OnTraceLog2(wxCommandEvent& event);
    void DumpAllScreenshot(wxCommandEvent& event);
#endif

#ifdef CPU_CORE_TESTER
    void OnCPUCoreTester(wxCommandEvent& event);
    void OnCPUCoreTesterClick(wxCommandEvent& event);
#endif

    void OnZoom(wxZoomGestureEvent& event); // Updated signature

    void OnToolbarPower(wxCommandEvent& event);
    void OnToolbarFloppy(wxCommandEvent& event);
    void OnToolbarFloppyNew(wxCommandEvent& event);
    void OnToolbarProfile(wxCommandEvent& event);

    void insert_floppy_anim(wxString openfile);

    // Missing members
    int force_display_refresh;
    int use_mouse_scale;
    wxString resdir;
    wxString skinname;
    wxString skindir;
    wxString osslash; // Make it a member for convenience if used frequently, or I'll fix the usages.


    LisaEmConfig GetEmConfig();

    // Member variables
    LisaEmCore* m_core;
    LisaEmUI*   m_ui;

    //class LisaWin *win;
    int screensizex,screensizey;
    int depth;

    wxStopWatch runtime;               // idle loop stopwatch
    wxStopWatch soundsw;
    int soundplaying;

    uint16 lastt2;
    long    last_runtime_sample;
    long    last_decisecond;
    XTIMER clx;
    XTIMER lastclk;
    XTIMER cpu68k_reference;
    XTIMER last_runtime_cpu68k_clx;
    int dwx,dwy;

    wxString     wspaste_to_keyboard;
    char *paste_to_keyboard;
    uint32         idx_paste_to_kb;
    int            loops;
    float          throttle;
    float          clockfactor;
    float          mhzactual;
    float          saved_hidpi_scale;

    wxString floppy_to_insert;
    long lastcrtrefresh;
    long hostrefresh;
    long screen_paint_update;
    long onidle_calls;
    XTIMER cycles_wanted;

    wxTimer* m_emulation_timer;
    int barrier;

    DECLARE_EVENT_TABLE()
};

#endif // LISAEMFRAME_H
