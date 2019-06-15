#include "homodocker.hpp"
#include <iostream>
using namespace std;
using namespace docker;

int main(int argc, char** argv) {
    cout << "HomoDocker Start！" << endl;
    kimo_config config;
    config.host_name = "homolive";
    config.root_dir  = "./homolive";
    config.ip        = "192.168.0.100";
    config.bridge_name = "docker0";
    config.bridge_ip   = "192.168.0.1";
    kimo kimo(config);
    kimo.start();
    cout << "HomoDocker Exit!" << endl;
    return 0;
}