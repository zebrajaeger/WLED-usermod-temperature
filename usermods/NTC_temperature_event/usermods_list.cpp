#include "wled.h"
/*
 * Register your v2 usermods here!
 */
#ifdef USERMOD_NTC_TEMPARATURE_EVENT
#include "../usermods/NTC_temperature_event/NTC_temperature_event.h"
#endif

void registerUsermods()
{
#ifdef USERMOD_NTC_TEMPARATURE_EVENT
  usermods.add(new UsermodNtcTemperatureEvent());
#endif
}