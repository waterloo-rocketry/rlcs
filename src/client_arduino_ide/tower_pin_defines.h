#ifndef TOWER_PIN_DEFINES_H
#define TOWER_PIN_DEFINES_H

#define PIN_REMOTEFILL_POWER 26
#define PIN_REMOTEFILL_SELECT 27

#define PIN_REMOTEVENT_POWER 25
#define PIN_REMOTEVENT_SELECT 24

#define PIN_LINACTUATOR_POWER 22
#define PIN_LINACTUATOR_SELECT 23

#define PIN_IGNITION_PRIMARY_POWER 28
#define PIN_IGNITION_PRIMARY_SELECT 29
#define PIN_IGNITION_SECONDARY_POWER 37
#define PIN_IGNITION_SECONDARY_SELECT 36

//daq pins
#define PIN_DAQ_MASS 5
#define PIN_DAQ_PRESSURE1 6
#define PIN_DAQ_PRESSURE2 7
//current sensor pins
#define PIN_CURRENT_IGNITION_PRI 9
#define PIN_CURRENT_IGNITION_SEC 15
//limit switch pins
#define PIN_LIMITSW_REMOTEFILL_OPN 34
#define PIN_LIMITSW_REMOTEFILL_CLS 35
#define PIN_LIMITSW_REMOTEVENT_OPN 33
#define PIN_LIMITSW_REMOTEVENT_CLS 32
#define PIN_LIMITSW_LINAC_EXT 31
#define PIN_LIMITSW_LINAC_RET 30

//pins for the seven segment display
#define PIN_SEVENSEG_D1 47
#define PIN_SEVENSEG_D2 46
#define PIN_SEVENSEG_A  39
#define PIN_SEVENSEG_B  40
#define PIN_SEVENSEG_C  43
#define PIN_SEVENSEG_D  45
#define PIN_SEVENSEG_E  42
#define PIN_SEVENSEG_F  41
#define PIN_SEVENSEG_G  38
#define PIN_SEVENSEG_DP 44

//pins for the SD card
#define PIN_SD_SS 53

#endif //ifndef TOWER_PIN_DEFINES_H
