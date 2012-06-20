/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Device/Driver/SilentWings.hpp"
#include "Device/Driver.hpp"
#include "Units/System.hpp"
#include "Device/Parser.hpp"
#include "NMEA/Info.hpp"
#include "Compiler.h"
#include "Math/Angle.hpp"

#include <stdlib.h>
#include <math.h>
//#include <math.h>

class SilentWingsDevice : public AbstractDevice {
public:
  virtual bool DataReceived(const void *data, size_t length,
                            struct NMEAInfo &info);
};

/**
 * Binary data coming from SW simulator
 * (http://wiki.silentwings.no/index.php?title=UDP_Output).
 *
 * TODO Still waiting an aswer from Silent Wings AS about the reliability of
 * this structure across the platforms/versions
 * 
 */

struct SilentWingsBinary {
    /** Millisec  Timestamp */
   unsigned int timestamp;
   /** Degrees   Position latitude */
   double position_latitude;        
   /** Degrees            longitude */
   double position_longitude;
   /** m         Altitude - relative to Sea-level */
   float  altitude_msl;
   /** m         Altitude above gnd */
   float  altitude_ground;
   /** m         gnd 45 degrees ahead (NOT IMPLEMENTED YET) */
   float  altitude_ground_45;       
   /** m         gnd straight ahead (NOT IMPLEMENTED YET) */
   float  altitude_ground_forward;
   /** Degrees */
   float  roll;
   /** Degrees */
   float  pitch;
   /** Degrees */
   float  yaw;
   /** Deg/sec   Roll speed. */
   float  d_roll;
   /** Deg/sec   Pitch speed. */
   float  d_pitch;                 
   /** Deg/sec   Yaw speed. */
   float  d_yaw;                     
   /** m/sec     Speed vector in body-axis */
   float  vx;                       
   float  vy;
   float  vz;
   /** m/sec     Speed vector in body-axis, relative to wind */
   float  vx_wind;
   float  vy_wind;
   float  vz_wind;
   /** m/sec     Equivalent (indicated) air speed. */
   float  v_eas;
   /** m/sec2    Acceleration vector in body axis */
   float  ax;
   float  ay;
   float  az;
   /** Degrees   Angle of attack */
   float  angle_of_attack;
   /** Degrees   Sideslip angle */
   float  angle_sideslip;           
   /** m/sec     TE-compensated variometer. */
   float  vario;
   /** Degrees   Compass heading. */
   float  heading;
   /** Deg/sec   Rate of turn. */
   float  rate_of_turn;
   /** pascal    Local air pressure (at aircraft altitude). */
   float  airpressure;
   /** Air density at aircraft altitude. */
   float  density;
   /** Celcius   Air temperature at aircraft altitude. */
   float  temperature;
} __attribute__ ((__packed__));


bool
SilentWingsDevice::DataReceived(const void *data, size_t length,
                            struct NMEAInfo &info)
{
  if(length % sizeof(SilentWingsBinary)){
    return false; // malformed data?
  }
  const SilentWingsBinary &binary = *(SilentWingsBinary *)data;
  info.location.latitude = Angle::Degrees(fixed(binary.position_latitude));
  info.location.longitude = Angle::Degrees(fixed(binary.position_longitude));
  info.location_available.Update(info.clock);
  info.gps_altitude = fixed(binary.altitude_msl);
  info.gps_altitude_available.Update(info.clock);
  info.ground_speed = fixed(hypot((double)binary.vx, (double)binary.vy));
  info.ground_speed_available.Update(info.clock);
  info.gps.fix_quality = FixQuality::SIMULATION;
  info.gps.real = false;
  info.gps.simulator = true;
  info.gps.fix_quality_available.Update(info.clock);
  info.time = fixed(binary.timestamp)/1000;
  info.time_available.Update(info.clock);
  
  info.alive.Update(info.clock);
 
  return true;
}

static Device *
SilentWingsCreateOnPort(const DeviceConfig &config, gcc_unused Port &com_port)
{
  return new SilentWingsDevice();
}

const struct DeviceRegister silentWingsDevice = {
  _T("Silent Wings"),
  _T("Silent Wings Sim. (binary)"),
  DeviceRegister::RAW_GPS_DATA,
  SilentWingsCreateOnPort,
};
