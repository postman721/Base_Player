#include "spectrumwidget.h"

#include <QPainter>
#include <QLinearGradient>
#include <algorithm>

SpectrumWidget::SpectrumWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(160);
}

void SpectrumWidget::setBars(const QVector<float> &b)
{
    bars = b;
    update();
}

void SpectrumWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    // glossy dark panel background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(26, 26, 32));
    bg.setColorAt(0.45, QColor(14, 14, 18));
    bg.setColorAt(1.0, QColor(8, 8, 10));
    p.fillRect(rect(), bg);

    // top gloss highlight
    p.setPen(Qt::NoPen);
    QLinearGradient gloss(0, 0, 0, height() * 0.45);
    gloss.setColorAt(0.0, QColor(255, 255, 255, 22));
    gloss.setColorAt(1.0, QColor(255, 255, 255, 0));
    p.setBrush(gloss);
    p.drawRect(QRect(0, 0, width(), int(height() * 0.45)));

    if (bars.isEmpty()) return;

    const int n = bars.size();
    const int w = width();
    const int h = height();

    const int gap = 3;
    const int barW = std::max(2, (w - (n - 1) * gap) / n);

    int x = 0;
    for (int i = 0; i < n; ++i) {
        const float v = std::clamp(bars[i], 0.0f, 1.0f);
        const int bh = int(v * float(h - 8));
        QRect r(x, h - bh - 4, barW, bh);

        QLinearGradient g(r.topLeft(), r.bottomLeft());
        g.setColorAt(0.0, QColor(140, 255, 210));
        g.setColorAt(0.35, QColor(55, 235, 170));
        g.setColorAt(1.0, QColor(15, 120, 90));
        p.fillRect(r, g);

        // glossy edge
        p.setPen(QColor(255, 255, 255, 70));
        p.drawLine(r.topLeft() + QPoint(0, 1), r.topRight() + QPoint(0, 1));

        x += barW + gap;
    }
}
