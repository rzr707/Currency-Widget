#ifndef  TRAY_H
#define  TRAY_H

#include "current.h"
#include "chart.h"
#include <QWidget>
#include <QSystemTrayIcon>
#include <QLabel>
#include <QMenu> // for context menu
#include <QPixmap>
#include <QMouseEvent>

class Tray : public QLabel {
    Q_OBJECT
private:
    QSystemTrayIcon* m_ptrayIcon;
    QMenu*           m_pcontextMenu;
    CurrencyWidget*  m_pwidgetPointer;
    QAction*         m_pactAutorun;
    QAction*         m_pactAnimationState;
    QIcon            m_iconGreenDot;

    QAction* pactBuyCurrency;
    QAction* pactSaleCurrency;
    QAction* pactCashCurrency;
    QAction* pactNoncashCurrency;

public:
    Tray(QWidget* pwgt = 0
        , CurrencyWidget* currwgt = 0)
        : QLabel("<H1>Currency Converter</H1>", pwgt) {

        m_pwidgetPointer = currwgt;

        m_iconGreenDot = QIcon(QPixmap(":/res/icons/greendot.png"));

        m_ptrayIcon = new QSystemTrayIcon(this);
        m_ptrayIcon->setIcon(QIcon(QPixmap(":/res/icons/icon.ico")));

        m_pcontextMenu               = new QMenu(this);
        QAction* pactHelp            = new QAction("Инструкция", this);
        QAction* pactExitApplication = new QAction("Закрыть", this);
        QAction* pactShowHide        = new QAction("Показать/Скрыть", this);
        QAction* pactResetCoords     = new QAction("Сбросить координаты", this);
        //график:
        QMenu*   pChartMenu            = new QMenu("Графики валют", m_pcontextMenu);
        QAction* pactShowUsdChart      = new QAction("График USD", pChartMenu);
        QAction* pactShowEurChart      = new QAction("График EUR", pChartMenu);
        QAction* pactShowRurChart      = new QAction("График RUR", pChartMenu);
        QAction* pactShowBitcoinChart  = new QAction("График Bitcoin", pChartMenu);
        QAction* pactOpenChartFromFile = new QAction("Загрузить из файла...", pChartMenu);

        pChartMenu->addAction(pactShowUsdChart);
        pChartMenu->addAction(pactShowEurChart);
        pChartMenu->addAction(pactShowRurChart);
        pChartMenu->addAction(pactShowBitcoinChart);
        pChartMenu->addSeparator();
        pChartMenu->addAction(pactOpenChartFromFile);

        //настройки налички:
        QMenu* pCurrencyMenu = new QMenu("Валюта", m_pcontextMenu);
        pactBuyCurrency = new QAction("Купля", pCurrencyMenu);
        pactSaleCurrency = new QAction("Продажа", pCurrencyMenu);
        pactCashCurrency = new QAction("Наличка", pCurrencyMenu);
        pactNoncashCurrency = new QAction("Безналичка", pCurrencyMenu);
        pCurrencyMenu->addAction(pactBuyCurrency);
        pCurrencyMenu->addAction(pactSaleCurrency);
        pCurrencyMenu->addSeparator();
        pCurrencyMenu->addAction(pactCashCurrency);
        pCurrencyMenu->addAction(pactNoncashCurrency);

        m_pactAnimationState         = new QAction("Анимация переходов", this);
        m_pactAnimationState->setIcon(m_pwidgetPointer->isAnimEnabled() ? m_iconGreenDot : QIcon());
        m_pactAutorun                  = new QAction("Автозапуск", this);
        m_pactAutorun->setIcon(currwgt->autorun() ? m_iconGreenDot : QIcon());

#ifndef Q_OS_WIN
        m_pactAutorun->setEnabled(false);
#endif

        m_pcontextMenu->addAction(pactHelp);
        m_pcontextMenu->addMenu(pCurrencyMenu);
        m_pcontextMenu->addMenu(pChartMenu);
        m_pcontextMenu->addAction(pactShowHide);
        m_pcontextMenu->addAction(m_pactAutorun);
        m_pcontextMenu->addAction(m_pactAnimationState);
        m_pcontextMenu->addAction(pactResetCoords);
        m_pcontextMenu->addAction(pactExitApplication);

        //currency menu signal-slot connections:
        connect(pactBuyCurrency, SIGNAL(triggered(bool)),
                this,            SLOT(slotBuyCurrencyClicked())
                );
        connect(pactSaleCurrency, SIGNAL(triggered(bool)),
                this,             SLOT(slotSaleCurrencyClicked())
                );
        connect(pactCashCurrency, SIGNAL(triggered(bool)),
                this,             SLOT(slotCashCurrencyClicked())
                );
        connect(pactNoncashCurrency, SIGNAL(triggered(bool)),
                this,                SLOT(slotNoncashCurrencyClicked())
                );

        connect(pactExitApplication, SIGNAL(triggered(bool)),
                this,                SLOT(slotExitClicked())
                );
        connect(pactHelp,            SIGNAL(triggered(bool)),
                this,                SLOT(slotHelp())
                );
        connect(pactShowHide,        SIGNAL(triggered(bool)),
                this,                SLOT(slotShowHideClicked())
                );
        // for doubleclick opening/hiding
        connect(m_ptrayIcon,         SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this,                SLOT(slotTrayActivated(QSystemTrayIcon::ActivationReason))
                );
        // for tray autorun check func
        connect(m_pactAutorun, SIGNAL(triggered(bool)),
                this,          SLOT(slotAutorun())
                );
        connect(m_pactAnimationState, SIGNAL(triggered(bool)),
                this,                 SLOT(slotAnimState(bool))
                );
        connect(pactShowBitcoinChart, SIGNAL(triggered(bool)),
                this,                 SLOT(slotShowBitcoinChart())
                );
        connect(pactShowUsdChart, SIGNAL(triggered(bool)),
                this,             SLOT(slotShowUsdChart())
                );
        connect(pactShowEurChart, SIGNAL(triggered(bool)),
                this,             SLOT(slotShowEuroChart())
                );
        connect(pactShowRurChart, SIGNAL(triggered(bool)),
                this,             SLOT(slotShowRubleChart())
                );

        connect(pactOpenChartFromFile, SIGNAL(triggered(bool)),
                 this,                  SLOT(slotOpenChartFromFile())
                 );

        connect(pactResetCoords, SIGNAL(triggered(bool)),
                this,            SLOT(slotResetCoords())
                );
        connect(this, SIGNAL(signalShowChart(eCurrencies)),
                this, SLOT(slotCreateChart(eCurrencies))
                );

        m_ptrayIcon->setContextMenu(m_pcontextMenu);
        updateCurrencyMenuIcons();
        m_ptrayIcon->show();

    }

