#ifndef __LISASKINH__
#define __LISASKINH__

#include <wx/wx.h>
#include <wx/fileconf.h>
#include "machine.h"


class LisaSkin
{
public:
    int screen_origin_x,
        screen_origin_y;

    // sound file names (still needed for audio)
    wxString floppy_eject;
    wxString floppy_insert;
    wxString floppy_insert_nopower;
    wxString floppy_motor1;
    wxString floppy_motor2;
    wxString poweroffclk;
    wxString lisa_power_switch01;
    wxString lisa_power_switch02;

    void Load(wxFileConfig* config);
    void Save(wxFileConfig* config);
};

#endif
