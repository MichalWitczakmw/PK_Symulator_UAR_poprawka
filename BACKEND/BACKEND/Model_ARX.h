#pragma once
#include <vector>
#include <deque>
#include <random>
#include <algorithm>

class Model_ARX {
public:
    // Konstruktor argumentowy (bez zak³ócenia)
    Model_ARX(
        const std::vector<double>& A,
        const std::vector<double>& B,
        int delay = 1,
        double noise_stddev = 0.0
    );

    // Jednokrokowa symulacja: wejœcie -> wyjœcie
    double symuluj(double u);

    // Ustawienia ograniczeñ
    void setOgrSter(double minU, double maxU, bool aktywne = true);
    void setOgrRegul(double minY, double maxY, bool aktywne = true);

    // Wy³¹czanie/w³¹czanie ograniczeñ
    void setOgrSterAktywne(bool aktywne);
    void setOgrRegulAktywne(bool aktywne);

    // Zmiana parametrów modelu w locie
    void setA(const std::vector<double>& A);
    void setB(const std::vector<double>& B);
    void setDelay(int delay);
    void setNoiseStddev(double stddev);

private:
    std::vector<double> m_A, m_B;
    int m_delay;
    double m_noiseStddev;

    double m_minU = -10.0, m_maxU = 10.0;
    double m_minY = -10.0, m_maxY = 10.0;
    bool m_ogrSter = true, m_ogrRegul = true;

    std::deque<double> m_bufU; // Bufor próbek sterowania (dla B)
    std::deque<double> m_bufDelayU; // Bufor opóŸnienia transportowego
    std::deque<double> m_bufY; // Bufor próbek regulowanej (dla A)

    // Generator zak³óceñ
    std::mt19937 m_rng;
    std::normal_distribution<double> m_noiseDist;

    // Pomocnicze funkcje ograniczeñ
    double saturate(double val, double minVal, double maxVal) {
        return std::max(minVal, std::min(maxVal, val));
    }
};