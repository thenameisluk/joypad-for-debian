#include "config.hpp"
#include <iostream>
#include <fstream>

std::map<std::string,std::string> getConfig(){
    std::map<std::string,std::string> out;

    std::ifstream config("/etc/joypad.conf");

    if(!config.is_open()){
        std::cout << "no config at /etc/joypad.conf" << std::endl;

        exit(2);
    }

    std::string line;
    std::string a = "";
    std::string b = "";

    std::cout << "Config :" << std::endl;

    while(std::getline(config,line)){
        bool key = true;
        a = "";
        b = "";

        for(int i = 0;i<line.length();i++){
            if(key){
                if(line.at(i)=='='){
                    key = false;
                }else{
                    a+=line.at(i);
                }
            }else{
                b+=line.at(i);
            }
        }
        if(key)
            continue;
        
        out[a] = b;
        
        std::cout << a << "->" << b << std::endl;
    }

    return out;
}