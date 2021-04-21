## CAN Library for Teensy 3.1 / 3.2, 3.5, 3.6

### Compatibility with the ACANxxxx libraries

This library is fully compatible with the MCP2515 CAN Controller ACAN2515, ACAN2515Tiny, ACAN2517 and ACAN2517FD libraries, it uses a very similar API and the same `CANMessage` class for handling messages.

### Demo sketchs

Two sketches are provided for demonstrating remote frame sending and receiving capabilities:

* **SendReceiveRemoteFramesWithFlexCan** sketch (in `flexcan-driver-example`) which uses the ACAN library, remote frames are sent and received;
* **SendReceiveRemoteFrames** sketch (in `teensyduino-library/ACAN/examples`) which uses the FlexCan library, the first remote frame is sent, no remote frame is received.

Theses two sketches need to establish a CAN network that connects CAN0 and CAN1. You can use a single AND gate, as 74HC08, powered on 3.3V:

* AND inputs are CANT0X and CAN1TX;
* AND outputs are CAN0RX and CAN1RX.

### Memory Footprint

Compiled with a CPU Speed of 180 MHz and Optimization *Smallest Code with LTO*.
<table>
    <tr>
        <td>Sketch</td><td>Code size</td><td>Ram size</td><td>Dynamic Ram size</td>
    </tr>
    <tr>
        <td>SendReceiveRemoteFramesWithFlexCan</td><td>9000 bytes</td><td>5032 bytes</td><td>0</td>
    </tr>
    <tr>
        <td> SendReceiveRemoteFrames </td><td>9268 bytes</td><td>2940 bytes</td><td>1536 bytes</td>
    </tr>
</table>

### ACAN library description
ACAN is a driver for the FlexCAN module built into the Teensy 3.1 / 3.2, 3.5, 3.6 microcontroller. It supports alternates pins. The two FlexCAN modules are supported on the Teensy 3.6.

The driver supports many bit rates, as standard 62.5 kbit/s, 125 kbit/s, 250 kbit/s, 500 kbit/s, and 1 Mbit/s. An efficient CAN bit timing calculator finds settings for them, but also for exotic bit rates as 842 kbit/s. If the wished bit rate cannot be achieved, the `begin` method does not configure the hardware and returns an error code.

> The Teensyduino library is the `teensyduino-library/ACAN` directory. Driver API is fully described by the PDF file in the `teensyduino-library/ACAN/extras` directory.

### Demo Sketch

> The demo sketch is in the `teensyduino-library/ACAN/examples/LoopBackDemo` directory.

Configuration is a four-step operation.

1. Instanciation of the `settings` object : the constructor has one parameter: the wished CAN bit rate. The `settings` is fully initialized.
2. You can override default settings. Here, we set the `mLoopBackMode` and `mSelfReceptionMode` properties to true, enabling to run demo code without any additional hardware (no CAN transceiver needed). We can also for example change the receive buffer size by setting the `mReceiveBufferSize` property.
3. Calling the `begin` method configures the driver and starts CAN bus participation. Any message can be sent, any frame on the bus is received. No default filter to provide.
4. You check the `errorCode` value to detect configuration error(s).

```cpp
void setup () {
  Serial.begin (9600) ;
  Serial.println ("Hello") ;
  ACANSettings settings (125 * 1000) ; // 125 kbit/s
  settings.mLoopBackMode = true ;
  settings.mSelfReceptionMode = true ;
  const uint32_t errorCode = ACAN::can0.begin (settings) ;
  if (0 == errorCode) {
    Serial.println ("Can0 ok") ;
  }else{
    Serial.print ("Error Can0: 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}
```

Now, an example of the `loop` function. As we have selected loop back mode, every sent frame is received.

```cpp
static unsigned gSendDate = 0 ;
static unsigned gSentCount = 0 ;
static unsigned gReceivedCount = 0 ;

void loop () {
  CANMessage message ;
  if (gSendDate < millis ()) {
    message.id = 0x542 ;
    const bool ok = ACAN::can0.tryToSend (message) ;
    if (ok) {
      gSendDate += 2000 ;
      gSentCount += 1 ;
      Serial.print ("Sent: ") ;
      Serial.println (gSentCount) ;
    }
  }
  if (ACAN::can0.receive (message)) {
    gReceivedCount += 1 ;
    Serial.print ("Received: ") ;
    Serial.println (gReceivedCount) ;
  }
}
```
`CANMessage` is the class that defines a CAN message. The `message` object is fully initialized by the default constructor. Here, we set the `id` to `0x542` for sending a standard data frame, without data, with this identifier.

The `ACAN::can0.tryToSend` tries to send the message. It returns `true` if the message has been sucessfully added to the driver transmit buffer.

The `gSendDate` variable handles sending a CAN message every 2000 ms.

`ACAN::can0.receive` returns `true` if a message has been received, and assigned to the `message`argument.

### Use of Optional Reception Filtering

The hardware defines two kinds of filters: *primary* and *secondary* filters. Depending the driver configuration, you can have up to 14 *primary* filters and 18 *secondary* filters.

This an setup example:

```cpp
  ACANSettings settings (125 * 1000) ;
  ...
   const ACANPrimaryFilter primaryFilters [] = {
    ACANPrimaryFilter (kData, kExtended, 0x123456, handle_myMessage_0)
  } ;
  const ACANSecondaryFilter secondaryFilters [] = {
    ACANSecondaryFilter (kData, kStandard, 0x234, handle_myMessage_1),
    ACANSecondaryFilter (kRemote, kStandard, 0x542, handle_myMessage_2)
  } ;
  const uint32_t errorCode = ACAN::can0.begin (settings,
                                               primaryFilters, 
                                               1, // Primary filter array size
                                               secondaryFilters,
                                               2) ; // Secondary filter array size
```
For example, the first filter catches extended data frames, with an identifier equal to `0x123456`. When a such frame is received, the `handle_myMessage_0` function is called. In order to achieve this by-filter dispatching, you should call `ACAN::can0.dispatchReceivedMessage` instead of `ACAN::can0.receive` in the `loop`function:


```cpp
void loop () {
  ACAN::can0.dispatchReceivedMessage () ; // Do not use ACAN::can0.receive any more
  ...
}
```
