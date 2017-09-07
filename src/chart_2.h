/**
  CHART VERSION 0.2
  @todo:
        1)  Сделать лучше ранжирование при слишком больших разницах мин-макс по оси Y
            (для примера загрузить файл chart_hundreds.txt)
        2)  Сделать правильные округление засечек оси Y без появления погрешностей в
            графиках

  CHANGELOG:
  03.13.2016:
  1) Добавил переменную double denominator, она отвечает за приращивание насечек оси Y,
     берётся разница МАХ - МИН значений выборки, если разница больше 100, то на оси отметки
     рисуются с приращиванием 10 единиц, если меньше 10, то приращивание равняется 1,
     если меньше 1, то приращивание = 0.2
  2) Теперь, если элементов в векторе больше 10, то подписуется каждая 2я Х-насечка,
     если больше 30 элементов - то каждая третья Х-насечка
  3) Подпись оси Х вынес наверх, чтобы при большом количестве насечек текст не находил
    один на другой
  4) Убрал переменную m_chartRecords
  5) Убрал запись первой строки в файла в m_strChartName,
     теперь там хранятся только время и значение точки
  6) Исходя из пункта 5, переменная m_strChartName стала не нужна, она была убрана
  7) Добавил в Х-координатах и Х-насечках значение (-20), чтобы последнее значение и
     насечка не сливалась с координатной стрелкой
  8) m_MARGIN увеличил до 55, чтобы помещались подписи Y-координаты с сотыми долями
  */
#ifndef CHART_H
#define CHART_H

#include <QtWidgets>
#include <limits>
#include <utility>
#include <math.h> //ceil() -- округление вверх

