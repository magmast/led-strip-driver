# Matrix Driver Bluetooth Protocol Specification

## Overview

This document specifies the Bluetooth protocol for controlling an LED matrix. It defines the structure and behavior of the LED Matrix Service, including characteristics for managing the LED chain length, selecting individual or groups of LEDs, and setting their colors.

**All numerical data communicated within this protocol, including values read from and written to characteristics, adhere to the little-endian byte order.** This convention applies to multi-byte numbers, ensuring consistent data interpretation across different client and server implementations.

## LED Matrix Service

- **UUID:** 4fd3af2a-10e8-474f-84d7-722bcfd3efc3

This service provides control over an LED matrix via Bluetooth, allowing for manipulation of individual or all LEDs in a matrix by setting their colors. It comprises the following characteristics:

### Chain Length Characteristic

- **UUID:** 410f7f12-e051-4b5d-a8ed-7d5619727b34
- **Data type:** `uint16`
- **Access:** Read only
- **Description:** Represents the total number of LEDs in the chain, providing the maximum exclusive index of LEDs in the matrix. This characteristic informs the client about the size of the LED matrix, enabling appropriate indexing.

### Index Characteristic

- **UUID:** 4fd3af2a-10e8-474f-84d7-722bcfd3efc3
- **Data type:** `int16`
- **Access:** Read, Write
- **Description:** Specifies the target index for color operations performed via the Color Characteristic. The microcontroller initializes this characteristic to 0 at startup, serving as the default target index. A special value of -1 indicates that any color operation applies to all LEDs simultaneously. The value of this characteristic may be left as set by a previous client or changed as needed. Clients are encouraged to explicitly set this characteristic when targeting a specific LED or range of LEDs to ensure the intended behavior.

### Color Characteristic

- **UUID:** 0c903aa6-de65-44c4-9cde-8873267e16c0
- **Data type:** `byte array`
- **Access:** Read, Write
- **Description:** Determines the color of the LED at the index specified by the Index Characteristic. The color data is represented as a 3-byte array in the RGB format, where the array is structured as [Red, Green, Blue]. Each byte corresponds to the intensity of the respective color component. Writing to this characteristic changes the color of the targeted LED(s). The behavior of reading this characteristic when the Index is set to -1 is undefined and should not be relied upon. Future protocol revisions may alter this behavior.

## Notes on Behavior and Conventions

- **Undefined Behavior (UB):** The protocol currently does not define the behavior of reading the Color Characteristic when the Index Characteristic is set to `-1`. Clients should avoid relying on the current implementation as it may change in future updates to ensure a more predictable and stable interface.

- **Error Handling:** Clients should implement error checking when writing to characteristics, especially to handle cases where the index might be out of bounds or the data format for the Color Characteristic does not match expectations.

## Security Considerations

As of the current version of this protocol, security measures such as encryption and authentication have not been implemented or considered in detail. This means that the communication between clients and the LED Matrix Service is unsecured. Consequently, the protocol is vulnerable to eavesdropping and unauthorized access, potentially allowing malicious parties to intercept communications or control the LED matrix without permission.

Users and implementers should be aware of these security limitations. Relying on this protocol for applications requiring confidentiality, integrity, or authentication is not recommended in its current state. Future revisions of the protocol will aim to address these security concerns.
