#include "Model_ARX.h"

Model_ARX::Model_ARX(const std::vector<double>& wspolczynnikA, const std::vector<double>& wspolczynnikB, int opoznienieTransportowe, double zaklocenie)
    : m_wspolczynnikA(wspolczynnikA), m_wspolczynnikB(wspolczynnikB), m_opoznienieTransportowe(std::max(1, opoznienieTransportowe)), m_oSSzum(zaklocenie),
    m_GeneratorZaklocen(std::random_device{}()), m_rozkladZaklocen(0.0, (zaklocenie > 0.0) ? zaklocenie : 1e-9)
{
    m_buforU.assign(wspolczynnikB.size(), 0.0);
    m_buforOpoznienia.assign(m_opoznienieTransportowe, 0.0);
    m_buforY.assign(wspolczynnikA.size(), 0.0);
}

double Model_ARX::symuluj(double sygnalSterujacy) 
{
    // Ograniczenie sterowania (jesli aktywne)
    if (m_ogrSterowania)
        sygnalSterujacy = nasycenie(sygnalSterujacy, m_minU, m_maxU);

    // Aktualizacja bufor opoznienia transportowego
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

    // Zaklocenie (jesli aktywne)
    double szum = (m_oSSzum > 0.0) ? m_rozkladZaklocen(m_GeneratorZaklocen) : 0.0;

    // Obliczenie wyniku
    double y = sumaB - sumaA + szum;

    // Ograniczenie regulowanej wartosci (jesli aktywne)
    if (m_ogrRegulowania)
        y = nasycenie(y, m_minY, m_maxY);

    // Aktualizacja bufora regulowanej
    m_buforY.push_back(y);
    m_buforY.pop_front();

    return y;
}

void Model_ARX::resetPamieci()
{
    std::fill(m_buforU.begin(),          m_buforU.end(),          0.0);
    std::fill(m_buforY.begin(),          m_buforY.end(),          0.0);
    std::fill(m_buforOpoznienia.begin(), m_buforOpoznienia.end(), 0.0);
}

// Konfiguracja ograniczen
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

// Zmiana parametrow modelu
void Model_ARX::setA(const std::vector<double>& A) 
{
    m_wspolczynnikA = A;
    //m_buforY.assign(A.size(), 0.0);
}
void Model_ARX::setB(const std::vector<double>& B) 
{
    m_wspolczynnikB = B;
    //m_buforU.assign(B.size(), 0.0);
}
void Model_ARX::setopoznienieTransport(int opoznienieTransportowe)
{
    /*
    int nowyK = std::max(1, opoznienieTransportowe);
    if (nowyK == m_opoznienieTransportowe)
        return; // nic się nie zmieniło

    // dostosuj rozmiar bufora, zachowując ile się da z historii
    if (nowyK > m_opoznienieTransportowe) {
        // większe opóźnienie: dokładamy z przodu zera
        m_buforOpoznienia.insert(
            m_buforOpoznienia.end(),
            nowyK - m_opoznienieTransportowe,
            0.0
            );
    } else {
        // mniejsze opóźnienie: ucinamy najstarsze elementy
        int roznica = m_opoznienieTransportowe - nowyK;
        if (roznica >= (int)m_buforOpoznienia.size())
            m_buforOpoznienia.assign(nowyK, m_buforOpoznienia.front());
        else
            m_buforOpoznienia.erase(m_buforOpoznienia.end() - roznica, m_buforOpoznienia.end());
    }
    */
    //m_opoznienieTransportowe = opoznienieTransportowe;
    //m_buforOpoznienia.push_back(opoznienieTransportowe);
    //m_buforOpoznienia.pop_back();
    int nowyK = std::max(1, opoznienieTransportowe);
    if (nowyK == m_opoznienieTransportowe)
        return;

    m_opoznienieTransportowe = nowyK;
    // NAJPROSTSZA poprawna wersja: po prostu ustaw nowy rozmiar bufora,
    // nawet kosztem utraty historii opóźnienia (ale bez psucia U/Y):
    m_buforOpoznienia.assign(m_opoznienieTransportowe, 0.0);
}

void Model_ARX::setOdchylenieZaklocen(double odchylenieZaklocenia) 
{
    m_oSSzum = odchylenieZaklocenia;
    double bezpieczneOdchylenie = (odchylenieZaklocenia > 0.0) ? odchylenieZaklocenia : 1e-9;
    m_rozkladZaklocen = std::normal_distribution<double>(0.0, bezpieczneOdchylenie);
}
