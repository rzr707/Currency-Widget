#include <QBitmap>
#include <QSystemTrayIcon>
#include "tray.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    qApp->setQuitOnLastWindowClosed(false);         // to prevent closing window
    app.setApplicationName("Currency converter");
    app.setWindowIcon(QIcon(":/res/icons/icon.ico"));

    CurrencyWidget wgt;
    Tray           tray(0, &wgt);

    QObject::connect(&tray, SIGNAL(exitClicked()),
                     qApp,  SLOT(quit())
                     );
    QObject::connect(&tray, SIGNAL(signalShowHide(bool)),
                     &wgt,  SLOT(setVisible(bool))
                     );
    QObject::connect(&tray, SIGNAL(signalResetCoords(QPoint)),
                     &wgt , SLOT(slotRecieveCoords(QPoint))
                     );
    QObject::connect(&tray, SIGNAL(signalBuySaleCurrency(eApiQuery)),
                     &wgt,  SLOT(slotApiChange(eApiQuery))
                     );
    QObject::connect(&tray, SIGNAL(signalCashNoncashCurrency(eExchangeType)),
                     &wgt,  SLOT(slotExchangeChange(eExchangeType))
                     );

    wgt.show();
    return app.exec();
}
