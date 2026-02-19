/**************************************************************************************\
*                                                                                      *
*              The Lisa Emulator Project  V1.2.9      DEV 2019.09.09                   *
*                             http://lisaem.sunder.net                                 *
*                                                                                      *
*                  Copyright (C) 1998, 2019 Ray A. Arachelian                          *
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
*                                                                                      *
\**************************************************************************************/

#include <wx/defs.h>
#include <wx/string.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>

#include <machine.h>
#include <LisaSkin.h>


void LisaSkin::Load(wxFileConfig* config)
{
      screen_origin_x         = 0;
      screen_origin_y         = 0;

      floppy_eject            = config->Read(_T("/sounds/floppy_eject"),         _T("floppy_eject.wav") );
      floppy_insert           = config->Read(_T("/sounds/floppy_insert"),        _T("floppy_insert_sound.wav") );
      floppy_insert_nopower   = config->Read(_T("/sounds/floppy_insert_nopower"),_T("floppy_insert_sound_nopower.wav") );
      floppy_motor1           = config->Read(_T("/sounds/floppy_motor1"),        _T("floppy_motor1.wav") );
      floppy_motor2           = config->Read(_T("/sounds/floppy_motor2"),        _T("floppy_motor2.wav") );
      lisa_power_switch01     = config->Read(_T("/sounds/powerswitch_push"),     _T("lisa_power_switch01.wav") );
      lisa_power_switch02     = config->Read(_T("/sounds/powerswitch_release"),  _T("lisa_power_switch02.wav") );
      poweroffclk             = config->Read(_T("/sounds/poweroff"),             _T("poweroffclk.wav") );
}


void LisaSkin::Save(wxFileConfig* config)
{
      config->Write(_T("/sounds/floppy_eject"),          floppy_eject );
      config->Write(_T("/sounds/floppy_insert"),         floppy_insert );
      config->Write(_T("/sounds/floppy_insert_nopower"), floppy_insert_nopower );

      config->Write(_T("/sounds/floppy_motor1"),         floppy_motor1 );
      config->Write(_T("/sounds/floppy_motor2"),         floppy_motor2 );
      config->Write(_T("/sounds/powerswitch_push"),      lisa_power_switch01 );
      config->Write(_T("/sounds/powerswitch_release"),   lisa_power_switch02 );
      config->Write(_T("/sounds/poweroff"),              poweroffclk );

      config->Flush();
}
