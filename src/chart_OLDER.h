/**
  CHART VERSION 0.1
  @todo: Переделать рассчёт точек по отношению к оси Y
         строка 136
         1)Если записей более 10, то выводить текст на осях каждую вторую засечку
         (как по вертикали, так и по горизонтали)
         2) Сделать разбитие засечек не только на 10 целых единиц, а также на единицы,
            и десятые

  CurrencyConverter:
  1) добавил в начале к Х-координатам везде "xAxisLine.x1() +"

  */
#ifndef CHART_H
#define CHART_H

#include <QtWidgets>
#include <limits>
#include <utility>
#include <math.h> //ceil() -- округление вверх

class Drawer : public QWidget {
private:
    static const uint   m_MARGIN = 35;  //отступ слева и снизу от осей (пикс.)
    QString             m_strFilename;
    QVector<QPoint>     m_lDotsPrice;
    QVector<double>     m_lPrice;
    QVector<QString>    m_lTime;
    std::pair<double, double> m_pairMinMax;
    int                 m_nRecords;  //считает количество точек кривой

private:
    void paintEvent(QPaintEvent*) {
        QPainter painter;
        QBrush   brush(Qt::black, Qt::SolidPattern);
        QPen     axisPen(brush, 1.5);
        QLine    yAxisLine(m_MARGIN, m_MARGIN, m_MARGIN, height() - m_MARGIN);
        QLine    xAxisLine(m_MARGIN, m_MARGIN, width() - m_MARGIN, m_MARGIN);
        //Для ширины строки
        QString  strYAxis = "Price";
        QFont    font(this->font());
        QFontMetrics metrics(font);

        painter.begin(this);
            m_lDotsPrice.clear(); //clear list

            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(axisPen);
            //draw y axis: ok
            painter.translate(0, height());
            painter.scale(1, -1);

            painter.drawLine(yAxisLine);
            painter.drawLine(xAxisLine);
            //y axis arrow ok
            painter.drawLine(yAxisLine.x2(), yAxisLine.y2(), yAxisLine.x2() + 3, yAxisLine.y2() - 10);
            painter.drawLine(yAxisLine.x2(), yAxisLine.y2(), yAxisLine.x2() - 3, yAxisLine.y2() - 10);
            //x axis arrow ok
            painter.drawLine(xAxisLine.x2(), xAxisLine.y2(), xAxisLine.x2() - 10, xAxisLine.y2() + 3);
            painter.drawLine(xAxisLine.x2(), xAxisLine.y2(), xAxisLine.x2() - 10, xAxisLine.y2() - 3);
            //y axis name: ok
            painter.save();
            painter.scale(-1, 1);
            painter.rotate(-180);       //для переворачивания Y надписи
            painter.drawText(yAxisLine.x2() - 10,      //x coord
                             -(yAxisLine.y2() + 15),         //y coord
                             metrics.width(strYAxis),   //text bounding rect width
                             metrics.height(),          //text bounding rect height
                             Qt::AlignCenter,           //flags
                             strYAxis                   //string
                             );

            //x axis name ok:
            strYAxis = "Time";
            painter.drawText(xAxisLine.x2() - 10,      //x coord
                             -(xAxisLine.y2() + metrics.height() + 10),         //y coord
                             metrics.width(strYAxis),   //text bounding rect width
                             metrics.height(),          //text bounding rect height
                             Qt::AlignCenter,           //flags
                             strYAxis                   //string
                             );
            painter.restore();

            //y axis подпись//////////////////////////////////////////////////////////////:
            double nNotchCount = ceil((m_pairMinMax.second - m_pairMinMax.first) / 10);

            //количество пикселей на 1 засечку
            int yAxisNotchStepPixels =
                    (yAxisLine.y2() - yAxisLine.y1())  / (nNotchCount + 2); //magic number

             //для текста (нижнее значение)
            double nNotch = floor(m_pairMinMax.first);

            for(int i = 1; i < nNotchCount + 1; ++i) {  //magic number
                //количество засечек:
                //засечки у-axis
                painter.save();
                axisPen.setWidth(1.5);
                painter.setPen(axisPen);
                painter.drawLine(yAxisLine.x1() - 5,
                                 yAxisLine.y1() + yAxisNotchStepPixels * i,
                                 yAxisLine.x1() + 5,
                                 yAxisLine.y1() + yAxisNotchStepPixels * i
                                 );
                //(yAxisValueOffset + yAxisLine.y2()) / nNotchCount * (i + 1)
                painter.restore();
                //текст y-axis
                painter.save();
                painter.scale(-1, 1);
                painter.rotate(-180);       //для переворачивания Y надписи
                painter.drawText(yAxisLine.x1() - metrics.width(QString::number(nNotch)) * 1.5,      //x coord
                                 -(yAxisLine.y1() + yAxisNotchStepPixels * i
                                   + 0.5 * metrics.height()),         //y coord
                                 metrics.width(QString::number(nNotch)),
                                 metrics.height(),
                                 Qt::AlignCenter,
                                 QString::number(nNotch) //КОСТЫЛЬ
                                 );
                painter.restore();
                nNotch += 10;
            }
            ////////////////////////////////////////////////////////////////////////////////////


            //сделать насечки на осях, подписать их
            //(количество насечек == m_nRecords)
            int xAxisValueOffset = (xAxisLine.x2() - xAxisLine.x1()) / (m_lPrice.size()); //3 - magic number
            double yAxisValueOffset = yAxisNotchStepPixels;
            double onePointPixel = yAxisValueOffset / 10;
            axisPen.setWidth(5);
            painter.setPen(axisPen);

            for(int i = 0; i < m_lPrice.size(); ++i) {

                m_lDotsPrice.push_back(QPoint(xAxisLine.x1() + xAxisValueOffset * (i + 1),
                                       yAxisLine.y1() + (yAxisValueOffset) + (m_lPrice.at(i) - m_pairMinMax.first) * onePointPixel)
                                       );
                painter.drawPoint(xAxisLine.x1() + xAxisValueOffset * (i + 1),
                        yAxisLine.y1() +   ( yAxisValueOffset) + (m_lPrice.at(i) - m_pairMinMax.first) * onePointPixel
                                  );  //ТУТ (вроде заработало)
                //(yAxisLine.y1() + yAxisValueOffset) + (m_lPrice.at(i) - m_pairMinMax.first) * nNotchCount//похоже на правду

                //test line draw
                painter.save();
                axisPen.setWidth(1.5);
                painter.setPen(axisPen);
                //насечки ось Х ok
                painter.drawLine(xAxisLine.x1() + xAxisValueOffset * (i + 1), xAxisLine.y1() - 5,
                                 xAxisLine.x1() + xAxisValueOffset * (i + 1), xAxisLine.y1() + 5);
                painter.restore();

                //x axis подпись ok:
                painter.save();
                painter.scale(-1, 1);
                painter.rotate(-180);       //для переворачивания надписи
                painter.drawText(xAxisLine.x1() + (xAxisValueOffset * (i + 1)) - 0.5 * metrics.width(m_lTime.at(i)),      //x coord
                                 -(xAxisLine.y2() - metrics.height() + 5),         //y coord
                                 metrics.width(m_lTime.at(i)),   //text bounding rect width
                                 metrics.height(),          //text bounding rect height
                                 Qt::AlignCenter,           //flags
                                 m_lTime.at(i)                   //string
                                 );

                painter.restore();
                setMinimumWidth(xAxisValueOffset * (i));

            }

        axisPen.setWidth(1.5);
        painter.setPen(axisPen);
        for(int i = 0; i < m_lDotsPrice.size() - 1; ++i) {
            painter.drawLine(m_lDotsPrice.at(i),
                             m_lDotsPrice.at(i + 1)
                             );
        }

        painter.end();
    }

public:
    Drawer(QString strFilename = "chart.dat"
           , QWidget* pwgt = 0)
        : QWidget(pwgt)
    {
        setMinimumSize(175, 175);
        m_strFilename = strFilename;

        //charts load from file:
        fillChart(m_strFilename);
        m_pairMinMax = findMinMax(m_lPrice);
    }

    void fillChart(QString filename) {
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Error opening the file " << file.fileName();
        }
        QTextStream stream(&file);

        m_nRecords = 0;

        while(!stream.atEnd()) {
            ++m_nRecords;

            m_lTime.push_back(stream.readLine());
            m_lPrice.push_back(stream.readLine().toDouble());

        }

    }

    std::pair<double, double> findMinMax(const QVector<double>& l) {
        std::pair<double, double> result;
        result.first = findMin(l);

        double max = result.first;
        for(auto iter = l.begin(); iter != l.end(); ++iter) {
            if(*iter > max) {
                max = *iter;
            }
        }
        result.second = max;

        return result;
    }

    double findMin(const QVector<double>& l) {
        double min = std::numeric_limits<double>::max();
        for(auto iter = l.begin(); iter != l.end(); ++iter) {
            if(*iter < min) {
                min = *iter;
            }
        }
        return min;
    }

};

#endif // CHART_H
