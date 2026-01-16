#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QWidget>
#include <QVector>

class SpectrumWidget : public QWidget
{
public:
    explicit SpectrumWidget(QWidget *parent = nullptr);

    void setBars(const QVector<float> &b);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<float> bars;
};

#endif // SPECTRUMWIDGET_H