class Drawer : public QWidget {
private:
    static const uint         m_MARGIN = 55;  //отступ слева и снизу от осей (пикс.)
    QString                   m_strFilename;
    QVector<QPoint>           m_lDotsPrice;
    QVector<double>           m_lPrice;
    QVector<QString>          m_lTime;
    std::pair<double, double> m_pairMinMax;

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
                             -(xAxisLine.y2() + metrics.height() + 5),         //y coord
                             metrics.width(strYAxis),   //text bounding rect width
                             metrics.height(),          //text bounding rect height
                             Qt::AlignCenter,           //flags
                             strYAxis                   //string
                             );
            painter.restore();

            //y axis подпись//////////////////////////////////////////////////////////////:
            double minMaxDelta =  m_pairMinMax.second - m_pairMinMax.first;
            double nNotchCount
                    = //ceil((m_pairMinMax.second - m_pairMinMax.first) / 10);
                      // m_pairMinMax.second - m_pairMinMax.first;
                      1;

            if(minMaxDelta < 1) {

                nNotchCount = 9;
            } else if(minMaxDelta < 10) {

                nNotchCount = 5;
            } else if(minMaxDelta > 100) {

                nNotchCount = ceil(minMaxDelta / 10);
            } else {
                nNotchCount = minMaxDelta;
            }
            //чилсо определяет += к nNotch;
            double denominator = 10;

            if(nNotchCount > 100) {
                denominator = 10;
            } else if(nNotchCount < 10) {
                denominator = 0.5;
            } else {
                denominator = 1;
            }

            //test:
            if(minMaxDelta > 70) {
                nNotchCount = minMaxDelta / 10;
                denominator = 10;
            }


            qDebug() << "nNotchCount: " << QString::number(nNotchCount);

            //количество пикселей на 1 засечку
            int yAxisNotchStepPixels =
                    (yAxisLine.y2() - yAxisLine.y1())  / (nNotchCount + 2); //magic number

             //для текста (нижнее значение)
            double nNotch = m_pairMinMax.second - m_pairMinMax.first < 1
                            ? m_pairMinMax.first
                            : ceil(m_pairMinMax.first);

            for(int i = 1, z = 0; i < nNotchCount + 1; ++i, ++z) {  //magic number
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
                nNotch += denominator;
            }
            ////////////////////////////////////////////////////////////////////////////////////
            //сделать насечки на осях, подписать их
            //(количество насечек == m_nRecords)
            int xAxisValueOffset = (xAxisLine.x2() - xAxisLine.x1()) / (m_lPrice.size()); //3 - magic number
            double yAxisValueOffset = yAxisNotchStepPixels;
            double onePointPixel = yAxisValueOffset / denominator;
            axisPen.setWidth(5);
            painter.setPen(axisPen);

            for(int i = 0, z = 0; i < m_lPrice.size(); ++i, ++z) {

                m_lDotsPrice.push_back(QPoint(xAxisLine.x1() + xAxisValueOffset * (i + 1) - 20,
                                       yAxisLine.y1() +        (yAxisValueOffset) + (m_lPrice.at(i) - m_pairMinMax.first) * onePointPixel)
                                       );
                painter.drawPoint(xAxisLine.x1() + xAxisValueOffset * (i + 1)  - 20,
                        yAxisLine.y1() +   ( yAxisValueOffset) + (m_lPrice.at(i) - m_pairMinMax.first) * onePointPixel
                                  );  //ТУТ (вроде заработало)
                //(yAxisLine.y1() + yAxisValueOffset) + (m_lPrice.at(i) - m_pairMinMax.first) * nNotchCount//похоже на правду

                //test line draw
                painter.save();
                axisPen.setWidth(1.5);
                painter.setPen(axisPen);
                //насечки ось Х ok
                painter.drawLine(xAxisLine.x1() + xAxisValueOffset * (i + 1)  - 20, xAxisLine.y1() - 5,
                                xAxisLine.x1() + xAxisValueOffset * (i + 1)  - 20, xAxisLine.y1() + 5);
                painter.restore();

                //Если количество записей большое - подписывать каждую 2 ячейку:
                if(m_lPrice.size() > 30 && z == 3)
                {
                    z = -1;
                    setMinimumWidth(xAxisValueOffset * (i));
                    continue;
                }
                else if(m_lPrice.size() > 10 && z == 1)
                {
                    //painter.restore();
                    z = -1;
                    setMinimumWidth(xAxisValueOffset * (i));
                    continue;
                }
                //x axis подпись ok:
                painter.save();
                painter.scale(-1, 1);
                painter.rotate(-180);       //для переворачивания надписи
                painter.drawText(xAxisLine.x1() + (xAxisValueOffset * (i + 1)) - 0.5 * metrics.width(m_lTime.at(i))  - 20,      //x coord
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

                                        /* DROPDOWN EVENTS SECTION:  */

    virtual void dragEnterEvent(QDragEnterEvent* pe) {
        if(pe->mimeData()->hasUrls()) {
            pe->acceptProposedAction();
        }
    }

    virtual void dropEvent(QDropEvent* pe) {
        const QMimeData* pMimeData = pe->mimeData();
        qDebug() << "dropEvent started";
        if(pMimeData->hasText() && pMimeData->urls().size() == 1) {
            qDebug() << "Dropping file " << pMimeData->urls().at(0).toLocalFile();
            fillChart(pMimeData->urls().at(0).toLocalFile());
            setWindowTitle(QString("DROPDOWN: " + pMimeData->urls().at(0).toLocalFile()
                                   .mid(0, pMimeData->urls().at(0).toLocalFile().lastIndexOf('.'))
                                   )
                           );
            m_pairMinMax = findMinMax(m_lPrice);
            repaint();
        }
    }

    virtual void dragMoveEvent(QDragMoveEvent* pe) {

            if(pe->mimeData()->hasUrls()) {
                pe->accept();
            }
    }

public:
    Drawer(QString strFilename = "chart.txt"
           , bool isDropDownAllowed = false
           , QWidget* pwgt = 0)
           : QWidget(pwgt)
    {
        setAcceptDrops(isDropDownAllowed);

        setMinimumSize(175, 175);
        m_strFilename = strFilename;

        //charts load from file:
        fillChart(m_strFilename);
        m_pairMinMax = findMinMax(m_lPrice);
         qDebug() << "min m_lPrice: " << m_pairMinMax.first << "\nmax m_lPrice" << m_pairMinMax.second;

    }

    void fillChart(QString filename) {
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Error opening the file " << file.fileName();
        }
        QTextStream stream(&file);

        //если векторы не пусты - очистить их (для dropdown-фичи)
        if(!m_lPrice.isEmpty() || !m_lDotsPrice.isEmpty() || !m_lTime.isEmpty()) {
            m_lPrice.clear();
            m_lDotsPrice.clear();
            m_lTime.clear();
        }

        while(!stream.atEnd()) {
            m_lTime.push_back(stream.readLine());
            m_lPrice.push_back(stream.readLine().toDouble());
            qDebug() << "m_lTime pushed: " << m_lTime.last();
            qDebug() << "m_lPrice pushed: " << QString::number(m_lPrice.last());
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
