#include "Model_ARX.h"

Model_ARX::Model_ARX(const vector<double>& wspolczynnikA, const vector<double>& wspolczynnikB, int opoznienieTransportowe, double zaklocenie)
    : m_wspolczynnikA(wspolczynnikA), m_wspolczynnikB(wspolczynnikB), m_opoznienieTransportowe(max(1, opoznienieTransportowe)), m_oSSzum(zaklocenie),
    m_GeneratorZaklocen(random_device{}()), m_rozkladZaklocen(0.0, (zaklocenie > 0.0) ? zaklocenie : 1e-9)
{
    m_buforU.assign(wspolczynnikB.size(), 0.0);
    m_buforOpoznienia.assign(m_opoznienieTransportowe, 0.0);
    m_buforY.assign(wspolczynnikA.size(), 0.0);
}

double Model_ARX::symuluj(double sygnalSterujacy) 
{
	// Ograniczenie sterowania (jeœli aktywne)
    if (m_ogrSterowania)
        sygnalSterujacy = nasycenie(sygnalSterujacy, m_minU, m_maxU);

    // Aktualizacja bufor opóŸnienia transportowego
    m_buforOpoznienia.push_back(sygnalSterujacy);
    double tymczasoweOpoznione = m_buforOpoznienia.front();
    m_buforOpoznienia.pop_front();

    // Aktualizacja bufor sterowania do splotu z B
    m_buforU.push_back(tymczasoweOpoznione);
    m_buforU.pop_front();

    // Obliczenie B
    double sumaB = 0.0;
    for (size_t i = 0; i < m_wspolczynnikB.size(); ++i)
        sumaB += m_wspolczynnikB[i] * m_buforU[m_wspolczynnikB.size() - 1 - i];

    // Obliczenie A
    double sumaA = 0.0;
    for (size_t i = 0; i < m_wspolczynnikA.size(); ++i)
        sumaA += m_wspolczynnikA[i] * m_buforY[m_wspolczynnikA.size() - 1 - i];

    // Zak³ócenie (jeœli aktywne)
    double szum = (m_oSSzum > 0.0) ? m_rozkladZaklocen(m_GeneratorZaklocen) : 0.0;

    // Obliczenie wyniku
    double y = sumaB - sumaA + szum;

	// Ograniczenie regulowanej wartoœci (jeœli aktywne)
    if (m_ogrRegulowania)
        y = nasycenie(y, m_minY, m_maxY);

    // Aktualizacja bufora regulowanej
    m_buforY.push_back(y);
    m_buforY.pop_front();

    return y;
}

// Konfiguracja ograniczeñ 
void Model_ARX::setOgrSterowania(double minU, double maxU, bool aktywne) 
{
    m_minU = minU; m_maxU = maxU; m_ogrSterowania = aktywne;
}
void Model_ARX::setOgrRegulowania(double minY, double maxY, bool aktywne) 
{
    m_minY = minY; m_maxY = maxY; m_ogrRegulowania = aktywne;
}
void Model_ARX::setOgrSterowaniaAktywne(bool aktywne) 
{
    m_ogrSterowania = aktywne;
}
void Model_ARX::setOgrRegulowaniaAktywne(bool aktywne) 
{
    m_ogrRegulowania = aktywne;
}

// Zmiana parametrów modelu
void Model_ARX::setA(const std::vector<double>& A) 
{
    m_wspolczynnikA = A;
    m_buforY.assign(A.size(), 0.0);
}
void Model_ARX::setB(const std::vector<double>& B) 
{
    m_wspolczynnikB = B;
    m_buforU.assign(B.size(), 0.0);
}
void Model_ARX::setopoznienieTransport(int opoznienieTransportowe) 
{
    m_opoznienieTransportowe = std::max(1, opoznienieTransportowe);
    m_buforOpoznienia.assign(m_opoznienieTransportowe, 0.0);
}
void Model_ARX::setOdchylenieZaklocen(double odchylenieZaklocenia) 
{
    m_oSSzum = odchylenieZaklocenia;
    double bezpieczneOdchylenie = (odchylenieZaklocenia > 0.0) ? odchylenieZaklocenia : 1e-9;
    m_rozkladZaklocen = std::normal_distribution<double>(0.0, bezpieczneOdchylenie);
}