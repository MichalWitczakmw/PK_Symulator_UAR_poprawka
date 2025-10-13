#include "Model_ARX.h"

Model_ARX::Model_ARX(const std::vector<double>& A, const std::vector<double>& B, int delay, double noise_stddev)
    : m_A(A), m_B(B), m_delay(std::max(1, delay)), m_noiseStddev(noise_stddev),
    m_rng(std::random_device{}()), m_noiseDist(0.0, (noise_stddev > 0.0) ? noise_stddev : 1e-9)
{
    m_bufU.assign(B.size(), 0.0);
    m_bufDelayU.assign(m_delay, 0.0);
    m_bufY.assign(A.size(), 0.0);
}

double Model_ARX::symuluj(double u) {
    // 1. OGRANICZENIE sterowania
    if (m_ogrSter)
        u = saturate(u, m_minU, m_maxU);

    // 2. Bufor opóŸnienia transportowego
    m_bufDelayU.push_back(u);
    double uDelay = m_bufDelayU.front();
    m_bufDelayU.pop_front();

    // 3. Bufor sterowania do splotu z B
    m_bufU.push_back(uDelay);
    m_bufU.pop_front();

    // 4. Bufor regulowanej do splotu z A
    // (bufor uzupe³niany WYLICZON¥ wartoœci¹ w kroku poprzednim)

    // 5. Obliczenie splotu z B (sterowanie)
    double sumB = 0.0;
    for (size_t i = 0; i < m_B.size(); ++i)
        sumB += m_B[i] * m_bufU[m_B.size() - 1 - i];

    // 6. Obliczenie splotu z A (regulowana)
    double sumA = 0.0;
    for (size_t i = 0; i < m_A.size(); ++i)
        sumA += m_A[i] * m_bufY[m_A.size() - 1 - i];

    // 7. Zak³ócenie (jeœli aktywne)
    double noise = (m_noiseStddev > 0.0) ? m_noiseDist(m_rng) : 0.0;

    // 8. Wynik
    double y = sumB - sumA + noise;

    // 9. OGRANICZENIE regulowanej wartoœci
    if (m_ogrRegul)
        y = saturate(y, m_minY, m_maxY);

    // 10. Aktualizacja bufora regulowanej
    m_bufY.push_back(y);
    m_bufY.pop_front();

    return y;
}

// --- Konfiguracja ograniczeñ ---
void Model_ARX::setOgrSter(double minU, double maxU, bool aktywne) {
    m_minU = minU; m_maxU = maxU; m_ogrSter = aktywne;
}
void Model_ARX::setOgrRegul(double minY, double maxY, bool aktywne) {
    m_minY = minY; m_maxY = maxY; m_ogrRegul = aktywne;
}
void Model_ARX::setOgrSterAktywne(bool aktywne) {
    m_ogrSter = aktywne;
}
void Model_ARX::setOgrRegulAktywne(bool aktywne) {
    m_ogrRegul = aktywne;
}

// --- Zmiana parametrów modelu ---
void Model_ARX::setA(const std::vector<double>& A) {
    m_A = A;
    m_bufY.assign(A.size(), 0.0);
}
void Model_ARX::setB(const std::vector<double>& B) {
    m_B = B;
    m_bufU.assign(B.size(), 0.0);
}
void Model_ARX::setDelay(int delay) {
    m_delay = std::max(1, delay);
    m_bufDelayU.assign(m_delay, 0.0);
}
void Model_ARX::setNoiseStddev(double stddev) {
    m_noiseStddev = stddev;
    double safeStddev = (stddev > 0.0) ? stddev : 1e-9;
    m_noiseDist = std::normal_distribution<double>(0.0, safeStddev);
}