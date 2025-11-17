#pragma once

#include <algorithm>
#include <vector>
#include <numeric>

class Regulator_PID 
{
public:
    enum class LiczCalk { Wew, Zew };

    Regulator_PID(double kp = 1.0, double ti = 0.0, double td = 0.0);

    void setKp(double kp) { m_kp = kp; }
    void setTi(double ti) { m_ti = ti; }
    void setTd(double td) { m_td = td; }
    void setOgraniczenia(double min, double max)
    {
        m_ogrMin = min;
        m_ogrMax = max;
    }
    void setLiczCalk(LiczCalk typ);
    void setStalaCalk(double stala);

    void resetPamieci();

    double symuluj(double uchyb);

    double getSumaE() const;
    double getSumaEdivTi() const;
    double getUchybPoprzedni() const;

    double getKp() const { return m_kp; }
    double getTi() const { return m_ti; }
    double getTd() const { return m_td; }
    double getOgrMin() const { return m_ogrMin; }
    double getOgrMax() const { return m_ogrMax; }
    LiczCalk getLiczCalk() const { return m_liczCalk; }

private:
    double ograniczDoZakresu(double val, double min, double max) const;

    double m_kp;
    double m_ti;
    double m_td;

    LiczCalk m_liczCalk;
    double m_ogrMin;
    double m_ogrMax;
    double m_sumUchybow; 
    double m_sumCalkiWew;  
    double m_uchybPoprzedni;
};