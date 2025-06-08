# nvs-handler by Paul Mulligan

## Description

A simple key–value Non-volatile Storage handler in C that runs on a 32-bit microcontroller

## Details

This is a simple design which simulates functionality which could be used for accessing NVS key-value data for an MX25 NOR flash chip. 

From the MX25 datasheet, the flash page size for writing is 256 bytes and the minumum erasable sector size is 4K bytes. 

This design obviously does not implement a full driver for this NOR flash chip. It just overwrites a binary file on each flash commit with the latest updated nvs key-value data along with a crc for detecting partial writes, or other potential issues with the flash memory. 

nvs_init() - It opens and reads a file called "nvs_flash_crc.bin". It creates it if it doesn't exist. Key-value data is read into dynamic memory in RAM. If there is a crc error detected, this will not be loaded into RAM. 

nvs_deinit() - It frees the dynamic memory used for the NVS data .

nvs_set_uint32() - set a uint32 value based on its key. If the key is not found, a new key-value entry will be created. (This does not get written to the file yet until nvs_commit() is called)

nvs_set_string() - set a string value based on its key. If the key is not found, a new key-value entry will be created. (This does not get written to the file yet until nvs_commit() is called)

nvs_get_uint32() - get uint32 value based on the provided key.

nvs_get_string() - get string value based on the provided key.

nvs_delete_entry() - Delete an NVS entry. It just updates a flag which should signal to the driver to delete from flash later. 

nvs_commit() - Writes the NVS data to a file, simulating the writing of data to the flash. Many assumptions are made here as commented in the code.


### Dependencies

This application can be tested on a Linux system. 

This has so far been tested on Ubuntu 22.04 with gcc. 

To build and run the application, it's necessary to have _build essential_ installed which contains libraries and tools for building C applications. 

Install on Debian or Ubuntu as follows : 

```
sudo apt-get install build-essential

```

### Installing

Clone the repository anywhere on your system using SSH or HTTPs. Go to the "<>Code" dropdown menu at the top right of the project for the options. Or else download the project zip file and extract it.


### Building the application

Build the application by simply going into the project folder and running make :

```
cd nvs-handler

make

```
Clean the application by running :

```

make clean

```

### Executing program

If the project built successfuly, there will be an executable file called _main.exe_ generated in the root of the project folder. 

Execute this to run the application. 

There is simple test code in the main() function for testing the NVS API functions.

## Author

Paul Mulligan

mulligan252@yahoo.ie
