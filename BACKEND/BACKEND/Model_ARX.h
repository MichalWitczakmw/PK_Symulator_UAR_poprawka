#pragma once
#include <vector>
#include <deque>
#include <random>
#include <algorithm>

using namespace std;

class Model_ARX 
{

private:
    vector<double> m_wspolczynnikA, m_wspolczynnikB;
    int m_opoznienieTransportowe;
    double m_oSSzum;
    double m_minU = -10.0, m_maxU = 10.0;
    double m_minY = -10.0, m_maxY = 10.0;
    bool m_ogrSterowania = true, m_ogrRegulowania = true;

    deque<double> m_buforU; // Bufor próbek sterowania (dla B)
    deque<double> m_buforOpoznienia; // Bufor opóŸnienia transportowego
    deque<double> m_buforY; // Bufor próbek regulowanej (dla A)

    // Generator zak³óceñ
    mt19937 m_GeneratorZaklocen;
    normal_distribution<double> m_rozkladZaklocen;

    // Pomocnicze funkcje ograniczeñ
    double nasycenie(double wartosc, double minWartosc, double maxWartosc) 
    {
        return max(minWartosc, min(maxWartosc, wartosc));
    }

public:

    Model_ARX(
        const vector<double>& wspolczynnikA,
        const vector<double>& wspolczynnikB,
        int opoznienieTransportowe = 1,
        double zaklocenie = 0.0
    );

    // Jednokrokowa symulacja
    double symuluj(double sygnalSterujacy);

    // Ustawienia ograniczeñ
    void setOgrSterowania(double minU, double maxU, bool aktywne = true);
    void setOgrRegulowania(double minY, double maxY, bool aktywne = true);

    // Wy³¹czanie/w³¹czanie ograniczeñ
    void setOgrSterowaniaAktywne(bool aktywne);
    void setOgrRegulowaniaAktywne(bool aktywne);

    // Zmiana parametrów modelu w locie
    void setA(const vector<double>& A);
    void setB(const vector<double>& B);
    void setopoznienieTransport(int opoznienieTransportowe);
    void setOdchylenieZaklocen(double odchylenieZaklocenia);


};