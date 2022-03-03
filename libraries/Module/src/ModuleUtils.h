/*
  Library with some useful functions for coding a module
  Created by Lindtrs, 2021.
*/
#ifndef ModuleUtils_h
#define ModuleUtils_h

#include "Arduino.h"
// #include <ESP8266WebServer.h>


class ModuleUtils
{
  public:
    ModuleUtils();
    void create_network(const char * ssid, const char * password, const char * dns="");
    void join_network(const char * ssid, const char * password, const char * hostname="");
    const char * post_http_request(const char * address, const char * data);
    String get_http_request(String address);
    bool save_file(const char * path, const char * content);
    bool delete_file(const char * path);
    bool rename_file(const char * path1, const char * path2);
    bool append_file(const char * path, const char * content);
    const char * read_file(const char * path);
  private:
};

// extern ESP8266WebServer server;
extern ModuleUtils module_utils;


#endif