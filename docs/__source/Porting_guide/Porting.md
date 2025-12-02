![alt_text](_images/CygnusLogo_REACH-horiz-bg_light.png "image_tooltip")

# Porting Reach to a New Chip

I3 Product Design

version 3

November, 2025	Chuck Peplinski	

The original Reach version was built in the SiLabs ecosystem. It was soon after ported to the Nordic ecosystem where it is used by the Trico project. The SiLabs port is “bare metal,” in that there is no RTOS. It focuses on being as small as possible to prove its viability in small memory systems. But this is just one possible constraint. Other environments have different constraints. We have recently ported Reach to two new chips, namely the Microchip PIC32WM_BZ6204 ("BZ6") and the RealTek BW16 (AmebaD). Each of these are FreeRTOS systems.

[Here is a slide deck](../Getting Started/Cygnus_Reach_Overview.pdf) that gives some introductory background.

# Ecosystems

Let’s compare and contrast five Reach ecosystems that we are familiar with.

- SiLabs (BLE)
- Nordic (BLE)
- Microchip Harmony on PIC32WM_BZ6204 (AKA “BZ6”) (BLE)
- RealTek BW16 (AmebaD) (BLE + MQTT)
- Direct MQTT

Each of these systems has its own unique communication stack.

## SiLabs

SiLabs has no RTOS. It has limited code and data space. “User space” is split between a function called in the main loop and handlers for BLE stack events.

## Nordic/Zephyr

Nordic uses the Zephyr (RT)OS. Reach runs in a task which communicates with the BLE stack callbacks from the BLE system. User space is mostly made up of code in a separate task. The existing examples are quite closely coupled to Reach.

## Microchip Harmony

Harmony uses FreeRTOS. BLE support runs in an opaque task. Reach runs in its own thread which receives queued notifications from the BLE task. The BZ6 has plenty of code and data space. User space is in one or more other threads that have their own data structures. From the point of view of the “user space” code, Reach is being added to an existing system.

## RealTek BW16 AmebaD

The RealTek AmebaD similarly uses FreeRTOS. While “user space” code can be built onto the part (it has plenty of memory), it is treated here as a “Reach CoProcessor” (RCP). The user space host processor connects to the Reach CoProcessor by a serial port.

## Direct MQTT

MQTT is used instead of using BLE to transmit data to and from a cloud host.

# Reach Overview

Reach should be integrated into your app as three submodules.

- **Reach C Stack** — The “library”
- **Protobuf** — The “interface” shared with the Cygnus app and tools.
- **Utils** — Contains a python script to generate the Reach data structures based on a JSON format description.

Further there will be an **Integrations** directory that connects the Reach stack to the communication facilities (BLE stack, etc.) of your platform.

The code generation python script found in the utils directory is applied to a JSON format description of your project to produce a boiler plate version of the code to support your product. You must then extend this code with product specific code to do things like actually read the data that parameters use. The generated code uses comments to delineate the start and end of user code that is preserved when the code is regenerated.

# Porting Steps

The task can be broken down into a few key steps.

1. Explore the (BLE) interface
2. Advertise Reach
3. Integrate i3_log()
4. Respond to Get Device Info
5. Complete the Reach port

# 1. Explore the (BLE) Interface

Reach includes an “integrations” layer that is designed to be customized to the current platform. The **Integrations** directory typically includes a subdirectory for this particular platform. This code is designed to be reused in multiple projects using the platform. The separate subdirectories allow us to distribute a package with multiple integrations, but the integration directory is committed with the specific project. Reach was originally designed and optimized for use over BLE but it can function over other interfaces.

The code in the integrations directory connects Reach to the native features on the platform. You can refer to existing integrations and reuse as much as is convenient. Here we assume you are working on a new platform. The first step is to understand how the native features work.

## BLE

When a Reach device is a peripheral, it uses one service and one characteristic. Reach needs to be able to receive data written to a characteristic and it needs to notify the central device with changed data. Reach relies on BLE 4 generation MTU lengths of 247 bytes.

- Configure your device to advertise a service and use LightBlue or nRF Connect to receive data written by the central.
- Configure your device to enable notifications on this characteristic and then demonstrate that you can notify the central with data.
- Prove that the data packet (MTU) can be as large as 247 bytes.

## MQTT

MQTT transport of Reach has been explored but it has not yet been productized. When a device uses an MQTT interface it publishes and subscribes to one topic.

- We have not defined a standard topic name for Reach topics.
- Prove that you can publish on this topic and be notified by a subscription.
- We will want to use QoS that guarantees each message is delivered once.
- Be sure to check that the message is delivered.

