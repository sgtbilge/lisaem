/**************************************************************************************\
*                                                                                      *
*              The Lisa Emulator Project  V1.2.9      DEV 2007.12.04                   *
*                             http://lisaem.sunder.net                                 *
*                                                                                      *
*                  Copyright (C) 1998, MMXX Ray A. Arachelian                          *
*                                All Rights Reserved                                   *
*                                                                                      *
*           This program is free software; you can redistribute it and/or              *
*           modify it under the terms of the GNU General Public License                *
*           as published by the Free Software Foundation; either version 2             *
*           of the License, or (at your option) any later version.                     *
*                                                                                      *
*           This program is distributed in the hope that it will be useful,            *
*           but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*           MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*           GNU General Public License for more details.                               *
*                                                                                      *
*           You should have received a copy of the GNU General Public License          *
*           along with this program;  if not, write to the Free Software               *
*           Foundation, Inc., 59 Temple Place #330, Boston, MA 02111-1307, USA.        *
*                                                                                      *
*                   or visit: http://www.gnu.org/licenses/gpl.html                     *
*                                                                                      *
\**************************************************************************************/

#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/config.h>
#include <wx/event.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "machine.h"

#include "LisaConfig.h"
#include "LisaConfigFrame.h"

#include "machine.h"

#include <errno.h>

// from vars.c
extern "C" {
  extern int cheat_ram_test;
  extern int sound_effects_on;
  extern int skins_on_next_run;
  extern int hle;
  extern int macworks4mb;
  extern int double_sided_floppy;
  extern void save_configs(void);
  extern uint8 floppy_iorom;
  extern int consoletermwindow;
};

extern wxString get_config_filename(void);


extern void turn_skins_on(void);
extern void turn_skins_off(void);

extern "C" float hidpi_scale;

static wxString normalize_user_path(wxString value)
{
    value.Trim();
    value.Trim(false);
    if (value.IsEmpty()) return value;

    value = wxExpandEnvVars(value);

    if (value == _T("~"))
    {
        return wxGetHomeDir();
    }

    if (value.StartsWith(_T("~/")))
    {
        wxString home = wxGetHomeDir();
        if (home.EndsWith(_T("/"))) home.RemoveLast();
        return home + value.Mid(1);
    }

    return value;
}

static wxString get_default_profile_disks_dir()
{
    wxStandardPathsBase& stdp = wxStandardPaths::Get();
    wxString sep = wxFileName::GetPathSeparator(wxPATH_NATIVE);
    wxString dir = stdp.GetUserDataDir() + sep + _T("PROFILE_DISKS");

    if (!wxFileName::DirExists(dir)) wxFileName::Mkdir(dir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

    return dir;
}

// fix for hidpi_scale <1.0 after 2020.01.26 changes
#define HIDPISCALE (hidpi_scale<1.0 ? 1.0 : hidpi_scale)

enum {
        ID_NOTEBOOK=2001,
        ID_PREFS_OPEN,
        ID_PREFS_SAVEAS,
        ID_PREFS_HINT,
        ID_APPLY,
        ID_REVERT,
        ID_PICK_ROM,
        ID_PICK_DPROM,
        ID_PICK_KB_B,
        ID_PICK_IOROM_B,
        ID_PICK_PPORT_B,
        ID_PICK_PROFILE,
        ID_PICK_PROFILES1H,
        ID_PICK_PROFILES1L,
        ID_PICK_PROFILES2H,
        ID_PICK_PROFILES2L,
        ID_PICK_PROFILES3H,
        ID_PICK_PROFILES3L,
        ID_PICK_PROFILESB1H,
        ID_PICK_PROFILESB1L,
        ID_PICK_PROFILESB2H,
        ID_PICK_PROFILESB2L,
        ID_PICK_PROFILESB3H,
        ID_PICK_PROFILESB3L,
        ID_PICK_IWDIR,
        ID_SERNO_INFO,
        ID_ZAP_PRAM,
        ID_SAVE_PRAM,
        ID_LOAD_PRAM
};

BEGIN_EVENT_TABLE(LisaConfigFrame, wxFrame)
  EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK,  LisaConfigFrame::OnNoteBook)
  EVT_NOTEBOOK_PAGE_CHANGING(ID_NOTEBOOK, LisaConfigFrame::OnNoteBook)
  EVT_SIZE(                              LisaConfigFrame::OnResize)
  EVT_BUTTON(ID_PREFS_OPEN,              LisaConfigFrame::OnOpenPrefs)
  EVT_BUTTON(ID_PREFS_SAVEAS,            LisaConfigFrame::OnSavePrefs)
  EVT_BUTTON(ID_SERNO_INFO,               LisaConfigFrame::OnSernoInfo)
  EVT_BUTTON(ID_APPLY,                    LisaConfigFrame::OnApply)
  EVT_BUTTON(ID_REVERT,                   LisaConfigFrame::OnRevert)
  EVT_BUTTON(ID_ZAP_PRAM,                 LisaConfigFrame::OnZapPram)
  EVT_BUTTON(ID_SAVE_PRAM,                LisaConfigFrame::OnSavePram)
  EVT_BUTTON(ID_LOAD_PRAM,                LisaConfigFrame::OnLoadPram)  

  EVT_BUTTON(ID_PICK_ROM,                 LisaConfigFrame::OnPickRom)
  EVT_BUTTON(ID_PICK_DPROM,               LisaConfigFrame::OnPickDRom)

  EVT_BUTTON(ID_PICK_PROFILE,             LisaConfigFrame::OnPickProFile)

  EVT_BUTTON(ID_PICK_PROFILESB1H,         LisaConfigFrame::OnPickProFile1H)
  EVT_BUTTON(ID_PICK_PROFILESB2H,         LisaConfigFrame::OnPickProFile2H)
  EVT_BUTTON(ID_PICK_PROFILESB3H,         LisaConfigFrame::OnPickProFile3H)
  EVT_BUTTON(ID_PICK_PROFILESB1L,         LisaConfigFrame::OnPickProFile1L)
  EVT_BUTTON(ID_PICK_PROFILESB2L,         LisaConfigFrame::OnPickProFile2L)
  EVT_BUTTON(ID_PICK_PROFILESB3L,         LisaConfigFrame::OnPickProFile3L)
  EVT_BUTTON(ID_PICK_IWDIR,               LisaConfigFrame::OnPickIWDir)
END_EVENT_TABLE()

const int idth[4]={0, ID_PICK_PROFILES1H,  ID_PICK_PROFILES2H,  ID_PICK_PROFILES3H};
const int idtl[4]={0, ID_PICK_PROFILES1L,  ID_PICK_PROFILES2L,  ID_PICK_PROFILES3L}; // why is this not used? bug?
const int idbh[4]={0, ID_PICK_PROFILESB1H, ID_PICK_PROFILESB2H, ID_PICK_PROFILESB3H};
const int idbl[4]={0, ID_PICK_PROFILESB1L, ID_PICK_PROFILESB2L, ID_PICK_PROFILESB3L};
// idth - upper text field idbh - button upper
// using wxID_ANY for lower for some reason(why?), idbl - button lower line ~493

