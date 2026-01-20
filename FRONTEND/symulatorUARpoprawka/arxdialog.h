#ifndef ARXDIALOG_H
#define ARXDIALOG_H

#include "ui_arxdialog.h"
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class ArxDialog; }
QT_END_NAMESPACE

class ArxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ArxDialog(QWidget *parent = nullptr);
    ~ArxDialog();

    // proste gettery na dane z okienka
    QString coeffA() const;
    QString coeffB() const;

    int delay() const;
    double noise() const;
    double minU() const;
    double maxU() const;
    double minY() const;
    double maxY() const;
    bool useLimits() const;
    void pokazBlad(const QString &tekst);

    void ustawZKonfiguracji(const QString &tekstA,
                            const QString &tekstB,
                            int opoznienie,
                            double szum,
                            double minU,
                            double maxU,
                            double minY,
                            double maxY,
                            bool uzywajOgraniczen);


    void setTekstA(const QString& t);
    void setTekstB(const QString& t);
    void setOpoznienie(int k);
    void setSzum(double s);
    void setMinMax(double minU, double maxU, double minY, double maxY);
    void setOgraniczeniaAktywne(bool on);


private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();

    void on_checkBoxOgraniczenie_toggled(bool checked);

private:
    Ui::ArxDialog *ui;

    QVector<double> parseCoeffs(const QString &text, bool *ok) const;
};

#endif // ARXDIALOG_H
