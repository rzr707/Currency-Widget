/********************************CHART VERSION 0.3******************************************
  @todo:
        0)  Пофиксить баг с вылетом 'ASSERT failure in QVector<T>::at: "index out of range" '
            программа вылетает тогда, когда мало элементов в файле
            Если точек меньше, чем насечек на оси Х (< 11) - происходит вылет FIXED
            Причина: действительно выходил за рамки вектора
        0.1) Пофиксить баг: если элементов меньше 10, график уезжает влево и происходит вылет FIXED
             (Причина: setMinimumWidth() )

        1)  Сделать лучше ранжирование при слишком больших разницах мин-макс по оси Y
            (для примера загрузить файл chart_hundreds.txt) (СМ. Changelog 0.3 - (1) ) +++
        2)  Добавить динамический рассчёт засечек на Х-оси и координат Х точек вектора
            с данными валют ++++
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

  CHANGELOG 0.3.2:
  07.12.2016:
  1) Теперь точки хранятся в QPointF, где точность координат с плавающей запятой,
     что позволило динамически рисовать точки на оси Х отнсоительно отрезков времени
     корректно, а также перерисовывать их на лету при изменении размеров окна.
  2) Все переменные, относящиеся к работе Х-оси и Х-координат были переведены с
     uint в double для повышения точности рассчётов и для фикса багов, связанных
     с округлением дробей до целых в uint
  3) Тип всех переменных, относящиеся к динамическому рассчёту размера засечек / точек
     на оси Y, был сменён на double, что позволило убрать "дёрганье" оси Y при
     изменении размера окна.
  4) Сделана проверка на пустоту файла - если файл пустой (0 байт), то окно закры-
     вается, предупредив об этом в консоли отладки
  5) Если в векторах только 1 точка, рисуется только 1 точка (пофиксен вылет)
  6) Теперь точки абсолютно корректно отображаются по отношению к обеим
     осям координат и корректно изменяют свои координаты при изменении размеров
     окна.
  7) Минимальная ширина окна поставлена в 455 пикселов.

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
    static const uint         m_MARGIN = 55;  // left and bottom margin from axis
    QString                   m_strFilename;
    QString                   m_strYAxisName;
    QVector<QPointF>           m_lDotsPrice;
    QVector<double>           m_lPrice;
    QVector<QString>          m_lTime;
    QVector<uint>             m_lTimeMins;
    std::pair<double, double> m_pairMinMax;
    std::pair<uint, uint>     m_pairMinMaxTimeMins;

private:
    void paintEvent(QPaintEvent*) {
        QPainter painter;
        QBrush   brush(Qt::black, Qt::SolidPattern);
        QPen     axisPen(brush, 1.5);
        QLine    yAxisLine(m_MARGIN, m_MARGIN, m_MARGIN, height() - m_MARGIN);
        QLine    xAxisLine(m_MARGIN, m_MARGIN, width() - m_MARGIN, m_MARGIN);
        // string width
        QString  strYAxis = "Price: " + m_strYAxisName;
        QFont    font(this->font());
        QFontMetrics metrics(font);

        painter.begin(this);
            m_lDotsPrice.clear(); // clear list

            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(axisPen);
            // draw y axis:
            painter.translate(0, height());
            painter.scale(1, -1);

            painter.drawLine(yAxisLine);
            painter.drawLine(xAxisLine);
            // y axis arrow
            painter.drawLine(yAxisLine.x2(), yAxisLine.y2(), yAxisLine.x2() + 3, yAxisLine.y2() - 10);
            painter.drawLine(yAxisLine.x2(), yAxisLine.y2(), yAxisLine.x2() - 3, yAxisLine.y2() - 10);
            // x axis arrow
            painter.drawLine(xAxisLine.x2(), xAxisLine.y2(), xAxisLine.x2() - 10, xAxisLine.y2() + 3);
            painter.drawLine(xAxisLine.x2(), xAxisLine.y2(), xAxisLine.x2() - 10, xAxisLine.y2() - 3);
            // y axis name:
            painter.save();
            painter.scale(-1, 1);
            painter.rotate(-180);       //reversy Y title
            painter.drawText(yAxisLine.x2() - metrics.width(strYAxis) * 0.5,      //x coord
                             -(yAxisLine.y2() + 15),         // y coord
                             metrics.width(strYAxis),   // text bounding rect width
                             metrics.height(),          // text bounding rect height
                             Qt::AlignCenter,           // flags
                             strYAxis                   // string
                             );

            // x axis name ok
            strYAxis = "Time";
            painter.drawText(xAxisLine.x2() - 10,      // x coord
                             -(xAxisLine.y2() + metrics.height() + 5),         // y coord
                             metrics.width(strYAxis),   // text bounding rect width
                             metrics.height(),          // text bounding rect height
                             Qt::AlignCenter,           // flags
                             strYAxis                   // string
                             );
            painter.restore();

            // y axis title

            /// \brief minMaxDelta
            /// difference between minumin and maximum value
            double minMaxDelta =  m_pairMinMax.second - m_pairMinMax.first;
            double nNotchCount = 1;  //будет переназначено ниже

            // calculate currency value
            // between 2 notches
            double denominator = 10;

            // 10 notches on Y axis
            nNotchCount = m_lPrice.size() > 1? 10 : 1;
            denominator = minMaxDelta / static_cast<double>(nNotchCount);

            if(minMaxDelta == 0) {
                nNotchCount = 1;
                denominator = 1;
            }

            // pixels on 1 notch count
            double yAxisNotchStepPixels =
                    static_cast<double>((yAxisLine.y2() - yAxisLine.y1()))
                    /
                    static_cast<double>((nNotchCount + 2.0)); //magic number

             // for text (bottom value)
            double nNotch = m_pairMinMax.second - m_pairMinMax.first < 1
                            ? m_pairMinMax.first
                            : ceil(m_pairMinMax.first);

            for(int i = 1; i < nNotchCount + 2; ++i) {  //magic number
                // y-axis notches
                painter.save();
                axisPen.setWidth(1.5);
                painter.setPen(axisPen);
                painter.drawLine(yAxisLine.x1() - 5,
                                 yAxisLine.y1() + yAxisNotchStepPixels * i,
                                 yAxisLine.x1() + 5,
                                 yAxisLine.y1() + yAxisNotchStepPixels * i
                                 );
                painter.restore();
                // y-axis text:
                painter.save();
                painter.scale(-1, 1);
                painter.rotate(-180);       //mirror Y text
                painter.drawText(yAxisLine.x1() - metrics.width(QString::number(nNotch)) - 10,      //x coord
                                 -(yAxisLine.y1() + yAxisNotchStepPixels * i
                                   + 0.5 * metrics.height()),         // y coord
                                 metrics.width(QString::number(nNotch)),
                                 metrics.height(),
                                 Qt::AlignCenter,
                                 QString::number(nNotch)
                                 );
                painter.restore();
                nNotch += denominator;
            }
            //сделать насечки на осях, подписать их
            //(количество насечек == m_nRecords)
            //int xAxisValueOffset = (xAxisLine.x2() - xAxisLine.x1()) / (m_lPrice.size()); //3 - magic number
            double yAxisValueOffset = yAxisNotchStepPixels;
            double onePointPixel = yAxisValueOffset / denominator;
            axisPen.setWidth(5);
            painter.setPen(axisPen);

            //time mins on x axis (dynamic):
            int xAxisNotchCount  = m_lPrice.size() > 1 ? 10 : 1;
            /*
            double uintTimePerNotch = (m_pairMinMaxTimeMins.second - m_pairMinMaxTimeMins.first) / xAxisNotchCount;
            //qDebug() << "uintTimePerNotch: " << QString::number(uintTimePerNotch);
            */
            double incrementor =
                    static_cast<double>((m_pairMinMaxTimeMins.second - m_pairMinMaxTimeMins.first) /
                                        static_cast<double>(xAxisNotchCount)
                                        );
            ////qDebug() << "incrementor: " << QString::number(incrementor);

            double strMinTime  = m_pairMinMaxTimeMins.first;

            //10 отметок, как и на оси Y

            //если ставить 1.0 - то остаётся неправильное смещение точек
            double xAxisValueOffset
                    //костыль: ручная подгонка координат. координаты точек и насечек х-оси сошлись переделать (1.0)
                    = static_cast<double>((xAxisLine.x2() - xAxisLine.x1()) / (xAxisNotchCount + 1.0));
            //qDebug() << "xAxisValueOffset: " << xAxisValueOffset;

            //таким образом заработала динамическая привязка х-координат точки к х-оси:
            double minPerOnePixel
            //костыль: добавил "- xAxisValueOffset" - координаты точек и насечек х-оси сошлись
            = static_cast<double>((xAxisLine.x2() - xAxisLine.x1()) - xAxisValueOffset);
            if(m_lPrice.size() > 1) {
                minPerOnePixel /=
                        static_cast<double>((m_pairMinMaxTimeMins.second - m_pairMinMaxTimeMins.first));
            }

            //рисуются насечки: (теперь в отдельном цикле) 06 12 16:
            ////qDebug() << "x axis name loop";
            for(int i = 0; i < xAxisNotchCount + 1; ++i)  { // + 1 == out_of_range error FIXED
                ////qDebug() << QString::number(i);
                //line draw
                painter.save();
                axisPen.setWidth(1.5);
                painter.setPen(axisPen);
                //насечки ось Х 06 12 16:
                painter.drawLine(xAxisLine.x1() + xAxisValueOffset * (i + 1.0)  - 20, xAxisLine.y1() - 5,
                                 xAxisLine.x1() + xAxisValueOffset * (i + 1.0)  - 20, xAxisLine.y1() + 5
                                 );
                painter.restore();

                //x axis подпись ok:
                painter.save();
                painter.scale(-1, 1);
                painter.rotate(-180);       //для переворачивания надписи
                painter.drawText(xAxisLine.x1() + (xAxisValueOffset * (i + 1)) - 0.5 * metrics.width(getTimeFromMins(strMinTime)) - 20,      //x coord
                                 -(xAxisLine.y2() - metrics.height() + 5),         //y coord
                                 //out_of_range FIXED:
                                 metrics.width(m_lTime.last()),
                                 metrics.height(),          //text bounding rect height
                                 Qt::AlignCenter,           //flags
                                 getTimeFromMins(strMinTime)//string
                                 //QString::number(strMinTime) //test
                                 );

                painter.restore();

                strMinTime += incrementor;
                setMinimumWidth(455);
            }

            // draw dots:
            for(int i = 0; i < m_lPrice.size(); ++i) {
                QPointF xyPoint(
                 xAxisLine.x1() + xAxisValueOffset + (m_lTimeMins.at(i) - m_pairMinMaxTimeMins.first) * minPerOnePixel - 20,
                 yAxisLine.y1() + yAxisValueOffset + (m_lPrice.at(i) - m_pairMinMax.first) * onePointPixel
                 );

                m_lDotsPrice.push_back(xyPoint);
                painter.drawPoint(xyPoint);
            }

        // draw lines
        axisPen.setWidth(1.5);
        painter.setPen(axisPen);
        for(int i = 0; i < m_lDotsPrice.size() - 1; ++i) {
            painter.drawLine(m_lDotsPrice.at(i),
                             m_lDotsPrice.at(i + 1)
                             );
        }

        painter.end();
    }

    virtual void dragEnterEvent(QDragEnterEvent* pe) {
        if(pe->mimeData()->hasUrls()) {
            pe->acceptProposedAction();
        }
    }

    virtual void dropEvent(QDropEvent* pe) {
        const QMimeData* pMimeData = pe->mimeData();
        if(pMimeData->hasText() && pMimeData->urls().size() == 1) {
            fillChart(pMimeData->urls().at(0).toLocalFile());
            setWindowTitle(QString("DROPDOWN: " + pMimeData->urls().at(0).toLocalFile()
                                   .mid(0, pMimeData->urls().at(0).toLocalFile().lastIndexOf('.'))
                                   )
                           );
            m_pairMinMax = findMinMax(m_lPrice);
            m_pairMinMaxTimeMins = findMinMax(m_lTimeMins);
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

        // load charts from file:
        fillChart(m_strFilename);
        m_pairMinMax = findMinMax(m_lPrice);
        m_pairMinMaxTimeMins = findMinMax(m_lTimeMins);
         qDebug() << "min m_lPrice: " << m_pairMinMax.first
                  << "\nmax m_lPrice" << m_pairMinMax.second
                  << "\nmin m_lTimeMins: " << m_pairMinMaxTimeMins.first
                  << "\nmax m_lTimeMins" << m_pairMinMaxTimeMins.second
                  << "\nm_pairMinMaxTimeMins first" << m_pairMinMaxTimeMins.first
                  << "\nm_pairMinMaxTimeMins second" << m_pairMinMaxTimeMins.second;

    }

    void fillChart(QString filename) {
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly)) {
            //qDebug() << "Error opening the file " << file.fileName();
        }

        QTextStream stream(&file);

        // if vectors are'nt empty - clear them (for dropdown feature)
        if(!m_lPrice.isEmpty() || !m_lDotsPrice.isEmpty() || !m_lTime.isEmpty() || !m_lTimeMins.isEmpty()) {
            m_lPrice.clear();
            m_lDotsPrice.clear();
            m_lTime.clear();
            m_lTimeMins.clear();
        }

        if(file.size() == 0) {
            qDebug() << "THE FILE " << filename << " IS EMPTY! Closing the window...";
            close();
        }

        while(!stream.atEnd()) {
            m_lTime.push_back(stream.readLine());
            m_lPrice.push_back(stream.readLine().toDouble());
            m_lTimeMins.push_back(getMins(m_lTime.last()));

            if(m_lTime.last().trimmed() == "") {
                qDebug() << "CRITICAL ERROR: m_lTime.last() == \"\"! Closing the window...";
                close();
            }
        }

        qDebug() << "fillChart m_lTimeMins: " << m_lTimeMins;

    }

    // from string time to uint minutes
    uint getMins(const QString& s) {
        return QDateTime::fromString(s, "hh:mm").time().hour() * 60
                + QDateTime::fromString(s, "hh:mm").time().minute();
    }

    //from uint minutes to string time
    QString getTimeFromMins(const uint mins) {
        QString hour = QString::number(mins / 60);
        QString min = QString::number(mins % 60);

        if(min.toInt() < 10) {
            min.push_front('0');
        }

        return hour + ':' + min;
    }

    template<typename T>
    std::pair<T, T> findMinMax(const QVector<T>& l) {
        std::pair<T, T> result;
        result.first = findMin(l);

        T max = result.first;
        for(auto iter = l.begin(); iter != l.end(); ++iter) {
            if(*iter > max) {
                max = *iter;
            }
        }
        result.second = max;

        return result;
    }

    template<typename T>
    T findMin(const QVector<T>& l) {
        T min = std::numeric_limits<T>::max();
        for(auto iter = l.begin(); iter != l.end(); ++iter) {
            if(*iter < min) {
                min = *iter;
            }
        }
        return min;
    }

public slots:
    // chart update
    void slotLoadChart(void) {
        qDebug() << "Reloading chart " << m_strFilename;
        fillChart(m_strFilename);
        m_pairMinMax = findMinMax(m_lPrice);
        m_pairMinMaxTimeMins = findMinMax(m_lTimeMins);

        repaint();
    }

};

#endif // CHART_H