    void updateCurrencyMenuIcons() {
            pactBuyCurrency->setIcon(iconSetter(m_pwidgetPointer->getApiQueryType() == BUY));
            pactSaleCurrency->setIcon(iconSetter(m_pwidgetPointer->getApiQueryType() == SALE));
            pactCashCurrency->setIcon(iconSetter(m_pwidgetPointer->getExchangeType() == CASH));
            pactNoncashCurrency->setIcon(iconSetter(m_pwidgetPointer->getExchangeType() == NON_CASH));
    }

    QIcon iconSetter(bool b) {
        if(b) {
            return m_iconGreenDot;
        }
        return QIcon();
    }

public slots:
    void slotHelp() {
        QMessageBox::information(this, "Инструкция", m_pwidgetPointer->getInstruction());
    }

    void slotExitClicked()     { emit exitClicked();                                  }
    void slotShowHideClicked() { emit signalShowHide(!m_pwidgetPointer->isVisible()); }

    void slotTrayActivated(QSystemTrayIcon::ActivationReason reason) {
        if(reason == QSystemTrayIcon::DoubleClick){
            slotShowHideClicked();
        }
    }

    void slotAutorun() {
        if(m_pwidgetPointer->autorun()) {
            m_pwidgetPointer->setAutorun(!m_pwidgetPointer->autorun());
             m_pactAutorun->setIcon(QIcon());
        }
        else {
            m_pwidgetPointer->setAutorun(!m_pwidgetPointer->autorun());
            m_pactAutorun->setIcon(m_iconGreenDot);
        }
    }

