MCUXpresso IDE
--------------

========================
Version 10.0.2 (Jul 2017)
=========================

* Fixed an issue where an MCU provided in both SDK and
  pre-installed support (e.g. LPC5411x) could in some
  circumstances erroneously reselect part support for a project
  from the wrong source

* Solved an issue with attempting to install a non-IDE-compatible
  SDK into a non-default location
  
* Solved some issues with creating projects in non-default
  locations

* Fixed an issue with the Properties view displaying information
  for the wrong device in some circumstances (for pre-installed
  parts)

* Fixed an issue with incorrect dependency selection in project 
  wizards in some circumstances when switching between boards

* It is now possible to create makefile projects for SDK MCUs

* Enhanced the Memory Configuration Editor to give an error if
  there is no RAM defined for a project
  
* Enhanced the Heap & Stack Editor to allow heap size to be set
  to zero

* Added a "Create Srecord" option to the Binary Utilities menu

* Implemented a port auto-discovery mechanism for LinkServer,
  SEGGER and P&E Micro debug sessions, to improve concurrent
  debug session behavior

* Improved editing of launch configurations via double-click, and
  also improved their presentation in the Project Explorer

* Fixed some issues with manually generated launch configurations 
  (including non-stop setting)
  
* Added probe icons in the "Launch Configuration Selection" dialog

* Fixed a failure to display registers for certain peripherals
  in Peripheral View for SDK-based MCUs
 
* Resolved an issue with Peripherals failing to display if a '_' 
  character was used in a register name

* Fixed some issues with the semihosting console:
  - Space/newline characters were occasionally lost when printing
    only a single character
  - Empty strings were sometimes mishandled
      
* Fixed an issue with "Terminate All" in some circumstances when 
  multiple debug connections were active   

* Improved synchronization of the state of MUCXpresso IDE's blue
  debug button on the toolbar with the one on the Quickstart
  Panel, particularly when moving between tabs

* Stopped the Registers view triggering a null pointer error when
  an n/a value was clicked

* Fixed a LinkServer issue with the setting of watchpoints when 
  debugging Cortex-M0/M0+ based MCUs

* Fixed an issue with auto-core selection when debugging
  triple-core LPC43xx devices
  
* Fixed an issue with display of performance counters in
  LinkServer SWO Trace

* LinkServer FreeRTOS Thread Aware Debug is now only available in
  all-stop debug connections, not in non-stop. However, FreeRTOS
  TAD views are still available for non-stop connections

* Fixed an issue with LinkServer debug connections losing target
  control when an attempt was made to use more hardware
  breakpoint units than were implemented by the MCU

* Fixed an issue where peripherals could fail to be displayed in 
  LinkServer debug connections in some circumstances

* Enhanced LinkServer support for debugging RAM-only projects

* Reduced the startup time for LinkServer debug sessions

* Improved the performance of LinkServer semihosting

* Enhanced LinkServer to allow restricted parsing
  of the MCU debug AP bus, allowing support for MCUs with 
  incomplete Coresight implementations
  - Debug of Kinetis KL28 MCUs is now supported via LinkServer

* Fixed an issue with MCUXpresso IDE mistakenly attempting to use 
  JTAG instead of SWD connections for certain CMSIS-DAP probes 
  with multicore MCUs that have no JTAG support in hardware
  
* Fixed an issue with MCUXpresso IDE in some circumstances trying
  to make a LinkServer multicore debug connection to a slave core 
  without correctly selecting the core
  
* Fixed the Save button for the LinkServer SWO Trace ITM console 

* LinkServer no longer leaves the MCU's CPU in debug mode when 
  terminating a debug connection (so semihosted I/O will now
  hard fault, rather than causing the CPU to enter debug)

* Made various improvements to the LinkServer GUI flash
  programmer

* Upgraded to a later version of the SEGGER software (v6.16b)

* Improved SEGGER support, including:
  - Support for concurrent debugging via multiple SEGGER probes
  - Improved SEGGER launch configuration UI, providing more 
    options directly and adding a new Startup tab
  - Fixed an issue with terminating a debug session when multiple
    SEGGER debug sessions are active
  - Enhanced automatic part selection for SEGGER debug
    connections of pre-installed MCUs
  - Changed the SEGGER device dropdown to accept free-form text 
  - Fixed an issue seen when debugging with an external SEGGER
    GDB server, which triggered telnet console issues
  - SEGGER server errors now appear in the dialog when debug
    connections fail
  - Fixed a server shutdown when a debug session was terminated
    to leave the board in a running state
   
* Upgraded to a later version of the P&E Micro plugin (v3.0.3)
   
* Improved P&E Micro support, including:
  - Support for concurrent debugging via multiple P&E Micro
    probes
  - Fixed an issue with the Quickstart Teminate/Build/Debug
    button when used with P&E Micro debug connections
   
* Various other bug-fixes and UI tweaks, including:
  - Quickstart Panel floating point options made device
    specific         
  - Added a link to the Error Log view in the invalid SDK
    exception error dialog
  - Fixed a LinkServer semihosting input issue (e.g. scanf
    needing extra carriage returns)
  - Fixed an issue with handling the LPC5411x SDK versus
    preinstalled support board selection
  - Removed non-stop Probe Discovery options that had been
    presented for the LinkServer GUI Flash Programmer Mass erase
    function
  - Added an option to the Project Wizard to allow import of 
    "miscellaneous" files from SDK project structure and SDK
    components into the generated project
  - Fixed an issue with library project creation pulling in
    startup code

* Changed the name of the Linux installer from .run to .bin
  to avoid issues with the Flexera download system

* Fixed the Linux P&E Micro udev rules setup for users who have
  Kinetis Design Studio installed
    
* Documentation fixes and minor enhancements to reflect product
  changes
  
* Enhancements to LPC84x support, including ROM divide support
  in the new project wizard
 
* Added PN7xxxx to the list of pre-installed MCUs  

* Added QN9080-specific LinkServer debug support
    

=========================
Version 10.0.0 (Mar 2017)
=========================
 * First release

