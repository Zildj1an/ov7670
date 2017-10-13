# ov7670

Grabs pixel data from the camera and compresses it down to a 20x15 array, averaging the luminance to get values. Sends the 20x15 frame over serial to your PC.

## Requirements

`avr-gcc` and toolchain. For the Mac, I use [Crosspack](https://www.obdev.at/products/crosspack/index.html).

## Wiring

### Connections (Arduino / ov7670)

```
A5->SIOC
A4<->SIOD
11-> (convert 5v to 3.3v) XCLK
A0<-0
A1<-1
A2<-2
A3<-3
4<-4
5<-5
6<-6
7<-7
3<-VSYNC
2<-PCLK
GND->PWDN
3.3->RESET
GND->GND
3.3v->3.3v
```