#ifndef  CURRENT_H
#define  CURRENT_H

#include <QtNetwork>
#include <QWidget>
#include <QLabel>
#include <QFile>
#include <QBitmap>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QMouseEvent>
#include <QBoxLayout>
#include <QFontDialog>
#include <QColorDialog>
#include <QFontMetrics>
#include <QTimer>
#include <QSettings>
#include <QPoint>
#include <QFont> //for antialiasing

#ifdef Q_OS_WIN
    #include <Windows.h> //IsWow64Process()
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDateTime>

#include <QMessageBox>

enum eCurrencies   { USD = 0, EUR, RUR, BTC };
enum eApiQuery     { SALE = 0, BUY = 1      };
enum eExchangeType { CASH = 0, NON_CASH = 1 };

class CurrencyWidget : public QLabel {
    Q_OBJECT
private:
    static const uint       m_UPDATE_TIME_MSEC = 60000;
    bool                    m_bFirstLaunch;        // true on first app launch
    bool                    m_bIsAutorun;
    bool                    m_bIsAnimationEnabled;
    QPoint                  m_mouseCoords;
    QString                 m_strCurrency;
    QString                 m_strInstruction;
    QNetworkAccessManager*  m_nam;
    QStringList             m_strListCurrNames;
    QList<QString>          m_currencies;
    QPushButton*            m_pcmdExit;
    QPushButton*            m_pcmdFont;
    QPushButton*            m_pcmdColor;
    QHBoxLayout*            m_phbl;
    QTimer*                 m_pBtnTimer;         // timer for exit button
    QTimer*                 m_pInfoUpdateTimer;  // timer for currency exchange
    QTimer*                 m_pChartTimer;       // timer for chart (every 20 mins adds node into file)
    QString                 m_osVersion;
    QSettings*              m_pSettings;
    QPropertyAnimation*     m_anim;
    QGraphicsOpacityEffect* m_pGraphics;
    eApiQuery               m_eApiQueryType;
    eExchangeType           m_eExchangeType;


protected:
    virtual void mousePressEvent(QMouseEvent* pe) {
        if(pe->type() == QMouseEvent::MouseButtonDblClick) {
            slotOpenSettings();
        }
        m_mouseCoords = pe->pos();
    }

