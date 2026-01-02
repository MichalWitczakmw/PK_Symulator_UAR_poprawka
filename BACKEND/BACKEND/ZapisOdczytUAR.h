#pragma once

#include <string>
#include <fstream>
//#include <nlohmann/json.hpp>
#include "SymulacjaUAR.h"

//using json = nlohmann::json;

class SymulacjaUAR;

class ZapisOdczytUAR
{
public:
    // Zapis konfiguracji do pliku JSON
    bool zapiszDoPliku(const string& sciezka, const SymulacjaUAR& symulacja) const;

    // Odczyt konfiguracji z pliku JSON
    bool odczytajZPliku(const string& sciezka, SymulacjaUAR& symulacja) const ;
};

