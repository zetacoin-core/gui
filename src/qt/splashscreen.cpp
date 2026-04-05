// Copyright (c) 2011-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bitcoin-build-config.h> // IWYU pragma: keep

#include <qt/splashscreen.h>

#include <clientversion.h>
#include <common/system.h>
#include <interfaces/handler.h>
#include <interfaces/node.h>
#include <interfaces/wallet.h>
#include <qt/guiutil.h>
#include <qt/networkstyle.h>
#include <qt/walletmodel.h>
#include <util/translation.h>

#include <functional>

#include <QApplication>
#include <QCloseEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QScreen>


SplashScreen::SplashScreen(const NetworkStyle* networkStyle)
    : QWidget()
{
    // Layout constants
    const int splashW = 480;
    const int splashH = 320;
    const int panelW = splashW / 3;           // Left panel = 33%
    const int textX = panelW + 8;             // Text starts 8px right of panel
    const int textPaddingRight = 15;
    const int titleY = 55;
    const int titleVersionVSpace = 20;
    const int titleCopyrightVSpace = 45;

    float fontFactor            = 1.0;
    float devicePixelRatio      = 1.0;
    devicePixelRatio = static_cast<QGuiApplication*>(QCoreApplication::instance())->devicePixelRatio();

    // define text to place
    QString titleText       = CLIENT_NAME;
    QString versionText     = QString("Version %1").arg(QString::fromStdString(FormatFullVersion()));
    QString copyrightText   = QString::fromUtf8(CopyrightHolders("\xc2\xA9 ").c_str());
    const QString& titleAddText    = networkStyle->getTitleAddText();

    QString font            = QApplication::font().toString();

    // create a bitmap according to device pixelratio
    QSize splashSize(splashW*devicePixelRatio, splashH*devicePixelRatio);
    pixmap = QPixmap(splashSize);

    // change to HiDPI if it makes sense
    pixmap.setDevicePixelRatio(devicePixelRatio);

    QPainter pixPaint(&pixmap);

    // draw right side background (light gray)
    pixPaint.fillRect(QRect(QPoint(0,0), splashSize), QColor(240, 240, 240));

    // draw left panel: teal gradient
    QLinearGradient tealGrad(QPoint(0, 0), QPoint(panelW, splashH));
    tealGrad.setColorAt(0.0, QColor(13, 79, 90));    // #0D4F5A
    tealGrad.setColorAt(0.4, QColor(26, 107, 133));   // #1A6B85
    tealGrad.setColorAt(0.7, QColor(29, 111, 137));   // #1D6F89
    tealGrad.setColorAt(1.0, QColor(20, 80, 94));     // #14505E
    pixPaint.fillRect(QRect(0, 0, panelW, splashH), tealGrad);

    // draw binary code overlay on left panel (subtle)
    {
        pixPaint.save();
        pixPaint.setClipRect(QRect(0, 0, panelW, splashH));
        QFont monoFont("Courier New", 6);
        monoFont.setWeight(QFont::Bold);
        pixPaint.setFont(monoFont);
        pixPaint.setPen(QColor(255, 255, 255, 22)); // ~8.5% opacity
        const char* binaryLines[] = {
            "01001010 11010010 00110101 10101100 01110010",
            "10110101 00101101 11001010 01010110 10110100",
            "00101010 11010110 01010100 10110010 11001101",
            "11001010 01010110 10110100 00101101 01001010",
            "01010101 10110010 11001100 10100101 00101010",
            "10100110 01011010 01110101 10011010 01101100",
            "01110010 11001010 01010110 10110100 00101101",
            "11001101 00101011 10100110 01011010 01110101",
            "00110101 10101100 01110010 11001010 01010110",
            "01101011 10010101 01011010 10110010 01010101",
            "10011010 01101100 10101001 01010110 11001010",
            "10100101 00101010 11010110 01010100 10110010",
            "10101100 01001010 11010010 00110101 10101100",
            "10100101 00101010 11010110 01010100 10110010",
            "01001010 11010010 00101101 01101011 10010101",
            "10100110 01011010 01110101 10011010 01101100",
            "01110010 11001010 01010110 10110100 00101101",
            "11001101 00101011 10100110 01011010 01110101",
            "10010101 01011010 10110010 01010101 11001100",
            "01101100 10101001 01010110 11001010 00110101",
            "00101101 01101011 10010101 01011010 10110010",
            "01110101 10011010 01101100 10101001 01010110",
            "11001100 10100101 00101010 11010110 01010100",
            "00110101 10101100 01110010 11001010 01010110",
            "01101011 10010101 01011010 10110010 01010101",
            "10011010 01101100 10101001 01010110 11001010",
        };
        int lineH = 13;
        for (int i = 0; i < 26; i++) {
            int xOff = (i % 5) * (-8) - 15;
            pixPaint.drawText(xOff, 10 + i * lineH, QString(binaryLines[i]));
        }
        pixPaint.restore();
    }

    // draw the embossed zeta symbol centered on left panel
    // Loaded from compiled Qt resource (:/splash/embossed-zeta)
    QPixmap icon(":/splash/embossed-zeta");
    int iconSize = panelW - 30;
    QRect rectIcon(
        (panelW - iconSize) / 2,
        (splashH - iconSize) / 2,
        iconSize, iconSize
    );
    pixPaint.setOpacity(0.25);
    pixPaint.drawPixmap(rectIcon, icon);
    pixPaint.setOpacity(1.0);

    // Title text
    pixPaint.setPen(QColor(51, 51, 51));
    pixPaint.setFont(QFont(font, 22*fontFactor));
    QFontMetrics fm = pixPaint.fontMetrics();
    int titleTextWidth = GUIUtil::TextWidth(fm, titleText);
    int maxTextW = splashW - textX - textPaddingRight;
    if (titleTextWidth > maxTextW) {
        fontFactor = fontFactor * maxTextW / titleTextWidth;
        pixPaint.setFont(QFont(font, 22*fontFactor));
        fm = pixPaint.fontMetrics();
        titleTextWidth = GUIUtil::TextWidth(fm, titleText);
    }
    pixPaint.drawText(textX, titleY, titleText);

    // Version text
    pixPaint.setPen(QColor(100, 100, 100));
    pixPaint.setFont(QFont(font, 11*fontFactor));
    pixPaint.drawText(textX, titleY + titleVersionVSpace, versionText);

    // Copyright text
    {
        pixPaint.setPen(QColor(136, 136, 136));
        pixPaint.setFont(QFont(font, 9*fontFactor));
        const int y = titleY + titleCopyrightVSpace;
        QRect copyrightRect(textX, y, maxTextW, splashH - y - 80);
        pixPaint.drawText(copyrightRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, copyrightText);
    }

    // draw additional text if special network
    if(!titleAddText.isEmpty()) {
        QFont boldFont = QFont(font, 10*fontFactor);
        boldFont.setWeight(QFont::Bold);
        pixPaint.setFont(boldFont);
        fm = pixPaint.fontMetrics();
        int titleAddTextWidth  = GUIUtil::TextWidth(fm, titleAddText);
        pixPaint.drawText(splashW - titleAddTextWidth - 10, 15, titleAddText);
    }

    pixPaint.end();

    // Set window title
    setWindowTitle(titleText + " " + titleAddText);

    // Resize window and move to center of desktop, disallow resizing
    QRect r(QPoint(), QSize(pixmap.size().width()/devicePixelRatio,pixmap.size().height()/devicePixelRatio));
    resize(r.size());
    setFixedSize(r.size());
    move(QGuiApplication::primaryScreen()->geometry().center() - r.center());

    installEventFilter(this);

    GUIUtil::handleCloseWindowShortcut(this);
}

