# raspi_fan_pwm
[For Raspberry Pi] Program for controlling a PWM fan such as "Raspberry Pi 4 Case Fan" depending on the CPU temperature.

## 1. How to use
### 1-1. Wire Raspberry Pi and PWM fan.
First of all, Wire Raspberry Pi and PWM fan.  

According to the "Raspberry Pi 4 Case Fan" instruction,  
  VCC (Red) lead is connected to 5V pin (GPIO 4 pin).  
  GND (Black) lead is connected to Ground pin (GPIO 6 pin).  
  PWM (Blue) lead is connected to GPIO 14 (GPIO 8 pin).  
  
However, I recommend following connection for silent fan drive. (With some jumper cables.)  
  VCC (Red) lead is connected to 3.3V pin (GPIO 1 pin).  
  GND (Black) lead is connected to Ground pin (GPIO 6 pin).  
  PWM (Blue) lead is connected to GPIO 12 (GPIO 32 pin).  
  
### 1-2. Install "pigpio"
This program needs "pigpio", so you have to install this.  
If you use Raspbian, you can install following commands.  
```
sudo apt update
sudo apt install pigpio
```

### 1-3. Compile the program
Compile the program such as a following command. (Don't forget to link pigpio library.)
```
g++ -O2 -lpigpio fan_pwm.cpp -o fan_pwm.out
```

### 1-4. Execute as a service
To control PWM fan in background, Make a service.  
First, make "/lib/systemd/system/fan_pwm.service" with the following contents.
```
[Unit]
Description=fan_pwm

[Service]
ExecStart=~~~ Path of fan_pwm.out ~~~
KillMode=process
Type=simple
Restart=no

[Install]
WantedBy=multi-user.target
```

Next, enable the service.
```
sudo systemctl enable fan_pwm.service
```

Then, PWM fan drives according to the settings.  
You can the operating state with the following command.
```
systemctl status fan_pwm.service
```

## 2. Why do I make this program?
Official PWM fan control emits a noise when PWM fan drives. (That is "dtoverlay=gpio-fan".)  
I feel it too noisy.  
That's all.

## 3. License
This program is licenced under CC0.  
For detail, see [LICENSE](./LICENSE) file.

## 4. Post-Script
I'm not a native English speaker.  
I'm sorry if my English is hard to understand. 
