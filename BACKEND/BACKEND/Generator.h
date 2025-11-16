#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>

class Generator
{
public:
    enum class TypSygna³u { Prostok¹t, Sinus };

    Generator(TypSygna³u typ = TypSygna³u::Prostok¹t,
        double amplituda = 1.0,
        double okres = 100.0,
        double wypelnienie = 0.5,
        double skladowaStala = 0.0);

    void setTyp(TypSygna³u typ) { m_typ = typ; }
    void setAmplituda(double amp) { m_amplituda = amp; }
    void setOkres(double okres) { m_okres = okres; }
    void setWypelnienie(double wyp) { m_wypelnienie = wyp; }
    void setSkladowaStala(double skl) { m_skladowaStala = skl; }

    double generujWartosc();
    void reset() { m_indeks = 0; }

private:
    TypSygna³u m_typ;
    double m_amplituda;
    double m_okres;
    double m_wypelnienie;
    double m_skladowaStala;
    unsigned long long m_indeks;
};
