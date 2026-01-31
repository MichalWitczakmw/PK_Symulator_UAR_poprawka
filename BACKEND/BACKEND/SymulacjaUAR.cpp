#include "SymulacjaUAR.h"

double SymulacjaUAR::symuluj(double pobudzenie)
{
	m_uchyb = pobudzenie - m_poprzedniaWartoscRegulowana;
	m_sterowanie = m_regulator.symuluj(m_uchyb);
	m_wartoscRegulowana = m_model.symuluj(m_sterowanie);
	m_poprzedniaWartoscRegulowana = m_wartoscRegulowana;

	return m_wartoscRegulowana;
}

void SymulacjaUAR::wykonajKrok()
{
	m_wartoscZadana = m_generator.generujWartosc();
	m_uchyb = m_wartoscZadana - m_poprzedniaWartoscRegulowana;
	m_sterowanie = m_regulator.symuluj(m_uchyb);
	m_wartoscRegulowana = m_model.symuluj(m_sterowanie);
	m_poprzedniaWartoscRegulowana = m_wartoscRegulowana;
}

void SymulacjaUAR::reset()
{
	m_regulator.resetPamieci();
	m_generator.reset();
    m_model.resetPamieci();
	m_uchyb = 0.0;
	m_sterowanie = 0.0;
	m_wartoscZadana = 0.0;
	m_wartoscRegulowana = 0.0;
	m_poprzedniaWartoscRegulowana = 0.0;
}
