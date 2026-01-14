#include "arxdialog.h"
#include "ui_arxdialog.h"

#include <QMessageBox>

ArxDialog::ArxDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ArxDialog)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());

    setWindowTitle("ARX");

    // Styl zblizony do mockupu (ciemne tlo, jasne pola)
    // setStyleSheet(
    //     "QDialog { background-color: #202020; color: #f0f0f0; }"
    //     "QGroupBox { border: 1px solid #666666; border-radius: 6px; "
    //     "margin-top: 18px; font-weight: bold; }"
    //     "QGroupBox::title { subcontrol-origin: margin; left: 10px; "
    //     "padding: 0 3px; }"
    //     "QLabel { color: #f0f0f0; }"
    //     "QLineEdit, QDoubleSpinBox, QSpinBox { "
    //     "background-color: #404040; color: #ffffff; "
    //     "border: 1px solid #606060; border-radius: 4px; "
    //     "padding: 2px 4px; }"
    //     "QCheckBox { color: #f0f0f0; }"
    //     "QPushButton { "
    //     "background-color: #505050; color: #ffffff; "
    //     "border-radius: 6px; padding: 4px 16px; }"
    //     "QPushButton:hover { background-color: #707070; }"
    //     "QPushButton:pressed { background-color: #404040; }"
    //     );

    int w = qMax(ui->pushButton->width(), ui->pushButton_2->width());
    ui->pushButton->setFixedWidth(w);
    ui->pushButton_2->setFixedWidth(w);

    int h = ui->pushButton->height();
    ui->pushButton->setFixedHeight(h);
    ui->pushButton_2->setFixedHeight(h);

    // Domyslne wartosci
    ui->lineEditA->setPlaceholderText("-0.4, -0.4, 0.6");
    ui->lineEditB->setPlaceholderText("-0.4, -0.4, 0.6");

    ui->spinBoxOpoznienie->setRange(0, 100);
    ui->spinBoxOpoznienie->setValue(2);

    ui->doubleSpinBoxSzum->setDecimals(2);
    ui->doubleSpinBoxSzum->setSingleStep(0.01);
    ui->doubleSpinBoxSzum->setRange(0.0, 1.0);
    ui->doubleSpinBoxSzum->setValue(0.01);

    ui->doubleSpinBoxMin->setRange(-1000.0, 0.0);
    ui->doubleSpinBoxMin->setValue(-10.0);

    ui->doubleSpinBoxMax->setRange(0.0, 1000.0);
    ui->doubleSpinBoxMax->setValue(10.0);

    ui->checkBoxOgraniczenie->setChecked(true);

    // Przypisz przyciski do slotow
    connect(ui->pushButton,   &QPushButton::clicked,
            this, &ArxDialog::on_okButton_clicked);
    connect(ui->pushButton_2, &QPushButton::clicked,
            this, &ArxDialog::on_cancelButton_clicked);
}

ArxDialog::~ArxDialog()
{
    delete ui;
}

// Gettery

QString ArxDialog::coeffA() const
{
    return ui->lineEditA->text();
}

QString ArxDialog::coeffB() const
{
    return ui->lineEditB->text();
}

int ArxDialog::delay() const
{
    return ui->spinBoxOpoznienie->value();
}

double ArxDialog::noise() const
{
    return ui->doubleSpinBoxSzum->value();
}

double ArxDialog::minVal() const
{
    return ui->doubleSpinBoxMin->value();
}

double ArxDialog::maxVal() const
{
    return ui->doubleSpinBoxMax->value();
}

bool ArxDialog::useLimits() const
{
    return ui->checkBoxOgraniczenie->isChecked();
}

// Proste parsowanie tekstu wspolczynnikow "a1, a2, a3"

QVector<double> ArxDialog::parseCoeffs(const QString &text, bool *ok) const
{
    QVector<double> coeffs;
    *ok = true;

    const auto parts = text.split(',', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        *ok = false;
        return coeffs;
    }

    for (const QString &part : parts) {
        bool okNum = false;
        double val = part.trimmed().replace(',', '.').toDouble(&okNum);
        if (!okNum) {
            *ok = false;
            coeffs.clear();
            return coeffs;
        }
        coeffs.append(val);
    }
    return coeffs;
}

// OK / Anuluj

void ArxDialog::on_okButton_clicked()
{
    bool okA, okB;
    QVector<double> A = parseCoeffs(ui->lineEditA->text(), &okA);
    QVector<double> B = parseCoeffs(ui->lineEditB->text(), &okB);

    if (!okA || !okB) {
        QMessageBox::warning(this, tr("Blad"),
                             tr("Niepoprawny format wspolczynnikow A lub B.\n"
                                "Uzyj formatu: -0.4, -0.4, 0.6"));
        return;
    }

    // jesli chcesz, mozesz tu od razu wyslac A, B do modelu ARX

    accept();   // zamyka dialog (QDialog::Accepted)
}

void ArxDialog::pokazBlad(const QString &tekst)
{
    QMessageBox::warning(this, tr("Blad"), tekst);
}


void ArxDialog::on_cancelButton_clicked()
{
    reject();   // zamyka dialog (QDialog::Rejected)
}

static QString wektorNaTekst(const QVector<double> &v)
{
    QStringList parts;
    for (double x : v)
        parts << QString::number(x, 'g', 6);
    return parts.join(", ");
}

void ArxDialog::ustawZKonfiguracji(const QString &tekstA,
                                   const QString &tekstB,
                                   int opoznienie,
                                   double szum,
                                   double minVal,
                                   double maxVal,
                                   bool uzywajOgraniczen)
{
    ui->lineEditA->setText(tekstA);
    ui->lineEditB->setText(tekstB);
    ui->spinBoxOpoznienie->setValue(opoznienie);
    ui->doubleSpinBoxSzum->setValue(szum);
    ui->doubleSpinBoxMin->setValue(minVal);
    ui->doubleSpinBoxMax->setValue(maxVal);
    ui->checkBoxOgraniczenie->setChecked(uzywajOgraniczen);
}

void ArxDialog::setTekstA(const QString& t) { ui->lineEditA->setText(t); }
void ArxDialog::setTekstB(const QString& t) { ui->lineEditB->setText(t); }
void ArxDialog::setOpoznienie(int k) { ui->spinBoxOpoznienie->setValue(k); }
void ArxDialog::setSzum(double s) { ui->doubleSpinBoxSzum->setValue(s); }
void ArxDialog::setMinMax(double min, double max) {
    ui->doubleSpinBoxMin->setValue(min);
    ui->doubleSpinBoxMax->setValue(max);
}
void ArxDialog::setOgraniczeniaAktywne(bool on) {
    ui->checkBoxOgraniczenie->setChecked(on);
}
