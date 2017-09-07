/********************************CHART VERSION 0.3******************************************
  @todo:
        1)  Сделать лучше ранжирование при слишком больших разницах мин-макс по оси Y
            (для примера загрузить файл chart_hundreds.txt) (СМ. Changelog 0.3 - (1) ) +++
        2)  Добавить динамический рассчёт засечек на Х-оси и координат Х точек вектора
            с данными валют
        3)  Сделать правильные округление засечек оси Y без появления погрешностей в
            графиках
        4)  Создать новый класс ChartPoint, который будет хранить координаты QPoint,
            а также значение QString strValue, переписать таким образом программу,
            чтобы при наведении или клике по точке на графике появлялся тултип с
            значением strValue данной точки (через QMouseEvent click)
        5)  Переделать формат данных в удобоваримый (JSON или XML),
            написать парсер / использовать Qt-парсер элементов для их
            вытаскивания из файла
        6)  Возможно, добавить полосу прокрутки, чтобы можно было
            удобно просматривать очень широкие графики
        7)  Сделать возможность построения месячного графика курсов валют:
            - открывается папка с файлами чартов,
            - оттуда считываются все названия файлов,
            - сортируются в порядке от 1 до 30 дня текущего месяца,
            - затем последовательно открываются в отсортированном порядке,
              с каждого файла берётся последнее значение (или среднее общее значение)
              и заносится как одна точка, дальше следущий файл и т.п.
            - значения точек заносятся в файл типа 'USD_month_year_summary.dat'
              вместо времени заносятся даты, формат типа:

              ~~USD_month_year_summary.dat~~
              01.12
              555.55
              02.12
              666.66

              ...

              31.12
              1244.123
              ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            - затем создаётся виджет чарта, основанный на сформированном файле.

  CHANGELOG 0.3.1:
  05.12.2016:
  1) Добавил слот slotLoadChart(), динамически обновляющий текущий чарт
     (объект CurrencyWidget при удачной записи данных в файлы чартов посы-
      лает сигнал signalRefreshCharts(), слоты и сигналы соединяются в
      tray.h

  CHANGELOG 0.3:
  04.14.2016:
  1) Сделал унифицированное количество верхних отметок, теперь их количетво
     всегда равно 10.
  2) Добавлена проверка, если дельта мин макс равна нулю, рисуется только 3 засечки
  3) Добавлена строка strYAxisName в класс и в стандартный конструктор,
     в ней устанавливается название Y-оси

  CHANGELOG 0.2:
  03.13.2016:
  1) Добавил переменную double denominator, она отвечает за приращивание насечек оси Y,
     берётся разница МАХ - МИН значений выборки, если разница больше 100, то на оси отметки
     рисуются с приращиванием 10 единиц, если меньше 10, то приращивание равняется 1,
     если меньше 1, то приращивание = 0.2
  2) Теперь, если элементов в векторе больше 10, то подписуется каждая 2я Х-насечка,
     если больше 30 элементов - то каждая третья Х-насечка
  3) Подпись оси Х вынес наверх, чтобы при большом количестве насечек текст не находил
    один на другой
  4) Убрал переменную m_chartRecords, замена - метод m_lPrice.size()
  5) Убрал запись первой строки в файла в m_strChartName,
     теперь там хранятся только время и значение точки
  6) Исходя из пункта 5, переменная m_strChartName стала не нужна, она была убрана
  7) Добавил в Х-координатах и Х-насечках значение (-20), чтобы последнее значение и
     насечка не сливалась с координатной стрелкой
  8) m_MARGIN увеличил до 55, чтобы помещались подписи Y-координаты с сотыми долями
*****************************************************************************************/
#ifndef CHART_H
#define CHART_H

#include <QtWidgets>
#include <limits>  //std::numeric_limits<int>()::max() / min()
#include <utility> //std::pair
#include <math.h>  //ceil() -- округление вверх