    virtual void mouseMoveEvent(QMouseEvent* pe) {
        move(pe->globalPos() - m_mouseCoords);
        // if widget is moved - there is exit button appears for 3 seconds
        m_pcmdExit->setVisible(true);
        m_pBtnTimer->start(3000);
    }

public:
    CurrencyWidget(QWidget* pwgt = 0) : QLabel(pwgt) {
        m_osVersion = QSysInfo::productType();

        m_bIsAnimationEnabled = true;

        m_pGraphics = new QGraphicsOpacityEffect(this);
        m_pGraphics->setOpacity(1.0);
        setGraphicsEffect(m_pGraphics);
        m_anim = new QPropertyAnimation(m_pGraphics, "opacity");
        m_anim->setEasingCurve(QEasingCurve::Linear);

        m_anim->setEndValue(1.0);
        m_anim->setDuration(650);

        m_eApiQueryType = SALE;
        m_eExchangeType = CASH;

        m_bFirstLaunch = true;
        m_bIsAutorun   = false;
        m_pSettings  = new QSettings("rzr707", "Currency Converter");
        // Timer
        m_pInfoUpdateTimer = new QTimer(this);
        connect(m_pInfoUpdateTimer, SIGNAL(timeout()),
                this              , SLOT(slotGetCurrencyInfo())
                );
        m_pInfoUpdateTimer->start(60000);

        m_pBtnTimer = new QTimer(this);
        connect(m_pBtnTimer, SIGNAL(timeout()),
                this,        SLOT(slotExitButton())
                );
        m_pBtnTimer->start(3000);

        m_pChartTimer = new QTimer(this);
        connect(m_pChartTimer, SIGNAL(timeout()),
                this,          SLOT(slotWriteCharts())
                );
        m_pChartTimer->start(10000);

        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

        /* Qt::tool for hiding application from из taskbar,
         * Qt::FramelessWindowHint removes frame with close/hide buttons
        */
        setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnBottomHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        m_pcmdExit = new QPushButton("x", this);
        m_pcmdExit->setGeometry(this->geometry());
        m_pcmdExit->setFixedSize(10, 10);
        connect(m_pcmdExit, SIGNAL(clicked(bool)),
                qApp,       SLOT(quit())
                );

        m_nam = new QNetworkAccessManager(this);
        connect(m_nam, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(slotFinished(QNetworkReply*))
                );

        slotGetCurrencyInfo();

        m_strListCurrNames << "USD/UAH: " << "EUR/UAH: " << "RUR/UAH: " << "USD/BTC: ";

        m_strInstruction = "\t<center><h1>Программа курс валют (по ПриватБанку)</h1></center><br>"
                           "<br>1) Для смены настроек шрифта и/или цвета"
                           " - двойной клик левой кнопкой мыши по тексту."
                           "<br>2) Для перемещения виджета, нажмите и"
                           " держите лев. кнопку мыши и тяните виджет на приглянувшееся место."
                           "<br>3) Чтобы скрыть виджет, дважды кликните левой кнопкой "
                           "по иконке в трее (или выбор соотв. подпункта)."
                           "<br>4) В меню \"Графики валют\" можно посмотреть график изменения цен."
                           "<br>5) Для завершения программы потянуть за текст"
                           " и нажать на появившуюся в тексте точку "
                           "(или закрыть приложение через область уведомлений)."
                           "<br>6) Для смены данных на куплю/продажу банком валюты "
                           "или на курс наличной/безналичной валюты, выберите пункт "
                           "\"Валюта\" в области уведомлений (трей)."
                           "<br>7) Настройки автозагрузки будут работать только в "
                           "случае запуска приложения от учётной записи администратора "
                           "( win 7, 8, 8.1, 10 etc)."
                           "<br>P.S. Найденные баги и предложения по развитию "
                           "можно присылать на vanekk13@gmail.com";


        //setup layout
        m_phbl = new QHBoxLayout(this);
        m_phbl->setMargin(0);
        m_phbl->setSpacing(0);
        m_phbl->addWidget(m_pcmdExit);
        m_phbl->addStretch(1);
        setLayout(m_phbl);

        if(m_osVersion == "windows" || m_osVersion == "winrt") {
            readSettings();
        }

        if(m_bFirstLaunch) {
            m_bFirstLaunch = false;
            QPalette pal;
            pal.setColor(QPalette::WindowText, Qt::red);
            setPalette(pal);

            QPoint p = QApplication::desktop()->screenGeometry().center();
            move(p);
            resize(350, 250);
            QMessageBox::information(this, "Инструкция", m_strInstruction);
        }

    }

    void animateFadeInOut() {
        if(m_bIsAnimationEnabled) {
            m_anim->setStartValue(0.4);
            m_anim->start();
        }
    }

    bool          isAnimEnabled(void)    { return m_bIsAnimationEnabled; }
    void          setAnimEnabled(bool b) { m_bIsAnimationEnabled = b;    }
    QString       getInstruction(void)   { return m_strInstruction;      }
    eApiQuery     getApiQueryType(void)  { return m_eApiQueryType;       }
    eExchangeType getExchangeType(void)  { return m_eExchangeType;       }

    QList<QString> parseCurrency(const QString& str, eApiQuery status) {
        if(str.isEmpty()) {
            qDebug() << "STRING IS EMPTY!";
        }

        QList<QString> list;
        QString     source = str;
        int indexSaleBuy = 0;
        int indexSymbol = 0;

        // json response parse:
        while(indexSaleBuy != -1) {
            indexSaleBuy = source.indexOf(status == SALE ? "sale" : "buy");
            indexSymbol = source.indexOf((status == SALE ? "}" : ","), indexSaleBuy);
            qDebug() << "indexSale is: " << indexSaleBuy << "indexSymbol is: " << indexSymbol;
            qDebug() << "source after searching is: " << source;
            QString result = source.mid(indexSaleBuy, indexSymbol - indexSaleBuy);
            source = source.mid(indexSymbol, source.length() - indexSymbol);
            qDebug() << "result is: " << result;
            list.push_back(result);
        }

        QList<QString> listStrTemp;
        for(int i = 0; i < list.size(); ++i ) {
            int firstSymbol = (status == SALE  ? 7 : 6);   // всегда 7й(6й для buy) символ - первые двойные кавычки
            int lastSymbol = list.at(i).indexOf(".") + 3;  // first 3 digits after dot
            listStrTemp.push_back(list.at(i).mid(firstSymbol, lastSymbol - firstSymbol));
            qDebug() << "list.at(i) is: " << list.at(i);

        }

        QList<QString> strCashString;
        if(m_eExchangeType == CASH) {
           strCashString.push_back(listStrTemp.at(2));
           strCashString.push_back(listStrTemp.at(0));
           strCashString.push_back(listStrTemp.at(1));
           strCashString.push_back(listStrTemp.at(3));

           listStrTemp = strCashString;
        }

        return listStrTemp;
    }

