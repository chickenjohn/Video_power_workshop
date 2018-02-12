# Video Power Workshop
*A Brief introduction of this project.*

## What is it used for?
Video power workshop is a simple toolset of controling system based on Digilent Zybo board and OpenCV Video library. With only a Zybo board, a VGA screen and a usb webcam you'll be able to build a whiteboard to write on it using a simple blue pen. you can also control the system by gestures. It can reflect on your head movement as well, though it is just do nothing but moving a block simultaneously with your head.

## Why would you build that?
The program is build for us young students to get further familiar with embedded system on FPGA like Xilinx Zynq devices. By making such a thing come true, we've learned how to configure the embedded GNU/Linux operating system, develop the video processing system, set up a simple GUI using Qt libs and control the peripheral devices. 

## What is involved in the designing?(The running environment)
There are actually several parts of the system and a variety of toolset has been involved in.
### Hardware Structures
The hardware structures is mainly based on the Xillybus design of Zybo hardware. There are plenty of samples on the Internet about how to make it work on your board. Considering that the evalution license given by Xillybus.com, the hardware struction will not be attached here. Visiting the website of Xillybus and Digilent.Inc may offer you some additional information. 

*Digilent Zybo*
[http://www.digilentinc.com/Products/Detail.cfm?NavPath=2,400,1198&Prod=ZYBO](http://www.digilentinc.com/Products/Detail.cfm?NavPath=2,400,1198&Prod=ZYBO "Digilent Zybo ")

*Xillybus.com*
[http://www.xillybus.com/download](http://www.xillybus.com/download "xillybus")

### Operating Systems
We choose the Xillinux operating system for controlling. The Xillinux is mainly based on Ubuntu system. It works pretty well on the Zybo board. You can download if from *here*:
[http://www.xillybus.com/Xillinux](http://www.xillybus.com/Xillinux "xillinux")

### Drivers and running environment
The needed drivers are all contained in the Xillinux system. Actually the kernel comes from the Xilinx linux core:[https://github.com/Xilinx/linux-xlnx](https://github.com/Xilinx/linux-xlnx "xilinxcore")

A Qt library and a OpenCV library must be cross compiled to fit for the Zybo. The version information is given below.

>Qt 4.8.4

>gcc-linaro-arm-gnueabihf 4.8.2 (linaro-1.13.1-4.8-2013.09 - Linaro GCC 2013.09)

>OpenCV 2.4.9

### Softwares
All the softwares are based on Qt library and OpenCV library. So it can be compiled on the platform supported by these two things, for example, a PC.

## License 
The software is under the GPL v2.0.

## Further information
You may contact me for further information. My email address showed on Github is avalible.