# 2. Advertise Reach

## BLE

Reach requires your device to advertise a service with a specified 128 bit UUID. This is specified in the Reach Programmers Introduction on github. Reach requires this service to include one characteristic with a specified UUID. The Cygnus app will write (without confirmation) to this characteristic. The device will notify the central of data on this characteristic. The Cygnus app does not read except in the context of a notification.

The SiLabs Thunderboard advertises this raw data as captured by nRF Connect:

```text
0x02010603030018110730F940B6BF89A2A6C24EB3796992D5ED1B095265616368657220233200
```

This decodes as:

| Type          | AD Type | Value |
|---------------|---------|-------|
| Flags         | 0x01    | 0x06  |
| 16-bit UUID   | 0x03    | 0x1800 (Generic Access) |
| 128-bit UUID  | 0x07    | ED D5 92 69 79 B3 4E C2 A6 A2 89 BF B6 40 F9 30 |
| Local Name    | 0x09    | "Reacher #2" |

Formatted in C:

```c
static const uint8_t advPayload[] = {
    2,  0x01, 0x06,                          // Flags
    3,  0x03, 0x00, 0x18,                    // 16-bit UUID: 0x1800
    17, 0x07,                                // 128-bit UUID
        0x30, 0xF9, 0x40, 0xB6, 0xBF, 0x89, 0xA2, 0xA6,
        0xC2, 0x4E, 0xB3, 0x79, 0x69, 0x92, 0xD5, 0xED,
    11, 0x09,                                // Complete Local Name
        'R','e','a','c','h','e','r',' ','#','2'
};
```

The SiLabs version manages to display these 10 characters for the name. The BZ6 version uses the scan response to advertise a long name.

When your advertisement satisfies the Cygnus app your device will be listed with the Cygnus icon.

## MQTT

I don’t think this is necessary if the device is publishing and subscribing to the Reach topic.

# 3. Integrate i3_log()

The Reach stack is configured to supply a lot of diagnostic information via the printf-like i3_log() function. Any port should first support this to enable further debugging during bringup. The Reach C stack includes a weak implementation of the i3_log() functions that simply calls printf. If this works you can simply use it. In a multithreaded environment it makes sense to consistently protect i3_log() with a mutex so that it can be used reliably throughout the system. The BZ6 implementation satisfies this requirement.

I3_assert() should also be tested to prove that it stops and indicates its presence reliably both with the debugger (where it should stop and show the call stack) and without the debugger (where it should print an error message before halting).

These features are required to support the debugging features that Reach relies on. These debugging features can well be used by the user space code.

# 4. Respond to Get Device Info

With a proper advertisement and i3_log() ported you can bring in the Reach C stack and connect it to the BLE stack so that the Cygnus app (or web page) can discover the device. The host will issue a device information request. To respond to the request for device information you will need to:

- Define your device’s response using the code generation script in Utils to parse an appropriate JSON description.
  - You can copy the JSON from a reference port and change something like the device name to see your handiwork.

- Receive the 19 byte request as a BLE write / MQTT subscription.
- Feed the received data to Reach by calling cr_store_coded_prompt().
- Run the cr_process() function in the Reach task to compose a response.
- Call your implementation of crcb_send_coded_response() to notify the central of the result.

The response to get device info (GDI) is on the order of 100 bytes, so it will fail if 247 byte MTU’s are not supported.

When this basic exchange is operating, what remains is to fill in things that are specific to your device.

# 5. Complete the Reach Port

The generated Reach code has places to add your own code. In a new project this user code will be empty. You can copy enough things to test from the reference code. For instance, the “commands” will all return “20” (not handled) until you put in handlers. You can copy these from the Thunderboard example. Eventually you will want to create reasonable handlers for each service and verify that they work correctly.

Note that there are init functions generated for each service. These need to be called before the service is accessed. When the parameter repo contains the name of the device, the parameter repo needs to be initialized before the communication stack.

## Commands

- You should see the names of the commands and pressing the buttons in the host interface you should see the action.

## Parameters

- You should see the list of parameters.
- You should verify that you can read values and write values.

## Time

- You should verify that you can set the local time and read it back.

## Files

- You should be able to upload something like a 50k file from the host and store it at /dev/null.
- The debug mode of the Cygnus app measures the transfer rate which should ideally be north of 100k.

## (Remote) CLI

The CLI service includes a default command handler in cli.c. There are a couple of standard commands such as “help” and “ver” and “/”. You should connect this to the CLI of the device. Then when you enable the remote CLI (using a command) you should see the output of i3_log() at the host and commands from the host should flow to the device.
