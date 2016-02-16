/*
 *      Copyright (C) 2014-2015 Garrett Brown
 *      Copyright (C) 2014-2015 Team XBMC
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "JoystickInterfaceXInput.h"
#include "JoystickXInput.h"
#include "XInputDLL.h"
#include "api/JoystickTypes.h"
#include "log/Log.h"

#include <Xinput.h>

using namespace JOYSTICK;

#define MAX_JOYSTICKS 4

static JOYSTICK_BATTERY_LEVEL ExtractBatteryLevel(const XINPUT_BATTERY_INFORMATION& batteryInfo)
{
  if (batteryInfo.BatteryType != BATTERY_TYPE_ALKALINE && batteryInfo.BatteryType != BATTERY_TYPE_NIMH)
    return JOYSTICK_BATTERY_LEVEL_NOTSUPPORTED;

  switch (batteryInfo.BatteryLevel)
  {
    case BATTERY_LEVEL_EMPTY:
      return JOYSTICK_BATTERY_LEVEL_EMPTY;

    case BATTERY_LEVEL_LOW:
      return JOYSTICK_BATTERY_LEVEL_LOW;

    case BATTERY_LEVEL_MEDIUM:
      return JOYSTICK_BATTERY_LEVEL_MEDIUM;

    case BATTERY_LEVEL_FULL:
      return JOYSTICK_BATTERY_LEVEL_FULL;

    default:
      break;
  }

  return JOYSTICK_BATTERY_LEVEL_NOTSUPPORTED;
}

const char* CJoystickInterfaceXInput::Name(void) const
{
  return INTERFACE_XINPUT;
}

bool CJoystickInterfaceXInput::Initialize(void)
{
  return CXInputDLL::Get().Load();
}

void CJoystickInterfaceXInput::Deinitialize(void)
{
  CXInputDLL::Get().Unload();
}

bool CJoystickInterfaceXInput::ScanForJoysticks(JoystickVector& joysticks)
{
  XINPUT_STATE_EX controllerState; // No need to memset, only checking for controller existence

  for (unsigned int i = 0; i < MAX_JOYSTICKS; i++)
  {
    if (!CXInputDLL::Get().GetState(i, controllerState))
      continue;

    isyslog("Found a XInput controller on port %u", i);
    // create the XInput joystick
    JoystickPtr joystick = JoystickPtr(new CJoystickXInput(i));

    // get the battery information for the XInput joystick
    XINPUT_BATTERY_INFORMATION batteryInfo;
    if (CXInputDLL::Get().GetBatteryInformation(i, CXInputDLL::BatteryDeviceType::Controller, batteryInfo))
      joystick->SetBatteryLevel(ExtractBatteryLevel(batteryInfo));

    joysticks.push_back(joystick);
  }

  return true;
}
