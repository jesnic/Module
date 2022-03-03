/*
  Library with some useful functions for coding a module
  Created by Lindtrs, 2021.
*/
#ifndef Module_h
#define Module_h

#define CONFIG_SIZE 256
#define CONFIG_PATH "/config.json"
#define DEFAULT_SSID "Config"
#define DEFAULT_PASSWORD "12345678"
#define DEFAULT_DNS "config"

#include "Arduino.h"
#include "ModuleUtils.h"

class ModuleConfig
{

  struct config {
    const char * ssid;
    const char * password;
  };

  public:
    ModuleConfig();
    ModuleConfig(const char * ap_SSID, const char * ap_password, const char * ap_DNS, const char * config_path);
    void begin();
    bool reset();
    bool check();
    bool save(const char *  ssid, const char *  password);
    const char * load_SSID();
    const char * load_password();
    void listen();
    void config_page();
    void finish_page();
  private:
    const char * _config_path;
    const char * _ap_SSID; // Access point SSID
    const char * _ap_password; // Access point password
    const char * _ap_DNS; // Access point DNS
};

#endif