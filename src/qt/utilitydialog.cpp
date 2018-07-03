// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/iop-config.h"
#endif

#include "utilitydialog.h"

#include "ui_helpmessagedialog.h"

#include "iopgui.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "intro.h"
#include "paymentrequestplus.h"
#include "guiutil.h"

#include "clientversion.h"
#include "init.h"
#include "util.h"

#include <QCloseEvent>
#include <QLabel>
#include <QRect>
#include <QPushButton>
#include <QRegExp>
#include <QTextTable>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QSettings>

#include <iostream>

/** "Help message" or "About" dialog box */
HelpMessageDialog::HelpMessageDialog(QWidget *parent, bool about, bool update) :
    QDialog(parent),
    ui(new Ui::HelpMessageDialog)
{
    ui->setupUi(this);
    
    ui->okButton->button(QDialogButtonBox::Ok)->setIcon(QIcon());

    QString version = tr(PACKAGE_NAME) + " " + tr("version") + " " + QString::fromStdString(FormatFullVersion());
    /* On x86 add a bit specifier to the version so that users can distinguish between
     * 32 and 64 bit builds. On other architectures, 32/64 bit may be more ambiguous.
     */
#if defined(__x86_64__)
    version += " " + tr("(%1-bit)").arg(64);
#elif defined(__i386__ )
    version += " " + tr("(%1-bit)").arg(32);
#endif
    if(update)
    {
        


        //ui->horizontalSpacerButtons->setVisible(true);
        QString updateInfo = tr("checking for update...");
        resize(780,200);
        ui->aboutMessage->setTextFormat(Qt::RichText);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        text = updateInfo;
        ui->aboutMessage->setText(updateInfo);
        ui->helpMessage->setVisible(false);
    } else if (about)
    {
        setWindowTitle(tr("About %1").arg(tr(PACKAGE_NAME)));
        resize(780,400);
        /// HTML-format the license message from the core
        QString licenseInfo = QString::fromStdString(LicenseInfo());
        QString licenseInfoHTML = licenseInfo;
        // Make URLs clickable
        QRegExp uri("<(.*)>", Qt::CaseSensitive, QRegExp::RegExp2);
        uri.setMinimal(true); // use non-greedy matching
        licenseInfoHTML.replace(uri, "<a href=\"\\1\">\\1</a>");
        // Replace newlines with HTML breaks
        licenseInfoHTML.replace("\n", "<br>");
        ui->aboutMessage->setTextFormat(Qt::RichText);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        text = version + "\n" + licenseInfo;
        ui->aboutMessage->setText(version + "<br><br>" + licenseInfoHTML);
        ui->aboutMessage->setWordWrap(true);
        ui->helpMessage->setVisible(false);
    } else {
        setWindowTitle(tr("Command-line options"));
        resize(780,400);
        QString header = tr("Usage:") + "\n" +
            "  iop-qt [" + tr("command-line options") + "]                     " + "\n";
        QTextCursor cursor(ui->helpMessage->document());
        cursor.insertText(version);
        cursor.insertBlock();
        cursor.insertText(header);
        cursor.insertBlock();

        std::string strUsage = HelpMessage(HMM_IOP_QT);
        const bool showDebug = gArgs.GetBoolArg("-help-debug", false);
        strUsage += HelpMessageGroup(tr("UI Options:").toStdString());
        if (showDebug) {
            strUsage += HelpMessageOpt("-allowselfsignedrootcertificates", strprintf("Allow self signed root certificates (default: %u)", DEFAULT_SELFSIGNED_ROOTCERTS));
        }
        strUsage += HelpMessageOpt("-choosedatadir", strprintf(tr("Choose data directory on startup (default: %u)").toStdString(), DEFAULT_CHOOSE_DATADIR));
        strUsage += HelpMessageOpt("-lang=<lang>", tr("Set language, for example \"de_DE\" (default: system locale)").toStdString());
        strUsage += HelpMessageOpt("-min", tr("Start minimized").toStdString());
        strUsage += HelpMessageOpt("-rootcertificates=<file>", tr("Set SSL root certificates for payment request (default: -system-)").toStdString());
        strUsage += HelpMessageOpt("-splash", strprintf(tr("Show splash screen on startup (default: %u)").toStdString(), DEFAULT_SPLASHSCREEN));
        strUsage += HelpMessageOpt("-resetguisettings", tr("Reset all settings changed in the GUI").toStdString());
        if (showDebug) {
            strUsage += HelpMessageOpt("-uiplatform", strprintf("Select platform to customize UI for (one of windows, macosx, other; default: %s)", IoPGUI::DEFAULT_UIPLATFORM));
        }
        QString coreOptions = QString::fromStdString(strUsage);
        text = version + "\n" + header + "\n" + coreOptions;

        QTextTableFormat tf;
        tf.setBorderStyle(QTextFrameFormat::BorderStyle_None);
        tf.setCellPadding(2);
        QVector<QTextLength> widths;
        widths << QTextLength(QTextLength::PercentageLength, 35);
        widths << QTextLength(QTextLength::PercentageLength, 65);
        tf.setColumnWidthConstraints(widths);

        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);

        for (const QString &line : coreOptions.split("\n")) {
            if (line.startsWith("  -"))
            {
                cursor.currentTable()->appendRows(1);
                cursor.movePosition(QTextCursor::PreviousCell);
                cursor.movePosition(QTextCursor::NextRow);
                cursor.insertText(line.trimmed());
                cursor.movePosition(QTextCursor::NextCell);
            } else if (line.startsWith("   ")) {
                cursor.insertText(line.trimmed()+' ');
            } else if (line.size() > 0) {
                //Title of a group
                if (cursor.currentTable())
                    cursor.currentTable()->appendRows(1);
                cursor.movePosition(QTextCursor::Down);
                cursor.insertText(line.trimmed(), bold);
                cursor.insertTable(1, 2, tf);
            }
        }
        ui->helpMessage->moveCursor(QTextCursor::Start);
        ui->scrollArea->setVisible(false);
        ui->aboutLogo->setVisible(false);
    }
}

