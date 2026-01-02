#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),m_symulator(SymulatorUAR(this))
{
    ui->setupUi(this);

    // Utworzenie symulatora


    // Połączenie przycisków
    connect(ui->StartPB, &QPushButton::clicked, this, &MainWindow::on_StartPB_clicked);
    connect(ui->StopPB, &QPushButton::clicked, this, &MainWindow::on_StopPB_clicked);
    connect(ui->ResetPB, &QPushButton::clicked, this, &MainWindow::on_ResetPB_clicked);

    // Połączenie sygnału symulatora z aktualizacją tekstu
    connect(&m_symulator, &SymulatorUAR::stanSymulacjiZmieniony,
            this, &MainWindow::aktualizujWyswietlacz);

    // Domyślne ustawienia przycisków
    ui->StopPB->setEnabled(false);
    ui->ResetPB->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_StartPB_clicked()
{
    // Uruchom symulację z domyślnym interwałem 200ms
    m_symulator.uruchom(200);

    // Zmień stan przycisków
    ui->StartPB->setEnabled(false);
    ui->StopPB->setEnabled(true);
    ui->ResetPB->setEnabled(true);

    ui->obliczonyKrok->clear();
    qDebug() << "START kliknięty";
}

void MainWindow::on_StopPB_clicked()
{
    m_symulator.zatrzymaj();

    // Zmień stan przycisków
    ui->StartPB->setEnabled(true);
    ui->StopPB->setEnabled(false);
    ui->ResetPB->setEnabled(true);

    qDebug() << "STOP kliknięty";
}

void MainWindow::on_ResetPB_clicked()
{
    m_symulator.resetuj();

    // Zmień stan przycisków (reset nie zatrzymuje)
    ui->StartPB->setEnabled(!m_symulator.isRunning());
    ui->StopPB->setEnabled(m_symulator.isRunning());
    ui->ResetPB->setEnabled(true);

    ui->obliczonyKrok->clear();
    qDebug() << "RESET kliknięty";
}

void MainWindow::aktualizujWyswietlacz()
{
    // Pobierz aktualne wartości symulacji
    double w = m_symulator.getWartoscZadana();
    double y = m_symulator.getWartoscRegulowana();
    double e = m_symulator.getUchyb();
    double u = m_symulator.getSterowanie();
    double czas = m_symulator.getCzasSymulacji();

    // Format tekstu: krok | w | y | e | u | czas
    QString tekst = QString("Krok: %1 | w: %2 | y: %3 | e: %4 | u: %5 | t: %6s")
                        .arg((int)m_symulator.getHistoriaCzas().size())
                        .arg(w, 0, 'f', 3)
                        .arg(y, 0, 'f', 3)
                        .arg(e, 0, 'f', 3)
                        .arg(u, 0, 'f', 3)
                        .arg(czas, 0, 'f', 2);

    // Dodaj nową linię (bez czyszczenia poprzednich)
    QString obecnyTekst = ui->obliczonyKrok->toPlainText();
    ui->obliczonyKrok->setPlainText(obecnyTekst + "\n" + tekst);

    // Przewiń do dołu (ostatnia linia widoczna)
    QTextCursor cursor = ui->obliczonyKrok->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->obliczonyKrok->setTextCursor(cursor);
}
