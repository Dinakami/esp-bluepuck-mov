# ESP Resbit

The goal of this project is to build software to emulate the functionality and behavior of a Research Bit ("Resbit") device on an ESP-32 SoC.

## Objective 1

Create a BLE GATT Server that matches the advertisement, services, and characteristics of the Resbit.

Refer to this document for specifics: [ESP Resbit GATT Server Info](./esp_resbit_gatt_server_info.pdf)

## Objective 2

Build functionality to allow a client to connect and write the time through the Information Service.

## Objective 3

Simulate sensor data to store and transfer through the Summary Service

## Resources

- [Basic Introduction To BLE](https://learn.adafruit.com/introduction-to-bluetooth-low-energy/gatt)

```
This is a short article to gain some familiarity with the concepts of Bluetooth Low Energy
```

- [GATT Server Example](https://github.com/espressif/esp-idf/tree/master/examples/bluetooth/bluedroid/ble/gatt_server_service_table)

 ```
The example Server code should be the starting point of the project.
It has a great tutorial walkthrough which explains the code, section by section.
 ```