SplashScreen::~SplashScreen()
{
    if (m_node) unsubscribeFromCoreSignals();
}

void SplashScreen::setNode(interfaces::Node& node)
{
    assert(!m_node);
    m_node = &node;
    subscribeToCoreSignals();
    if (m_shutdown) m_node->startShutdown();
}

void SplashScreen::shutdown()
{
    m_shutdown = true;
    if (m_node) m_node->startShutdown();
}

bool SplashScreen::eventFilter(QObject * obj, QEvent * ev) {
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
        if (keyEvent->key() == Qt::Key_Q) {
            shutdown();
        }
    }
    return QObject::eventFilter(obj, ev);
}

static void InitMessage(SplashScreen *splash, const std::string &message)
{
    bool invoked = QMetaObject::invokeMethod(splash, "showMessage",
        Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(message)),
        Q_ARG(int, Qt::AlignBottom|Qt::AlignHCenter),
        Q_ARG(QColor, QColor(55,55,55)));
    assert(invoked);
}

static void ShowProgress(SplashScreen *splash, const std::string &title, int nProgress, bool resume_possible)
{
    InitMessage(splash, title + std::string("\n") +
            (resume_possible ? SplashScreen::tr("(press q to shutdown and continue later)").toStdString()
                                : SplashScreen::tr("press q to shutdown").toStdString()) +
            strprintf("\n%d", nProgress) + "%");
}

void SplashScreen::subscribeToCoreSignals()
{
    // Connect signals to client
    m_handler_init_message = m_node->handleInitMessage(std::bind(InitMessage, this, std::placeholders::_1));
    m_handler_show_progress = m_node->handleShowProgress(std::bind(ShowProgress, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_handler_init_wallet = m_node->handleInitWallet([this]() { handleLoadWallet(); });
}

void SplashScreen::handleLoadWallet()
{
#ifdef ENABLE_WALLET
    if (!WalletModel::isWalletEnabled()) return;
    m_handler_load_wallet = m_node->walletLoader().handleLoadWallet([this](std::unique_ptr<interfaces::Wallet> wallet) {
        m_connected_wallet_handlers.emplace_back(wallet->handleShowProgress(std::bind(ShowProgress, this, std::placeholders::_1, std::placeholders::_2, false)));
        m_connected_wallets.emplace_back(std::move(wallet));
    });
#endif
}

void SplashScreen::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    m_handler_init_message->disconnect();
    m_handler_show_progress->disconnect();
    for (const auto& handler : m_connected_wallet_handlers) {
        handler->disconnect();
    }
    m_connected_wallet_handlers.clear();
    m_connected_wallets.clear();
}

void SplashScreen::showMessage(const QString &message, int alignment, const QColor &color)
{
    curMessage = message;
    curAlignment = alignment;
    curColor = color;
    update();
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap);
    // Draw status message: horizontally centered in right section, near the bottom
    int panelW = width() / 3;
    QRect msgRect(panelW, 0, width() - panelW, height());
    msgRect.adjust(5, 5, -5, -5);
    painter.setPen(curColor);
    painter.drawText(msgRect, Qt::AlignHCenter | Qt::AlignBottom, curMessage);
}

void SplashScreen::closeEvent(QCloseEvent *event)
{
    shutdown(); // allows an "emergency" shutdown during startup
    event->ignore();
}
