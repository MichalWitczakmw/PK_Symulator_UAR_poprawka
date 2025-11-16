#pragma once

#include <algorithm>
#include <vector>
#include <numeric>

class Regulator_PID {
public:
    enum class LiczCalk { Wew, Zew };

    Regulator_PID(double kp = 1.0, double ti = 0.0, double td = 0.0)
        : m_kp(kp), m_ti(ti), m_td(td),
        m_liczCalk(LiczCalk::Zew),
        m_ogrMin(-1e10), m_ogrMax(1e10),
        m_sumUchybow(0.0), m_sumEdivTi(0.0),
        m_uchybPoprzedni(0.0)
    {
    }

    void setKp(double kp) { m_kp = kp; }

    // Zmiana TI. Zgodnie z wymaganiami:
    // - jeœli jestesmy w trybie Zew -> zmiana TI powinna spowodowaæ natychmiastowy skok wartoœci I
    //   (czyli nie korygujemy pamiêci).
    // - jeœli jesteœmy w trybie Wew -> zmiana TI powinna wp³ywaæ tylko na przysz³e przyrosty (nie
    //   korygujemy sumy, bo m_sumEdivTi zawiera elementy e_j/Ti_j z Ti_j z chwili zapisu).
    void setTi(double ti) { m_ti = ti; }

    void setTd(double td) { m_td = td; }
    void setOgraniczenia(double min, double max) { m_ogrMin = min; m_ogrMax = max; }

    // Przy przelaczaniu sposobu liczenia ca³ki wykonujemy konwersjê sum,
    // tak aby prze³¹czenie nie powodowa³o skoku wartoœci I:
    // - przejœcie Zew -> Wew: m_sumEdivTi = m_sumUchybow / TI_current
    // - przejœcie Wew -> Zew: m_sumUchybow = m_sumEdivTi * TI_current
    // (gdy TI == 0 nie dokonujemy konwersji — integral jest traktowany jako wy³¹czony)
    void setLiczCalk(LiczCalk typ) {
        if (typ == m_liczCalk) return;

        // wykonaj konwersjê tylko gdy TI > 0 (w przeciwnym razie ca³ka by³a wy³¹czona)
        if (m_ti > 0.0) {
            if (typ == LiczCalk::Wew) {
                // Zew -> Wew
                m_sumEdivTi = m_sumUchybow / m_ti;
            }
            else {
                // Wew -> Zew
                m_sumUchybow = m_sumEdivTi * m_ti;
            }
        }
        m_liczCalk = typ;
    }

    void setStalaCalk(double stala) { setTi(stala); }

    // reset pamiêci czêœci ca³kuj¹cej (rêcznie wywo³ywane)
    void resetPamieci() {
        m_sumUchybow = 0.0;
        m_sumEdivTi = 0.0;
        m_uchybPoprzedni = 0.0;
    }

    // symulacja jednego kroku: podajemy bie¿¹cy uchyb (e)
    double symuluj(double uchyb) {
        // Jeœli TI == 0: czêœæ ca³kuj¹ca wy³¹czona -> NIE aktualizujemy pamiêci i zwracamy 0 dla I.
        if (m_ti != 0.0) {
            // Aktualizujemy obie sumy przy dodaniu próbki:
            // - m_sumUchybow = suma e_j (do trybu "Zew")
            // - m_sumEdivTi = suma (e_j / Ti_j) (do trybu "Wew"), gdzie Ti_j to TI w chwili dodania
            m_sumUchybow += uchyb;
            m_sumEdivTi += (uchyb / m_ti);
        }

        double czP = m_kp * uchyb;
        double czI = obliczCalkujaca();
        double czD = obliczRozniczkujaca(uchyb);

        double u = czP + czI + czD;
        u = ograniczDoZakresu(u, m_ogrMin, m_ogrMax);
        return u;
    }

private:
    // Ca³ka (zwraca wartoœæ I bez mno¿enia przez Kp, bo P jest liczone osobno)
    // - jeœli TI == 0 -> 0
    // - Zew: wynik = (1/TI) * sum(e_j)
    // - Wew: wynik = sum(e_j / Ti_j)
    double obliczCalkujaca() const {
        if (m_ti <= 0.0) return 0.0;

        if (m_liczCalk == LiczCalk::Zew) {
            return (1.0 / m_ti) * m_sumUchybow;
        }
        else {
            return m_sumEdivTi;
        }
    }

    // Ró¿niczka (dt = 1 w testach)
    double obliczRozniczkujaca(double uchyb) {
        if (m_td <= 0.0) return 0.0;
        double wynik = m_td * (uchyb - m_uchybPoprzedni);
        m_uchybPoprzedni = uchyb;
        return wynik;
    }

    double ograniczDoZakresu(double val, double min, double max) const {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }

    // parametry regulatora
    double m_kp;
    double m_ti;
    double m_td;

    LiczCalk m_liczCalk;
    double m_ogrMin, m_ogrMax;

    // pamiêæ czêœci ca³kuj¹cej:
    // - m_sumUchybow : suma e_j (dla trybu Zew)
    // - m_sumEdivTi  : suma e_j / Ti_j (dla trybu Wew)
    double m_sumUchybow;
    double m_sumEdivTi;

    // pamiêæ do czêœci ró¿niczkuj¹cej
    double m_uchybPoprzedni;
};