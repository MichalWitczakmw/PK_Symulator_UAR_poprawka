#pragma once

#include <QString>

class SymulatorUAR;

// Klasa odpowiedzialna tylko za zapis/odczyt konfiguracji UAR w JSON (Qt).
class ZapisOdczytUAR
{
public:
    ZapisOdczytUAR() = default;
    ~ZapisOdczytUAR() = default;

    // Zapis konfiguracji (ARX, PID, Generator) do pliku JSON.
    bool zapiszDoPliku(const QString& sciezka,
                       const SymulatorUAR& symulator) const;

    // Odczyt konfiguracji z pliku JSON i ustawienie jej w SymulatorUAR.
    bool odczytajZPliku(const QString& sciezka,
                        SymulatorUAR& symulator) const;
};
