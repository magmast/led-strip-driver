# LED Strip Bluetooth Protocol Specification

## Overview

This document outlines the Bluetooth protocol for managing an LED strip, specifying the structure and functionality of the LED Strip Service. It includes characteristics for determining the LED strip length, selecting starting points for LED operations, and configuring their colors. This protocol ensures a unified approach to controlling LED strips across various client and server implementations by adhering to consistent data interpretation standards.

**All numerical data within this protocol, including values exchanged through characteristics, follows the little-endian byte order.** This standardization applies to multi-byte numerical values, facilitating compatibility across different platforms.

## LED Strip Service

- **UUID:** 4fd3af2a-10e8-474f-84d7-722bcfd3efc3

The service enables Bluetooth-based manipulation of an LED strip, allowing colors of individual LEDs or groups of LEDs to be set. The service is structured with the following characteristics:

### Chain Length Characteristic

- **UUID:** 410f7f12-e051-4b5d-a8ed-7d5619727b34
- **Data type:** `uint16`
- **Access:** Read only
- **Description:** Indicates the total count of LEDs in the strip, which also represents the highest valid index for LED operations. This characteristic informs clients about the strip's size, enabling accurate indexing.

### Index Characteristic

- **UUID:** 4fd3af2a-10e8-474f-84d7-722bcfd3efc3
- **Data type:** `uint16`
- **Access:** Read, Write
- **Description:** Designates the starting index for operations performed through the Color Characteristic. The microcontroller initializes this to 0 at startup. This characteristic's value determines the starting LED for color assignments. The Index Characteristic strictly accepts non-negative values, with each representing a valid LED index within the strip.

### Color Characteristic

- **UUID:** 0c903aa6-de65-44c4-9cde-8873267e16c0
- **Data type:** `byte array`
- **Access:** Read, Write
- **Description:** Adjusts the color(s) of the LED(s) beginning from the index specified by the Index Characteristic. Data is formatted as a sequence of 3-byte RGB sets ([Red, Green, Blue]), with each byte reflecting the intensity of the respective color component. Writing to this characteristic updates the color of the targeted LED(s). Importantly, partial updates are supported, allowing for modification of individual color components without altering the entire color value. For instance, writing a single byte 0xFF when the index is set to 16 updates only the red component of that LED's color. Similarly, a sequence like 0xFF 0x00 0x00 0x00 0xFF would turn the first LED red and the next LED green, showcasing the protocol's flexibility in color adjustment. Reading from this characteristic retrieves color data for as many LEDs as possible, starting at the specified index, within the constraints of communication or buffer limits.

## Behavioral Notes and Conventions

- **Batch Operations:** The protocol supports batch operations for setting colors, allowing a sequence of colors to be written in a single operation, starting from the specified index. This enhancement facilitates efficient updates for multiple LEDs.

- **Reading Color Data:** Reading from the Color Characteristic returns the color information of LEDs in batches, starting from the index specified by the Index Characteristic. This behavior aligns with the write operation, ensuring symmetry in read/write interactions.

## Error Handling

Clients should implement robust error checking, particularly for write operations, to manage scenarios such as out-of-bounds indices or improperly formatted data for the Color Characteristic.

## Security Considerations

The current protocol version does not incorporate specific security mechanisms like encryption or authentication, leaving communications susceptible to eavesdropping and unauthorized access. This vulnerability underscores the importance of cautious protocol deployment, particularly in scenarios demanding confidentiality, integrity, or authentication. Future protocol updates will endeavor to enhance security features.
