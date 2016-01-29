# RESOL Arduino Pt1000 Shield Values

## VBus address:
* 0x7770 (Broadcast) + DIP-Switch value

## Input:

### Sensor resistor:

#### Format:

I32 Resisitor in Ohm with 3 digits

#### ValueIDs:

* Sensor1: 0x1001
* Sensor2: 0x1002
* …….
* Sensor7: 0x1007
* Sensor8: 0x1008


### DIP-Switch:

#### Format:

U8 DIP-Switch bit mask

#### ValueID:

* Sensor1: 0x1009


## Config:

### UART Baudrate:

#### Format:

U32 Baudrate (Range: 2400 - 115200, default 9600)

#### ValueID:

* Uart baudrate: 0x0001



### ADC priority channel:

#### Format:

I32 Channel (Range: -1..7, default -1)
Channel < 0 disables the priority feature.

#### ValueID:

* ADC priority channel: 0x100A