void HelpMessageDialog::update(bool success, bool latest, QString latestVersion, QString changeLog)
{
    QString updateInfo;
    if(!success){
        setWindowTitle(tr("Connection error"));
        updateInfo = tr("Could not reach the update Server\n");
    }else if(!latest)
    {
        setWindowTitle(tr("Update Available"));
        updateInfo = tr("A new Update is available!\n");
        updateInfo.append("<br/><br/>\n");
        updateInfo.append(tr("Version: \n")).append(latestVersion);
        updateInfo.append("<br/><br/>\n");
        updateInfo.append(tr("Changelog: \n"));
        updateInfo.append("<br/>\n");
        updateInfo.append(changeLog);
        resize(780,400);
        downloadButton = new QPushButton(tr("download"));
        ui->okButton->addButton(downloadButton, QDialogButtonBox::AcceptRole);
        connect(downloadButton, SIGNAL(clicked()), this, SLOT(on_downloadButton_accepted()));
    } else
    {
        setWindowTitle(tr("Latest Version"));
        updateInfo = tr("You have the current Version!<br><br>") + latestVersion;
    }
    ui->aboutMessage->setText(updateInfo);
}


HelpMessageDialog::~HelpMessageDialog()
{
    delete ui;
}

void HelpMessageDialog::printToConsole()
{
    // On other operating systems, the expected action is to print the message to the console.
    fprintf(stdout, "%s\n", qPrintable(text));
}

void HelpMessageDialog::showOrPrint()
{
#if defined(WIN32)
    // On Windows, show a message box, as there is no stderr/stdout in windowed applications
    exec();
#else
    // On other operating systems, print help text to console
    printToConsole();
#endif
}

void HelpMessageDialog::on_okButton_accepted()
{
    close();
}

void HelpMessageDialog::on_downloadButton_accepted()
{
    QDesktopServices::openUrl(QUrl("https://github.com/Internet-of-People/iop-core/releases",QUrl::TolerantMode)); 
}

/** "Shutdown" window */
ShutdownWindow::ShutdownWindow(QWidget *parent, Qt::WindowFlags f):
    QWidget(parent, f)
{
    QSettings settings; 
    if(settings.value("theme").toString() == "dark") 
    {   
        QString fontGray = "rgb(204,204,204)";
        QString medGray = "rgb(45,45,45)";
        setStyleSheet("QWidget {background: " + medGray + "; color: " + fontGray + ";}"); 
    } 
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QLabel(
        tr("%1 is shutting down...").arg(tr(PACKAGE_NAME)) + "<br /><br />" +
        tr("Do not shut down the computer until this window disappears.")));
    setLayout(layout);
}

QWidget *ShutdownWindow::showShutdownWindow(IoPGUI *window)
{
    if (!window)
        return nullptr;

    // Show a simple window indicating shutdown status
    QWidget *shutdownWindow = new ShutdownWindow();
    shutdownWindow->setWindowTitle(window->windowTitle());

    // Center shutdown window at where main window was
    const QPoint global = window->mapToGlobal(window->rect().center());
    shutdownWindow->move(global.x() - shutdownWindow->width() / 2, global.y() - shutdownWindow->height() / 2);
    shutdownWindow->show();
    return shutdownWindow;
}

void ShutdownWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
}