    ~CurrencyWidget() {
#ifdef Q_OS_WIN
            writeSettings();
            writeAutorun(m_bIsAutorun);
#endif
    }

    bool autorun(void)      { return m_bIsAutorun; }
    void setAutorun(bool b) { m_bIsAutorun = b;    }

    /* Settings load */
    void readSettings() {
        m_pSettings->beginGroup("/settings");
            m_bFirstLaunch = m_pSettings->value("/firstlaunch", true).toBool();
            m_bIsAutorun   = m_pSettings->value("/autorun").toBool();
            m_bIsAnimationEnabled = m_pSettings->value("/animation").toBool();
            setPalette( m_pSettings->value("/palette").value<QPalette>());
            setFont( m_pSettings->value("/font").value<QFont>());
            resize( m_pSettings->value("/width").toInt(),  m_pSettings->value("/height").toInt());
            move(m_pSettings->value("/position").value<QPoint>());
            m_eApiQueryType = static_cast<eApiQuery>(m_pSettings->value("/api_query_type", 0).toInt());
            m_eExchangeType = static_cast<eExchangeType>(m_pSettings->value("/exchange_type", 0).toInt());
         m_pSettings->endGroup();
    }

    void writeSettings() {
         m_pSettings->beginGroup("/settings");
             m_pSettings->setValue("/palette", palette() );
             m_pSettings->setValue("/width",   width()   );
             m_pSettings->setValue("/height",  height()  );
             m_pSettings->setValue("/font",    font()    );
             m_pSettings->setValue("/position", pos());
             m_pSettings->setValue("/firstlaunch", m_bFirstLaunch);
             m_pSettings->setValue("/autorun", m_bIsAutorun);
             m_pSettings->setValue("/animation", m_bIsAnimationEnabled);
             m_pSettings->setValue("/api_query_type", static_cast<int>(m_eApiQueryType));
             m_pSettings->setValue("/exchange_type", static_cast<int>(m_eExchangeType));
        m_pSettings->endGroup();
    }

    //  Write app to autorun settings
    //  For win32 and win64 there are different registry paths
#ifdef Q_OS_WIN
    void writeAutorun(bool b) {
        BOOL bIs64 = FALSE;                             //windows.h
        IsWow64Process(GetCurrentProcess(), &bIs64);    //windows.h (if 64bit - returns true)
        qDebug() << "bIs64 is: " << bIs64;
        QSettings* autorun = nullptr;
        if(bIs64) {
            // User must run app under administrator account
            autorun = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",
                              QSettings::NativeFormat
                              );
        }
        else {
             autorun = new QSettings("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                        QSettings::NativeFormat
                           );
        }
        QString path = qApp->applicationFilePath();
        path.replace('/', '\\');
        if( b ) {
            autorun->setValue("/CurrencyConverter", path);
        }
        else {
            autorun->remove("/CurrencyConverter");
        }
    }
#endif

    QString getDate() { return QDate::currentDate().toString("dd_MM_yy"); }

    bool writeChart(eCurrencies c = BTC) {
        QString currency;
        eCurrencies currentCurrency = c;

        switch(c) {
        case EUR:
            currency = "eur_";
            break;
        case RUR:
            currency = "rur_";
            break;
        case USD:
            currency = "usd_";
            break;
        case BTC:
            currency = "btc_";
            break;
        default:
            currency = "btc_";
            break;
        }

        QDir d(QApplication::applicationDirPath());
        d.mkdir("charts");
        d.cd("charts");
        d.mkdir(getDate());

        QString filename = QApplication::applicationDirPath() +
                "/charts/" + getDate() + "/" + currency + getDate() + ".dat";



        qDebug() << "filename: " << filename;

        QFile file(filename);
        if(!file.open(QIODevice::Text | QIODevice::ReadWrite | QIODevice::Append)) {
            qDebug() << "ERROR OPENING THE '" << filename << "'";
            return false;
        }

        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toString("hh:mm")
               << '\n'
               << m_currencies.at(static_cast<int>(currentCurrency))
               << '\n';
        file.close();

        qDebug() << "Currency "
                 << QString::number(static_cast<int>(currentCurrency))
                 << " has been written to the file "
                 << filename
                 << " succesfully!";

        /*
         * signal is emitted when chart files updated
         * and sent to Tray object, which will
         * dynamically update every opened chart window contents
        */
        emit signalRefreshCharts();

        return true;
    }


