#ifndef ARXDIALOG_H
#define ARXDIALOG_H

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
    double minVal() const;
    double maxVal() const;
    bool useLimits() const;
    void pokazBlad(const QString &tekst);

    void ustawZKonfiguracji(const QString &tekstA,
                            const QString &tekstB,
                            int opoznienie,
                            double szum,
                            double minVal,
                            double maxVal,
                            bool uzywajOgraniczen);

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::ArxDialog *ui;

    QVector<double> parseCoeffs(const QString &text, bool *ok) const;
};

#endif // ARXDIALOG_H
