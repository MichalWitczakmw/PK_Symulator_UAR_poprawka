#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>

    enum class TypSygnalu { Prostokat, Sinus };

class Generator
{
public:


    Generator(TypSygnalu typ = TypSygnalu::Sinus,
        double amplituda = 1.0,
        double okres = 2.0,
        double wypelnienie = 0.5,
        double skladowaStala = 0.0)
        : m_typ(typ),
          m_amplituda(amplituda),
          m_okres(okres),
          m_wypelnienie(wypelnienie),
          m_skladowaStala(skladowaStala),
		  m_indeks(0)
    {}

    void setTyp(TypSygnalu typ) { m_typ = typ; }
    void setAmplituda(double amp) { m_amplituda = amp; }
    void setOkres(double okres) { m_okres = okres; }
    void setWypelnienie(double wyp) { m_wypelnienie = wyp; }
    void setSkladowaStala(double skl) { m_skladowaStala = skl; }

    double generujWartosc();
    void reset() { m_indeks = 0; }

    TypSygnalu getTyp() const { return m_typ; }
    double getAmplituda() const { return m_amplituda; }
    double getOkres() const { return m_okres; }
    double getWypelnienie() const { return m_wypelnienie; }
    double getSkladowaStala() const { return m_skladowaStala; }


private:
    TypSygnalu m_typ;
    double m_amplituda;
    double m_okres;
    double m_wypelnienie;
    double m_skladowaStala;
    unsigned long long m_indeks;
};
