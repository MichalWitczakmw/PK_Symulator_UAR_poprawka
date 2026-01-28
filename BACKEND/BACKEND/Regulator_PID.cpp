#include "Regulator_PID.h"

Regulator_PID::Regulator_PID(double kp, double ti, double td)
    : m_kp(kp), m_ti(ti), m_td(td),
    m_liczCalk(LiczCalk::Zew),
    m_ogrMin(-10.0), m_ogrMax(10.0),
    m_sumUchybow(0.0), m_sumCalkiWew(0.0),
    m_uchybPoprzedni(0.0)
{}



void Regulator_PID::setLiczCalk(LiczCalk typ) 
{
    if (typ == m_liczCalk) 
        return;
    if (m_ti > 0.0) 
    {
        if (typ == LiczCalk::Wew) 
            // Zew -> Wew
            m_sumCalkiWew = m_sumUchybow / m_ti;
        else 
            // Wew -> Zew
            m_sumUchybow = m_sumCalkiWew * m_ti;
    }
    m_liczCalk = typ;
}

void Regulator_PID::setStalaCalk(double stala) { setTi(stala); }

void Regulator_PID::resetPamieci() 
{
    m_sumUchybow = 0.0;
    m_sumCalkiWew = 0.0;
    m_uchybPoprzedni = 0.0;
}

double Regulator_PID::symuluj(double uchyb) 
{

    double czP = m_kp * uchyb;

    double czI = 0.0;
    if (m_ti > 0.0) 
    {
        m_sumUchybow += uchyb;
        m_sumCalkiWew += (uchyb / m_ti);

        if (m_liczCalk == LiczCalk::Zew) 
            czI = (1.0 / m_ti) * m_sumUchybow;
        else 
            czI = m_sumCalkiWew;
    }

    double czD = 0.0;
    if (m_td > 0.0) 
        czD = m_td * (uchyb - m_uchybPoprzedni);

    m_uchybPoprzedni = uchyb;

    //return ograniczDoZakresu(czP + czI + czD, m_ogrMin, m_ogrMax);
    return czP + czI + czD;
}

double Regulator_PID::getSumaE() const { return m_sumUchybow; }
double Regulator_PID::getSumaEdivTi() const { return m_sumCalkiWew; }
double Regulator_PID::getUchybPoprzedni() const { return m_uchybPoprzedni; }

double Regulator_PID::ograniczDoZakresu(double val, double min, double max) const 
{
    if (val < min) 
        return min;
    if (val > max) 
        return max;

    return val;
}