public slots:
    void slotFinished(QNetworkReply* reply) {
        if(reply->error() == QNetworkReply::NoError) {
            m_strCurrency = reply->readAll();
            qDebug() << m_strCurrency;

            if(m_strCurrency == "<error>invalid request</error>") {
                QMessageBox::critical(this,
                                      "Критическая ошибка!",
                                      "Неверный ответ сервера на GET-запрос. "
                                      "Приложение будет остановлено.",
                                      QMessageBox::Ok
                                      );
                qApp->quit();
            }

            m_currencies = parseCurrency(m_strCurrency, m_eApiQueryType);
            animateFadeInOut();
            setText("EUR/UAH: " + m_currencies.at(1) + "\n"
                   +"USD/UAH: " + m_currencies.at(0) + "\n"
                   +"RUR/UAH: " + m_currencies.at(2) + "\n"
                   +"BTC/USD: " + m_currencies.at(3)
                    );

        } else {
            qDebug() << "GET query to Privat24 API is failed. Termination..";
            QMessageBox::critical(this,
                                  "Критическая ошибка",
                                  "Нет связи с Privat24, "
                                  "попробуйте перезапустить"
                                  " приложение позже.");
            qApp->quit();
        }

        //Обнулить таймер
        m_pInfoUpdateTimer->start(m_UPDATE_TIME_MSEC);

        reply->deleteLater();
    }

    // Font Settings
    void slotOpenSettings() {
        setFont(QFontDialog::getFont(0, this->font(), this, "Изменить шрифт..."));
        QFontMetrics metrics(this->font());
        this->resize(metrics.width(text()) / 3.5, metrics.height() * 5);

        QColor color = QColorDialog::getColor(palette().color(QPalette::WindowText),
                                              this,
                                              "Сменить цвет...",
                                              QColorDialog::ShowAlphaChannel);

        if(!color.isValid()) { // color is not valid when 'cancel' button clicked
            return;
        }
        QPalette pal;
        pal.setColor(QPalette::WindowText, color);
        setPalette(pal);


    }

    void slotWriteCharts() {
        writeChart(USD);
        writeChart(EUR);
        writeChart(RUR);
        writeChart(BTC);
        m_pChartTimer->start(1000 * 60 * 20);
    }

    void slotGetCurrencyInfo() {
        /* Privat24 API:
         * 04.12.2016: поменял запрос на безналичный курс, т.к. наличный не присылал валидных данных.
         *
         * https://api.privatbank.ua/p24api/pubinfo?exchange&json&coursid=11 - безналичный курс валют
         * https://api.privatbank.ua/p24api/pubinfo?json&exchange&coursid=5  - наличный курс валют
         */
        QString strGetExchangeType =
                m_eExchangeType == CASH ?
                "https://api.privatbank.ua/p24api/pubinfo?json&exchange&coursid=5"
                                        :
                "https://api.privatbank.ua/p24api/pubinfo?exchange&json&coursid=11";
        m_nam->get(QNetworkRequest(QUrl(strGetExchangeType)));
    }

    // moves widget to center of screen to coord p
    void slotRecieveCoords(QPoint p) {
        qDebug() << "slotRecieveCoords: " << p;
        QEasingCurve c(QEasingCurve::OutBounce);
        c.setAmplitude(c.amplitude() * 0.5);

        QPropertyAnimation* pAnim = new QPropertyAnimation(this, "geometry");
        pAnim->setDuration(1000);
        pAnim->setStartValue(this->geometry());
        pAnim->setEndValue(QRect(
                                 p,
                                 QSize(width(), height())
                                 )
                           );
        pAnim->setEasingCurve(c);
        pAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void slotExitButton() {
        m_pcmdExit->setVisible(false);
    }

    void slotExchangeChange(eExchangeType signal) {
        m_eExchangeType = signal;
        slotGetCurrencyInfo();
    }

    void slotApiChange(eApiQuery signal) {
        m_eApiQueryType = signal;
        slotGetCurrencyInfo();
    }

signals:
     /*
     * Signal sent to notify every opened chart windows
     * to update themselves
     */
    void signalRefreshCharts();
    void signalShowSettings();

};

#endif // !CURRENT_H