LisaConfigFrame::LisaConfigFrame(const wxString& title, LisaConfig *lisaconfig)
  : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(620*HIDPISCALE,560*HIDPISCALE),
                 wxDEFAULT_FRAME_STYLE|wxCLIP_CHILDREN |//|wxNO_FULL_REPAINT_ON_RESIZE)
                 wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCAPTION|
                 wxTAB_TRAVERSAL|wxCLOSE_BOX|wxNO_FULL_REPAINT_ON_RESIZE)
{

  my_lisaconfig = lisaconfig;
  serialabox = NULL;
  serialbbox = NULL;

  pportopts[0] = wxT("ProFile");
  pportopts[1] = wxT("ADMP");
  pportopts[2] = wxT("Nothing");

  wpportopts[0] = wxT("Widget-10");
  wpportopts[1] = wxT("ADMP");
  wpportopts[2] = wxT("Nothing");

  nothingonly[0]=_T("Nothing");
  nothingonly[1]=_T("Loopback");

  serportopts[0]=_T("Nothing");
  serportopts[1]=_T("Loopback");
  serportopts[2]=_T("Pipe");
  serportopts[3]=_T("File");
  serportopts[4]=_T("ImageWriter");
  serportopts[5]=_T("Terminal");

  #ifndef __MSVCRT__
   serportopts[6]=_T("TelnetD");
   serportopts[7]=_T("Shell");
   serportopts[8]=_T("Serial");
   serialopts=9;
  #else
   serialopts=6;
  #endif

  slotcard[0]=_T("Dual Parallel");
  slotcard[1]=_T("Nothing");

    const int minW = (int)(420 * HIDPISCALE);
    const int minH = (int)(360 * HIDPISCALE);
    SetSizeHints(minW, minH);

    // Reduced height from 650 to 560 to fit on screens with notch/menu bar
  thenoteBook = new wxNotebook(this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
  CreateNotebook(thenoteBook);

    wxWindow *hint = new wxStaticText(this, ID_PREFS_HINT, _T("Apply saves all tabs. Turn Lisa off before changing ProFile paths."));
    wxWindow *revertBtn = new wxButton(this, ID_REVERT, wxT("Revert"));
  b_apply = new wxButton(this, ID_APPLY, wxT("Apply"));

    wxBoxSizer *footerSizer = new wxBoxSizer(wxHORIZONTAL);
    footerSizer->Add(hint, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, (int)(8 * HIDPISCALE));
    footerSizer->Add(revertBtn, 0, wxRIGHT, (int)(8 * HIDPISCALE));
    footerSizer->Add(b_apply, 0);

    wxBoxSizer *rootSizer = new wxBoxSizer(wxVERTICAL);
    rootSizer->Add(thenoteBook, 1, wxEXPAND);
    rootSizer->Add(footerSizer, 0, wxEXPAND | wxALL, (int)(10 * HIDPISCALE));
    SetSizer(rootSizer);

    // Keep initial size usable on smaller displays/notched menu bars by
    // clamping to the current display work area.
    {
        wxRect work = wxGetClientDisplayRect();
        const int prefW = (int)(620 * HIDPISCALE);
        const int prefH = (int)(560 * HIDPISCALE);
        const int maxW = wxMax(minW, work.GetWidth() - (int)(40 * HIDPISCALE));
        const int maxH = wxMax(minH, work.GetHeight() - (int)(40 * HIDPISCALE));
        const int initW = wxMin(prefW, maxW);
        const int initH = wxMin(prefH, maxH);
        SetSize(initW, initH);

      int px = work.x + (work.GetWidth()  - initW) / 2;
      int py = work.y + (work.GetHeight() - initH) / 2;
      if (px < work.x) px = work.x;
      if (py < work.y) py = work.y;
      SetPosition(wxPoint(px, py));
    }

  RelayoutResponsive();
}

void LisaConfigFrame::OnNoteBook(wxNotebookEvent& WXUNUSED(event))
     {
      // The LisaTest loopback adapter attaches to both serial ports at once.
      // if one is set to loopback so, must the other follow.
      if (!serialabox || !serialbbox) return;

      if (serialabox->GetSelection()==1 || serialbbox->GetSelection()==1)
      {
       serialabox->SetSelection(1);
       serialbbox->SetSelection(1);
      }

     }

void LisaConfigFrame::RelayoutResponsive(void)
{
    if (!thenoteBook) return;

    const int margin = (int)(10 * HIDPISCALE);

    if (GetSizer()) GetSizer()->Layout();

    auto update_page_scroll = [&](wxWindow *page)
    {
        wxScrolledWindow *scrolled = wxDynamicCast(page, wxScrolledWindow);
        if (!scrolled) return;

        int pageW = scrolled->GetClientSize().GetWidth();
        if (pageW < 1) pageW = 1;

        int contentBottom = 0;
        const wxWindowList& children = scrolled->GetChildren();
        for (wxWindowList::const_iterator it = children.begin(); it != children.end(); ++it)
        {
            wxWindow *child = *it;
            if (!child || !child->IsShown()) continue;

            wxRect r = child->GetRect();
            if (r.GetBottom() > contentBottom) contentBottom = r.GetBottom();
        }

        int virtualH = contentBottom + margin * 2;
        if (virtualH < 1) virtualH = 1;

        scrolled->SetVirtualSize(pageW, virtualH);
        scrolled->FitInside();
        scrolled->SetScrollRate(0, (int)wxMax(8, (int)(16 * HIDPISCALE)));
    };

    for (size_t i = 0; i < thenoteBook->GetPageCount(); ++i)
    {
        wxWindow *page = thenoteBook->GetPage(i);
        if (!page) continue;

        if (page->GetSizer()) page->GetSizer()->Layout();
        update_page_scroll(page);
    }

    Layout();
}

void LisaConfigFrame::OnResize(wxSizeEvent& event)
{
    RelayoutResponsive();
    event.Skip();
}

extern "C" uint8 floppy_ram[2048];
extern "C" int islisarunning(void);
//extern "C" void get_lisa_serialnumber(uint32 *plant, uint32 *year, uint32 *day, uint32 *sn, uint32 *prefix, uint32 *net)

void  LisaConfigFrame::OnSernoInfo(wxCommandEvent& WXUNUSED(event))
{
    wxString myserno=serialtxt->GetValue();
    wxString hexserno, decserno;
    wxString text, trailer;
    unsigned long serno_i=0;

//    uint32 plant, year, day, sn, prefix, net;
//    get_lisa_serialnumber(&plant, &year, &day, &sn, &prefix, &net);

    hexserno=_T("0x") + myserno.SubString(9,12); // serial number
    hexserno.ToULong(&serno_i,16);
    decserno << serno_i;
    
    if      (myserno.SubString(8,8)=='1') trailer=_T("st");
    else if (myserno.SubString(8,8)=='2') trailer=_T("nd");
    else if (myserno.SubString(8,8)=='3') trailer=_T("rd");
    else                                  trailer=_T("th");

//                                               01 23 45 67 89 ab cd ef 01 23 45 67 89 ab
//floppy.c:get_lisa_serialnumber:1532:serial240: ff 02 83 08 10 40 50 ff 00 10 16 35 04 70 00 00
//                                               XX XX XP XP XY XY XD XD XD XS XS XS XS XX XX XX
//                                                      3  8 |0  0 |0  f  0| 0  6  5  4 |

    text =
    _T("Your Lisa's serial number was built\nin Apple Plant #")+
    myserno.SubString(2,3) +  // plant code
    _T(" on the ")+
    myserno.SubString(6,8) + // day of year
    trailer +_T(" day of 19")+
    myserno.SubString(4,5) + // year
    _T("\nwith serial #")+
    myserno.SubString(9,12)+ // serial number
    _T(" (") + decserno + _T(")\n")+
    _T("It has the applenet id: ")+
    myserno.SubString(16,18)+ // applenet prefix
    _T(":")+
    myserno.SubString(19,23); // applenet node number

    wxMessageBox(text,_T("About your Lisa's Serial Numer"), wxICON_INFORMATION | wxOK);
    
}

void LisaConfigFrame::OnOpenPrefs(wxCommandEvent& WXUNUSED(event))
{
    wxWindow *target = wxTheApp ? wxTheApp->GetTopWindow() : NULL;
    if (!target) return;
    wxCommandEvent e(wxEVT_MENU, wxID_OPEN);
    wxPostEvent(target, e);
    Close();
}

void LisaConfigFrame::OnSavePrefs(wxCommandEvent& WXUNUSED(event))
{
    wxWindow *target = wxTheApp ? wxTheApp->GetTopWindow() : NULL;
    if (!target) return;
    wxCommandEvent e(wxEVT_MENU, wxID_SAVEAS);
    wxPostEvent(target, e);
}

void  LisaConfigFrame::OnSavePram(wxCommandEvent& WXUNUSED(event))
{
    char *filename;
    FILE *F;

    wxFileDialog x(NULL,                        wxT("Save the Lisa PRAM to a file"),
                                                wxEmptyString,
                                                wxT("lisaem.pram"),
                                                wxT("PRAM (*.pram)|*.pram|All (*.*)|*.*"),
                                                (long int)wxFD_SAVE|wxFD_OVERWRITE_PROMPT,wxDefaultPosition);

    if (x.ShowModal()==wxID_OK) filename=(char *)(const char *)(x.GetPath().c_str());
    else return;
    errno=0;
    F=fopen(filename,"wb");
    if (!F) {
              wxMessageBox(_T("Could not open the PRAM file for writing."),_T("File Error!"), wxICON_INFORMATION | wxOK);
              return;
            }
        

    fwrite(&floppy_ram[0x180/2],(0x200-0x180)/2,1,F);
    if (errno)
       {
          wxMessageBox(_T("An error occured while attempting to write to the PRAM file.  The PRAM was not saved!"),
                       _T("Save Error!"), wxICON_INFORMATION | wxOK);
       }

    fclose(F);
}


void  LisaConfigFrame::OnLoadPram(wxCommandEvent& WXUNUSED(event))
{
    char *filename;
    FILE *F;
    int count=0;
    uint8 backup[(0x200-0x180)/2];

    if (islisarunning())
       {
          wxMessageBox(_T("Cannot Load PRAM while the Lisa is running."),_T("Lisa is running!"), wxICON_INFORMATION | wxOK);
          return;
       }

    wxString text=wxT("Loading the PRAM will overwrite the current PRAM with the data saved in a file!\n\n"
                      "You should only use this in emergencies as this can cause more problems than fix.  "
                      "If you say Yes, this will happen even if you do not press the Apply button on the Preferences window and it cannot be undone!\n"
                      "\n\nReally Load PRAM?");
    wxString title=wxT("Load PRAM?");
    wxMessageDialog w(this,text, title, wxICON_QUESTION  | wxYES_NO |wxNO_DEFAULT,wxDefaultPosition );
    if (w.ShowModal()!=wxID_YES) return;

    wxFileDialog x(NULL,                        wxT("Load a Lisa PRAM file"),
                                                wxEmptyString,
                                                wxT("lisaem.pram"),
                                                wxT("PRAM (*.pram)|*.pram|All (*.*)|*.*"),
                                                (long int)wxFD_OPEN|wxFD_FILE_MUST_EXIST,wxDefaultPosition);

    if (x.ShowModal()==wxID_OK) filename=(char *)(const char *)(x.GetPath().c_str());
    else return;
    
    memcpy(backup,&floppy_ram[0x180/2],(0x200-0x180)/2);  // make a backup
    errno=0;
    F=fopen(filename,"rb");
    if (!F) {
              wxMessageBox(_T("Could not open the PRAM file for reading."),_T("File Error!"), wxICON_INFORMATION | wxOK);
              return;
            }
        
    count=fread(&floppy_ram[0x180/2],(0x200-0x180)/2,1,F);
    if (errno || count!=1)
       {
          memcpy(&floppy_ram[0x180/2],backup,(0x200-0x180)/2);  // restore the backup if there was a failure.
          wxMessageBox(_T("An error occured while attempting to read from the PRAM file.  The PRAM was not overwrriten."),
                       _T("Load Error!"), wxICON_INFORMATION | wxOK);
       }
    fclose(F);
}


void  LisaConfigFrame::OnZapPram(wxCommandEvent& WXUNUSED(event))
{
    if (islisarunning())
       {
          wxMessageBox(_T("Cannot Zap PRAM while the Lisa is running."),_T("Lisa is running!"), wxICON_INFORMATION | wxOK);
          return;
       }

    wxString text=wxT("Zapping the PRAM will zero out the Parameter RAM.\n\n"
                      "You should only use this in emergencies as this can cause more problems than fix.  "
                      "If you say Yes, this will happen even if you do not press the Apply button on the Preferences window.\n"
                      "\n\nReally ZAP PRAM?");
    wxString title=wxT("Zap PRAM?");
    wxMessageDialog w(this,text, title, wxICON_QUESTION  | wxYES_NO |wxNO_DEFAULT,wxDefaultPosition );
    if (w.ShowModal()!=wxID_YES) return;
        
    memset(&floppy_ram[0x180/2],0,(0x200-0x180)/2);
}


void  LisaConfigFrame::OnApply(wxCommandEvent& WXUNUSED(event))
{
 // --- configuration page ----------------------------------------------

 // if it hasn't been built yet, don't touch!
 if (!serialtxt)           return;
 if (!m_rompath)           return;
 if (!kbbox)               return;
 if (!serialabox)          return;
 if (!serialbbox)          return;
 if (!serialaxon)          return;
 if (!serialbxon)          return;
 if (!m_propath)           return;
 if (!pportbox)            return;

 if (!m_text_propathh[1])  return;
 if (!m_text_propathl[1])  return;
 if (!m_text_propathh[2])  return;
 if (!m_text_propathl[2])  return;
 if (!m_text_propathh[3])  return;
 if (!m_text_propathl[3])  return;

 wxString rompath = normalize_user_path(m_rompath->GetValue());
 if (rompath.IsEmpty())
 {
   thenoteBook->SetSelection(0);
   m_rompath->SetFocus();
   wxMessageBox(_T("ROM path is required."), _T("Missing required value"), wxICON_INFORMATION | wxOK);
   return;
 }

 if (!wxFileName::FileExists(rompath))
 {
   thenoteBook->SetSelection(0);
   m_rompath->SetFocus();
   wxMessageBox(_T("ROM file does not exist. Please pick a valid Lisa ROM file."),
                _T("Invalid ROM path"), wxICON_INFORMATION | wxOK);
   return;
 }

 wxString dualrompath = normalize_user_path(m_dprompath->GetValue());
 wxString profile_path = normalize_user_path(m_propath->GetValue());

 if (pportbox->GetSelection()==0)
 {
   wxString profile_path = m_propath->GetValue();
   profile_path.Trim();
   profile_path.Trim(false);
   if (profile_path.IsEmpty())
   {
     thenoteBook->SetSelection(1);
     m_propath->SetFocus();
     wxMessageBox(_T("Built-in ProFile path is required when Parallel Port is set to ProFile."),
                  _T("Missing required value"), wxICON_INFORMATION | wxOK);
     return;
   }
 }

 if (islisarunning())
 {
   bool storage_changed = false;

   if (!pportopts[pportbox->GetSelection()].IsSameAs(my_lisaconfig->parallel, false)) storage_changed=true;
   if (!m_propath->GetValue().IsSameAs(my_lisaconfig->parallelp, false))               storage_changed=true;

   if (!slotcard[sloton[1]->GetSelection()].IsSameAs(my_lisaconfig->slot1, false))     storage_changed=true;
   if (!slotcard[sloton[2]->GetSelection()].IsSameAs(my_lisaconfig->slot2, false))     storage_changed=true;
   if (!slotcard[sloton[3]->GetSelection()].IsSameAs(my_lisaconfig->slot3, false))     storage_changed=true;

   if (!pportopts[pportboxh[1]->GetSelection()].IsSameAs(my_lisaconfig->s1h, false))   storage_changed=true;
   if (!pportopts[pportboxl[1]->GetSelection()].IsSameAs(my_lisaconfig->s1l, false))   storage_changed=true;
   if (!pportopts[pportboxh[2]->GetSelection()].IsSameAs(my_lisaconfig->s2h, false))   storage_changed=true;
   if (!pportopts[pportboxl[2]->GetSelection()].IsSameAs(my_lisaconfig->s2l, false))   storage_changed=true;
   if (!pportopts[pportboxh[3]->GetSelection()].IsSameAs(my_lisaconfig->s3h, false))   storage_changed=true;
   if (!pportopts[pportboxl[3]->GetSelection()].IsSameAs(my_lisaconfig->s3l, false))   storage_changed=true;

   if (!m_text_propathh[1]->GetValue().IsSameAs(my_lisaconfig->s1hp, false))           storage_changed=true;
   if (!m_text_propathl[1]->GetValue().IsSameAs(my_lisaconfig->s1lp, false))           storage_changed=true;
   if (!m_text_propathh[2]->GetValue().IsSameAs(my_lisaconfig->s2hp, false))           storage_changed=true;
   if (!m_text_propathl[2]->GetValue().IsSameAs(my_lisaconfig->s2lp, false))           storage_changed=true;
   if (!m_text_propathh[3]->GetValue().IsSameAs(my_lisaconfig->s3hp, false))           storage_changed=true;
   if (!m_text_propathl[3]->GetValue().IsSameAs(my_lisaconfig->s3lp, false))           storage_changed=true;

   if (storage_changed)
   {
     wxMessageBox(_T("Storage and ProFile attachment changes require the Lisa to be powered off."),
                  _T("Power off required"), wxICON_INFORMATION | wxOK);
     return;
   }
 }

 my_lisaconfig->myserial= serialtxt->GetValue();
 my_lisaconfig->rompath = rompath;
 my_lisaconfig->dualrom = dualrompath;

 my_lisaconfig->serial1xon=serialaxon->GetValue() ? "1":"0";
 my_lisaconfig->serial2xon=serialbxon->GetValue() ? "1":"0";
 
consoletermwindow = console_term->GetValue() ? 1:0;
 /*
 fprintf(stderr,"myserial number:%s\n",my_lisaconfig->myserial.c_str());
 fprintf(stderr,"rompath        :%s\n",my_lisaconfig->rompath.c_str());
 fprintf(stderr,"dual rom path  :%s\n",my_lisaconfig->dualrom.c_str());
 */

 uint16 kbids[]={ 0xBF2f, 0xAF2f, 0xAD2d, 0xAE2e };
 my_lisaconfig->kbid=kbids[kbbox->GetSelection()];


 uint8 ioromids[]={ 0xa8, 0x88, 0x89, 0xa9, 0x40 };
 my_lisaconfig->iorom=ioromids[iorombox->GetSelection()];


 int memsizes[]={512,1024,1536,2048};
 my_lisaconfig->mymaxlisaram=memsizes[cpurambox->GetSelection()];
 cheat_ram_test=cheats->GetValue() ?1:0;

 hle=hle_cheats->GetValue() ?1:0;
 macworks4mb=0; // doesn't work yet // macwx4mb->GetValue() ? 1:0;

 sound_effects_on = soundeffects->GetValue()?1:0;
 double_sided_floppy=doublesided->GetValue() ?1:0;
 
 int last_skins_on=skins_on_next_run;   // disable this to require restart.
 skins_on_next_run= skinson->GetValue()?1:0;

 if (skins_on_next_run!=last_skins_on)
 {
    if (skins_on_next_run)   turn_skins_on();
    else                     turn_skins_off();
 }


 // --- ports ------------------------------------------------------------
 my_lisaconfig->serial1_setting=serportopts[serialabox->GetSelection()];
 my_lisaconfig->serial2_setting=serportopts[serialbbox->GetSelection()];
 my_lisaconfig->serial1_param  =serialaparam->GetValue();
 my_lisaconfig->serial2_param  =serialbparam->GetValue();

 /*
 fprintf(stderr,"serial a setting:%s\n",my_lisaconfig->serial1_setting.c_str());
 fprintf(stderr,"serial b setting:%s\n",my_lisaconfig->serial2_setting.c_str());
 fprintf(stderr,"serial a param  :%s\n",my_lisaconfig->serial1_param.c_str());
 fprintf(stderr,"serial a param  :%s\n",my_lisaconfig->serial2_param.c_str());
 */

 my_lisaconfig->parallel=pportopts[pportbox->GetSelection()];
 my_lisaconfig->parallelp=profile_path;

 //fprintf(stderr,"parallel port   :%s\n",my_lisaconfig->parallel.c_str());
 //fprintf(stderr,"parameter       :%s\n\n",my_lisaconfig->parallelp.c_str());
 //fflush(stderr);



 // --- slots ------------------------------------------------------------

    my_lisaconfig->slot1=slotcard[sloton[1]->GetSelection()];
    my_lisaconfig->slot2=slotcard[sloton[2]->GetSelection()];
    my_lisaconfig->slot3=slotcard[sloton[3]->GetSelection()];

    my_lisaconfig->s1h=pportopts[pportboxh[1]->GetSelection()];
    my_lisaconfig->s1l=pportopts[pportboxl[1]->GetSelection()];
    my_lisaconfig->s2h=pportopts[pportboxh[2]->GetSelection()];
    my_lisaconfig->s2l=pportopts[pportboxl[2]->GetSelection()];
    my_lisaconfig->s3h=pportopts[pportboxh[3]->GetSelection()];
    my_lisaconfig->s3l=pportopts[pportboxl[3]->GetSelection()];

    my_lisaconfig->s1hp=normalize_user_path(m_text_propathh[1]->GetValue());
    my_lisaconfig->s1lp=normalize_user_path(m_text_propathl[1]->GetValue());
    my_lisaconfig->s2hp=normalize_user_path(m_text_propathh[2]->GetValue());
    my_lisaconfig->s2lp=normalize_user_path(m_text_propathl[2]->GetValue());
    my_lisaconfig->s3hp=normalize_user_path(m_text_propathh[3]->GetValue());
    my_lisaconfig->s3lp=normalize_user_path(m_text_propathl[3]->GetValue());

 // --- imagewriter settings ---------------------------------------------
   my_lisaconfig->iw_dipsw_1=(dipsw1_123->GetSelection() ) |
                             (dipsw1_4->GetSelection()<<3) |
                             (dipsw1_5->GetSelection()<<4) |
                             (dipsw1_67->GetSelection()<<5)|
                             (dipsw1_8->GetValue() ? 128:0 );
   my_lisaconfig->iw_png_on =iw_img_box->GetValue();
    my_lisaconfig->iw_png_path =normalize_user_path(iw_img_path->GetValue());


   save_configs();
   Close();
}

void LisaConfigFrame::OnRevert(wxCommandEvent& WXUNUSED(event))
{
    wxMessageDialog d(this,
                      _T("Discard unapplied preference changes and close this window?"),
                      _T("Revert changes?"),
                      wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT,
                      wxDefaultPosition);
    if (d.ShowModal()==wxID_YES) Close();
}





wxScrolledWindow *LisaConfigFrame::CreateSlotConfigPage(wxNotebook *parent, int slot)
{
    if ( slot<1 || slot>3) return NULL;

    wxScrolledWindow *panel = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxTAB_TRAVERSAL);
    panel->SetScrollRate(0, (int)wxMax(8, (int)(16 * HIDPISCALE)));
  wxBoxSizer *root = new wxBoxSizer(wxVERTICAL);
  const int border = (int)(10 * HIDPISCALE);
  const int gap = (int)(8 * HIDPISCALE);
  const int browseW = (int)(100 * HIDPISCALE);

    wxString u,l, cu, cl,s ;

    switch (slot)
     {
        case 1: s=my_lisaconfig->slot1;u=my_lisaconfig->s1hp; l=my_lisaconfig->s1lp; cu=my_lisaconfig->s1h; cl=my_lisaconfig->s1l; break;
        case 2: s=my_lisaconfig->slot2;u=my_lisaconfig->s2hp; l=my_lisaconfig->s2lp; cu=my_lisaconfig->s2h; cl=my_lisaconfig->s2l; break;
        case 3: s=my_lisaconfig->slot3;u=my_lisaconfig->s3hp; l=my_lisaconfig->s3lp; cu=my_lisaconfig->s3h; cl=my_lisaconfig->s3l; break;
     }

    sloton[slot] = new wxRadioBox(panel, wxID_ANY,
        wxT("slot:"), wxDefaultPosition,  wxDefaultSize, 2, slotcard, 1, wxRA_SPECIFY_COLS,
        wxDefaultValidator, wxT("radioBox"));
    if (s.IsSameAs(slotcard[0],false)) sloton[slot]->SetSelection(0); else sloton[slot]->SetSelection(1);
      root->Add(sloton[slot], 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, border);


    //----- upper slot -------------------------------------------------------------------------------------------------

    pportboxh[slot] = new wxRadioBox(panel, wxID_ANY,
        wxT("Upper Parallel Port: (Connector 2 in LOS)"), wxDefaultPosition, wxDefaultSize, 3, pportopts, 0, wxRA_SPECIFY_COLS,
        wxDefaultValidator, wxT("radioBox"));
      root->Add(pportboxh[slot], 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, border);

    // default to nothing for these.
    if      (cu.IsSameAs(_T("PROFILE"),false))          pportboxh[slot]->SetSelection(0);
    else if (cu.IsSameAs(_T("ADMP"),false))             pportboxh[slot]->SetSelection(1);
    else                                                pportboxh[slot]->SetSelection(2);

      wxBoxSizer *upperPath = new wxBoxSizer(wxHORIZONTAL);
      m_text_propathh[slot] = new wxTextCtrl(panel, idth[slot], u);
      upperPath->Add(m_text_propathh[slot], 1, wxRIGHT, gap);
      upperPath->Add(new wxButton(panel, idbh[slot], wxT("browse"), wxDefaultPosition, wxSize(browseW, -1)), 0);
      root->Add(upperPath, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, border);

    //----- lower slot --------------------------------------------------------------------------------------------------

    pportboxl[slot] = new wxRadioBox(panel, wxID_ANY,
        wxT("Lower Parallel Port: (Connector 1 in LOS)"), wxDefaultPosition, wxDefaultSize, 3, pportopts, 0, wxRA_SPECIFY_COLS,
        wxDefaultValidator, wxT("radioBox"));
      root->Add(pportboxl[slot], 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, border);

    // default to nothing for these.
    if      (cl.IsSameAs(_T("PROFILE"),false))          pportboxl[slot]->SetSelection(0);
    else if (cl.IsSameAs(_T("ADMP"),false))             pportboxl[slot]->SetSelection(1);
    else                                                pportboxl[slot]->SetSelection(2);

      wxBoxSizer *lowerPath = new wxBoxSizer(wxHORIZONTAL);
      m_text_propathl[slot] = new wxTextCtrl(panel, idtl[slot], l);
      lowerPath->Add(m_text_propathl[slot], 1, wxRIGHT, gap);
      lowerPath->Add(new wxButton(panel, idbl[slot], wxT("browse"), wxDefaultPosition, wxSize(browseW, -1)), 0);
      root->Add(lowerPath, 0, wxEXPAND | wxALL, border);

      panel->SetSizer(root);


    return panel;
}