    void slotAnimState(bool) {
        bool isAnimated = m_pwidgetPointer->isAnimEnabled();
        m_pwidgetPointer->setAnimEnabled(!isAnimated);
        if(!isAnimated) {
            m_pactAnimationState->setIcon(m_iconGreenDot);
        } else {
            m_pactAnimationState->setIcon(QIcon());
        }
    }

    void slotShowBitcoinChart(void) { emit signalShowChart(BTC); }
    void slotShowUsdChart(void)     { emit signalShowChart(USD); }
    void slotShowEuroChart(void)    { emit signalShowChart(EUR); }
    void slotShowRubleChart(void)   { emit signalShowChart(RUR); }

    void slotResetCoords() {
        emit signalResetCoords(QApplication::desktop()->screenGeometry().center());
    }

    void slotCreateChart(eCurrencies c) {
        QString s1, s2;

        switch(c) {
            case USD:
            s1 = "usd_";
            s2 = "USD/UAH";
            break;
        case EUR:
            s1 = "eur_";
            s2 = "EUR/UAH";
            break;
        case RUR:
            s1 = "rur_";
            s2 = "RUR/UAH";
            break;
        case BTC:
            s1 = "btc_";
            s2 = "BTC/USD";
            break;
        }
        Drawer* chart
                = new Drawer(QApplication::applicationDirPath()
                             + "/charts/"
                             + m_pwidgetPointer->getDate()
                             + "/" + s1 + m_pwidgetPointer->getDate() + ".dat",
                             s2
                             );
        // Динамическое обновление открытого чарта
        connect(m_pwidgetPointer, SIGNAL(signalRefreshCharts()),
                chart,            SLOT(slotLoadChart())
                );
        //
        chart->setAttribute(Qt::WA_DeleteOnClose);
        chart->setVisible(true);
        chart->setWindowTitle("Курс " + s2 + " за "
                              + m_pwidgetPointer->getDate().replace('_', '.')
                              );
        chart->show();
    }

    void slotOpenChartFromFile() {
        QString strCustomChartFilename
                = QFileDialog::getOpenFileName(this,
                                               "Загрузить чарт из файла",
                                               QApplication::applicationDirPath() + "/charts",
                                               "Файлы чартов (*.dat *.txt)"
                                               );
        Drawer* chart = new Drawer(strCustomChartFilename);
        chart->setAttribute(Qt::WA_DeleteOnClose);
        chart->setVisible(true);

        // преобразует абсолютный путь файла в строку типа "%ВАЛЮТА% за dd.MM.yy"
        QString strChartTitle = strCustomChartFilename
                .mid(strCustomChartFilename.lastIndexOf('/') + 1)
                    .replace("usd_", "USD/UAH за ")
                        .replace("eur_", "EUR/UAH за ")
                            .replace("rur_", "RUR/UAH за ")
                                .replace("btc_", "BTC/USD за ")
                                  .replace('_', '.');
        strChartTitle.chop(4);


        chart->setWindowTitle("График " + strChartTitle);

        chart->show();
    }

    void slotBuyCurrencyClicked() {
        emit signalBuySaleCurrency(BUY);
        updateCurrencyMenuIcons();
    }

    void slotSaleCurrencyClicked() {
        emit signalBuySaleCurrency(SALE);
        updateCurrencyMenuIcons();
    }

    void slotCashCurrencyClicked() {
        emit signalCashNoncashCurrency(CASH);
        updateCurrencyMenuIcons();
    }

    void slotNoncashCurrencyClicked() {
        emit signalCashNoncashCurrency(NON_CASH);
        updateCurrencyMenuIcons();
    }

signals:
    void exitClicked();
    void signalShowHide(bool);
    void signalResetCoords(QPoint p);
    void signalShowChart(eCurrencies c);
    void signalBuySaleCurrency(eApiQuery c);
    void signalCashNoncashCurrency(eExchangeType t);

};

#endif // !TRAY_H
