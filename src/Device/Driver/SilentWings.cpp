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

struct SilentWingsBinary {
   unsigned int timestamp;          // Millisec  Timestamp
   double position_latitude;        // Degrees   Position latitude,
   double position_longitude;       // Degrees            longitude,
   float  altitude_msl;             // m         Altitude - relative to Sea-level
   float  altitude_ground;          // m         Altitude above gnd
   float  altitude_ground_45;       // m         gnd 45 degrees ahead (NOT IMPLEMENTED YET),
   float  altitude_ground_forward;  // m         gnd straight ahead (NOT IMPLEMENTED YET).
   float  roll;                     // Degrees
   float  pitch;                    // Degrees
   float  yaw;                      // Degrees
   float  d_roll;                   // Deg/sec   Roll speed.
   float  d_pitch;                  // Deg/sec   Pitch speed.
   float  d_yaw;                    // Deg/sec   Yaw speed.
   float  vx;                       // m/sec     Speed vector in body-axis
   float  vy;
   float  vz;
   float  vx_wind;                  // m/sec     Speed vector in body-axis, relative to wind
   float  vy_wind;
   float  vz_wind;
   float  v_eas;                    // m/sec     Equivalent (indicated) air speed.
   float  ax;                       // m/sec2    Acceleration vector in body axis
   float  ay;
   float  az;
   float  angle_of_attack;          // Degrees   Angle of attack
   float  angle_sideslip;           // Degrees   Sideslip angle
   float  vario;                    // m/sec     TE-compensated variometer.
   float  heading;                  // Degrees   Compass heading.
   float  rate_of_turn;             // Deg/sec   Rate of turn.
   float  airpressure;              // pascal    Local air pressure (at aircraft altitude).
   float  density;                  // Air density at aircraft altitude.
   float  temperature;              // Celcius   Air temperature at aircraft altitude.
} __attribute__ ((__packed__));


bool
SilentWingsDevice::DataReceived(const void *data, size_t length,
                            struct NMEAInfo &info)
{
  if(length % sizeof(SilentWingsBinary)){
    return false; // malformed data?
  }
  const SilentWingsBinary &binary = *(SilentWingsBinary *)data;
  info.location.latitude = Angle::Degrees((fixed)binary.position_latitude);
  info.location.longitude = Angle::Degrees((fixed)binary.position_longitude);
  info.location_available.Update(info.clock);

  info.gps_altitude = (fixed)binary.altitude_msl;
  info.gps_altitude_available.Update(info.clock);

  info.ground_speed = (fixed)hypot((double)binary.vx, (double)binary.vy);
  info.ground_speed_available.Update(info.clock);

//  I've seen trackis not necessary, it will be automatically
//  computed (form the fixes?)
//  info.track = Angle::Degrees((fixed)binary.heading);
//  info.track_available.Update(info.clock);

  info.gps.fix_quality = FixQuality::SIMULATION;
  info.gps.real = false;
  info.gps.simulator = true;
  info.gps.fix_quality_available.Update(info.clock);

  info.time = (fixed)binary.timestamp;
  info.time_available.Update(info.clock);
  
  info.alive.Update(info.clock);
  
  // debug stuff
  int lad,lam,las,lod,lom,los;
  bool lap,lop;
  info.location.latitude.ToDMS(lad,lam,las,lap);
  info.location.longitude.ToDMS(lod,lom,los,lop);

  printf("%.2f   %02i°%02i'%02i\"%s %02i°%02i'%02i\"%s (%lf, %lf)\n",
    info.clock, lad,lam,las,lap?"N":"S", lod,lom,los,lop?"E":"W",
    binary.position_latitude, binary.position_longitude);
  return true;
}

static Device *
SilentWingsCreateOnPort(const DeviceConfig &config, gcc_unused Port &com_port)
{
  return new SilentWingsDevice();
}

const struct DeviceRegister silentWingsDevice = {
  _T("Silent Wings"),
  _T("SW Soaring Simulator"),
  DeviceRegister::RAW_GPS_DATA,
  SilentWingsCreateOnPort,
};
