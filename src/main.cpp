//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
//#define USE_OTA_BOOTLOADER

#define USE_WOR
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Switch.h>
#include <ResetOnBoot.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED_PIN 4
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8

#define RELAY1_PIN 3


#define GARDENA_ON 5
#define GARDENA_PWM 6
#define GARDENA_SLEEP 7

#define vor true
#define back false

#include "drv8838.h"
drv8838 gardena = drv8838(GARDENA_ON, GARDENA_PWM, GARDENA_SLEEP);
//#include <DRV8835MotorShield.h>
//DRV8835MotorShield motors;

// number of available peers per channel
#define PEERS_PER_CHANNEL 8

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
    {0x42, 0xA2, 0xB7},     // Device ID
    "papa42a2b7",           // Device Serial
    {0x00, 0x6c},           // Device Model
    0x12,                   // Firmware Version
    as::DeviceType::Switch, // Device Type
    {0x01, 0x00}            // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10, 11, 12, 13> RadioSPI;
typedef AskSin<StatusLed<LED_PIN>, BattSensor<AsyncMeter<InternalVCC>>, Radio<RadioSPI, 2>> Hal;

DEFREGISTER(Reg0, DREG_INTKEY, DREG_LEDMODE, MASTERID_REGS, DREG_LOWBATLIMIT)
class SwList0 : public RegList0<Reg0>
{
public:
  SwList0(uint16_t addr) : RegList0<Reg0>(addr) {}
  void defaults()
  {
    clear();
    lowBatLimit(22);
  }
};

// setup the device with channel type and number of channels
class SwitchType : public MultiChannelDevice<Hal, SwitchChannel<Hal, PEERS_PER_CHANNEL, SwList0>, 1, SwList0>
{
public:
  typedef MultiChannelDevice<Hal, SwitchChannel<Hal, PEERS_PER_CHANNEL, SwList0>, 1, SwList0> DevType;
  SwitchType(const DeviceInfo &i, uint16_t addr) : DevType(i, addr) {}
  virtual ~SwitchType() {}

  virtual void configChanged()
  {
    DevType::configChanged();
    uint8_t lowbat = getList0().lowBatLimit() / 3;
    DDECLN(lowbat);
    if (lowbat > 0)
    {
      battery().low(lowbat);
    }
  }
};

Hal hal;
SwitchType sdev(devinfo, 0x20);
ResetOnBoot<SwitchType> resetOnBoot(sdev);
ConfigToggleButton<SwitchType> cfgBtn(sdev);
#ifndef USE_WOR
BurstDetector<Hal> bd(hal);
#endif

void initPeerings(bool first)
{
  // create internal peerings - CCU2 needs this
  if (first == true)
  {
    HMID devid;
    sdev.getDeviceID(devid);
    for (uint8_t i = 1; i <= sdev.channels(); ++i)
    {
      Peer ipeer(devid, i);
      sdev.channel(i).peer(ipeer);
    }
  }
}
void changeGardenaState()
{
  boolean state = digitalRead(RELAY1_PIN);
  Serial.print("State: ");
  Serial.println(state);

  if (state)
  {
    gardena.sleep(state);
    gardena.setDirection(state);
    gardena.setSpeed(255);
    Serial.println("Gardena ein ");

    _delay_ms(250);
    gardena.setDirection(!state);
    gardena.setSpeed(0);
    gardena.sleep(!state);
  }
  else
  {
    gardena.sleep(!state);
    gardena.setDirection(!state);
    gardena.setSpeed(100);
    Serial.println("Gardena aus ");

    _delay_ms(150);
    gardena.setDirection(state);
    gardena.setSpeed(0);
    gardena.sleep(state);
  }
 
}


void setup()
{

  DINIT(57600, ASKSIN_PLUS_PLUS_IDENTIFIER);
  bool first = sdev.init(hal);
  sdev.channel(1).init(RELAY1_PIN);
  buttonISR(cfgBtn, CONFIG_BUTTON_PIN);
  initPeerings(first);
#ifndef USE_WOR
  // start burst detection
  bd.enable(sysclock);
#endif
  // stay on for 15 seconds after start
  hal.activity.stayAwake(seconds2ticks(15));
  // measure battery every hour
  hal.battery.init(seconds2ticks(60UL * 60), sysclock);
  resetOnBoot.init();
  sdev.initDone();
  attachInterrupt(digitalPinToInterrupt(RELAY1_PIN), changeGardenaState, CHANGE);
  changeGardenaState();
 

  //if (sdev.getMasterID() == HMID::broadcast) { DPRINTLN(F("START PAIRING")); sdev.startPairing(); } // start pairing of no master id is present
}

void loop()
{
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if (worked == false && poll == false)
  {
    hal.activity.savePower<Sleep<>>(hal);
  }
}