class Drawer : public QWidget {
    Q_OBJECT
private:
    static const uint         m_MARGIN = 55;  //отступ слева и снизу от осей (пикс.)
    QString                   m_strFilename;
    QString                   m_strYAxisName;
    QVector<QPoint>           m_lDotsPrice;
    QVector<double>           m_lPrice;
    QVector<QString>          m_lTime;
   // QGraphicsOpacityEffect*   m_pOpacity;
    std::pair<double, double> m_pairMinMax;

private:
    void paintEvent(QPaintEvent*) {
        QPainter painter;
        QBrush   brush(Qt::black, Qt::SolidPattern);
        QPen     axisPen(brush, 1.5);
        QLine    yAxisLine(m_MARGIN, m_MARGIN, m_MARGIN, height() - m_MARGIN);
        QLine    xAxisLine(m_MARGIN, m_MARGIN, width() - m_MARGIN, m_MARGIN);
        //Для ширины строки
        QString  strYAxis = "Price: " + m_strYAxisName;
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
            painter.drawText(yAxisLine.x2() - metrics.width(strYAxis) * 0.5,      //x coord
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

            //y axis подпись///////////////////////////////////////////////

            /// \brief minMaxDelta
            /// Разница между максимальным и минимальным элементом выборки
            double minMaxDelta =  m_pairMinMax.second - m_pairMinMax.first;
            double nNotchCount = 1;  //будет переназначено ниже

            //тут считается количество
            //единиц валюты в промежутке между 2 засечками:
            double denominator = 10;

            /////////////////////DEPRECATED: DELETE THIS /////////////////////
            if(minMaxDelta < 1) {                                          ///
            //                                                             ///
                nNotchCount = 9;                                           ///
            } else if(minMaxDelta < 10) {                                  ///
            //                                                             ///
                nNotchCount = 5;                                           ///
            } else if(minMaxDelta > 100) {                                 ///
            //                                                             ///
                nNotchCount = ceil(minMaxDelta / 10);                      ///
            } else {                                                       ///
                nNotchCount = minMaxDelta;                                 ///
            }                                                              ///
            //чилсо определяет += к nNotch;                                ///
                                                                           ///
            //                                                             ///
            if(nNotchCount > 100) {                                        ///
                denominator = 10;                                          ///
            } else if(nNotchCount < 10) {                                  ///
                denominator = 0.5;                                         ///
            } else {                                                       ///
                denominator = 1;                                           ///
            }                                                              ///
            //                                                             ///
            //test:                                                        ///
            if(minMaxDelta > 70) {                                         ///
                nNotchCount = minMaxDelta / 10;                            ///
                denominator = 10;                                          ///
            }                                                              ///
            //////////////////////////////////////////////////////////////////

            //@todo: Всего на оси Y 10 насечек
            // нижняя == min
            // верхняя == max
            // шаг = (max - min) / 10;
            //Во всех случаях будут 10 засечек по оси Y
            nNotchCount = 10;
            denominator = minMaxDelta / nNotchCount;

            if(minMaxDelta == 0) {
                nNotchCount = 1;
                denominator = 1;
            }

            //qDebug() << "nNotchCount: " << QString::number(nNotchCount);

            //количество пикселей на 1 засечку
            int yAxisNotchStepPixels =
                    (yAxisLine.y2() - yAxisLine.y1())  / (nNotchCount + 2); //magic number //need deletion

             //для текста (нижнее значение)
            double nNotch = m_pairMinMax.second - m_pairMinMax.first < 1
                            ? m_pairMinMax.first
                            : ceil(m_pairMinMax.first);

            for(int i = 1, z = 0; i < nNotchCount + 2; ++i, ++z) {  //magic number
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
                painter.drawText(yAxisLine.x1() - metrics.width(QString::number(nNotch)) - 10,      //x coord
                                 -(yAxisLine.y1() + yAxisNotchStepPixels * i
                                   + 0.5 * metrics.height()),         //y coord
                                 metrics.width(QString::number(nNotch)),
                                 metrics.height(),
                                 Qt::AlignCenter,
                                 QString::number(nNotch) //КОСТЫЛЬ
                                 );
                painter.restore();
                nNotch +=
                        //из-за округления каждого раза denominator, происходят значительные сдвиги
                        //в погрешности отметок оси Y
                        //ceil(denominator); //04.12 added ceil()
                        denominator;
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
                                       yAxisLine.y1() + (yAxisValueOffset) + (m_lPrice.at(i) - m_pairMinMax.first) * onePointPixel)
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
          , QString yAxisName = ""
          , bool isDropDownAllowed = false
          , QWidget* pwgt = 0)
          : QWidget(pwgt)
    {
        setAcceptDrops(isDropDownAllowed);

        setMinimumSize(175, 175);
        m_strFilename = strFilename;
        m_strYAxisName = yAxisName;

        //m_pOpacity = new QGraphicsOpacityEffect(this);
        //m_pOpacity->setOpacity(1.0);
        //setGraphicsEffect(m_pOpacity);

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
            //qDebug() << "m_lTime pushed: " << m_lTime.last();
            //qDebug() << "m_lPrice pushed: " << QString::number(m_lPrice.last());
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

public slots:
    //обновляет чарт
    void slotLoadChart(void) {
        qDebug() << "Reloading chart " << m_strFilename;
        fillChart(m_strFilename);
        m_pairMinMax = findMinMax(m_lPrice);

        /* Не работает корректно, для него скорее всего нужно создавать
         * данный виджет как потомок другого виджета
        QPropertyAnimation* pAnim = new QPropertyAnimation(m_pOpacity, "opacity");
        pAnim->setStartValue(0.4);
        pAnim->setEndValue(1.0);
        pAnim->start(QAbstractAnimation::DeleteWhenStopped);
        */

        repaint();
    }

};

#endif // CHART_H