wxScrolledWindow *LisaConfigFrame::CreateMainConfigPage(wxNotebook *parent)
{
    wxScrolledWindow *panel = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxTAB_TRAVERSAL);
    panel->SetScrollRate(0, (int)wxMax(8, (int)(16 * HIDPISCALE)));
  wxBoxSizer *root = new wxBoxSizer(wxVERTICAL);
  const int border = (int)(10 * HIDPISCALE);
  const int gap = (int)(8 * HIDPISCALE);
  const int browseW = (int)(100 * HIDPISCALE);

   // Tell the user what config file we're using.
   wxString t;
   t=_T("Prefs file: ") + get_config_filename();

  wxBoxSizer *prefsRow = new wxBoxSizer(wxHORIZONTAL);
  prefsRow->Add(new wxStaticText(panel, wxID_ANY, t), 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, gap);
  prefsRow->Add(new wxButton(panel, ID_PREFS_OPEN, wxT("Open...")), 0, wxRIGHT, gap);
  prefsRow->Add(new wxButton(panel, ID_PREFS_SAVEAS, wxT("Save As...")), 0);
  root->Add(prefsRow, 0, wxEXPAND | wxALL, border);

    wxString ramsize[] = { wxT("0.5 MB"), wxT("1 MB"), wxT("1.5 MB"),  wxT("2 MB*") };

  cpurambox = new wxRadioBox(panel, wxID_ANY,wxT("RAM:"), wxDefaultPosition, wxDefaultSize,
          4,
       ramsize, 0, wxRA_SPECIFY_COLS,
       wxDefaultValidator, wxT("radioBox"));
    root->Add(cpurambox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, border);


    switch(my_lisaconfig->mymaxlisaram)
    {
     case  512:  cpurambox->SetSelection(0); break;
     case 1024:  cpurambox->SetSelection(1); break;
     case 1536:  cpurambox->SetSelection(2); break;
     case 2048:  cpurambox->SetSelection(3); break;
     default:    cpurambox->SetSelection(2);
    }

   // doesn't work yet
   //macwx4mb = new wxCheckBox(panel, wxID_ANY, wxT("4MB RAM MacWorks"), wxPoint(320 * HIDPISCALE,y+(ya/4)), wxDefaultSize,wxCHK_2STATE);
   //macwx4mb->SetValue( (bool)(macworks4mb) );

    wxFlexGridSizer *fieldGrid = new wxFlexGridSizer(3, 3, gap, gap);
    fieldGrid->AddGrowableCol(1, 1);

    fieldGrid->Add(new wxStaticText(panel, wxID_ANY, _T("Lisa Serial Number:")), 0, wxALIGN_CENTER_VERTICAL);
    serialtxt = new wxTextCtrl(panel, wxID_ANY, my_lisaconfig->myserial);
    fieldGrid->Add(serialtxt, 1, wxEXPAND);
    fieldGrid->Add(new wxButton(panel, ID_SERNO_INFO, wxT("info"), wxDefaultPosition, wxSize(browseW, -1)), 0);

    fieldGrid->Add(new wxStaticText(panel, wxID_ANY, _T("Lisa ROM:")), 0, wxALIGN_CENTER_VERTICAL);
    m_rompath = new wxTextCtrl(panel, wxID_ANY, my_lisaconfig->rompath);
    fieldGrid->Add(m_rompath, 1, wxEXPAND);
    b_rompath = new wxButton(panel, ID_PICK_ROM, wxT("browse"), wxDefaultPosition, wxSize(browseW, -1));
    fieldGrid->Add(b_rompath, 0);

    fieldGrid->Add(new wxStaticText(panel, wxID_ANY, _T("Dual Parallel Card ROM:")), 0, wxALIGN_CENTER_VERTICAL);
    m_dprompath = new wxTextCtrl(panel, wxID_ANY, my_lisaconfig->dualrom);
    fieldGrid->Add(m_dprompath, 1, wxEXPAND);
    b_dprompath = new wxButton(panel, ID_PICK_DPROM, wxT("browse"), wxDefaultPosition, wxSize(browseW, -1));
    fieldGrid->Add(b_dprompath, 0);

    root->Add(fieldGrid, 0, wxEXPAND | wxALL, border);


    wxString kbid[] = { wxT("US"), wxT("UK"), wxT("FR"), wxT("DE")};
    kbbox = new wxRadioBox(panel, wxID_ANY, wxT("Keyboard:"), wxDefaultPosition, wxDefaultSize, 4, kbid, 0, wxRA_SPECIFY_COLS,
                           wxDefaultValidator, wxT("radioBox"));  // y+=ya+ya/2;
    switch(my_lisaconfig->kbid)
    {
     case 0xAD2d:   kbbox->SetSelection(2); break;
     case 0xAE2e:   kbbox->SetSelection(3); break;
     case 0xAF2f:   kbbox->SetSelection(1); break;
     case 0xBF2f:
     default:       kbbox->SetSelection(0);
    }

     wxString iorom[] = { wxT("A8"), wxT("88"), wxT("89"), wxT("A9"), wxT("40") };
     iorombox = new wxRadioBox(panel, wxID_ANY,wxT("I/O ROM:"), wxDefaultPosition, wxDefaultSize, 5, iorom, 0, wxRA_SPECIFY_COLS,
       wxDefaultValidator, wxT("radioBox"));
    switch(my_lisaconfig->iorom)
    {
     case 0x88 : iorombox->SetSelection(1); break;
     case 0x89 : iorombox->SetSelection(2); break;
     case 0xa9 : iorombox->SetSelection(3); break;
     case 0x40 : iorombox->SetSelection(4); break;
     case 0xa8 :
     default:    iorombox->SetSelection(0);
    }

    wxBoxSizer *radioRow = new wxBoxSizer(wxHORIZONTAL);
    radioRow->Add(kbbox, 1, wxRIGHT, gap);
    radioRow->Add(iorombox, 1);
    root->Add(radioRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, border);

    doublesided = new wxCheckBox(panel, wxID_ANY, wxT("SunRem 2x Sided Sony"));
    doublesided->SetValue((bool)(double_sided_floppy));
    soundeffects = new wxCheckBox(panel, wxID_ANY, wxT("Sound Effects"));
    soundeffects->SetValue((bool)(sound_effects_on));
    skinson = new wxCheckBox(panel, wxID_ANY, wxT("Lisa Skins"));
    skinson->SetValue((bool)(skins_on_next_run));
    cheats = new wxCheckBox(panel, wxID_ANY, wxT("Boot ROM speedup hacks"));
    cheats->SetValue((bool)(cheat_ram_test));
    hle_cheats  = new wxCheckBox(panel, wxID_ANY, wxT("Hard Drive Acceleration"));
    hle_cheats->SetValue((bool)(hle));
    console_term = new wxCheckBox(panel, wxID_ANY, wxT("Console Terminal"));
    console_term->SetValue((bool)consoletermwindow);

    wxBoxSizer *checks = new wxBoxSizer(wxVERTICAL);
    checks->Add(doublesided, 0, wxBOTTOM, (int)(4 * HIDPISCALE));
    checks->Add(soundeffects, 0, wxBOTTOM, (int)(4 * HIDPISCALE));
    checks->Add(skinson, 0, wxBOTTOM, (int)(4 * HIDPISCALE));
    checks->Add(cheats, 0, wxBOTTOM, (int)(4 * HIDPISCALE));
    checks->Add(hle_cheats, 0, wxBOTTOM, (int)(4 * HIDPISCALE));
    checks->Add(console_term, 0);
    root->Add(checks, 0, wxLEFT | wxRIGHT | wxTOP, border);

    wxBoxSizer *pramRow = new wxBoxSizer(wxHORIZONTAL);
    pramRow->Add(new wxStaticText(panel, wxID_ANY, _T("PRAM:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, gap);
    pramRow->Add(new wxButton(panel, ID_SAVE_PRAM, wxT("Save")), 0, wxRIGHT, gap);
    pramRow->Add(new wxButton(panel, ID_LOAD_PRAM, wxT("Load")), 0, wxRIGHT, gap);
    pramRow->Add(new wxButton(panel, ID_ZAP_PRAM, wxT("Zap")), 0);
    root->Add(pramRow, 0, wxALIGN_RIGHT | wxALL, border);

    panel->SetSizer(root);

    return panel;
}


wxScrolledWindow *LisaConfigFrame::CreatePortsConfigPage(wxNotebook *parent)
{
    wxScrolledWindow *panel = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxTAB_TRAVERSAL);
    panel->SetScrollRate(0, (int)wxMax(8, (int)(16 * HIDPISCALE)));
  wxBoxSizer *root = new wxBoxSizer(wxVERTICAL);
  const int border = (int)(10 * HIDPISCALE);
  const int gap = (int)(8 * HIDPISCALE);
  const int browseW = (int)(100 * HIDPISCALE);
    int i;

  wxBoxSizer *serialARow = new wxBoxSizer(wxHORIZONTAL);
  serialARow->Add(new wxStaticText(panel, wxID_ANY, _T("Serial A:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, gap);

#ifndef ALLOWSERIALA
  serialabox = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1 /* serialopts */, nothingonly);
    serialabox->SetSelection(0);
#else
  serialabox = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, serialopts, serportopts);
     for (i=0; i<serialopts; i++)
         if (my_lisaconfig->serial1_setting.IsSameAs(serportopts[i],false) ) serialabox->SetSelection(i);
#endif
  serialARow->Add(serialabox, 1);
  root->Add(serialARow, 0, wxEXPAND | wxALL, border);

  wxBoxSizer *serialAParamRow = new wxBoxSizer(wxHORIZONTAL);
  serialaparam = new wxTextCtrl(panel, wxID_ANY, my_lisaconfig->serial1_param);
  serialAParamRow->Add(serialaparam, 1, wxRIGHT, gap);
  serialaxon = new wxCheckBox(panel,wxID_ANY,  wxT("Xon/off"));
  serialaxon->SetValue((bool) (my_lisaconfig->serial1xon.IsSameAs(_T("1"), false)) );
  serialAParamRow->Add(serialaxon, 0, wxALIGN_CENTER_VERTICAL);
  root->Add(serialAParamRow, 0, wxEXPAND | wxLEFT | wxRIGHT, border);

  wxBoxSizer *serialBRow = new wxBoxSizer(wxHORIZONTAL);
  serialBRow->Add(new wxStaticText(panel, wxID_ANY, _T("Serial B:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, gap);
  serialbbox = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, serialopts, serportopts);
  serialBRow->Add(serialbbox, 1);
  root->Add(serialBRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, border);
//    serialbbox = new wxRadioBox(panel, wxID_ANY,
//        wxT("Serial B:"), wxPoint(10 * HIDPISCALE, y), wxSize(380 * HIDPISCALE,128 * HIDPISCALE), serialopts, serportopts, 2, wxRA_SPECIFY_COLS,
//        wxDefaultValidator, wxT("radioBox"));                                                       y+=ya+ya;

  wxBoxSizer *serialBParamRow = new wxBoxSizer(wxHORIZONTAL);
  serialbparam = new wxTextCtrl(panel, wxID_ANY, my_lisaconfig->serial2_param);
  serialBParamRow->Add(serialbparam, 1, wxRIGHT, gap);
  serialbxon = new wxCheckBox(panel,wxID_ANY, wxT("Xon/Off"));
  serialbxon->SetValue((bool) (my_lisaconfig->serial2xon.IsSameAs(_T("1"), false)));
  serialBParamRow->Add(serialbxon, 0, wxALIGN_CENTER_VERTICAL);
  root->Add(serialBParamRow, 0, wxEXPAND | wxLEFT | wxRIGHT, border);

    for (i=0; i<serialopts; i++)
        if (my_lisaconfig->serial2_setting.IsSameAs(serportopts[i],false) ) serialbbox->SetSelection(i);

  root->Add(new wxStaticText(panel, wxID_ANY,
               _T("Note: Parallel/ProFile storage changes require Lisa powered off.")),
        0, wxEXPAND | wxALL, border);

    if (floppy_iorom!=0x88)
    {
        pportbox = new wxRadioBox(panel, wxID_ANY,
      wxT("Parallel Port:"), wxDefaultPosition, wxDefaultSize, 3, pportopts, 0, wxRA_SPECIFY_COLS,
            wxDefaultValidator, wxT("radioBox"));                             
            my_lisaconfig->parallelp.Replace(_T("widget"),_T("profile"),true);
    }
    else
    {
        pportbox = new wxRadioBox(panel, wxID_ANY,
      wxT("Parallel Port:"), wxDefaultPosition, wxDefaultSize, 1, wpportopts, 0, wxRA_SPECIFY_COLS,
            wxDefaultValidator, wxT("radioBox"));                             
            my_lisaconfig->parallelp.Replace(_T("profile"),_T("widget"),true);
    }
  root->Add(pportbox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, border);

    // default to profile for builtin parallel port
    if      (my_lisaconfig->parallel.IsSameAs(_T("Nothing"),false)) pportbox->SetSelection(2);
    else if (my_lisaconfig->parallel.IsSameAs(_T("ADMP"),false))    pportbox->SetSelection(1);
    else                                                            pportbox->SetSelection(0);

  wxBoxSizer *profilePathRow = new wxBoxSizer(wxHORIZONTAL);
  m_propath = new wxTextCtrl(panel, wxID_ANY, my_lisaconfig->parallelp);
  profilePathRow->Add(m_propath, 1, wxRIGHT, gap);
  b_propath = new wxButton(panel, ID_PICK_PROFILE, wxT("browse"), wxDefaultPosition, wxSize(browseW, -1));
  profilePathRow->Add(b_propath, 0);
  root->Add(profilePathRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, border);

  panel->SetSizer(root);


    return panel;
}


wxScrolledWindow *LisaConfigFrame::CreatePrinterConfigPage(wxNotebook *parent)
{
    wxScrolledWindow *panel = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxTAB_TRAVERSAL);
    panel->SetScrollRate(0, (int)wxMax(8, (int)(16 * HIDPISCALE)));
  wxBoxSizer *root = new wxBoxSizer(wxVERTICAL);
  const int border = (int)(10 * HIDPISCALE);
  const int gap = (int)(8 * HIDPISCALE);
  const int browseW = (int)(100 * HIDPISCALE);

  root->Add(new wxStaticText(panel, wxID_ANY, _T("ImageWriter/ADMP DIP Switch 1:")), 0, wxLEFT | wxRIGHT | wxTOP, border);

    wxString fontopt[]={  wxT("000 American"),      //    ESC Z,^G,^@
                          wxT("001 German"),        //    ESC Z,^C,^@,ESC D,^D,^@
                          wxT("010 American 2"),    //    ESC Z,^E,^@,ESC D,^B,^@
                          wxT("011 French"),        //    ESC Z,^A,^@,ESC D,^F,^@
                          wxT("100 Italian"),       //    ESC Z,^F,^@,ESC D,^A,^@
                          wxT("101 Sweedish"),      //    ESC Z,^B,^@,ESC D,^E,^@
                          wxT("110 British"),       //    ESC Z,^D,^@,ESC D,^C,^@
                          wxT("111 Spanish") };     //    ESC D,^G,^@

    wxBoxSizer *pins123Row = new wxBoxSizer(wxHORIZONTAL);
    pins123Row->Add(new wxStaticText(panel, wxID_ANY, _T("pins123: Font")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, gap);
    dipsw1_123 = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 8, fontopt);
    pins123Row->Add(dipsw1_123, 1);
    dipsw1_123->SetSelection(my_lisaconfig->iw_dipsw_1 & 7);
    root->Add(pins123Row, 0, wxEXPAND | wxALL, border);


       // bit 4-    72 lines (on) 66 lines (off)
       // bit 5-    If on, AND all data with 127 (strip 8th bit)

    wxString bit4opt[]={ wxT("off - 66 lines"), wxT("on - 72 lines") };
    dipsw1_4 = new wxRadioBox(panel, wxID_ANY,
      wxT("pin4: lines"), wxDefaultPosition, wxDefaultSize, 2, bit4opt, 0, wxRA_SPECIFY_COLS,
      wxDefaultValidator, wxT("radioBox"));
    dipsw1_4->SetSelection( !!(my_lisaconfig->iw_dipsw_1 & 8) );
    root->Add(dipsw1_4, 0, wxEXPAND | wxLEFT | wxRIGHT, border);


    wxString bit5opt[]={ wxT("off - 8 bit"), wxT("on - 7 bit data") };
    dipsw1_5 = new wxRadioBox(panel, wxID_ANY,
      wxT("pin5: bits"), wxDefaultPosition, wxDefaultSize, 2, bit5opt, 0, wxRA_SPECIFY_COLS,
      wxDefaultValidator, wxT("radioBox"));
    dipsw1_5->SetSelection( !!(my_lisaconfig->iw_dipsw_1 & 16) );
    root->Add(dipsw1_5, 0, wxEXPAND | wxLEFT | wxRIGHT, border);

    wxString bit67opt[]={ wxT("00 Elite Prop."),
                          wxT("01 Elite 12cpi"),
                          wxT("10 Ultracondensed 17cpi"),
                          wxT("11 Pica 10cpi")              };

    wxBoxSizer *pins67Row = new wxBoxSizer(wxHORIZONTAL);
    pins67Row->Add(new wxStaticText(panel, wxID_ANY, _T("pins67: Pitch")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, gap);
    dipsw1_67 = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, bit67opt);
    pins67Row->Add(dipsw1_67, 1);
    dipsw1_67->SetSelection( (my_lisaconfig->iw_dipsw_1>>5) & 3 );
    root->Add(pins67Row, 0, wxEXPAND | wxALL, border);

    dipsw1_8 = new wxCheckBox(panel, wxID_ANY, wxT("pin8: Auto LF after CR"));
    dipsw1_8->SetValue((bool) !!(my_lisaconfig->iw_dipsw_1 & 128) );
    root->Add(dipsw1_8, 0, wxLEFT | wxRIGHT, border);

    iw_img_box = new wxCheckBox(panel, wxID_ANY, wxT("Print to images"));
    iw_img_box->SetValue((bool)(!!my_lisaconfig->iw_png_on) );
    root->Add(iw_img_box, 0, wxLEFT | wxRIGHT, border);

    wxBoxSizer *imgPathRow = new wxBoxSizer(wxHORIZONTAL);
    iw_img_path = new wxTextCtrl(panel, wxID_ANY, my_lisaconfig->iw_png_path);
    imgPathRow->Add(iw_img_path, 1, wxRIGHT, gap);
    iw_img_path_b = new wxButton(panel, ID_PICK_IWDIR, wxT("browse"), wxDefaultPosition, wxSize(browseW, -1));
    imgPathRow->Add(iw_img_path_b, 0);
    root->Add(imgPathRow, 0, wxEXPAND | wxALL, border);

    panel->SetSizer(root);


    return panel;
}


void LisaConfigFrame::CreateNotebook(wxNotebook *parent)
{
    wxScrolledWindow *panel1 = CreateMainConfigPage( parent );
    wxScrolledWindow *panel2 = CreatePortsConfigPage(parent );
    wxScrolledWindow *panel3 = CreateSlotConfigPage( parent,1);
    wxScrolledWindow *panel4 = CreateSlotConfigPage( parent,2);
    wxScrolledWindow *panel5 = CreateSlotConfigPage( parent,3);
    wxScrolledWindow *panel6 = CreatePrinterConfigPage(parent);

  parent->AddPage( panel1, wxT("General"),        false, -1);
  parent->AddPage( panel2, wxT("Ports & Storage"),false, -1);
  parent->AddPage( panel3, wxT("Storage Slot 1"), false, -1);
  parent->AddPage( panel4, wxT("Storage Slot 2"), false, -1);
  parent->AddPage( panel5, wxT("Storage Slot 3"), false, -1);
  parent->AddPage( panel6, wxT("Printing"),       false, -1);

  parent->SetSelection(0);
}



void LisaConfigFrame::OnPickRom(wxCommandEvent& WXUNUSED(event))
{
  wxFileDialog x(NULL, wxT("Open a Lisa Boot ROM") );
  if (x.ShowModal()==wxID_OK) m_rompath->SetValue(x.GetPath());
  //wxString  x=wxFileSelector( wxT("Open a Lisa Boot ROM") );
  //if (x.Len()>3) m_rompath->SetValue(x);
}

void LisaConfigFrame::OnPickDRom(wxCommandEvent& WXUNUSED(event))
{
  wxFileDialog x(NULL, wxT("Open a Lisa Dual Parallel Card ROM") );
  if (x.ShowModal()==wxID_OK) m_dprompath->SetValue(x.GetPath());
}


void LisaConfigFrame::OnPickProFile(wxCommandEvent& WXUNUSED(event))
{
wxString default_dir = get_default_profile_disks_dir();
wxFileDialog open(this,                         wxT("Open ProFile drive image:"),
                                                default_dir,
                                                wxEmptyString,  // No default filename when opening
                                                wxT("Disk Copy (*.dc42)|*.dc42|All (*.*)|*.*"),
                                                (long int)wxFD_OPEN|wxFD_FILE_MUST_EXIST,wxDefaultPosition);

 if (open.ShowModal()==wxID_OK)                 m_propath->SetValue(open.GetPath());
}

// slot 1

void LisaConfigFrame::OnPickProFile1H(wxCommandEvent& WXUNUSED(event))
{
 wxString default_dir = get_default_profile_disks_dir();
 wxFileDialog open(NULL, wxT("Open ProFile drive image for upper port of Slot 1:"),
                                                default_dir,
                                                wxEmptyString,  // No default filename when opening
                                                wxT("Disk Copy (*.dc42)|*.dc42|All (*.*)|*.*"),
                                                wxFD_OPEN|wxFD_FILE_MUST_EXIST);
 if (open.ShowModal()==wxID_OK)                 m_text_propathh[1]->SetValue(open.GetPath());
}

void LisaConfigFrame::OnPickProFile1L(wxCommandEvent& WXUNUSED(event))
{
 wxString default_dir = get_default_profile_disks_dir();
 wxFileDialog open(NULL,  wxT("Open ProFile drive image for lower port of Slot 1:"),
                                                default_dir,
                                                wxEmptyString,  // No default filename when opening
                                                wxT("Disk Copy (*.dc42)|*.dc42|All (*.*)|*.*"),
                                                wxFD_OPEN|wxFD_FILE_MUST_EXIST);
 if (open.ShowModal()==wxID_OK)                 m_text_propathl[1]->SetValue(open.GetPath());
}


// slot 2

void LisaConfigFrame::OnPickProFile2H(wxCommandEvent& WXUNUSED(event))
{
 wxString default_dir = get_default_profile_disks_dir();
 wxFileDialog open(NULL,  wxT("Open ProFile drive image for upper port of Slot 2:"),
                                                default_dir,
                                                wxEmptyString,  // No default filename when opening
                                                wxT("Disk Copy (*.dc42)|*.dc42|All (*.*)|*.*"),
                                                wxFD_OPEN|wxFD_FILE_MUST_EXIST);

 if (open.ShowModal()==wxID_OK)                 m_text_propathh[2]->SetValue(open.GetPath());

}

void LisaConfigFrame::OnPickProFile2L(wxCommandEvent& WXUNUSED(event))
{
 wxString default_dir = get_default_profile_disks_dir();
 wxFileDialog open(NULL,  wxT("Open ProFile drive image for lower port of Slot 2:"),
                                                default_dir,
                                                wxEmptyString,  // No default filename when opening
                                                wxT("Disk Copy (*.dc42)|*.dc42|All (*.*)|*.*"),
                                                wxFD_OPEN|wxFD_FILE_MUST_EXIST);

 if (open.ShowModal()==wxID_OK)                 m_text_propathl[2]->SetValue(open.GetPath());

}


// slot 3
void LisaConfigFrame::OnPickProFile3H(wxCommandEvent& WXUNUSED(event))
{
 wxString default_dir = get_default_profile_disks_dir();
 wxFileDialog open(NULL, wxT("Open ProFile drive image for upper port of Slot 3:"),
                                                default_dir,
                                                wxEmptyString,  // No default filename when opening
                                                wxT("Disk Copy (*.dc42)|*.dc42|All (*.*)|*.*"),
                                                wxFD_OPEN|wxFD_FILE_MUST_EXIST);

 if (open.ShowModal()==wxID_OK)                 m_text_propathh[3]->SetValue(open.GetPath());

}

void LisaConfigFrame::OnPickProFile3L(wxCommandEvent& WXUNUSED(event))
{
 wxString default_dir = get_default_profile_disks_dir();
 wxFileDialog open(NULL, wxT("Open ProFile drive image for lower port of Slot 3:"),
                                                default_dir,
                                                wxEmptyString,  // No default filename when opening
                                                wxT("Disk Copy (*.dc42)|*.dc42|All (*.*)|*.*"),
                                                wxFD_OPEN|wxFD_FILE_MUST_EXIST);

 if (open.ShowModal()==wxID_OK)                 m_text_propathl[3]->SetValue(open.GetPath());

}


void LisaConfigFrame::OnPickIWDir(wxCommandEvent& WXUNUSED(event))
{
 wxDirDialog dir(NULL, _T("Where should I save the print-out images?") );
 if (dir.ShowModal() == wxID_OK) iw_img_path->SetValue(dir.GetPath());
}


extern void invalidate_configframe(void);

LisaConfigFrame::~LisaConfigFrame()
{
  invalidate_configframe();   // prevent crash on reopening of preferences
}
