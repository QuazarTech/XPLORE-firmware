# ``XPLORE-Firmware``

### Introduction

**How this project fits into the scheme of things** :
- The ``XPLORE`` system is a sophisticated research grade tool that allows for the measurement of  electrical and magnetic transport properties of materials over a wide range of temperature. The system is divided into several modules (Fig.~\ref{fig:xplore_electronics}) described in the following sections, each of which allows the user to explore numerous important properties of their sample.

<!--- Add XPLORE image --->

- Each of XPLORE's modules has separate hardware/software

- Each complete module is made up of the following inter-related layers :
*Hardware* -> **Firmware** -> *Driver* -> *Application* -> *GUI*

**Functions of Firmware :**
- The firmware is C++ code that runs on the microcontroller (Atmega644).

**Structure of Firmware :**
- The firmware's code resides in the folder `ModuleName/code`. `ModuleName/code` further contains the following directories :

    1. `app`   : Application code for XPLORE module
    2. `avr`   : AVR specific libraries
    3. `stl`   : C++ standard template libraries
    4. `sys`   : Libraries for ICs/LCDs on the hardware
    5. `utils` : Other utility libraries
    
`app` is further divided into `app` and `src`, where `app/app` contains all header (`.h`) files and `app/src` contains all `.cxx` files.

**Project Page :**

**Known Issues :** https://github.com/QuazarTech/XPLORE-firmware/issues

### Requirements

- XPLORE Physical Quantites Measurement System (Modules on which you want to modify software)

- Hardware requirements
  - Processor : Intel i3 or above
  - Storage : Atleast 10 GB
  - RAM : Atleast 4 GB

- OS requirements
  The XPLORE system has been tested on the following Operating Systems :
    - Ubuntu 16.04

- List of Dependencies:
  - python 2.7x
  - cmake
  - swig
  - avr-gcc toolchain
  
  Use the following command to get dependencies : 
  ```
  sudo apt-get install python cmake swig gcc-avr binutils-avr avr-libc gdb-avr avrdude
  ```
Found a missing dependency? Open an issue here : https://github.com/QuazarTech/XPLORE-firmware/issues

To burn the firmware on the respective XPLORE module :

 1. Navigate to `XPLORE-firmware/ModuleName/` (eg, `XPLORE-firmware/XSMU-2.4.0/`)
 2. In the termina, type `make clean` to clean all old dependency and output files
 3. Now, type `make ihx` to generate Intel Hex file.
 4. Make sure the XPLORE Module's USB cable is connected to the computer
 5. Switch off the XPLORE Module by turning off its power supply.
 6. Hold press the (Red) Firmware Upgrade button behind the XPLORE Module's electronics box. Simultaneously switch on the module's power supply. The LCD on the module should be blank. If not, redo step 5 and 6.
 7. In the terminal, type `make burn`. Wait for firmware to be burnt on the module's controller.
 8. Switch off the XPLORE module's power supply, and switch it on again (this time without pressing the firmware upgrade button). The LCD should not be blank now.

### Configuration


### Testing


### Troubleshooting and FAQ


### Maintainers

- Active :
  - Krishnendu Chatterjee (kc@quazartech.com)
  - Nishant Gupta (nishant@quazartech.com)
  - Gitansh Kataria (gitansh@quazartech.com)

- Past