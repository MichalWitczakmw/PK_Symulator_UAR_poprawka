#pragma once

#include "Model_ARX.h"
#include "Regulator_PID.h"
#include "Generator.h"

class SymulacjaUAR
{
public:
    SymulacjaUAR(
        Model_ARX model,
        Regulator_PID regulator,
        Generator generator)
        : m_model(std::move(model)),
        m_regulator(std::move(regulator)),
        m_generator(std::move(generator)),
        m_uchyb(0.0),
        m_sterowanie(0.0),
        m_wartoscZadana(0.0),
        m_wartoscRegulowana(0.0)
    {}
    SymulacjaUAR(Model_ARX model, Regulator_PID regulator)
        : m_model(std::move(model)),
        m_regulator(std::move(regulator)),
        m_generator(Generator()), // domyœlny generator: typ Prostok¹t, A=1, okres=100, wype³nienie=0.5, S=0
        m_uchyb(0.0),
        m_sterowanie(0.0),
        m_wartoscZadana(0.0),
        m_wartoscRegulowana(0.0)
    {}
    //dla testu
    double symuluj(double pobudzenie);

    // Mo¿e pozostaæ: wykonajKrok — obs³uguje sprzê¿enie z generatorem wartoœci zadanej
    void wykonajKrok();

    // Resetowanie
    void reset();

    // Dostêp do komponentów (do konfiguracji)
    Model_ARX& model() { return m_model; }
    Regulator_PID& regulator() { return m_regulator; }
    Generator& generator() { return m_generator; }

    // Dostêp do wartoœci (opcjonalnie)
    double wartoscZadana() const { return m_wartoscZadana; }
    double wartoscRegulowana() const { return m_wartoscRegulowana; }
    double sterowanie() const { return m_sterowanie; }
    double uchyb() const { return m_uchyb; }

private:
    Model_ARX m_model;
    Regulator_PID m_regulator;
    Generator m_generator;

    double m_uchyb;
    double m_sterowanie;
    double m_wartoscZadana;
    double m_wartoscRegulowana;

    double m_poprzedniaWartoscRegulowana = 0.0;
};
