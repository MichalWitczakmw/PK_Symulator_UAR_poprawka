#include "Generator.h"

Generator::Generator(TypSygna³u typ,
    double amplituda,
    double okres,
    double wypelnienie,
    double skladowaStala)
    : m_typ(typ),
    m_amplituda(amplituda),
    m_okres(okres),
    m_wypelnienie(wypelnienie),
    m_skladowaStala(skladowaStala),
    m_indeks(0)
{}

double Generator::generujWartosc() 
{
    double wartosc = 0.0;

    if (m_typ == TypSygna³u::Sinus) 
    {
        double t = (2.0 * M_PI * m_indeks) / m_okres;
        wartosc = m_amplituda * sin(t) + m_skladowaStala;
    }
    else
    {
        double modul = std::fmod(m_indeks, m_okres);
        wartosc = (modul < m_wypelnienie * m_okres ? m_amplituda : 0.0) + m_skladowaStala;
    }

    m_indeks++;
    return wartosc;
}