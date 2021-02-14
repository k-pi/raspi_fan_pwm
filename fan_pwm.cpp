#include <fstream>
#include <iostream>
#include <csignal>
#include <pigpio.h>
#include <cerrno>
#include <climits>
#include <thread>
#include <cstring>
#include <string>


//Settings
  //File described CPU temperature
  const std::string CPU_TEMP_FILE_PATH = "/sys/class/thermal/thermal_zone0/temp";

  //PWM GPIO PIN
  const int GPIO_PIN = 12;

  //PWM Frequency
  //Supplement : If you feel that fan driving sound is noisy, it's worth trying to change FREQ.
  const int FREQ = 30000;  // Hz

  //Thresholds of CPU Temperature and PWM Duty Ratio
  //When CPU temperature is under CPU_TEMP_MIN_THRESHOLD, PWM Duty Ratio is set to DEFAULT_DUTY_RATIO.
  //When CPU temperature is over CPU_TEMP_MAX_THRESHOLD, PWM Duty Ratio is set to MAX_DUTY_RATIO.
  //When CPU temperature is between these, PWM Duty Ratio is set to between MIN_DUTY_RATIO and MAX_DUTY_RATIO with linear interpolation.
  //Supplement : DUTY_RATIO must be 0 or between 500000 and 1000000 under 3.3V drive voltage with "Raspberry Pi 4 Case Fan".
  const long CPU_TEMP_MIN_THRESHOLD = 60 * 1000;  // Millidegree(s) Celsius
  const long CPU_TEMP_MAX_THRESHOLD = 75 * 1000;  // Millidegree(s) Celsius

  const int DEFAULT_DUTY_RATIO = 50 * 10000;  // Parts per million (ppm)
  const int MIN_DUTY_RATIO = 50 * 10000;  // Parts per million (ppm)
  const int MAX_DUTY_RATIO = 100 * 10000;  // Parts per million (ppm)

  //Time interval of checking CPU temperature
  const int CHECK_INTERVAL = 5;  // Second(s)

  //Interval of status output to standard output
  //Supplement : If INTERVAL_STD_OUT is set to "60 / CHECK_INTERVAL", status is output at about 60 seconds interval even if CHECK_INTERVAL is any value.
  const int INTERVAL_STD_OUT = 60 / CHECK_INTERVAL;  // The number of times of checking CPU temperature


void signal_handler(int sig)
{
    gpioHardwarePWM(GPIO_PIN, 0, 0);
    gpioTerminate();
    exit(0);
}

void void_func(int sig)
{
    
}

int main(int argc, char* argv[])
{
    int DUTY_RATIO = DEFAULT_DUTY_RATIO;
    int CHECK_COUNT = 0;

    std::ifstream cpu_temp_file;
    std::string cpu_temp_string;
    int cpu_temp;

    if (MAX_DUTY_RATIO < DEFAULT_DUTY_RATIO)
      return -1;
    
    if (MAX_DUTY_RATIO < MIN_DUTY_RATIO)
      return -2;

//    gpioCfgSocketPort(8889);  // If Port 8888 is binded to any other service, you have to uncomment this line.
    
    if (gpioInitialise() < 0)
      return 1;

    gpioSetMode(GPIO_PIN, PI_OUTPUT);

//  Signal handling
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTSTP, void_func);
    signal(SIGCONT, void_func);

    while (1) {
       char *error_str;

       cpu_temp_file.open(CPU_TEMP_FILE_PATH, std::ios::in);
       std::getline(cpu_temp_file, cpu_temp_string);
       errno = 0;
       cpu_temp = strtol(cpu_temp_string.c_str(), &error_str, 10);

       if (!cpu_temp_file || strlen(error_str) != 0 || cpu_temp == LONG_MAX || (cpu_temp == LONG_MIN && errno == ERANGE) || *error_str != '\0') {
         DUTY_RATIO = MAX_DUTY_RATIO;
         std::cout << "cpu_temp_file can't be opened. set DUTY_RATIO to " << DUTY_RATIO/10000 << "%..." << std::endl;
       }else{
         if (cpu_temp <= CPU_TEMP_MIN_THRESHOLD){
           DUTY_RATIO = DEFAULT_DUTY_RATIO;
         }else if (cpu_temp >= CPU_TEMP_MAX_THRESHOLD){
           DUTY_RATIO = MAX_DUTY_RATIO;
         }else{
           DUTY_RATIO = MIN_DUTY_RATIO + (MAX_DUTY_RATIO - MIN_DUTY_RATIO) * (static_cast<double>(cpu_temp - CPU_TEMP_MIN_THRESHOLD) / static_cast<double>(CPU_TEMP_MAX_THRESHOLD - CPU_TEMP_MIN_THRESHOLD));
         }

         if (CHECK_COUNT == 0){
           std::cout << "cpu_temp is " << static_cast<double>(cpu_temp)/1000.0 << ". set DUTY_RATIO to " << static_cast<double>(DUTY_RATIO)/10000.0 << "%..." << std::endl;
         }
       }

       cpu_temp_file.close();

       if (gpioHardwarePWM(GPIO_PIN, FREQ, DUTY_RATIO) < 0)
         return 2;

       CHECK_COUNT++;
       if (CHECK_COUNT == INTERVAL_STD_OUT)
         CHECK_COUNT = 0;

       std::this_thread::sleep_for(std::chrono::seconds(CHECK_INTERVAL));
    }

}
